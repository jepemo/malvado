// Malvado 2; SPDX-License-Identifier: Apache-2.0
#![cfg(feature = "assets")]
use malvado::{
    Error,
    asset::{Format, Source},
    atlas::Atlas,
};
use std::{ffi::OsString, os::unix::ffi::OsStringExt, path::Path};
#[test]
fn content_detection_and_existing_native_importers() {
    let root = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../..");
    for (path, expected) in [
        ("tests/fixtures/red.png", Format::Png),
        ("tests/fixtures/atlas.json", Format::Atlas),
        ("examples/aseprite/sheet.json", Format::Aseprite),
        ("tests/fixtures/fpg/bennugd-8.fpg", Format::Fpg),
    ] {
        let path = root.join(path);
        let source = Source::open(&path).unwrap();
        assert_eq!(source.format(), expected);
        assert_eq!(source.path(), path);
        assert_eq!(source.bytes(), std::fs::read(&path).unwrap());
        if expected == Format::Atlas {
            let atlas = Atlas::parse(std::str::from_utf8(source.bytes()).unwrap(), 8, 8).unwrap();
            assert_eq!(atlas.sprite(42).unwrap().pivot, (2, 4));
        }
        if expected == Format::Aseprite {
            assert_eq!(source.dimensions(), Some((8, 8)));
        } else {
            assert_eq!(source.dimensions(), None);
        }
    }
}
#[test]
fn snapshots_paths_unknown_content_and_failures() {
    let directory =
        std::env::temp_dir().join(format!("malvado source test {}", std::process::id()));
    std::fs::create_dir_all(&directory).unwrap();
    let path = directory.join("not-a-png.png");
    for (bytes, format) in [
        (&b""[..], Format::Unknown),
        (&b"{\"kind\":\"graphics-library\"}"[..], Format::Atlas),
        (&b"{\"kind\":\"malvado-fpg-8\"}"[..], Format::FpgManifest),
        (
            &b"{\"kind\":\"graphics-library\",\"meta\":{},\"frames\":[]}"[..],
            Format::Unknown,
        ),
        (
            &b"{\"kind\":\"graphics-library\"}\0hidden"[..],
            Format::Unknown,
        ),
    ] {
        std::fs::write(&path, bytes).unwrap();
        let source = Source::open(path.clone()).unwrap();
        assert_eq!(source.format(), format);
        assert_eq!(
            source.image_path("nested/../image.png").unwrap(),
            directory.join("nested/../image.png")
        );
        assert_eq!(source.image_path(&path).unwrap(), path);
        assert!(matches!(source.image_path(""), Err(Error::InvalidArgument)));
        assert!(matches!(
            source.image_path("bad\0path"),
            Err(Error::InvalidArgument)
        ));
        std::fs::remove_file(&path).unwrap();
        assert_eq!(source.bytes(), bytes);
        assert_eq!(source.path(), path);
    }
    // Referenced names remain byte paths even on filesystems that require UTF-8.
    let name = OsString::from_vec(b"asset-\xff.json".to_vec());
    std::fs::write(&path, b"{}").unwrap();
    let source = Source::open(&path).unwrap();
    assert_eq!(source.image_path(&name).unwrap(), directory.join(&name));
    std::fs::remove_file(&path).unwrap();
    assert!(matches!(Source::open(&path), Err(Error::Asset(_))));
    assert!(matches!(Source::open(""), Err(Error::Asset(_))));
    assert!(matches!(
        Source::open("bad\0path"),
        Err(Error::InvalidArgument)
    ));
    drop(source);
    #[cfg(target_os = "linux")]
    {
        let byte_path = directory.join(name);
        std::fs::write(&byte_path, b"{}").unwrap();
        let source = Source::open(&byte_path).unwrap();
        assert_eq!(source.path(), byte_path);
        std::fs::remove_file(byte_path).unwrap();
    }
    std::fs::remove_dir(directory).unwrap();
}
