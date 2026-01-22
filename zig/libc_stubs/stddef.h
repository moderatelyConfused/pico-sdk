// Stub stddef.h for freestanding pico-sdk builds
#ifndef _STDDEF_H
#define _STDDEF_H

typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

#define offsetof(type, member) __builtin_offsetof(type, member)

// For wchar_t
#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;
#endif

#endif
