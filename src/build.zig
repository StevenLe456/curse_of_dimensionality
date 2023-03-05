const std = @import("std");

pub fn build(b: *std.build.Builder) !void {
    const flags = [_][]const u8{
        "-Wall",
        "-Wextra",
        "-Werror=return-type",
    };
    const cflags = flags ++ [_][]const u8{
        "-std=c99",
    };
    const cxxflags = cflags ++ [_][]const u8{
        "-std=c++11", "-fno-exceptions",
    };

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    var sources = std.ArrayList([]const u8).init(b.allocator);
    {
        var dir = try std.fs.cwd().openIterableDir(".", .{});
        var walker = try dir.walk(b.allocator);
        defer walker.deinit();
        const allowed_exts = [_][]const u8{ ".c", ".cpp", ".cxx", ".c++", ".cc"};
        while (try walker.next()) |entry| {
            const ext = std.fs.path.extension(entry.basename);
            const include_file = for (allowed_exts) |e| {
                if (std.mem.eql(u8, ext, e))
                    break true;
            } else false;
            if (include_file) {
                // we have to clone the path as walker.next() or walker.deinit() will override/kill it
                try sources.append(b.dupe(entry.path));
            }
        }
    }
    var lib = b.addSharedLibrary("game-linux", null, b.version(0, 0, 1));
    lib.setTarget(target);
    if (lib.target.isWindows()) {
        lib = b.addSharedLibrary("game-windows", null, b.version(0, 0, 1));
    } else {
        if (lib.target.isDarwin()) {
            lib = b.addSharedLibrary("game-macos", null, b.version(0, 0, 1));
        }
    }
    lib.setTarget(target);
    lib.setBuildMode(mode);
    lib.addCSourceFiles(sources.items, &cxxflags);
    lib.linkLibC();
    lib.linkLibCpp();
    lib.addIncludePath("../godot-cpp/include/");
    lib.addIncludePath("../godot-cpp/include/core/");
    lib.addIncludePath("../godot-cpp/godot-headers/");
    lib.addIncludePath("../godot-cpp/include/gen/");
    lib.addLibraryPath("../godot-cpp/zig-out/lib/");
    if (lib.target.isWindows()) {
        lib.linkSystemLibraryName("godot-windows");
    } else {
        if (lib.target.isDarwin()) {
            lib.linkSystemLibraryName("godot-macos");
        } else {
            lib.linkSystemLibraryName("godot-linux");
        }
    }
    lib.install();
}
