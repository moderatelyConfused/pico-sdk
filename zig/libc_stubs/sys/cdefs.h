// Stub sys/cdefs.h for freestanding pico-sdk builds
#ifndef _SYS_CDEFS_H
#define _SYS_CDEFS_H

#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

#ifndef __GNUC_PREREQ
#define __GNUC_PREREQ(maj, min) \
    ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#endif

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

// GCC/Clang inline attributes
#ifndef __always_inline
#define __always_inline __attribute__((__always_inline__)) inline
#endif

#ifndef __noinline
#define __noinline __attribute__((__noinline__))
#endif

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

#ifndef __used
#define __used __attribute__((__used__))
#endif

#ifndef __packed
#define __packed __attribute__((__packed__))
#endif

#ifndef __aligned
#define __aligned(x) __attribute__((__aligned__(x)))
#endif

#ifndef __section
#define __section(x) __attribute__((__section__(x)))
#endif

#ifndef __weak
#define __weak __attribute__((__weak__))
#endif

#ifndef __noreturn
#define __noreturn __attribute__((__noreturn__))
#endif

#ifndef __pure
#define __pure __attribute__((__pure__))
#endif

#ifndef __const
#define __const __attribute__((__const__))
#endif

#ifndef __restrict
#define __restrict restrict
#endif

// printf/scanf format checking attributes
#ifndef __printflike
#define __printflike(fmtarg, firstvararg) \
    __attribute__((__format__(__printf__, fmtarg, firstvararg)))
#endif

#ifndef __scanflike
#define __scanflike(fmtarg, firstvararg) \
    __attribute__((__format__(__scanf__, fmtarg, firstvararg)))
#endif

#endif
