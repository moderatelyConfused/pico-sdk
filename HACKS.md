# Zig Build System Hacks

This document tracks workarounds and hacks in the Zig build system that need to be resolved for full CMake parity.

## Linker Symbol Aliasing (--wrap replacement)

**Location:** `build.zig` - `setLinkerScriptWithWrapping()`, `getComponentSymbolAliases()`

**Problem:** Zig doesn't support the `--wrap` linker flag (see [ziglang/zig#3206](https://github.com/ziglang/zig/issues/3206)). The pico-sdk uses `--wrap` extensively to intercept C library functions like `printf`, `sprintf`, `malloc`, etc.

**Workaround:** Generate linker script symbol aliases that redirect symbols to their wrapped versions:
```ld
printf = __wrap_printf;
sprintf = __wrap_sprintf;
```

**Components affected:**
- `pico_stdio` - wraps printf, vprintf, puts, putchar, getchar
- `pico_printf` - wraps sprintf, snprintf, vsnprintf
- `pico_float` / `pico_double` - wraps AEABI and math functions
- `pico_divider` / `pico_bit_ops` / `pico_mem_ops` / `pico_int64_ops` - wraps AEABI functions

**Fix needed:** Zig needs to support `--wrap` linker flag, or provide an equivalent mechanism.

---

## Picolibc crt0 Symbol Compatibility

**Location:** `build.zig` - `setLinkerScriptWithWrapping()`

**Problem:** Picolibc's crt0 expects different linker symbol names than the pico-sdk linker scripts provide.

**Workaround:** Add linker script aliases to map picolibc symbol names to pico-sdk names:
```zig
const picolibc_aliases: []const SymbolAlias = &.{
    .{ .name = "__data_start", .target = "__data_start__" },
    .{ .name = "__data_source", .target = "__etext" },
    .{ .name = "__data_size", .target = "__data_end__ - __data_start__" },
    .{ .name = "__bss_start", .target = "__bss_start__" },
    .{ .name = "__bss_size", .target = "__bss_end__ - __bss_start__" },
    .{ .name = "__interrupt_vector", .target = "__VECTOR_TABLE" },
};
```

**Fix needed:** Either modify pico-sdk linker scripts to use picolibc-compatible names, or use a picolibc build configured for pico-sdk symbol names.

---

## Linker Script INCLUDE Directive

**Location:** `build.zig` - `setLinkerScript()`, `setLinkerScriptWithWrapping()`

**Problem:** Zig's linker (LLD) doesn't support the `INCLUDE` directive used by pico-sdk linker scripts:
```ld
INCLUDE memmap_default.ld
```

**Workaround:** Transform linker scripts at build time by replacing INCLUDE directives with the actual memory definitions:
```zig
const memmap_content = switch (chip) {
    .rp2040 => // RP2040 memory map
    .rp2350 => // RP2350 memory map
};
// Replace INCLUDE directive with actual content
```

**Fix needed:** Either LLD needs INCLUDE support, or pico-sdk linker scripts should be refactored to not use INCLUDE.

---

## TLS Setup Disabled

**Location:** `build.zig` - `addComponents()` for `pico_clib_interface`

**Problem:** Thread-Local Storage (TLS) setup in pico_clib_interface relies on toolchain-specific behavior that doesn't work with Zig's freestanding target.

**Workaround:** Disable TLS setup via compile definitions:
```zig
compile.root_module.addCMacro("PICO_RUNTIME_NO_INIT_PER_CORE_TLS_SETUP", "1");
compile.root_module.addCMacro("PICO_RUNTIME_SKIP_INIT_PER_CORE_TLS_SETUP", "1");
```

**Fix needed:** Investigate proper TLS support for Zig's freestanding ARM/RISC-V targets.

---

## LLVM Inline Assembly Literal Pool Handling

**Location:** `test/pico_divider_test/`

**Problem:** LLVM's integrated assembler doesn't correctly handle literal pools for inline assembly `ldr rx, =constant` pseudo-instructions in naked functions. GCC handles this correctly, but LLVM places the literal pool too far from the load instruction, causing "out of range pc-relative fixup" errors.

**Workaround:** The `pico_divider_test` is excluded from the Zig test build. The `-fno-integrated-as` flag doesn't work on Cortex-M0+ (Thumb-only architecture).

**Fix needed:** Either LLVM needs to fix inline assembly literal pool placement, or affected code needs to be rewritten to avoid the `ldr rx, =constant` pattern in naked functions.

---

## AEABI Helper Functions

**Location:** `zig/platform_wrappers.c`

**Problem:** Some AEABI runtime functions expected by pico-sdk code aren't provided by Zig's compiler-rt or picolibc.

**Workaround:** Provide stub implementations in platform_wrappers.c:
- `__aeabi_idiv0` / `__aeabi_ldiv0` - divide-by-zero handlers
- `__aeabi_cfcmpeq` / `__aeabi_cfcmple` / `__aeabi_cfrcmple` - float comparison helpers
- `__aeabi_cdcmpeq` / `__aeabi_cdcmple` / `__aeabi_cdrcmple` - double comparison helpers

**Fix needed:** These should ideally come from compiler-rt or be part of the SDK's standard AEABI support.

---

## Not Yet Implemented

These CMake features are not yet implemented in the Zig build system:

- **WiFi/Bluetooth support** - pico_cyw43_driver, pico_btstack, pico_lwip
- **Kitchen sink test** - comprehensive test linking nearly all SDK components
- **pico_time_test** - requires `struct tm` from full C library time.h
