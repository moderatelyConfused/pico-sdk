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

Create a minimal `build.zig.zon`:

```zig
.{
    .name = .my_project,
    .version = "0.1.0",
    .fingerprint = 0x0,  // Zig will tell you the correct value on first build
    .minimum_zig_version = "0.15.2",
    .dependencies = .{},
    .paths = .{
        "build.zig",
        "build.zig.zon",
        "src",
    },
}
```

Then fetch the pico-sdk dependency:

```bash
zig fetch --save git+https://github.com/moderatelyConfused/pico-sdk
```

This automatically adds the dependency with the correct hash to your `build.zig.zon`.

### 3. Create your build.zig

```zig
const std = @import("std");
const pico_sdk = @import("pico_sdk");

pub fn build(b: *std.Build) void {
    // Target chip and board selection
    const chip: pico_sdk.Chip = b.option(pico_sdk.Chip, "chip", "Target chip") orelse .rp2040;
    const board = b.option([]const u8, "board", "Target board") orelse "pico";

    // Get the cross-compilation target for the chip
    const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, .arm));
    const optimize = b.standardOptimizeOption(.{});

    // Get the pico-sdk dependency
    const sdk_dep = b.dependency("pico_sdk", .{});

    // Create executable
    const exe = b.addExecutable(.{
        .name = "my_app",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });

    // Add SDK includes and compile definitions
    pico_sdk.addTo(sdk_dep, exe, chip, board);

    // Add SDK components you need
    pico_sdk.addComponents(sdk_dep, exe, chip, &.{
        .pico_platform,
        .pico_runtime,
        .hardware_gpio,
    });

    b.installArtifact(exe);

    // Generate .bin file for flashing
    const bin = b.addObjCopy(exe.getEmittedBin(), .{ .format = .bin });
    bin.step.dependOn(&exe.step);
    const install_bin = b.addInstallBinFile(bin.getOutput(), "my_app.bin");
    b.getInstallStep().dependOn(&install_bin.step);
}
```

### 4. Build

```bash
zig build
```

Output will be in `zig-out/bin/`.

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
    pico_platform,
    pico_runtime,
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
    components: []const Component,
) void
```
Adds SDK component source files to a compile step. Use this to include only the SDK functionality you need.

**Example:**
```zig
pico_sdk.addComponents(sdk_dep, exe, chip, &.{
    .pico_platform,
    .hardware_gpio,
    .hardware_uart,
});
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

**Example:**
```zig
pico_sdk.addCSourceFiles(sdk_dep, exe, &.{
    "src/rp2_common/hardware_gpio/gpio.c",
});
```

## Examples

### Zig Blinky

**src/main.zig:**
```zig
const c = @cImport({
    @cInclude("hardware/gpio.h");
});

const LED_PIN = 25;

fn delay(cycles: u32) void {
    var i: u32 = 0;
    while (i < cycles) : (i += 1) {
        asm volatile ("nop");
    }
}

export fn _start() callconv(.c) noreturn {
    c.gpio_init(LED_PIN);
    c.gpio_set_dir(LED_PIN, true);

    while (true) {
        c.gpio_put(LED_PIN, true);
        delay(1_000_000);
        c.gpio_put(LED_PIN, false);
        delay(1_000_000);
    }
}

pub fn panic(msg: []const u8, _: ?*std.builtin.StackTrace, _: ?usize) noreturn {
    _ = msg;
    while (true) {}
}
```

### C Project with Zig Build

For pure C projects, create your executable without a Zig root source file:

**build.zig:**
```zig
const std = @import("std");
const pico_sdk = @import("pico_sdk");

pub fn build(b: *std.Build) void {
    const chip: pico_sdk.Chip = b.option(pico_sdk.Chip, "chip", "Target chip") orelse .rp2040;
    const board = b.option([]const u8, "board", "Target board") orelse "pico";

    const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, .arm));
    const optimize = b.standardOptimizeOption(.{});

    const sdk_dep = b.dependency("pico_sdk", .{});

    const exe = b.addExecutable(.{
        .name = "blinky",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = false,
        }),
    });

    pico_sdk.addTo(sdk_dep, exe, chip, board);
    pico_sdk.addComponents(sdk_dep, exe, chip, &.{
        .pico_platform,
        .pico_runtime,
        .hardware_gpio,
    });

    // Add your C source files
    exe.addCSourceFiles(.{
        .files = &.{ "src/main.c", "src/stubs.c" },
        .flags = &.{
            "-std=c11",
            "-ffreestanding",
            "-D__always_inline=__attribute__((__always_inline__)) inline",
            "-D__printflike(a,b)=__attribute__((__format__(__printf__,a,b)))",
            "-Dstatic_assert=_Static_assert",
        },
    });

    b.installArtifact(exe);
}
```

**src/main.c:**
```c
#include "hardware/gpio.h"

#define LED_PIN 25

static void delay(unsigned int cycles) {
    for (unsigned int i = 0; i < cycles; i++) {
        __asm__ volatile("nop");
    }
}

void _start(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);

    while (1) {
        gpio_put(LED_PIN, true);
        delay(1000000);
        gpio_put(LED_PIN, false);
        delay(1000000);
    }
}
```

### C Library Stubs

For freestanding builds, you need to provide stub implementations for C library functions the SDK references. Create `src/stubs.c`:

```c
#include <stdarg.h>

void _exit(int status) {
    (void)status;
    while (1) {}
}

void __assert_fail(const char *expr, const char *file, int line) {
    (void)expr; (void)file; (void)line;
    while (1) {}
}

int puts(const char *s) {
    (void)s;
    return 0;
}

int vprintf(const char *format, va_list ap) {
    (void)format; (void)ap;
    return 0;
}

void __unhandled_user_irq(void) {
    while (1) {}
}

__attribute__((section(".data")))
void *irq_handler_chain_slots[1] = {0};

__attribute__((section(".data")))
void *irq_handler_chain_first_slot[1] = {0};

void irq_handler_chain_remove_tail(void) {}
```

## Build Options

Pass options via `-D` flags:

```bash
# Build for RP2350 instead of RP2040
zig build -Dchip=rp2350 -Dboard=pico2

# Release build
zig build -Doptimize=ReleaseFast
```

## Supported Boards

The `board` option accepts any board name from `src/boards/include/boards/`. Common values:

- `pico` - Raspberry Pi Pico (RP2040)
- `pico_w` - Raspberry Pi Pico W (RP2040 + WiFi)
- `pico2` - Raspberry Pi Pico 2 (RP2350)

## Flashing

The build produces a `.bin` file in `zig-out/bin/`. To flash:

1. Hold BOOTSEL button on your Pico
2. Connect USB while holding BOOTSEL
3. Copy the `.bin` file to the mounted RPI-RP2 drive

Or use picotool:
```bash
picotool load zig-out/bin/my_app.bin
picotool reboot
```

## Troubleshooting

### Missing C library headers
The SDK includes stub headers in `zig/libc_stubs/` for freestanding builds. These are automatically added to the include path by `addTo()`.

### Undefined symbols at link time
You likely need to add more components or provide stub implementations. Common missing symbols:
- `puts`, `vprintf`, `_exit` - Add to your stubs.c
- `gpio_*` functions - Add `.hardware_gpio` component
- `irq_*` functions - Add `.hardware_irq` component

### Wrong target architecture
Ensure you're using `pico_sdk.getTarget()` to get the correct cross-compilation target:
```zig
const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, .arm));
```
