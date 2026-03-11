// log_throttle.hpp
#pragma once
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

namespace logthrottle {

// Thread-safe per-tag time throttle
inline bool should_log_throttle(const std::string& tag, int interval_ms) {
  using clock = std::chrono::steady_clock;
  static std::unordered_map<std::string, clock::time_point> last;
  static std::mutex m;
  auto now = clock::now();
  std::lock_guard<std::mutex> lk(m);
  auto it = last.find(tag);
  if (it == last.end() || (now - it->second) >= std::chrono::milliseconds(interval_ms)) {
    last[tag] = now;
    return true;
  }
  return false;
}

// Thread-safe per-tag "every N" throttle
inline bool should_log_every_n(const std::string& tag, int n) {
  static std::unordered_map<std::string, size_t> cnt;
  static std::mutex m;
  std::lock_guard<std::mutex> lk(m);
  return (cnt[tag]++ % (n > 0 ? n : 1)) == 0;
}

} // namespace logthrottle

// Stream-style macros: keep your existing << syntax
#define LOG_THROTTLED(TAG, INTERVAL_MS, STREAM_EXPR) \
  do { if (logthrottle::should_log_throttle((TAG), (INTERVAL_MS))) { \
         std::cout << STREAM_EXPR << std::endl; \
       } } while(0)

#define LOG_EVERY_N(TAG, N, STREAM_EXPR) \
  do { if (logthrottle::should_log_every_n((TAG), (N))) { \
         std::cout << STREAM_EXPR << std::endl; \
       } } while(0)
