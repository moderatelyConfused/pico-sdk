// Stub assert.h for freestanding pico-sdk builds
#ifndef _ASSERT_H
#define _ASSERT_H

#ifdef NDEBUG
#define assert(x) ((void)0)
#else
#define assert(x) ((void)((x) || (__assert_fail(#x, __FILE__, __LINE__), 0)))
#endif

void __assert_fail(const char *expr, const char *file, int line);

// C11 static_assert macro (maps to _Static_assert keyword)
#ifndef static_assert
#define static_assert _Static_assert
#endif

#endif
