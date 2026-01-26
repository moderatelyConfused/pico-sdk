const std = @import("std");
const pico_sdk = @import("pico_sdk");

pub fn build(b: *std.Build) void {
    // This test is RP2350 only (SHA256 hardware)
    const chip: pico_sdk.Chip = .rp2350;
    _ = b.option(pico_sdk.Chip, "chip", "Target chip (ignored, RP2350 only)");
    const cpu_arch = b.option(pico_sdk.CpuArch, "cpu_arch", "CPU architecture") orelse .arm;
    const board = b.option([]const u8, "board", "Target board") orelse "pico2";
    const optimize = b.standardOptimizeOption(.{});
    const usb_boot_on_exit = b.option(bool, "usb_boot_on_exit", "Reboot to BOOTSEL when test exits") orelse false;

    const sdk_dep = b.dependency("pico_sdk", .{});
    const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, cpu_arch));

    const exe = b.addExecutable(.{
        .name = "pico_sha256_test",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    pico_sdk.addTo(sdk_dep, exe, chip, board);
    exe.addIncludePath(sdk_dep.path("test/pico_test/include"));
    exe.addIncludePath(sdk_dep.path("test/pico_sha256_test"));

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
        .pico_sha256, // test-specific
        .hardware_sha256, // test-specific
        .hardware_dma, // test-specific
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

    // Reboot to BOOTSEL on exit (for automated testing)
    // Note: pico_runtime_init already includes bootrom.c which provides reset_usb_boot()
    if (usb_boot_on_exit) {
        exe.root_module.addCMacro("PICO_ENTER_USB_BOOT_ON_EXIT", "1");
    }

    pico_sdk.setLinkerScriptWithWrapping(sdk_dep, exe, chip, null, &.{
        .pico_stdio,
        .pico_printf,
    }, .default);

    // RP2350 doesn't need boot2

    exe.linkLibrary(pico_sdk.getPicolibc(sdk_dep, .{
        .target = target,
        .optimize = optimize,
    }));

    exe.addCSourceFile(.{
        .file = b.path("pico_sha256_test.c"),
        .flags = &.{},
    });

    b.installArtifact(exe);
}
