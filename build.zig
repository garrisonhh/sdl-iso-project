const std = @import("std");
const fs = std.fs;

fn addCSourcesInner(
    b: *std.Build,
    compile: *std.Build.Step.Compile,
    /// path to project
    project_dirpath: []const u8,
    /// path to folder
    project_subpath: []const u8,
) void {
    const dir = fs.cwd().openDir(project_subpath, .{
        .iterate = true,
    }) catch @panic("bad path");

    var iter = dir.iterate();
    while (iter.next() catch @panic("")) |entry| {
        const filepath = fs.path.join(b.allocator, &.{ project_subpath, entry.name }) catch @panic("OOM");
        const src_path = fs.path.relative(b.allocator, project_dirpath, filepath) catch @panic("OOM");

        if (entry.kind == .directory) {
            addCSourcesInner(b, compile, project_dirpath, filepath);
            continue;
        } else if (entry.kind != .file or !std.mem.endsWith(u8, entry.name, ".c")) {
            continue;
        }

        compile.addCSourceFile(.{
            .language = .c,
            .file = b.path(src_path),
            .flags = &.{"--std=c11"},
        });
    }
}

fn addCSources(
    b: *std.Build,
    compile: *std.Build.Step.Compile,
    /// relative path of directory within project
    subpath: []const u8,
) void {
    const project_dirpath = fs.path.dirname(@src().file) orelse "";
    const project_subpath = fs.path.join(b.allocator, &.{
        project_dirpath,
        subpath,
    }) catch @panic("OOM");

    addCSourcesInner(b, compile, project_dirpath, project_subpath);
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "iso",
        .optimize = optimize,
        .target = target,
        .link_libc = true,
    });

    exe.linkSystemLibrary("SDL2");
    exe.linkSystemLibrary("SDL2_image");
    exe.linkSystemLibrary("json-c");

    addCSources(b, exe, "lib/ghh");
    exe.addIncludePath(b.path("lib"));

    addCSources(b, exe, "src");

    const install_exe = b.addInstallArtifact(exe, .{});
    b.getInstallStep().dependOn(&install_exe.step);

    const run_step = b.step("run", "Run the game");
    const run_exe = b.addRunArtifact(exe);
    run_step.dependOn(&run_exe.step);
}
