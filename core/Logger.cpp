#include "core/Logger.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace jade {

namespace {
    // One mutex serializes all log writes so concurrent threads (e.g. the
    // future async streaming queue) don't interleave characters mid-line.
    std::mutex g_logMutex;

    // Optional file sink mirrored into by Logger::log. Guarded by g_logMutex
    // (both the open/close in setFileSink and the writes in log). Closed by
    // default; when closed, logging is console-only.
    std::ofstream g_fileSink;
}

void Logger::log(LogLevel level,
                 std::string_view file,
                 int line,
                 std::string_view message) {
    // Format once, outside the lock, so console and file sink emit the exact
    // same bytes and the mutex is held only for the actual writes.
    std::ostringstream record;
    record << '[' << timestamp() << "] "
           << '[' << levelToString(level) << "] "
           << file << ':' << line << " - "
           << message << '\n';
    const std::string text = record.str();

    std::lock_guard<std::mutex> lock(g_logMutex);

    // Errors go to stderr so they survive stdout redirection in tools/CI.
    // Push buffered stdout out first: it keeps combined redirects (2>&1) in
    // order and makes earlier INFO/WARN lines survive an imminent abort —
    // stderr is unit-buffered, stdout is not.
    std::ostream& out = (level == LogLevel::Error) ? std::cerr : std::cout;
    if (level == LogLevel::Error) {
        std::cout.flush();
    }

    out << text;

    // Mirror the identical line into the file sink, if one is open. Same
    // rationale as the stdout flush above: error lines must reach disk before
    // an imminent abort, so flush the sink on Error.
    if (g_fileSink.is_open()) {
        g_fileSink << text;
        if (level == LogLevel::Error) {
            g_fileSink.flush();
        }
    }
}

void Logger::setFileSink(const std::string& path) {
    // Capture the outcome under the lock, but log it only after the lock is
    // released: JADE_LOG_* re-enters Logger::log, which locks the same
    // non-recursive mutex — logging while still holding it would self-deadlock.
    bool opened = false;
    bool failed = false;
    {
        std::lock_guard<std::mutex> lock(g_logMutex);

        if (g_fileSink.is_open()) {
            g_fileSink.close();
        }
        // Clear any stale error bits (e.g. from a previous failed open) so
        // the stream can be reused.
        g_fileSink.clear();

        if (!path.empty()) {
            // Truncate rather than append: each setFileSink call starts a
            // fresh session log, like a dev server rewriting its log file on
            // restart. On failure the stream is left closed (console-only).
            g_fileSink.open(path, std::ios::out | std::ios::trunc);
            opened = g_fileSink.is_open();
            failed = !opened;
        }
    }

    if (opened) {
        JADE_LOG_INFO("File sink opened: " + path);
    } else if (failed) {
        JADE_LOG_ERROR("Failed to open file sink: " + path);
    }
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
