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

// ============================================================================
// OUT mask write instructions
// ============================================================================

void gpioc_lo_out_put(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #0, %0, c0, c0" : : "r" (x));
}

void gpioc_lo_out_xor(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #1, %0, c0, c0" : : "r" (x));
}

void gpioc_lo_out_set(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #2, %0, c0, c0" : : "r" (x));
}

void gpioc_lo_out_clr(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #3, %0, c0, c0" : : "r" (x));
}

void gpioc_hi_out_put(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #0, %0, c0, c1" : : "r" (x));
}

void gpioc_hi_out_xor(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #1, %0, c0, c1" : : "r" (x));
}

void gpioc_hi_out_set(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #2, %0, c0, c1" : : "r" (x));
}

void gpioc_hi_out_clr(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #3, %0, c0, c1" : : "r" (x));
}

void gpioc_hilo_out_put(uint64_t x) {
    pico_default_asm_volatile ("mcrr p0, #0, %0, %1, c0" : : "r" ((uint32_t)(x & 0xffffffffu)), "r" ((uint32_t)(x >> 32)));
}

void gpioc_hilo_out_xor(uint64_t x) {
    pico_default_asm_volatile ("mcrr p0, #1, %0, %1, c0" : : "r" ((uint32_t)(x & 0xffffffffu)), "r" ((uint32_t)(x >> 32)));
}

void gpioc_hilo_out_set(uint64_t x) {
    pico_default_asm_volatile ("mcrr p0, #2, %0, %1, c0" : : "r" ((uint32_t)(x & 0xffffffffu)), "r" ((uint32_t)(x >> 32)));
}

void gpioc_hilo_out_clr(uint64_t x) {
    pico_default_asm_volatile ("mcrr p0, #3, %0, %1, c0" : : "r" ((uint32_t)(x & 0xffffffffu)), "r" ((uint32_t)(x >> 32)));
}

// ============================================================================
// OE mask write instructions
// ============================================================================

void gpioc_lo_oe_put(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #0, %0, c0, c4" : : "r" (x));
}

void gpioc_lo_oe_xor(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #1, %0, c0, c4" : : "r" (x));
}

void gpioc_lo_oe_set(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #2, %0, c0, c4" : : "r" (x));
}

void gpioc_lo_oe_clr(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #3, %0, c0, c4" : : "r" (x));
}

void gpioc_hi_oe_put(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #0, %0, c0, c5" : : "r" (x));
}

void gpioc_hi_oe_xor(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #1, %0, c0, c5" : : "r" (x));
}

void gpioc_hi_oe_set(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #2, %0, c0, c5" : : "r" (x));
}

void gpioc_hi_oe_clr(uint32_t x) {
    pico_default_asm_volatile ("mcr p0, #3, %0, c0, c5" : : "r" (x));
}

void gpioc_hilo_oe_put(uint64_t x) {
    pico_default_asm_volatile ("mcrr p0, #0, %0, %1, c4" : : "r" ((uint32_t)(x & 0xffffffffu)), "r" ((uint32_t)(x >> 32)));
}

void gpioc_hilo_oe_xor(uint64_t x) {
    pico_default_asm_volatile ("mcrr p0, #1, %0, %1, c4" : : "r" ((uint32_t)(x & 0xffffffffu)), "r" ((uint32_t)(x >> 32)));
}

void gpioc_hilo_oe_set(uint64_t x) {
    pico_default_asm_volatile ("mcrr p0, #2, %0, %1, c4" : : "r" ((uint32_t)(x & 0xffffffffu)), "r" ((uint32_t)(x >> 32)));
}

void gpioc_hilo_oe_clr(uint64_t x) {
    pico_default_asm_volatile ("mcrr p0, #3, %0, %1, c4" : : "r" ((uint32_t)(x & 0xffffffffu)), "r" ((uint32_t)(x >> 32)));
}

// ============================================================================
// Single-bit write instructions
// ============================================================================

void gpioc_bit_out_put(uint pin, bool val) {
    pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c0" : : "r" (pin), "r" (val));
}

void gpioc_bit_out_xor(uint pin) {
    pico_default_asm_volatile ("mcr p0, #5, %0, c0, c0" : : "r" (pin));
}

void gpioc_bit_out_set(uint pin) {
    pico_default_asm_volatile ("mcr p0, #6, %0, c0, c0" : : "r" (pin));
}

void gpioc_bit_out_clr(uint pin) {
    pico_default_asm_volatile ("mcr p0, #7, %0, c0, c0" : : "r" (pin));
}

void gpioc_bit_out_xor2(uint pin, bool val) {
    pico_default_asm_volatile ("mcrr p0, #5, %0, %1, c0" : : "r" (pin), "r" (val));
}

void gpioc_bit_out_set2(uint pin, bool val) {
    pico_default_asm_volatile ("mcrr p0, #6, %0, %1, c0" : : "r" (pin), "r" (val));
}

void gpioc_bit_out_clr2(uint pin, bool val) {
    pico_default_asm_volatile ("mcrr p0, #7, %0, %1, c0" : : "r" (pin), "r" (val));
}

void gpioc_bit_oe_put(uint pin, bool val) {
    pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c4" : : "r" (pin), "r" (val));
}

void gpioc_bit_oe_xor(uint pin) {
    pico_default_asm_volatile ("mcr p0, #5, %0, c0, c4" : : "r" (pin));
}

void gpioc_bit_oe_set(uint pin) {
    pico_default_asm_volatile ("mcr p0, #6, %0, c0, c4" : : "r" (pin));
}

void gpioc_bit_oe_clr(uint pin) {
    pico_default_asm_volatile ("mcr p0, #7, %0, c0, c4" : : "r" (pin));
}

void gpioc_bit_oe_xor2(uint pin, bool val) {
    pico_default_asm_volatile ("mcrr p0, #5, %0, %1, c4" : : "r" (pin), "r" (val));
}

void gpioc_bit_oe_set2(uint pin, bool val) {
    pico_default_asm_volatile ("mcrr p0, #6, %0, %1, c4" : : "r" (pin), "r" (val));
}

void gpioc_bit_oe_clr2(uint pin, bool val) {
    pico_default_asm_volatile ("mcrr p0, #7, %0, %1, c4" : : "r" (pin), "r" (val));
}

// ============================================================================
// Indexed mask write instructions
// ============================================================================

void gpioc_index_out_put(uint reg_index, uint32_t val) {
    pico_default_asm_volatile ("mcrr p0, #8, %1, %0, c0" : : "r" (reg_index), "r" (val));
}

void gpioc_index_out_xor(uint reg_index, uint32_t mask) {
    pico_default_asm_volatile ("mcrr p0, #9, %1, %0, c0" : : "r" (reg_index), "r" (mask));
}

void gpioc_index_out_set(uint reg_index, uint32_t mask) {
    pico_default_asm_volatile ("mcrr p0, #10, %1, %0, c0" : : "r" (reg_index), "r" (mask));
}

void gpioc_index_out_clr(uint reg_index, uint32_t mask) {
    pico_default_asm_volatile ("mcrr p0, #11, %1, %0, c0" : : "r" (reg_index), "r" (mask));
}

void gpioc_index_oe_put(uint reg_index, uint32_t val) {
    pico_default_asm_volatile ("mcrr p0, #8, %1, %0, c4" : : "r" (reg_index), "r" (val));
}

void gpioc_index_oe_xor(uint reg_index, uint32_t mask) {
    pico_default_asm_volatile ("mcrr p0, #9, %1, %0, c4" : : "r" (reg_index), "r" (mask));
}

void gpioc_index_oe_set(uint reg_index, uint32_t mask) {
    pico_default_asm_volatile ("mcrr p0, #10, %1, %0, c4" : : "r" (reg_index), "r" (mask));
}

void gpioc_index_oe_clr(uint reg_index, uint32_t mask) {
    pico_default_asm_volatile ("mcrr p0, #11, %1, %0, c4" : : "r" (reg_index), "r" (mask));
}

// ============================================================================
// Read instructions
// ============================================================================

uint32_t gpioc_lo_out_get(void) {
    uint32_t lo;
    pico_default_asm_volatile ("mrc p0, #0, %0, c0, c0" : "=r" (lo));
    return lo;
}

uint32_t gpioc_hi_out_get(void) {
    uint32_t hi;
    pico_default_asm_volatile ("mrc p0, #0, %0, c0, c1" : "=r" (hi));
    return hi;
}

uint64_t gpioc_hilo_out_get(void) {
    uint32_t hi, lo;
    pico_default_asm_volatile ("mrrc p0, #0, %0, %1, c0" : "=r" (lo), "=r" (hi));
    return ((uint64_t)hi << 32) | lo;
}

uint32_t gpioc_lo_oe_get(void) {
    uint32_t lo;
    pico_default_asm_volatile ("mrc p0, #0, %0, c0, c4" : "=r" (lo));
    return lo;
}

uint32_t gpioc_hi_oe_get(void) {
    uint32_t hi;
    pico_default_asm_volatile ("mrc p0, #0, %0, c0, c5" : "=r" (hi));
    return hi;
}

uint64_t gpioc_hilo_oe_get(void) {
    uint32_t hi, lo;
    pico_default_asm_volatile ("mrrc p0, #0, %0, %1, c4" : "=r" (lo), "=r" (hi));
    return ((uint64_t)hi << 32) | lo;
}

uint32_t gpioc_lo_in_get(void) {
    uint32_t lo;
    pico_default_asm_volatile ("mrc p0, #0, %0, c0, c8" : "=r" (lo));
    return lo;
}

uint32_t gpioc_hi_in_get(void) {
    uint32_t hi;
    pico_default_asm_volatile ("mrc p0, #0, %0, c0, c9" : "=r" (hi));
    return hi;
}

uint64_t gpioc_hilo_in_get(void) {
    uint32_t hi, lo;
    pico_default_asm_volatile ("mrrc p0, #0, %0, %1, c8" : "=r" (lo), "=r" (hi));
    return ((uint64_t)hi << 32) | lo;
}

#endif // PICO_RP2350 && !__riscv
