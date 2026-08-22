#pragma once

// Loads Wavefront OBJ meshes from disk into GPU-ready Mesh objects.
// OBJ only for now (tinyobjloader) — enough for demo props; formats with
// skeletons/animation are a later phase's problem.

#include <filesystem>
#include <memory>

namespace jade {

class Mesh;

/// Load `assetRoot() / relative` as a triangulated OBJ and upload it as a Mesh.
/// Missing normals are replaced with flat per-face normals; vertex color is
/// forced to white (lighting supplies the shading, textures the albedo).
/// Throws AssetError (full path + parser message) on any failure, including a
/// file that parses but contains no triangle faces. Ownership transfers to the
/// caller via unique_ptr — Mesh owns GL handles and is non-movable (rule 4's
/// transfer allowance).
std::unique_ptr<Mesh> loadMeshObj(const std::filesystem::path& relative);

} // namespace jade
