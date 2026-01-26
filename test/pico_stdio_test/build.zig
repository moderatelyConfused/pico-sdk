const std = @import("std");
const pico_sdk = @import("pico_sdk");

const StdioBackend = enum {
    uart,
    usb,
};

pub fn build(b: *std.Build) void {
    const chip = b.option(pico_sdk.Chip, "chip", "Target chip") orelse .rp2040;
    const cpu_arch = b.option(pico_sdk.CpuArch, "cpu_arch", "CPU architecture (arm or riscv, rp2350 only)") orelse .arm;
    const board = b.option([]const u8, "board", "Target board") orelse switch (chip) {
        .rp2040 => "pico",
        .rp2350 => "pico2",
    };
    const optimize = b.standardOptimizeOption(.{});
    const stdio_backend = b.option(StdioBackend, "stdio", "Stdio backend to use") orelse .uart;

    const sdk_dep = b.dependency("pico_sdk", .{});
    const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, cpu_arch));

    const name = switch (stdio_backend) {
        .uart => "pico_stdio_test_uart",
        .usb => "pico_stdio_test_usb",
    };

    const exe = b.addExecutable(.{
        .name = name,
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    // Add SDK support
    pico_sdk.addTo(sdk_dep, exe, chip, board);

    // Add test framework include path
    exe.addIncludePath(sdk_dep.path("test/pico_test/include"));

    // Add components - full runtime plus test dependencies
    pico_sdk.addComponents(sdk_dep, exe, chip, cpu_arch, &.{
        // Core runtime
        .pico_platform,
        .pico_runtime,
        .pico_runtime_init,
        .pico_crt0,
        .pico_clib_interface,
        // Hardware
        .hardware_clocks,
        .hardware_pll,
        .hardware_xosc,
        .hardware_watchdog,
        .hardware_irq,
        .hardware_sync,
        .hardware_timer,
        .hardware_ticks,
        .hardware_gpio,
        .hardware_uart,
        .hardware_resets,
        // Standard library
        .pico_stdlib,
        .pico_stdio,
        .pico_time,
        .pico_sync,
        .pico_printf,
        .pico_bit_ops,
        // Test-specific
        .pico_multicore,
    });

    // Configure stdio based on backend
    switch (stdio_backend) {
        .uart => {
            pico_sdk.addComponent(sdk_dep, exe, chip, cpu_arch, .pico_stdio_uart);
            pico_sdk.configureStdio(exe, .{ .uart = true });
        },
        .usb => {
            pico_sdk.enableUSBStdio(sdk_dep, exe, .{
                .chip = chip,
                .cpu_arch = cpu_arch,
                .board = board,
            });
            exe.root_module.addCMacro("PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS", "-1");
        },
    }

    // Enable pico_printf wrapping
    exe.root_module.addCMacro("LIB_PICO_PRINTF_PICO", "1");
    exe.root_module.addCMacro("PICO_STDIO_SHORT_CIRCUIT_CLIB_FUNCS", "0");

    // Set linker script with symbol wrapping for stdio/printf
    pico_sdk.setLinkerScriptWithWrapping(sdk_dep, exe, chip, null, &.{
        .pico_stdio,
        .pico_printf,
    }, .default);

    // Add boot2 for RP2040
    if (chip == .rp2040) {
        pico_sdk.addBoot2(sdk_dep, exe, .boot2_w25q080);
    }

    // Link picolibc
    const picolibc = pico_sdk.getPicolibc(sdk_dep, .{
        .target = target,
        .optimize = optimize,
    });
    exe.linkLibrary(picolibc);

    // Add test source
    exe.addCSourceFile(.{
        .file = b.path("pico_stdio_test.c"),
        .flags = &.{},
    });

    b.installArtifact(exe);
}
