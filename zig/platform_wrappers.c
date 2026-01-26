// Wrapper functions for platform inline functions
// These are needed because Zig's @cImport doesn't handle static inline functions
// that contain inline assembly.
//
// We intentionally avoid including headers that define these functions
// as static inline to prevent redefinition errors.

#include <stdint.h>
#include <stdbool.h>

// Define pico_default_asm_volatile for ARM thumb
#ifdef __ARM_ARCH_ISA_THUMB
#define pico_default_asm_volatile(...) __asm volatile (".syntax unified\n" __VA_ARGS__)
#define pico_default_asm(...) __asm (".syntax unified\n" __VA_ARGS__)
#else
#define pico_default_asm_volatile(...) __asm volatile (__VA_ARGS__)
#define pico_default_asm(...) __asm (__VA_ARGS__)
#endif

// Define uint to match SDK
typedef unsigned int uint;

// ============================================================================
// Compiler memory barrier (works on all chips)
// ============================================================================

void __compiler_memory_barrier(void) {
    pico_default_asm_volatile ("" : : : "memory");
}

// ============================================================================
// Platform functions
// ============================================================================

#ifndef __riscv

// ARM implementations (RP2040 Cortex-M0+ and RP2350 Cortex-M33)

void busy_wait_at_least_cycles(uint32_t minimum_cycles) {
    pico_default_asm_volatile (
        "1: subs %0, #3\n"
        "bcs 1b\n"
        : "+r" (minimum_cycles) : : "cc", "memory"
    );
}

void __breakpoint(void) {
    pico_default_asm_volatile ("bkpt #0" : : : "memory");
}

uint __get_current_exception(void) {
    uint exception;
    pico_default_asm_volatile (
        "mrs %0, ipsr\n"
        "uxtb %0, %0\n"
        : "=l" (exception)
    );
    return exception;
}

#if defined(PICO_RP2350) && PICO_RP2350
// RP2350-only functions

bool pico_processor_state_is_nonsecure(void) {
    uint32_t tt;
    pico_default_asm_volatile (
        "movs %0, #0\n"
        "tt %0, %0\n"
        : "=r" (tt) : : "cc"
    );
    return !(tt & (1u << 22));
}

int32_t __mul_instruction(int32_t a, int32_t b) {
    pico_default_asm ("muls %0, %1" : "+l" (a) : "l" (b) : "cc");
    return a;
}

#elif defined(PICO_RP2040) && PICO_RP2040
// RP2040-only functions

int32_t __mul_instruction(int32_t a, int32_t b) {
    pico_default_asm ("muls %0, %1" : "+l" (a) : "l" (b) : "cc");
    return a;
}

#endif

// ============================================================================
// AEABI divider error handlers (ARM only)
// These are called by pico_divider on divide-by-zero
// ============================================================================

int32_t __aeabi_idiv0(int32_t r) {
    // Standard behavior: return the dividend passed in r0
    return r;
}

int64_t __aeabi_ldiv0(int64_t r) {
    // Standard behavior: return the dividend passed in r0:r1
    return r;
}

// ============================================================================
// Sync functions (ARM implementations)
// ============================================================================

void __nop(void) {
#if !__ARM_ARCH_6M__
    pico_default_asm_volatile ("nop.w");
#else
    pico_default_asm_volatile ("nop");
#endif
}

void __sev(void) {
    pico_default_asm_volatile ("sev");
}

void __wfe(void) {
    pico_default_asm_volatile ("wfe");
}

void __wfi(void) {
    pico_default_asm_volatile ("wfi");
}

void __dmb(void) {
    pico_default_asm_volatile ("dmb" : : : "memory");
}

void __dsb(void) {
    pico_default_asm_volatile ("dsb" : : : "memory");
}

void __isb(void) {
    pico_default_asm_volatile ("isb" : : : "memory");
}

#else // __riscv

// RISC-V implementations (RP2350 Hazard3)

void busy_wait_at_least_cycles(uint32_t minimum_cycles) {
    pico_default_asm_volatile (
        ".option push\n"
        ".option norvc\n"
        ".p2align 2\n"
        "1: \n"
        "addi %0, %0, -2 \n"
        "bgez %0, 1b\n"
        ".option pop"
        : "+r" (minimum_cycles) : : "cc", "memory"
    );
}

void __breakpoint(void) {
    __asm ("ebreak");
}

uint __get_current_exception(void) {
    // RISC-V implementation would need meicontext CSR access
    // For now, return 0 (not in exception)
    return 0;
}

bool pico_processor_state_is_nonsecure(void) {
    // NonSecure is an Arm concept
    return false;
}

int32_t __mul_instruction(int32_t a, int32_t b) {
    __asm ("mul %0, %0, %1" : "+r" (a) : "r" (b) : );
    return a;
}

// ============================================================================
// Sync functions (RISC-V specific implementations)
// ============================================================================

void __nop(void) {
    __asm volatile ("nop");
}

void __sev(void) {
    // RISC-V uses hazard3_unblock but we need a CSR write for that
    // For basic support, just use a fence
    __asm volatile ("fence" : : : "memory");
}

void __wfe(void) {
    // RISC-V uses hazard3_block - WFI is close but not exact
    __asm volatile ("wfi");
}

void __wfi(void) {
    __asm volatile ("wfi");
}

void __dmb(void) {
    __asm volatile ("fence rw, rw" : : : "memory");
}

void __dsb(void) {
    __asm volatile ("fence rw, rw" : : : "memory");
}

void __isb(void) {
    __asm volatile ("fence.i" : : : "memory");
}

#endif // __riscv
