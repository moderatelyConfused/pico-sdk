# Zig Build System Parity Blockers

This document tracks issues that prevent full CMake parity in the Zig build system.

## LLVM Inline Assembly Literal Pool Handling

**Location:** `test/pico_divider_test/`

**Problem:** LLVM's integrated assembler doesn't correctly handle literal pools for inline assembly `ldr rx, =constant` pseudo-instructions in naked functions. GCC handles this correctly, but LLVM places the literal pool too far from the load instruction, causing "out of range pc-relative fixup" errors.

**Impact:** The `pico_divider_test` cannot be built with Zig. The `-fno-integrated-as` flag doesn't work on Cortex-M0+ (Thumb-only architecture).

**Fix needed:** Either LLVM needs to fix inline assembly literal pool placement, or affected code needs to be rewritten to avoid the `ldr rx, =constant` pattern in naked functions.

---

## Not Yet Implemented

These CMake features are not yet implemented in the Zig build system:

- **WiFi/Bluetooth support** - pico_cyw43_driver, pico_btstack, pico_lwip
- **pico_time_test** - requires `struct tm` from full C library time.h
