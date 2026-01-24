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
