#pragma once

// Resolution of the on-disk directory all asset-relative paths hang off.
// Loaders join their relative paths onto assetRoot() so content moves with
// the executable instead of depending on the process working directory.

#include <filesystem>

namespace jade {

// Absolute (or, in the last-resort fallback, working-directory-relative) root
// of the asset tree. Resolution order:
//   1. `JADE_ASSET_ROOT` environment variable, when set and non-empty —
//      an explicit override, like NODE_PATH beating default module lookup.
//   2. `assets/` next to the running executable (platform-specific query).
//   3. Relative "assets" when the executable path cannot be determined
//      (logged once as a WARN).
// Resolved once per process and logged once at INFO; returns a copy.
std::filesystem::path assetRoot();

} // namespace jade
