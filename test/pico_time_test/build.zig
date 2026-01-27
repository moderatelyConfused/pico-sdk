const std = @import("std");
const pico_sdk = @import("pico_sdk");

pub fn build(b: *std.Build) void {
    const chip = b.option(pico_sdk.Chip, "chip", "Target chip") orelse .rp2040;
    const cpu_arch = b.option(pico_sdk.CpuArch, "cpu_arch", "CPU architecture") orelse .arm;
    const board = b.option([]const u8, "board", "Target board") orelse switch (chip) {
        .rp2040 => "pico",
        .rp2350 => "pico2",
    };
    const optimize = b.standardOptimizeOption(.{});
    const usb_boot_on_exit = b.option(bool, "usb_boot_on_exit", "Reboot to BOOTSEL when test exits") orelse false;

    const sdk_dep = b.dependency("pico_sdk", .{});
    const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, cpu_arch));

    const exe = b.addExecutable(.{
        .name = "pico_time_test",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    pico_sdk.addTo(sdk_dep, exe, chip, board);
    exe.addIncludePath(sdk_dep.path("test/pico_test/include"));

    // Base components for all chips
    const base_components: []const pico_sdk.Component = &.{
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
        .pico_stdlib,
        .pico_stdio,
        .pico_stdio_uart,
        .pico_time,
        .pico_sync,
        .pico_printf,
    };

    // RP2040-specific components (pico_aon_timer automatically includes hardware_rtc and pico_util)
    const rp2040_extra: []const pico_sdk.Component = &.{.pico_aon_timer};

    pico_sdk.addComponents(sdk_dep, exe, chip, cpu_arch, base_components);
    if (chip == .rp2040) {
        pico_sdk.addComponents(sdk_dep, exe, chip, cpu_arch, rp2040_extra);
        // Define LIB_PICO_AON_TIMER so the test includes pico/aon_timer.h
        exe.root_module.addCMacro("LIB_PICO_AON_TIMER", "1");
    }

    pico_sdk.configureStdio(exe, .{ .uart = true });
    exe.root_module.addCMacro("LIB_PICO_PRINTF_PICO", "1");
    exe.root_module.addCMacro("PICO_STDIO_SHORT_CIRCUIT_CLIB_FUNCS", "0");

    // Test-specific configuration
    exe.root_module.addCMacro("PICO_TIME_DEFAULT_ALARM_POOL_MAX_TIMERS", "250");

    // Reboot to BOOTSEL on exit (for automated testing)
    if (usb_boot_on_exit) {
        exe.root_module.addCMacro("PICO_ENTER_USB_BOOT_ON_EXIT", "1");
    }

    pico_sdk.setLinkerScriptWithWrapping(sdk_dep, exe, chip, null, &.{
        .pico_stdio,
        .pico_printf,
    }, .default);

    if (chip == .rp2040) {
        pico_sdk.addBoot2(sdk_dep, exe, .boot2_w25q080);
    }

    exe.linkLibrary(pico_sdk.getPicolibc(sdk_dep, .{
        .target = target,
        .optimize = optimize,
    }));

    // Disable UBSAN - the test may exercise edge cases that trigger it
    exe.addCSourceFile(.{
        .file = b.path("pico_time_test.c"),
        .flags = &.{"-fno-sanitize=undefined"},
    });

    b.installArtifact(exe);
}
