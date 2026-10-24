// Minimal single-header test harness for the dsky unit test suites.
//
// Usage in a suite's main.cpp:
//   #include "../common/test.h"
//   ... project headers ...
//
//   TEST(my_case) { CHECK(1 + 1 == 2); }
//   TESTS_MAIN;
//
// Each registered case runs once, in registration order. A failing check
// prints file:line and continues; the process exits non-zero if any check
// failed.

#ifndef DSKY_TESTS_COMMON_TEST_H
#define DSKY_TESTS_COMMON_TEST_H

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

namespace dskytest {

struct Stats {
  int checks = 0;
  int failures = 0;
  const char *current = "";
};

inline Stats &stats() { static Stats s; return s; }

struct TestCase {
  const char *name;
  void (*fn)();
};

inline std::vector<TestCase> &cases() { static std::vector<TestCase> v; return v; }

struct Registrar {
  Registrar(const char *name, void (*fn)()) { cases().push_back({name, fn}); }
};

} // namespace dskytest

#define TEST(name)                                                       \
  static void name();                                                    \
  static ::dskytest::Registrar dskytest_reg_##name(#name, &name);        \
  static void name()

#define CHECK(cond)                                                      \
  do {                                                                   \
    ::dskytest::stats().checks++;                                        \
    if (!(cond)) {                                                       \
      ::dskytest::stats().failures++;                                    \
      std::printf("FAIL %s:%d [%s]: CHECK(%s)\n", __FILE__, __LINE__,    \
                  ::dskytest::stats().current, #cond);                   \
    }                                                                    \
  } while (0)

#define CHECK_EQ(a, b)                                                   \
  do {                                                                   \
    ::dskytest::stats().checks++;                                        \
    auto dskytest_va_ = (a);                                             \
    auto dskytest_vb_ = (b);                                             \
    if (!(dskytest_va_ == dskytest_vb_)) {                               \
      ::dskytest::stats().failures++;                                    \
      std::ostringstream dskytest_oss_;                                  \
      dskytest_oss_ << a << " != " << b;                                 \
      std::printf("FAIL %s:%d [%s]: CHECK_EQ(%s, %s) -> %s\n", __FILE__, \
                  __LINE__, ::dskytest::stats().current, #a, #b,         \
                  dskytest_oss_.str().c_str());                          \
    }                                                                    \
  } while (0)

#define CHECK_THROWS(expr)                                               \
  do {                                                                   \
    ::dskytest::stats().checks++;                                        \
    bool dskytest_thrown_ = false;                                       \
    try { expr; } catch (...) { dskytest_thrown_ = true; }               \
    if (!dskytest_thrown_) {                                             \
      ::dskytest::stats().failures++;                                    \
      std::printf("FAIL %s:%d [%s]: CHECK_THROWS(%s) did not throw\n",   \
                  __FILE__, __LINE__, ::dskytest::stats().current, #expr);\
    }                                                                    \
  } while (0)

#define TESTS_MAIN                                                       \
  int main() {                                                           \
    int dskytest_failed_cases_ = 0;                                      \
    for (const auto &c : ::dskytest::cases()) {                          \
      ::dskytest::stats().current = c.name;                              \
      std::printf("[ RUN ] %s\n", c.name);                               \
      int dskytest_before_ = ::dskytest::stats().failures;               \
      c.fn();                                                            \
      if (::dskytest::stats().failures > dskytest_before_) {             \
        dskytest_failed_cases_++;                                        \
        std::printf("[FAIL ] %s\n", c.name);                             \
      } else {                                                           \
        std::printf("[ OK  ] %s\n", c.name);                             \
      }                                                                  \
    }                                                                    \
    std::printf("%d checks, %d failures (%d failed cases)\n",            \
                ::dskytest::stats().checks, ::dskytest::stats().failures,\
                dskytest_failed_cases_);                                 \
    return ::dskytest::stats().failures == 0 ? 0 : 1;                    \
  }

#endif // DSKY_TESTS_COMMON_TEST_H
