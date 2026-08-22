#include "core/Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace jade {

namespace {
    // One mutex serializes all log writes so concurrent threads (e.g. the
    // future async streaming queue) don't interleave characters mid-line.
    std::mutex g_logMutex;
}

void Logger::log(LogLevel level,
                 std::string_view file,
                 int line,
                 std::string_view message) {
    std::lock_guard<std::mutex> lock(g_logMutex);

    // Errors go to stderr so they survive stdout redirection in tools/CI.
    // Push buffered stdout out first: it keeps combined redirects (2>&1) in
    // order and makes earlier INFO/WARN lines survive an imminent abort —
    // stderr is unit-buffered, stdout is not.
    std::ostream& out = (level == LogLevel::Error) ? std::cerr : std::cout;
    if (level == LogLevel::Error) {
        std::cout.flush();
    }

    out << '[' << timestamp() << "] "
        << '[' << levelToString(level) << "] "
        << file << ':' << line << " - "
        << message << '\n';
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

std::string Logger::timestamp() {
    using namespace std::chrono;

    const auto now = system_clock::now();
    const auto t   = system_clock::to_time_t(now);
    const auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm{};
#if defined(_WIN32)
    // localtime is not thread-safe; use the platform-specific safe variant.
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

} // namespace jade
