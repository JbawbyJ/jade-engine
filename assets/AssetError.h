#pragma once

// Recoverable asset-loading failure: missing file, unreadable content, parse
// error. Thrown by the Phase 4 loaders (shader / texture / mesh). Content
// problems are not programmer invariants — never JADE_ASSERT on them; callers
// catch AssetError and take a graceful path (same pattern as WindowError).

#include <stdexcept>
#include <string>

namespace jade {

class AssetError : public std::runtime_error {
public:
    explicit AssetError(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace jade
