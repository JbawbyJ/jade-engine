#include "assets/MeshLoader.h"

#include <tiny_obj_loader.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

#include "assets/AssetError.h"
#include "assets/AssetPaths.h"
#include "core/Logger.h"
#include "math/MathTypes.h"
#include "renderer/Mesh.h"

namespace jade {

namespace {

constexpr float kDegenerateAreaEpsilon = 1e-12f; // squared cross length below this = degenerate
constexpr Vec3 kFallbackNormal{0.0f, 0.0f, 1.0f};
constexpr Vec3 kWhite{1.0f, 1.0f, 1.0f};

// tinyobj messages arrive newline-terminated; our logger adds its own.
std::string trimTrailingWhitespace(std::string text) {
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r' ||
                             text.back() == ' ' || text.back() == '\t')) {
        text.pop_back();
    }
    return text;
}

Vec3 readPosition(const tinyobj::attrib_t& attrib, int vertexIndex) {
    const std::size_t base = 3 * static_cast<std::size_t>(vertexIndex);
    return {attrib.vertices[base + 0], attrib.vertices[base + 1], attrib.vertices[base + 2]};
}

Vec3 readNormal(const tinyobj::attrib_t& attrib, int normalIndex) {
    const std::size_t base = 3 * static_cast<std::size_t>(normalIndex);
    return {attrib.normals[base + 0], attrib.normals[base + 1], attrib.normals[base + 2]};
}

Vec2 readTexCoord(const tinyobj::attrib_t& attrib, int texcoordIndex) {
    const std::size_t base = 2 * static_cast<std::size_t>(texcoordIndex);
    return {attrib.texcoords[base + 0], attrib.texcoords[base + 1]};
}

// Flat face normal via the triangle's edge cross product. Degenerate faces
// (zero area — collinear or duplicated corners) fall back to +Z rather than
// producing NaNs that would poison the lighting math downstream.
Vec3 flatFaceNormal(const Vec3& p0, const Vec3& p1, const Vec3& p2) {
    const Vec3 cross = glm::cross(p1 - p0, p2 - p0);
    const float lengthSq = glm::dot(cross, cross);
    if (lengthSq <= kDegenerateAreaEpsilon) {
        return kFallbackNormal;
    }
    return cross / std::sqrt(lengthSq);
}

// De-index every triangle into a flat Vertex array. OBJ indexes position /
// normal / uv independently per corner (three parallel index streams — like
// zipping separate JS arrays per element); GL wants one index per whole
// vertex, so we expand each face corner into its own Vertex.
// TODO(jade): dedupe identical corners into a shared index buffer when
// meshes get big enough for the memory to matter.
std::vector<Vertex> buildVertices(const tinyobj::attrib_t& attrib,
                                  const std::vector<tinyobj::shape_t>& shapes) {
    std::size_t totalIndices = 0;
    for (const tinyobj::shape_t& shape : shapes) {
        totalIndices += shape.mesh.indices.size();
    }

    std::vector<Vertex> vertices;
    vertices.reserve(totalIndices);

    std::size_t skippedFaces = 0;
    for (const tinyobj::shape_t& shape : shapes) {
        const tinyobj::mesh_t& mesh = shape.mesh;
        std::size_t indexOffset = 0;
        for (std::size_t face = 0; face < mesh.num_face_vertices.size(); ++face) {
            const std::size_t faceVerts = mesh.num_face_vertices[face];
            if (faceVerts != 3) {
                // Triangulation should make this unreachable; skip rather than
                // abort — bad content is recoverable, not a programmer invariant.
                ++skippedFaces;
                indexOffset += faceVerts;
                continue;
            }

            const tinyobj::index_t corners[3] = {mesh.indices[indexOffset + 0],
                                                 mesh.indices[indexOffset + 1],
                                                 mesh.indices[indexOffset + 2]};
            const Vec3 positions[3] = {readPosition(attrib, corners[0].vertex_index),
                                       readPosition(attrib, corners[1].vertex_index),
                                       readPosition(attrib, corners[2].vertex_index)};

            // Only pay for the cross product when some corner lacks an authored normal.
            const bool needsFlatNormal = corners[0].normal_index < 0 ||
                                         corners[1].normal_index < 0 ||
                                         corners[2].normal_index < 0;
            const Vec3 flatNormal = needsFlatNormal
                ? flatFaceNormal(positions[0], positions[1], positions[2])
                : kFallbackNormal;

            for (std::size_t corner = 0; corner < 3; ++corner) {
                const tinyobj::index_t& index = corners[corner];
                Vertex vertex;
                vertex.position = positions[corner];
                vertex.normal = index.normal_index >= 0 ? readNormal(attrib, index.normal_index)
                                                        : flatNormal;
                vertex.color = kWhite; // lighting shades, texture colors — vertex tint stays neutral
                vertex.texCoord = index.texcoord_index >= 0
                    ? readTexCoord(attrib, index.texcoord_index)
                    : Vec2{0.0f, 0.0f};
                vertices.push_back(vertex);
            }
            indexOffset += 3;
        }
    }

    if (skippedFaces > 0) {
        JADE_LOG_WARN("OBJ import skipped " + std::to_string(skippedFaces) +
                      " non-triangle face(s) that survived triangulation");
    }
    return vertices;
}

} // namespace

std::unique_ptr<Mesh> loadMeshObj(const std::filesystem::path& relative) {
    const std::filesystem::path fullPath = assetRoot() / relative;

    tinyobj::ObjReaderConfig config;
    config.triangulate = true;
    config.vertex_color = false; // color is forced to white below; skip parsing the extension

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(fullPath.string(), config)) {
        throw AssetError("Failed to load OBJ '" + fullPath.string() + "': " +
                         trimTrailingWhitespace(reader.Error()));
    }
    if (!reader.Warning().empty()) {
        JADE_LOG_WARN("OBJ '" + fullPath.string() + "': " +
                      trimTrailingWhitespace(reader.Warning()));
    }

    const std::vector<Vertex> vertices = buildVertices(reader.GetAttrib(), reader.GetShapes());
    if (vertices.empty()) {
        // Mesh JADE_ASSERTs on empty buffers — that abort is for programmer
        // errors. An empty .obj is a content error, so throw instead.
        throw AssetError("OBJ '" + fullPath.string() + "' contains no triangle faces");
    }

    // Trivial 0..N-1 index buffer to match the de-indexed vertices.
    std::vector<std::uint32_t> indices(vertices.size());
    std::iota(indices.begin(), indices.end(), 0u);

    JADE_LOG_INFO("Loaded OBJ '" + fullPath.string() + "': " +
                  std::to_string(vertices.size()) + " vertices, " +
                  std::to_string(vertices.size() / 3) + " triangles");

    return std::make_unique<Mesh>(vertices.data(), vertices.size(),
                                  indices.data(), indices.size());
}

} // namespace jade
