const std = @import("std");
const pico_sdk = @import("pico_sdk");

pub fn build(b: *std.Build) void {
    const chip = b.option(pico_sdk.Chip, "chip", "Target chip") orelse .rp2040;
    const cpu_arch: pico_sdk.CpuArch = .arm; // Float test is ARM only for now
    _ = b.option(pico_sdk.CpuArch, "cpu_arch", "CPU architecture (ignored, ARM only)");
    const board = b.option([]const u8, "board", "Target board") orelse switch (chip) {
        .rp2040 => "pico",
        .rp2350 => "pico2",
    };
    const optimize = b.standardOptimizeOption(.{});
    const usb_boot_on_exit = b.option(bool, "usb_boot_on_exit", "Reboot to BOOTSEL when test exits") orelse false;

    const sdk_dep = b.dependency("pico_sdk", .{});
    const target = b.resolveTargetQuery(pico_sdk.getTarget(chip, cpu_arch));

    // Build pico_float_test
    const float_test = buildFloatTest(b, sdk_dep, target, chip, board, optimize, usb_boot_on_exit, .float);
    b.installArtifact(float_test);

    // Build pico_double_test
    const double_test = buildFloatTest(b, sdk_dep, target, chip, board, optimize, usb_boot_on_exit, .double);
    b.installArtifact(double_test);
}

const TestType = enum { float, double };

fn buildFloatTest(
    b: *std.Build,
    sdk_dep: *std.Build.Dependency,
    target: std.Build.ResolvedTarget,
    chip: pico_sdk.Chip,
    board: []const u8,
    optimize: std.builtin.OptimizeMode,
    usb_boot_on_exit: bool,
    test_type: TestType,
) *std.Build.Step.Compile {
    const name = switch (test_type) {
        .float => "pico_float_test",
        .double => "pico_double_test",
    };

    const exe = b.addExecutable(.{
        .name = name,
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    pico_sdk.addTo(sdk_dep, exe, chip, board);
    exe.addIncludePath(sdk_dep.path("test/pico_test/include"));
    exe.addIncludePath(sdk_dep.path("test/pico_float_test/llvm"));

    pico_sdk.addComponents(sdk_dep, exe, chip, .arm, &.{
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
        .pico_float,
        .pico_double,
        .pico_stdlib,
        .pico_stdio,
        .pico_stdio_uart,
        .pico_time,
        .pico_sync,
        .pico_printf,
        .pico_bit_ops,
    });

    pico_sdk.configureStdio(exe, .{ .uart = true });
    exe.root_module.addCMacro("PICO_STDIO_SHORT_CIRCUIT_CLIB_FUNCS", "0");

    // Test-specific configuration
    exe.root_module.addCMacro("LIB_PICO_PRINTF_PICO", "1");
    switch (test_type) {
        .float => {},
        .double => {
            // pico_double_test needs NaN propagation for correct test behavior
            exe.root_module.addCMacro("PICO_FLOAT_PROPAGATE_NANS", "1");
            exe.root_module.addCMacro("PICO_DOUBLE_PROPAGATE_NANS", "1");
        },
    }

    // Reboot to BOOTSEL on exit (for automated testing)
    if (usb_boot_on_exit) {
        exe.root_module.addCMacro("PICO_ENTER_USB_BOOT_ON_EXIT", "1");
    }

    pico_sdk.setLinkerScriptWithWrapping(sdk_dep, exe, chip, null, &.{
        .pico_stdio,
        .pico_printf,
        .pico_float,
        .pico_double,
    }, .default);

    if (chip == .rp2040) {
        pico_sdk.addBoot2(sdk_dep, exe, .boot2_w25q080);
    }

    exe.linkLibrary(pico_sdk.getPicolibc(sdk_dep, .{
        .target = target,
        .optimize = optimize,
    }));

    // Add test source file - disable UBSAN as tests intentionally exercise
    // edge cases like negative float to unsigned int conversion (undefined behavior)
    const source_file = switch (test_type) {
        .float => "pico_float_test.c",
        .double => "pico_double_test.c",
    };
    exe.addCSourceFile(.{
        .file = b.path(source_file),
        .flags = &.{"-fno-sanitize=undefined"},
    });
    exe.addAssemblyFile(sdk_dep.path("test/pico_float_test/llvm/call_apsr.S"));

    // Add VFP-based __aeabi_cfcmple/cfcmpeq/cfrcmple implementations for RP2350
    // These are only needed by this test which explicitly calls them - normal code
    // uses VFP instructions directly (compiler emits vcmp.f32 + vmrs)
    if (chip == .rp2350) {
        exe.addAssemblyFile(b.path("float_cmp_vfp.S"));
    }

    return exe;
}
