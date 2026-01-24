// Wrapper functions for RP2350 GPIO coprocessor inline functions
// These are needed because Zig's @cImport doesn't handle static inline functions
// that contain inline assembly.
//
// We intentionally do NOT include gpio_coproc.h to avoid conflicting with the
// static inline definitions there. Instead, we provide non-static versions
// that Zig's linker can resolve.

#include "pico.h"

#if PICO_RP2350 && !defined(__riscv)

#include <stdint.h>
#include <stdbool.h>

// Forward declare the mask functions that we'll use
static inline void gpioc_lo_out_set_impl(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #2, %0, c0, c0" : : "r" (x));
}

static inline void gpioc_lo_out_clr_impl(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #3, %0, c0, c0" : : "r" (x));
}

static inline void gpioc_lo_oe_set_impl(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #2, %0, c4, c0" : : "r" (x));
}

static inline void gpioc_lo_oe_clr_impl(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #3, %0, c4, c0" : : "r" (x));
}

// Provide non-static definitions for the functions Zig needs
void gpioc_bit_out_put(uint pin, bool val) {
    if (val)
        gpioc_lo_out_set_impl(1u << pin);
    else
        gpioc_lo_out_clr_impl(1u << pin);
}

void gpioc_bit_oe_put(uint pin, bool val) {
    if (val)
        gpioc_lo_oe_set_impl(1u << pin);
    else
        gpioc_lo_oe_clr_impl(1u << pin);
}

#endif // PICO_RP2350 && !__riscv
