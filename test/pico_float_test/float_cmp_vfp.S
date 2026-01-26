/*
 * Copyright (c) 2024 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * VFP-based float comparison functions for RP2350
 *
 * These provide AEABI compare functions that properly set APSR flags
 * using VFP instructions. Needed because compiler-rt may not provide
 * flag-setting versions.
 */

#if !PICO_RP2040
#include "pico/asm_helper.S"

pico_default_asm_setup

.macro float_section name
#if PICO_FLOAT_IN_RAM
.section RAM_SECTION_NAME(\name), "ax"
#else
.section SECTION_NAME(\name), "ax"
#endif
.endm

.macro float_wrapper_section func
float_section WRAPPER_FUNC_NAME(\func)
.endm

// Flag-setting compare functions
// These set APSR flags as per ARM AEABI specification:
//   For cfcmple/cfrcmple: sets Z,C flags for three-way compare
//     a < b:  Z=0, C=0
//     a == b: Z=1, C=1
//     a > b:  Z=0, C=1
//     unordered: Z=0, C=1 (treat as >)

float_section __aeabi_cfcmple
// void __aeabi_cfcmple(float a, float b)
// Sets APSR flags for a <= b comparison
regular_func __aeabi_cfcmple
    vmov s14, r0        @ arg1 (a) to VFP
    vmov s15, r1        @ arg2 (b) to VFP
    vcmp.f32 s14, s15   @ compare s14 to s15, sets FPSCR flags
    vmrs APSR_nzcv, FPSCR @ copy FPSCR flags to APSR
    bvs cmp_nan         @ handle unordered (NaN) case
    bx lr

float_section __aeabi_cfrcmple
// void __aeabi_cfrcmple(float a, float b)
// Sets APSR flags for b <= a comparison (reversed)
regular_func __aeabi_cfrcmple
    vmov s14, r1        @ arg2 (b) to VFP (reversed)
    vmov s15, r0        @ arg1 (a) to VFP (reversed)
    vcmp.f32 s14, s15   @ compare s14 to s15 (b <= a)
    vmrs APSR_nzcv, FPSCR @ copy FPSCR flags to APSR
    bvs cmp_nan         @ handle unordered (NaN) case
    bx lr

float_section __aeabi_cfcmpeq
// void __aeabi_cfcmpeq(float a, float b)
// Sets APSR flags for a == b comparison (non-excepting)
regular_func __aeabi_cfcmpeq
    vmov s14, r0        @ arg1 (a) to VFP
    vmov s15, r1        @ arg2 (b) to VFP
    vcmp.f32 s14, s15   @ compare s14 to s15
    vmrs APSR_nzcv, FPSCR @ copy FPSCR flags to APSR
    bvs cmp_nan         @ handle unordered (NaN) case
    bx lr

// Common handler for NaN (unordered) case
// Per AEABI: unordered comparisons should set C=1, Z=0
cmp_nan:
    movs r12, #3        @ r12 = 3 (doesn't need to be preserved)
    lsrs r12, r12, #1   @ shift right: sets C=1, Z=0
    bx lr

// Return-value compare functions
// These return 1/0 in r0

float_section __aeabi_fcmpeq
// int __aeabi_fcmpeq(float a, float b)
// Returns 1 if a == b, 0 otherwise
regular_func __aeabi_fcmpeq
    vmov s14, r0
    vmov s15, r1
    vcmp.f32 s14, s15
    vmrs APSR_nzcv, FPSCR
    ite eq
    moveq r0, #1
    movne r0, #0
    bx lr

float_section __aeabi_fcmplt
// int __aeabi_fcmplt(float a, float b)
// Returns 1 if a < b, 0 otherwise
regular_func __aeabi_fcmplt
    vmov s14, r0
    vmov s15, r1
    vcmp.f32 s14, s15
    vmrs APSR_nzcv, FPSCR
    ite mi              @ mi = negative = less than
    movmi r0, #1
    movpl r0, #0
    bx lr

float_section __aeabi_fcmple
// int __aeabi_fcmple(float a, float b)
// Returns 1 if a <= b, 0 otherwise
regular_func __aeabi_fcmple
    vmov s14, r0
    vmov s15, r1
    vcmp.f32 s14, s15
    vmrs APSR_nzcv, FPSCR
    ite ls              @ ls = lower or same (unsigned <=, works for float result)
    movls r0, #1
    movhi r0, #0
    bx lr

float_section __aeabi_fcmpge
// int __aeabi_fcmpge(float a, float b)
// Returns 1 if a >= b, 0 otherwise
regular_func __aeabi_fcmpge
    vmov s14, r0
    vmov s15, r1
    vcmp.f32 s14, s15
    vmrs APSR_nzcv, FPSCR
    ite ge              @ ge = greater or equal
    movge r0, #1
    movlt r0, #0
    bx lr

float_section __aeabi_fcmpgt
// int __aeabi_fcmpgt(float a, float b)
// Returns 1 if a > b, 0 otherwise
regular_func __aeabi_fcmpgt
    vmov s14, r0
    vmov s15, r1
    vcmp.f32 s14, s15
    vmrs APSR_nzcv, FPSCR
    ite gt              @ gt = greater than
    movgt r0, #1
    movle r0, #0
    bx lr

float_section __aeabi_fcmpun
// int __aeabi_fcmpun(float a, float b)
// Returns 1 if unordered (either is NaN), 0 otherwise
regular_func __aeabi_fcmpun
    vmov s14, r0
    vmov s15, r1
    vcmp.f32 s14, s15
    vmrs APSR_nzcv, FPSCR
    ite vs              @ vs = overflow = unordered for float compare
    movvs r0, #1
    movvc r0, #0
    bx lr

#endif
