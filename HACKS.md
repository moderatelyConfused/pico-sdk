# Zig Build System Parity Blockers

This document tracks issues that prevent full CMake parity in the Zig build system.

## LLVM Inline Assembly Literal Pool Handling

**Location:** `test/pico_divider_test/`

**Problem:** LLVM's integrated assembler doesn't correctly handle literal pools for inline assembly `ldr rx, =constant` pseudo-instructions in naked functions. GCC handles this correctly, but LLVM places the literal pool too far from the load instruction, causing "out of range pc-relative fixup" errors.

**Impact:** The `pico_divider_test` cannot be built with Zig. The `-fno-integrated-as` flag doesn't work on Cortex-M0+ (Thumb-only architecture).

**Fix needed:** Either LLVM needs to fix inline assembly literal pool placement, or affected code needs to be rewritten to avoid the `ldr rx, =constant` pattern in naked functions.

---

## RP2350 Software Spin Lock EXTEXCLALL Timing

**Location:** `src/rp2_common/pico_clib_interface/picolibc_interface.c`

**Problem:** On RP2350 ARM, software spin locks use `ldaexb`/`strexb` exclusive access instructions which require the `EXTEXCLALL` bit in M33's ACTLR register to enable the global exclusive monitor. The SDK registers `spinlock_set_extexclall` as a per-core init at priority `ZZZZZ.01000`, but this runs *after* `runtime_init_default_alarm_pool` (priority `11000`) which needs spin locks.

In CMake+GCC builds with newlib, this timing issue doesn't manifest (possibly due to different linker behavior with preinit_array entries from static libraries). In Zig builds with picolibc, the preinit_array ordering causes spin lock acquisition to hang before EXTEXCLALL is set.

**Fix:** Set `EXTEXCLALL` early in `runtime_init()` before calling `__libc_init_array()`.

**Upstream:** Not yet reported.

---

## RP2350 Software Spin Lock Array Alignment

**Location:** `src/rp2_common/hardware_sync_spin_lock/sync_spin_lock.c`

**Problem:** The `_sw_spin_locks` array was not explicitly aligned. ARM exclusive access instructions (`ldaexb`/`strexb`) require proper alignment for the global exclusive monitor to work correctly. Without alignment, linkers may place the array at an odd address, causing exclusive accesses to fail spuriously.

**Fix:** Added `__attribute__((aligned(4)))` to the `_sw_spin_locks` array definition.

**Upstream:** Should be reported to pico-sdk.

---

## Not Yet Implemented

These CMake features are not yet implemented in the Zig build system:

- **WiFi/Bluetooth support** - pico_cyw43_driver, pico_btstack, pico_lwip
