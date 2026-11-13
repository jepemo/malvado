// Malvado 2; SPDX-License-Identifier: Apache-2.0
use std::env;
use std::path::{Path, PathBuf};
use std::process::{Command, Output};

fn run(command: &mut Command) -> Output {
    let output = command
        .output()
        .expect("cannot execute SDK probe tool (CMake/C compiler required)");
    if !output.status.success() {
        panic!(
            "Malvado SDK validation failed:\n{}\n{}",
            String::from_utf8_lossy(&output.stdout),
            String::from_utf8_lossy(&output.stderr)
        );
    }
    output
}

// CMake's imported-target glob cannot use Win32 verbatim path prefixes.
// Keep the canonical PathBuf for ownership checks and normalize only its CLI value.
fn cmake_sdk_path(path: &Path, windows: bool) -> String {
    let value = path.to_string_lossy();
    if !windows {
        return value.into_owned();
    }
    let value = value.strip_prefix(r"\\?\").unwrap_or(&value);
    let value = match value.strip_prefix(r"UNC\") {
        Some(unc) => format!("//{unc}"),
        None => value.to_owned(),
    };
    value.replace('\\', "/")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn cmake_paths() {
        assert_eq!(
            cmake_sdk_path(Path::new(r"\\?\C:\relocated sdk"), true),
            "C:/relocated sdk"
        );
        assert_eq!(
            cmake_sdk_path(Path::new(r"\\?\UNC\server\sdk"), true),
            "//server/sdk"
        );
        assert_eq!(cmake_sdk_path(Path::new(r"C:\sdk"), true), "C:/sdk");
        assert_eq!(
            cmake_sdk_path(Path::new("/tmp/relocated sdk"), false),
            "/tmp/relocated sdk"
        );
    }
}

fn main() {
    for name in [
        "MALVADO_SDK_PREFIX",
        "CC",
        "CMAKE_GENERATOR",
        "CFLAGS",
        "LDFLAGS",
    ] {
        println!("cargo:rerun-if-env-changed={name}");
    }
    println!("cargo:rerun-if-changed=native");
    let graphics = env::var_os("CARGO_FEATURE_GRAPHICS").is_some();
    let assets = env::var_os("CARGO_FEATURE_ASSETS").is_some();
    let host = env::var("HOST").unwrap();
    let target = env::var("TARGET").unwrap();
    assert_eq!(
        host, target,
        "Rust SDK cross compilation is not yet supported"
    );
    assert!(
        target.contains("apple-darwin")
            || target.contains("linux")
            || target == "x86_64-pc-windows-msvc",
        "Rust SDK requires native macOS, Linux or Windows x64 MSVC"
    );
    let windows = target == "x86_64-pc-windows-msvc";
    assert!(
        !windows || !graphics,
        "Rust Windows graphics are not yet supported"
    );
    let prefix = PathBuf::from(
        env::var_os("MALVADO_SDK_PREFIX")
            .expect("set MALVADO_SDK_PREFIX to an installed Malvado C SDK prefix"),
    )
    .canonicalize()
    .expect("MALVADO_SDK_PREFIX does not exist");
    assert!(
        !prefix.to_string_lossy().contains(['\n', '\r', ';', '"']),
        "unsupported SDK prefix characters"
    );
    // Re-run when an installed SDK is replaced without changing its prefix.
    println!("cargo:rerun-if-changed={}", prefix.display());
    let output = PathBuf::from(env::var_os("OUT_DIR").unwrap()).join("native");
    // Cargo reruns this script when SDK/tool settings change. A CMake cache
    // from a failed compiler or a different generator must not survive that
    // change. This directory contains only this script's generated probe files.
    if output.exists() {
        std::fs::remove_dir_all(&output).expect("cannot reset the generated native SDK probe");
    }
    let source = PathBuf::from(env::var_os("CARGO_MANIFEST_DIR").unwrap()).join("native");
    let config = if env::var("DEBUG").unwrap() == "true" {
        "Debug"
    } else {
        "Release"
    };
    let mut configure = Command::new("cmake");
    configure
        .arg("-S")
        .arg(source)
        .arg("-B")
        .arg(&output)
        .arg(format!("-DCMAKE_BUILD_TYPE={config}"))
        .arg(format!(
            "-DMALVADO_RUST_GRAPHICS={}",
            if graphics { "ON" } else { "OFF" }
        ))
        .arg(format!(
            "-DMALVADO_RUST_ASSETS={}",
            if assets { "ON" } else { "OFF" }
        ))
        .arg(format!(
            "-DCMAKE_C_FLAGS={}",
            env::var("CFLAGS").unwrap_or_default()
        ))
        .arg(format!(
            "-DCMAKE_EXE_LINKER_FLAGS={}",
            env::var("LDFLAGS").unwrap_or_default()
        ))
        .arg(format!(
            "-DMALVADO_SDK_PREFIX={}",
            cmake_sdk_path(&prefix, windows)
        ))
        .arg(format!(
            "-DEXPECTED_VERSION={}",
            env::var("CARGO_PKG_VERSION").unwrap()
        ));
    if let Some(cc) = env::var_os("CC") {
        configure.arg(format!("-DCMAKE_C_COMPILER={}", cc.to_string_lossy()));
    }
    run(&mut configure);
    run(Command::new("cmake")
        .arg("--build")
        .arg(&output)
        .arg("--clean-first")
        .arg("--config")
        .arg(config));
    let library = std::fs::read_to_string(output.join("native-library.txt")).unwrap();
    let library = PathBuf::from(library.trim()).canonicalize().unwrap();
    assert!(
        library.starts_with(&prefix),
        "CMake selected a library outside MALVADO_SDK_PREFIX"
    );
    assert_eq!(
        library.file_name().unwrap(),
        if windows {
            "malvado_core.lib"
        } else {
            "libmalvado_core.a"
        },
        "unsupported core archive name"
    );
    // Execute only the ABI/release queries before accepting the library.
    let probe = if output
        .join(if windows { "probe.exe" } else { "probe" })
        .exists()
    {
        output.join(if windows { "probe.exe" } else { "probe" })
    } else {
        output
            .join(config)
            .join(if windows { "probe.exe" } else { "probe" })
    };
    let layout = run(&mut Command::new(probe));
    let values: Vec<usize> = String::from_utf8(layout.stdout)
        .unwrap()
        .split_whitespace()
        .map(|value| value.parse().expect("invalid C ABI probe value"))
        .collect();
    std::fs::write(
        PathBuf::from(env::var_os("OUT_DIR").unwrap()).join("layout.rs"),
        format!("{values:?}"),
    )
    .unwrap();
    println!(
        "cargo:rustc-link-search=native={}",
        library.parent().unwrap().display()
    );
    println!("cargo:rustc-link-lib=static=malvado_core");
    if assets {
        let asset_library = std::fs::read_to_string(output.join("assets-library.txt")).unwrap();
        let asset_library = PathBuf::from(asset_library.trim()).canonicalize().unwrap();
        assert!(
            asset_library.starts_with(&prefix),
            "assets library outside MALVADO_SDK_PREFIX"
        );
        assert_eq!(
            asset_library.file_name().unwrap(),
            if windows {
                "malvado_assets.lib"
            } else {
                "libmalvado_assets.a"
            }
        );
        println!(
            "cargo:rustc-link-search=native={}",
            asset_library.parent().unwrap().display()
        );
        println!("cargo:rustc-link-lib=static=malvado_assets");
        let probe = if output
            .join(if windows {
                "assets-probe.exe"
            } else {
                "assets-probe"
            })
            .exists()
        {
            output.join(if windows {
                "assets-probe.exe"
            } else {
                "assets-probe"
            })
        } else {
            output.join(config).join(if windows {
                "assets-probe.exe"
            } else {
                "assets-probe"
            })
        };
        let layout = run(&mut Command::new(probe));
        let values: Vec<usize> = String::from_utf8(layout.stdout)
            .unwrap()
            .split_whitespace()
            .map(|v| v.parse().expect("invalid assets ABI probe"))
            .collect();
        std::fs::write(
            PathBuf::from(env::var_os("OUT_DIR").unwrap()).join("assets-layout.rs"),
            format!("{values:?}"),
        )
        .unwrap();
    }
    if graphics {
        let checks = std::fs::read_to_string(output.join("graphics-checks-library.txt")).unwrap();
        let checks = PathBuf::from(checks.trim()).canonicalize().unwrap();
        assert!(
            checks.starts_with(output.canonicalize().unwrap()),
            "verification archive outside probe build"
        );
        println!(
            "cargo:rustc-link-search=native={}",
            checks.parent().unwrap().display()
        );
        println!("cargo:rustc-link-lib=static=graphics-checks");
        for (file, archive, link) in [
            (
                "graphics-library.txt",
                "libmalvado_platform_raylib.a",
                "malvado_platform_raylib",
            ),
            ("raylib-library.txt", "libraylib.a", "raylib"),
        ] {
            let library = std::fs::read_to_string(output.join(file)).unwrap();
            let library = PathBuf::from(library.trim()).canonicalize().unwrap();
            assert!(
                library.starts_with(&prefix),
                "graphics dependency outside MALVADO_SDK_PREFIX"
            );
            assert_eq!(library.file_name().unwrap(), archive);
            println!(
                "cargo:rustc-link-search=native={}",
                library.parent().unwrap().display()
            );
            println!("cargo:rustc-link-lib=static={link}");
        }
        // The graphics adapter uses the core graphic constructors. Rescan the
        // core archive after the adapter on linkers that resolve left to right.
        println!("cargo:rustc-link-lib=static=malvado_core");
        if target.contains("apple-darwin") {
            for framework in ["OpenGL", "Cocoa", "IOKit", "CoreFoundation"] {
                println!("cargo:rustc-link-lib=framework={framework}");
            }
        } else {
            for library in ["GL", "dl", "pthread"] {
                println!("cargo:rustc-link-lib={library}");
            }
        }
        let probe = if output.join("graphics-probe").exists() {
            output.join("graphics-probe")
        } else {
            output.join(config).join("graphics-probe")
        };
        let layout = run(&mut Command::new(probe));
        let values: Vec<usize> = String::from_utf8(layout.stdout)
            .unwrap()
            .split_whitespace()
            .map(|v| v.parse().expect("invalid graphics ABI probe"))
            .collect();
        std::fs::write(
            PathBuf::from(env::var_os("OUT_DIR").unwrap()).join("graphics-layout.rs"),
            format!("{values:?}"),
        )
        .unwrap();
    }
    if windows {
        println!("cargo:rustc-link-lib=bcrypt");
    } else {
        println!("cargo:rustc-link-lib=m");
    }
}
