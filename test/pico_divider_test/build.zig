const std = @import("std");
const pico_sdk = @import("pico_sdk");

pub fn build(b: *std.Build) void {
    // This test is RP2040 only (hardware divider specific)
    const chip: pico_sdk.Chip = .rp2040;
    _ = b.option(pico_sdk.Chip, "chip", "Target chip (ignored, RP2040 only)");
    const cpu_arch: pico_sdk.CpuArch = .arm;
    _ = b.option(pico_sdk.CpuArch, "cpu_arch", "CPU architecture (ignored, ARM only)");
    const board = b.option([]const u8, "board", "Target board") orelse "pico";
    const optimize = b.standardOptimizeOption(.{});

    const sdk_dep = b.dependency("pico_sdk", .{});
    const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, cpu_arch));

    const exe = b.addExecutable(.{
        .name = "pico_divider_test",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    pico_sdk.addTo(sdk_dep, exe, chip, board);
    exe.addIncludePath(sdk_dep.path("test/pico_test/include"));

    pico_sdk.addComponents(sdk_dep, exe, chip, cpu_arch, &.{
        .pico_platform,
        .pico_runtime,
        .pico_runtime_init,
        .pico_crt0,
        .pico_clib_interface,
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
        .pico_divider, // test-specific
        .pico_stdlib,
        .pico_stdio,
        .pico_stdio_uart,
        .pico_time,
        .pico_sync,
        .pico_printf,
        .pico_bit_ops,
    });

    pico_sdk.configureStdio(exe, .{ .uart = true });
    exe.root_module.addCMacro("LIB_PICO_PRINTF_PICO", "1");
    exe.root_module.addCMacro("PICO_STDIO_SHORT_CIRCUIT_CLIB_FUNCS", "0");

    pico_sdk.setLinkerScriptWithWrapping(sdk_dep, exe, chip, null, &.{
        .pico_stdio,
        .pico_printf,
    }, .default);

    pico_sdk.addBoot2(sdk_dep, exe, .boot2_w25q080);

    exe.linkLibrary(pico_sdk.getPicolibc(sdk_dep, .{
        .target = target,
        .optimize = optimize,
    }));

    exe.addCSourceFile(.{
        .file = b.path("pico_divider_test.c"),
        .flags = &.{},
    });

    b.installArtifact(exe);
}
