/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Minimal C library interface for freestanding builds (no libc required)
// Use this when building with Zig or other toolchains without newlib/picolibc

#include "pico.h"
#include "pico/runtime.h"
#include "pico/runtime_init.h"
#include "hardware/sync.h"
#include <stddef.h>

void __attribute__((noreturn)) _exit(__unused int status) {
    while (1) {
        __wfi();
    }
}

void __attribute__((noreturn)) exit(int status) {
    _exit(status);
}

void __assert_func(__unused const char *file, __unused int line,
                   __unused const char *func, __unused const char *failedexpr) {
    while (1) {
        __breakpoint();
    }
}

// Signature matching Zig's libc_stubs/assert.h
void __assert_fail(__unused const char *expr, __unused const char *file, __unused int line) {
    while (1) {
        __breakpoint();
    }
}

// Minimal printf stubs for panic messages
int puts(__unused const char *s) {
    return 0;
}

int vprintf(__unused const char *format, __unused __builtin_va_list ap) {
    return 0;
}

// IRQ handler chain support - minimal stubs
// These are expected to be in .data section as arrays
__attribute__((section(".data")))
void *irq_handler_chain_slots[1] = {0};

__attribute__((section(".data")))
void *irq_handler_chain_first_slot[1] = {0};

void irq_handler_chain_remove_tail(void) {
    // Stub - not used in minimal builds
}

void runtime_init(void) {
    // Run preinit_array (SDK runtime initializers for clocks, etc.)
    runtime_run_initializers();

    // Run init_array (C++ constructors, etc.)
    extern void (*__init_array_start)(void);
    extern void (*__init_array_end)(void);
    for (void (**p)(void) = &__init_array_start; p < &__init_array_end; p++) {
        (*p)();
    }
}

// Basic string functions
size_t strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return p - s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s_ptr = src;
    while (n--) *d++ = *s_ptr++;
    return dest;
}

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

// Stub malloc/free - these panic since we don't have a heap
// For actual heap allocation, link against newlib or provide your own allocator
void *malloc(__unused size_t size) {
    panic("malloc not supported in minimal build");
    return 0;
}

void *calloc(__unused size_t nmemb, __unused size_t size) {
    panic("calloc not supported in minimal build");
    return 0;
}

void *realloc(__unused void *ptr, __unused size_t size) {
    panic("realloc not supported in minimal build");
    return 0;
}

void free(__unused void *ptr) {
    // Allow free(NULL) silently
    if (ptr) {
        panic("free not supported in minimal build");
    }
}

// Provide a stub for runtime_init_default_alarm_pool
// This is a weak symbol in time.c, so our strong definition overrides it
// This avoids the need for alarm pool initialization which requires malloc
void runtime_init_default_alarm_pool(void) {
    // Do nothing - alarm pool disabled in minimal builds
}
