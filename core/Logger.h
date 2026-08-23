#pragma once

// Severity-level logging facility for the Jade Engine.
// Use the JADE_LOG_* macros, never std::cout. The macros capture __FILE__/__LINE__
// automatically so log output always points back to the call site.

#include <cstdlib>
#include <string>
#include <string_view>

namespace jade {

enum class LogLevel {
    Info,
    Warn,
    Error,
};

/// Thread-safe, timestamped logger. All output goes through Logger::log so we
/// can later swap the sink (file, in-game console, network) in one place.
class Logger {
public:
    static void log(LogLevel level,
                    std::string_view file,
                    int line,
                    std::string_view message);

    /// Mirror every record (all levels) into a file at `path`, in addition to
    /// the console streams. The file is opened truncating — each call starts a
    /// fresh session log. An empty path closes the sink. If the file cannot be
    /// opened, the sink stays closed and the failure is logged as an error.
    static void setFileSink(const std::string& path);

private:
    static const char* levelToString(LogLevel level);
    static std::string timestamp();
};

} // namespace jade

// --- Public macros ---------------------------------------------------------
// Macros (not inline functions) so __FILE__/__LINE__ resolve to the caller.
#define JADE_LOG_INFO(msg)  ::jade::Logger::log(::jade::LogLevel::Info,  __FILE__, __LINE__, (msg))
#define JADE_LOG_WARN(msg)  ::jade::Logger::log(::jade::LogLevel::Warn,  __FILE__, __LINE__, (msg))
#define JADE_LOG_ERROR(msg) ::jade::Logger::log(::jade::LogLevel::Error, __FILE__, __LINE__, (msg))

// Hard invariant: log + abort if `cond` is false. Use only for programmer
// invariants that must never happen (e.g. a null handle the caller promised).
// Recoverable init failures throw (see WindowError) or use JADE_LOG_ERROR.
#define JADE_ASSERT(cond, msg)                                                \
    do {                                                                      \
        if (!(cond)) {                                                        \
            ::jade::Logger::log(::jade::LogLevel::Error, __FILE__, __LINE__,  \
                std::string("Assertion failed: ") + (msg));                   \
            std::abort();                                                     \
        }                                                                     \
    } while (0)
