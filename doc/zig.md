# Using pico-sdk with the Zig Build System

The pico-sdk can be used as a Zig package dependency, allowing you to build RP2040 and RP2350 projects using Zig's build system instead of CMake. This works for both Zig and C projects.

## Quick Start

### 1. Create your project structure

```
my-project/
├── build.zig
├── build.zig.zon
└── src/
    └── main.zig  (or main.c for C projects)
```

### 2. Add pico-sdk as a dependency

Create a `build.zig.zon`:

```zig
.{
    .name = "my_project",
    .version = "0.1.0",
    .fingerprint = 0x0,  // Zig will tell you the correct value on first build
    .minimum_zig_version = "0.15.0",
    .dependencies = .{
        .pico_sdk = .{
            .url = "git+https://github.com/moderatelyConfused/pico-sdk#master",
            .hash = "...",  // zig build will tell you the correct hash
        },
        .picotool = .{
            .url = "git+https://github.com/moderatelyConfused/picotool#master",
            .hash = "...",  // Optional: for upload support
        },
    },
    .paths = .{
        "build.zig",
        "build.zig.zon",
        "src",
    },
}
```

Or fetch with:

```bash
zig fetch --save git+https://github.com/moderatelyConfused/pico-sdk
zig fetch --save git+https://github.com/moderatelyConfused/picotool  # Optional
```

### 3. Create your build.zig

```zig
const std = @import("std");
const pico_sdk = @import("pico_sdk");

pub fn build(b: *std.Build) void {
    // Target chip, CPU architecture, and board selection
    const chip: pico_sdk.Chip = b.option(pico_sdk.Chip, "chip", "Target chip") orelse .rp2040;
    const cpu_arch: pico_sdk.CpuArch = b.option(pico_sdk.CpuArch, "cpu_arch", "CPU architecture") orelse .arm;
    const board = b.option([]const u8, "board", "Target board") orelse "pico";

    // Get the cross-compilation target for the chip
    const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, cpu_arch));
    const optimize = b.standardOptimizeOption(.{});

    // Get the pico-sdk dependency
    const sdk_dep = b.dependency("pico_sdk", .{});

    // Create a static library from SDK C sources
    const sdk_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "pico_sdk_runtime",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    // Add SDK includes and definitions
    pico_sdk.addTo(sdk_dep, sdk_lib, chip, board);

    // Add SDK components
    pico_sdk.addComponents(sdk_dep, sdk_lib, chip, cpu_arch, &.{
        .pico_crt0,
        .pico_platform,
        .pico_runtime,
        .hardware_gpio,
        .hardware_clocks,
        .hardware_pll,
        .hardware_xosc,
        .hardware_watchdog,
        .hardware_irq,
        .hardware_sync,
        .hardware_timer,
        .hardware_ticks,
    });

    // Create the executable
    const mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    mod.stack_protector = false;  // Required for freestanding

    const exe = b.addExecutable(.{
        .name = "my_app",
        .root_module = mod,
    });

    // Use CRT0's entry point
    exe.entry = .{ .symbol_name = "_entry_point" };

    // Link the SDK library
    exe.linkLibrary(sdk_lib);

    // Add SDK includes for @cImport
    pico_sdk.addTo(sdk_dep, exe, chip, board);

    // Add runtime init and minimal C library (linked directly to exe)
    pico_sdk.addComponents(sdk_dep, exe, chip, cpu_arch, &.{
        .pico_runtime_init,
        .pico_clib_minimal,
    });

    // Set linker script for proper memory layout
    pico_sdk.setLinkerScript(sdk_dep, exe, chip, null);

    // Add boot2 for RP2040 (required for flash boot)
    if (chip == .rp2040) {
        pico_sdk.addBoot2(sdk_dep, exe, .boot2_w25q080);
    }

    b.installArtifact(exe);
}
```

### 4. Create your application code

#### For Zig projects (main.zig)

```zig
const c = @cImport({
    @cInclude("hardware/gpio.h");
    @cInclude("hardware/timer.h");
});

const LED_PIN = 25;

export fn main() callconv(.c) c_int {
    c.gpio_init(LED_PIN);
    c.gpio_set_dir(LED_PIN, true);

    while (true) {
        c.gpio_put(LED_PIN, true);
        c.busy_wait_us(500_000);
        c.gpio_put(LED_PIN, false);
        c.busy_wait_us(500_000);
    }
}

pub fn panic(msg: []const u8, _: ?*std.builtin.StackTrace, _: ?usize) noreturn {
    _ = msg;
    while (true) {}
}
```

#### For C projects (main.c)

```c
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define LED_PIN 25

int main(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);  // true = output

    while (1) {
        gpio_put(LED_PIN, true);
        busy_wait_us(500000);  // 500ms
        gpio_put(LED_PIN, false);
        busy_wait_us(500000);
    }

    return 0;
}
```

### 5. Build and Upload

```bash
# Build for RP2040
zig build -Dchip=rp2040

# Build for RP2350
zig build -Dchip=rp2350

# Output is in zig-out/bin/
```

## API Reference

### Types

#### `Chip`
```zig
pub const Chip = enum {
    rp2040,
    rp2350,
};
```

#### `CpuArch`
```zig
pub const CpuArch = enum {
    arm,
    riscv,  // RP2350 only
};
```

#### `Component`
```zig
pub const Component = enum {
    // Hardware peripherals
    hardware_gpio,
    hardware_clocks,
    hardware_pll,
    hardware_xosc,
    hardware_watchdog,
    hardware_irq,
    hardware_sync,
    hardware_timer,
    hardware_ticks,
    hardware_uart,
    hardware_spi,
    hardware_i2c,
    hardware_pwm,
    hardware_adc,
    hardware_dma,
    hardware_pio,
    hardware_flash,
    hardware_divider,
    hardware_interp,
    hardware_exception,
    hardware_resets,        // Header-only
    hardware_rtc,           // RP2040 only
    hardware_vreg,
    hardware_xip_cache,

    // Platform and runtime
    pico_platform,
    pico_runtime,
    pico_runtime_init,
    pico_crt0,
    pico_time,
    pico_clib_minimal,

    // Standard I/O
    pico_stdio,             // Note: requires linker wrapping
    pico_stdio_uart,
    pico_stdio_semihosting,

    // Multicore
    pico_multicore,

    // Math and utility libraries
    pico_printf,            // Note: requires linker wrapping
    pico_malloc,            // Note: requires linker wrapping
    pico_float,             // Note: requires linker wrapping
    pico_double,            // Note: requires linker wrapping
    pico_divider,           // Note: requires linker wrapping (RP2040)
    pico_int64_ops,         // RP2040 only, requires linker wrapping
    pico_bit_ops,           // Note: requires linker wrapping
    pico_mem_ops,           // RP2040 only, requires linker wrapping
    pico_rand,
    pico_unique_id,
    pico_bootrom,
    pico_atomic,
    pico_sync,
    pico_stdlib,
};
```

**Linker Wrapping Support:** Some components (marked above) require symbol wrapping to function properly. Use `setLinkerScriptWithWrapping()` instead of `setLinkerScript()` to enable this:

```zig
// Enable wrapping for stdio and printf
pico_sdk.setLinkerScriptWithWrapping(sdk_dep, exe, chip, null, &.{
    .pico_stdio,
    .pico_printf,
});

// Also add the required compile definitions
pico_sdk.addWrappingDefinitions(exe, &.{ .pico_stdio, .pico_printf });
```

This generates linker script symbol aliases (e.g., `printf = __wrap_printf;`) that redirect standard library calls to SDK implementations.

**Note:** `pico_malloc` wrapping is not supported as it requires an actual malloc implementation from a libc. For memory allocation, use Zig's allocator or link against a libc.

#### `Boot2`
```zig
pub const Boot2 = enum {
    boot2_w25q080,      // Default for Pico boards (W25Q16JV flash)
    boot2_generic_03h,  // Generic 03h read command (slowest, most compatible)
    boot2_at25sf128a,
    boot2_is25lp080,
    boot2_w25x10cl,
};
```

### Functions

#### `getTarget`
```zig
pub fn getTarget(chip: Chip, cpu_arch: CpuArch) std.Target.Query
```
Returns the appropriate cross-compilation target for the specified chip and architecture.

**Example:**
```zig
const target = b.resolveTargetQuery(pico_sdk.getTarget(.rp2040, .arm));
```

#### `addTo`
```zig
pub fn addTo(
    sdk_dep: *std.Build.Dependency,
    compile: *std.Build.Step.Compile,
    chip: Chip,
    board: []const u8,
) void
```
Adds SDK include paths, compile definitions, and generated config headers to a compile step. Call this for any compile step that needs access to SDK headers.

**Example:**
```zig
pico_sdk.addTo(sdk_dep, exe, .rp2040, "pico");
```

#### `addComponents`
```zig
pub fn addComponents(
    sdk_dep: *std.Build.Dependency,
    compile: *std.Build.Step.Compile,
    chip: Chip,
    cpu_arch: CpuArch,
    components: []const Component,
) void
```
Adds SDK component source files to a compile step. Use this to include only the SDK functionality you need. The `cpu_arch` parameter is needed to select the correct source files for ARM vs RISC-V builds.

**Example:**
```zig
pico_sdk.addComponents(sdk_dep, exe, chip, cpu_arch, &.{
    .pico_platform,
    .hardware_gpio,
    .hardware_uart,
});
```

#### `setLinkerScript`
```zig
pub fn setLinkerScript(
    sdk_dep: *std.Build.Dependency,
    compile: *std.Build.Step.Compile,
    chip: Chip,
    flash_size_bytes: ?usize,
) void
```
Sets the linker script for proper Pico memory layout. This is required for the binary to run on hardware.

The `flash_size_bytes` parameter is optional:
- `null` uses the default (2MB for RP2040, 4MB for RP2350)
- Specify a custom size for boards with different flash

**Example:**
```zig
// Use default flash size
pico_sdk.setLinkerScript(sdk_dep, exe, chip, null);

// Specify 16MB flash
pico_sdk.setLinkerScript(sdk_dep, exe, chip, 16 * 1024 * 1024);
```

#### `addBoot2`
```zig
pub fn addBoot2(
    sdk_dep: *std.Build.Dependency,
    compile: *std.Build.Step.Compile,
    boot2_variant: Boot2,
) void
```
Adds the boot2 second-stage bootloader for RP2040. **This is required for RP2040 to boot from flash.** RP2350 does not need boot2.

The boot2 variant should match your board's flash chip:
- `boot2_w25q080` - Default for Pico boards (Winbond W25Q16JV)
- `boot2_generic_03h` - Most compatible, works with any flash

**Example:**
```zig
if (chip == .rp2040) {
    pico_sdk.addBoot2(sdk_dep, exe, .boot2_w25q080);
}
```

#### `addCSourceFiles`
```zig
pub fn addCSourceFiles(
    sdk_dep: *std.Build.Dependency,
    compile: *std.Build.Step.Compile,
    sources: []const []const u8,
) void
```
Low-level function to add specific SDK source files by path. Prefer `addComponents` for most use cases.

#### `getLinkerWrapSymbols`
```zig
pub fn getLinkerWrapSymbols(chip: Chip, component: Component) []const []const u8
```
Returns the list of function names that need linker wrapping for a component. Use this to see which functions would need `--wrap=<func>` linker flags for full functionality.

**Example:**
```zig
const wrap_symbols = pico_sdk.getLinkerWrapSymbols(.rp2040, .pico_stdio);
// Returns: "printf", "vprintf", "puts", "putchar", "getchar"
```

#### `componentRequiresWrapping`
```zig
pub fn componentRequiresWrapping(component: Component) bool
```
Returns true if a component requires linker wrapping to function properly.

**Example:**
```zig
if (pico_sdk.componentRequiresWrapping(.pico_printf)) {
    // This component needs wrapping for full functionality
}
```

#### `setLinkerScriptWithWrapping`
```zig
pub fn setLinkerScriptWithWrapping(
    sdk_dep: *std.Build.Dependency,
    compile: *std.Build.Step.Compile,
    chip: Chip,
    flash_size_bytes: ?usize,
    wrapped_components: []const Component,
) void
```
Sets the linker script with symbol wrapping support. This generates symbol aliases in the linker script that redirect standard library calls to SDK implementations (e.g., `printf = __wrap_printf;`).

**Example:**
```zig
pico_sdk.setLinkerScriptWithWrapping(sdk_dep, exe, chip, null, &.{
    .pico_stdio,
    .pico_printf,
    .pico_float,
});
```

#### `addWrappingDefinitions`
```zig
pub fn addWrappingDefinitions(compile: *std.Build.Step.Compile, components: []const Component) void
```
Adds compile definitions required for wrapped components to work correctly. Call this when using `setLinkerScriptWithWrapping`.

This sets up defines like `LIB_PICO_PRINTF_PICO=1` so that pico_stdio uses the SDK's printf implementation instead of trying to call through to `__real_vprintf`.

**Example:**
```zig
pico_sdk.addWrappingDefinitions(exe, &.{ .pico_stdio, .pico_printf });
```

## Complete Example

Here's a complete build.zig with UF2 generation and upload support:

```zig
const std = @import("std");
const pico_sdk = @import("pico_sdk");

pub fn build(b: *std.Build) void {
    const chip: pico_sdk.Chip = b.option(pico_sdk.Chip, "chip", "Target chip") orelse .rp2040;
    const cpu_arch: pico_sdk.CpuArch = b.option(pico_sdk.CpuArch, "cpu_arch", "CPU architecture") orelse .arm;
    const board = b.option([]const u8, "board", "Target board") orelse "pico";

    const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, cpu_arch));
    const optimize = b.standardOptimizeOption(.{});

    const sdk_dep = b.dependency("pico_sdk", .{});

    // SDK library
    const sdk_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "pico_sdk_runtime",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    pico_sdk.addTo(sdk_dep, sdk_lib, chip, board);
    pico_sdk.addComponents(sdk_dep, sdk_lib, chip, cpu_arch, &.{
        .pico_crt0,
        .pico_platform,
        .pico_runtime,
        .hardware_gpio,
        .hardware_clocks,
        .hardware_pll,
        .hardware_xosc,
        .hardware_watchdog,
        .hardware_irq,
        .hardware_sync,
        .hardware_timer,
        .hardware_ticks,
    });

    // Executable
    const mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    mod.stack_protector = false;

    const exe = b.addExecutable(.{
        .name = "blinky",
        .root_module = mod,
    });

    exe.entry = .{ .symbol_name = "_entry_point" };
    exe.linkLibrary(sdk_lib);

    pico_sdk.addTo(sdk_dep, exe, chip, board);
    pico_sdk.addComponents(sdk_dep, exe, chip, cpu_arch, &.{
        .pico_runtime_init,
        .pico_clib_minimal,
    });

    pico_sdk.setLinkerScript(sdk_dep, exe, chip, null);

    if (chip == .rp2040) {
        pico_sdk.addBoot2(sdk_dep, exe, .boot2_w25q080);
    }

    b.installArtifact(exe);

    // UF2 generation and upload (requires picotool dependency)
    const picotool_dep = b.dependency("picotool", .{});
    const picotool_exe = picotool_dep.artifact("picotool");

    // Generate UF2 for drag-and-drop upload
    const uf2_cmd = b.addRunArtifact(picotool_exe);
    uf2_cmd.addArgs(&.{ "uf2", "convert", "-t", "elf" });
    uf2_cmd.addFileArg(exe.getEmittedBin());
    const uf2_output = uf2_cmd.addOutputFileArg("blinky.uf2");
    uf2_cmd.step.dependOn(&exe.step);

    const install_uf2 = b.addInstallBinFile(uf2_output, "blinky.uf2");
    b.getInstallStep().dependOn(&install_uf2.step);

    // Upload step
    const upload_cmd = b.addRunArtifact(picotool_exe);
    upload_cmd.addArgs(&.{ "load", "-x", "-t", "elf" });
    upload_cmd.addFileArg(exe.getEmittedBin());
    upload_cmd.step.dependOn(&exe.step);

    const upload_step = b.step("upload", "Upload firmware to connected Pico");
    upload_step.dependOn(&upload_cmd.step);
}
```

Then:
```bash
zig build                      # Build ELF and UF2
zig build upload -Dchip=rp2040 # Upload to connected Pico
```

## C Project Example

For C projects, the build.zig is similar but uses `addCSourceFiles` instead of a Zig root source file:

```zig
const std = @import("std");
const pico_sdk = @import("pico_sdk");

pub fn build(b: *std.Build) void {
    const chip: pico_sdk.Chip = b.option(pico_sdk.Chip, "chip", "Target chip") orelse .rp2040;
    const cpu_arch: pico_sdk.CpuArch = b.option(pico_sdk.CpuArch, "cpu_arch", "CPU architecture") orelse .arm;
    const board = b.option([]const u8, "board", "Target board") orelse "pico";

    const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, cpu_arch));
    const optimize = b.standardOptimizeOption(.{});

    const sdk_dep = b.dependency("pico_sdk", .{});

    // SDK library
    const sdk_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "pico_sdk_runtime",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    pico_sdk.addTo(sdk_dep, sdk_lib, chip, board);
    pico_sdk.addComponents(sdk_dep, sdk_lib, chip, cpu_arch, &.{
        .pico_crt0,
        .pico_platform,
        .pico_runtime,
        .hardware_gpio,
        .hardware_clocks,
        .hardware_pll,
        .hardware_xosc,
        .hardware_watchdog,
        .hardware_irq,
        .hardware_sync,
        .hardware_timer,
        .hardware_ticks,
    });

    // Executable (no root_source_file for C projects)
    const exe = b.addExecutable(.{
        .name = "blinky",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    exe.entry = .{ .symbol_name = "_entry_point" };
    exe.linkLibrary(sdk_lib);

    pico_sdk.addTo(sdk_dep, exe, chip, board);
    pico_sdk.addComponents(sdk_dep, exe, chip, cpu_arch, &.{
        .pico_runtime_init,
        .pico_clib_minimal,
    });

    // Add C source files
    exe.addCSourceFiles(.{
        .files = &.{"src/main.c"},
        .flags = &.{
            "-std=c11",
            "-ffreestanding",
            "-ffunction-sections",
            "-fdata-sections",
            "-D__always_inline=__attribute__((__always_inline__)) inline",
            "-D__printflike(a,b)=__attribute__((__format__(__printf__,a,b)))",
            "-Dstatic_assert=_Static_assert",
        },
    });

    pico_sdk.setLinkerScript(sdk_dep, exe, chip, null);

    if (chip == .rp2040) {
        pico_sdk.addBoot2(sdk_dep, exe, .boot2_w25q080);
    }

    b.installArtifact(exe);

    // UF2 and upload steps (same as Zig example)
    const picotool_dep = b.dependency("picotool", .{});
    const picotool_exe = picotool_dep.artifact("picotool");

    const uf2_cmd = b.addRunArtifact(picotool_exe);
    uf2_cmd.addArgs(&.{ "uf2", "convert", "-t", "elf" });
    uf2_cmd.addFileArg(exe.getEmittedBin());
    const uf2_output = uf2_cmd.addOutputFileArg("blinky.uf2");
    uf2_cmd.step.dependOn(&exe.step);

    const install_uf2 = b.addInstallBinFile(uf2_output, "blinky.uf2");
    b.getInstallStep().dependOn(&install_uf2.step);

    const upload_cmd = b.addRunArtifact(picotool_exe);
    upload_cmd.addArgs(&.{ "load", "-x", "-t", "elf" });
    upload_cmd.addFileArg(exe.getEmittedBin());
    upload_cmd.step.dependOn(&exe.step);

    const upload_step = b.step("upload", "Upload firmware to connected Pico");
    upload_step.dependOn(&upload_cmd.step);
}
```

The key differences for C projects:
- No `root_source_file` in the executable's module
- Use `exe.addCSourceFiles()` to add your C source files
- The C compiler flags handle SDK header compatibility

## Build Options

Pass options via `-D` flags:

```bash
# Build for RP2350 instead of RP2040
zig build -Dchip=rp2350 -Dboard=pico2

# Release build
zig build -Doptimize=ReleaseFast

# Upload to device
zig build upload -Dchip=rp2040
```

## Supported Boards

The `board` option accepts any board name from `src/boards/include/boards/`. Common values:

- `pico` - Raspberry Pi Pico (RP2040)
- `pico_w` - Raspberry Pi Pico W (RP2040 + WiFi)
- `pico2` - Raspberry Pi Pico 2 (RP2350)

## Key Concepts

### Component Architecture

The SDK is split into components that can be selectively included:

1. **pico_crt0** - Startup code (vector table, memory init, calls main)
2. **pico_runtime_init** - Clock and peripheral initialization
3. **pico_clib_minimal** - Minimal C library stubs for freestanding builds
4. **pico_platform** - Platform abstraction layer
5. **pico_runtime** - Runtime support (hardware claim, etc.)
6. **hardware_*** - Individual hardware peripheral drivers

### Why Two Compile Steps?

The example uses a static library (`sdk_lib`) plus an executable (`exe`):

- `sdk_lib` contains SDK components that are always needed
- `exe` links `sdk_lib` and adds `pico_runtime_init` and `pico_clib_minimal` directly

The runtime init and clib minimal are added to `exe` directly because they use weak symbols and special linker sections that wouldn't be pulled from a static library.

### Boot2 (RP2040 Only)

RP2040 requires a 256-byte second-stage bootloader (boot2) at the start of flash. This code:
1. Configures the flash chip for fast XIP (execute-in-place) mode
2. Jumps to the main application

The Zig build system compiles boot2 from SDK sources, pads it to 252 bytes, calculates a CRC32 checksum, and embeds the 256-byte blob in your binary.

RP2350 has a different boot architecture and doesn't need boot2.

## Flashing

### Using UF2 (Drag and Drop)

1. Hold BOOTSEL button on your Pico
2. Connect USB while holding BOOTSEL
3. Copy `zig-out/bin/my_app.uf2` to the mounted RPI-RP2 drive

### Using picotool

```bash
# Upload and reboot
zig build upload -Dchip=rp2040

# Or manually:
picotool load -x zig-out/bin/my_app
```

## Troubleshooting

### "Pico second stage bootloader must be 256 bytes"

You need to add boot2 for RP2040:
```zig
if (chip == .rp2040) {
    pico_sdk.addBoot2(sdk_dep, exe, .boot2_w25q080);
}
```

### Undefined symbol: spin_locks_reset

Add the `hardware_sync` component.

### Undefined symbol: runtime_init

Add the `pico_runtime_init` component directly to your executable (not a static library).

### Binary uploads but doesn't run

1. Ensure you're using `setLinkerScript()` for proper memory layout
2. Ensure entry point is set: `exe.entry = .{ .symbol_name = "_entry_point" };`
3. For RP2040, ensure boot2 is added with `addBoot2()`

### Wrong LED pin

LED pin varies by board:
- Pico (RP2040): GPIO 25
- Pico W (RP2040): GPIO 0 (directly connected to WiFi chip)
- Pico 2 (RP2350): GPIO 25

### Build works but device doesn't blink

Check that you're building for the correct chip:
```bash
zig build -Dchip=rp2040   # For original Pico
zig build -Dchip=rp2350   # For Pico 2
```
