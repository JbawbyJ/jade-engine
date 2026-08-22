#include "assets/AssetPaths.h"

#include <cstdlib>
#include <optional>
#include <string>
#include <system_error>

#include "core/Logger.h"

#if defined(_WIN32)
    // windows.h is included here, and only here, so its macro pollution stays
    // inside this translation unit. Trim it (WIN32_LEAN_AND_MEAN) and stop it
    // from defining min/max macros (NOMINMAX). Note it still defines near/far
    // as empty macros — do not name identifiers near/far/min/max in this file.
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#elif defined(__APPLE__)
    #include <cstdint>
    #include <mach-o/dyld.h>
#endif

namespace jade {

namespace {

    constexpr const char* kAssetRootEnvVar = "JADE_ASSET_ROOT";

    // Step 1: explicit override. Returns the value of JADE_ASSET_ROOT when it
    // is set and non-empty, otherwise nullopt (empty counts as unset).
    std::optional<std::string> environmentOverride() {
#if defined(_WIN32)
        // GetEnvironmentVariableA instead of std::getenv: MSVC flags getenv
        // as unsafe (C4996) and the build is zero-warning.
        const DWORD required = GetEnvironmentVariableA(kAssetRootEnvVar, nullptr, 0);
        if (required == 0) {
            return std::nullopt; // unset
        }
        std::string value(required, '\0'); // `required` includes the terminator
        const DWORD written =
            GetEnvironmentVariableA(kAssetRootEnvVar, value.data(), required);
        if (written == 0 || written >= required) {
            return std::nullopt; // empty, or raced with a concurrent change
        }
        value.resize(written);
        return value;
#else
        const char* value = std::getenv(kAssetRootEnvVar);
        if (value == nullptr || value[0] == '\0') {
            return std::nullopt;
        }
        return std::string(value);
#endif
    }

    // Step 2: absolute path of the running executable, or an empty path when
    // the platform query fails. Each platform spells this differently; the
    // branches mirror the Logger's localtime split (standards rule 10).
    std::filesystem::path executablePath() {
#if defined(_WIN32)
        char buffer[MAX_PATH];
        const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        // length == MAX_PATH means the path was truncated — treat as failure
        // rather than resolving assets against a mangled directory.
        if (length == 0 || length >= MAX_PATH) {
            return {};
        }
        return std::filesystem::path(std::string(buffer, length));
#elif defined(__APPLE__)
        // Two-step dance: first call reports the required buffer size
        // (including the terminator), second call fills it.
        std::uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string buffer(size, '\0');
        if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
            return {};
        }
        // The buffer is NUL-terminated; c_str() trims the padding.
        return std::filesystem::path(buffer.c_str());
#else
        // Linux (and friends with procfs): /proc/self/exe is a symlink to the
        // real binary. error_code overload — a missing procfs must not throw.
        std::error_code ec;
        std::filesystem::path exe = std::filesystem::read_symlink("/proc/self/exe", ec);
        if (ec) {
            return {};
        }
        return exe;
#endif
    }

    // Full resolution chain. Runs exactly once (see assetRoot), so the WARN in
    // the fallback branch fires at most once per process.
    std::filesystem::path resolveAssetRoot() {
        if (std::optional<std::string> envRoot = environmentOverride()) {
            return std::filesystem::path(*envRoot);
        }

        const std::filesystem::path exe = executablePath();
        if (!exe.empty()) {
            return exe.parent_path() / "assets";
        }

        JADE_LOG_WARN("Executable path query failed; asset root falls back to "
                      "working-directory-relative \"assets\"");
        return std::filesystem::path("assets");
    }

} // namespace

std::filesystem::path assetRoot() {
    // Magic static: resolved once, thread-safe, and the INFO line below is
    // emitted exactly once — like a memoized config lookup.
    static const std::filesystem::path s_root = [] {
        std::filesystem::path root = resolveAssetRoot();
        JADE_LOG_INFO("Asset root: " + root.string());
        return root;
    }();
    return s_root;
}

} // namespace jade
