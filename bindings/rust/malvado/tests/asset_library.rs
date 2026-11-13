// Malvado 2; SPDX-License-Identifier: Apache-2.0
#![cfg(feature = "assets")]
use malvado::{
    Error, Game,
    asset::{Format, Library, Metadata, Source},
};
use std::path::Path;

#[test]
fn dispatch_real_metadata_and_transfer_owned_values() {
    let root = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../..");
    for (path, format) in [
        ("tests/fixtures/atlas.json", Format::Atlas),
        ("examples/aseprite/sheet.json", Format::Aseprite),
        ("tests/fixtures/fpg/bennugd-8.fpg", Format::Fpg),
    ] {
        let path = root.join(path);
        for requested in [None, Some(format), Some(Format::Unknown)] {
            let library = Library::open(&path, requested).unwrap();
            assert_eq!(library.format(), format);
            assert_eq!(library.source().path(), path);
            match library.metadata() {
                Metadata::Atlas(atlas) => {
                    if format == Format::Atlas {
                        assert_eq!(atlas.sprite(42).unwrap().pivot, (2, 4));
                    } else {
                        assert_eq!(
                            atlas
                                .animation_named("bounce")
                                .unwrap()
                                .unwrap()
                                .sample(100, true),
                            2
                        );
                    }
                }
                Metadata::Fpg(fpg) => {
                    assert_eq!(fpg.sprite(7).unwrap().pixels, &[0, 1, 1, 0]);
                    assert_eq!(fpg.point(7, 0).unwrap().x, 1);
                }
                _ => panic!("unexpected metadata family"),
            }
            let game = Game::new().unwrap();
            let retained = game.resource(library.into_metadata()).unwrap();
            drop(game);
            match &*retained {
                Metadata::Atlas(atlas) => assert!(!atlas.is_empty()),
                Metadata::Fpg(fpg) => assert_eq!(fpg.len(), 2),
                _ => unreachable!(),
            }
        }
    }
}
#[test]
fn manifest_source_removal_invalid_formats_and_cleanup() {
    let root = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../..");
    let path =
        std::env::temp_dir().join(format!("malvado-library-test-{}.json", std::process::id()));
    let fpg = Library::open(root.join("tests/fixtures/fpg/bennugd-8.fpg"), None)
        .unwrap()
        .into_metadata();
    let Metadata::Fpg(fpg) = fpg else {
        unreachable!()
    };
    let json = fpg.json(true).unwrap();
    std::fs::write(&path, json).unwrap();
    let source = Source::open(&path).unwrap();
    let library = Library::from_source(source, Some(Format::FpgManifest)).unwrap();
    std::fs::remove_file(&path).unwrap();
    assert_eq!(library.format(), Format::FpgManifest);
    assert_eq!(
        library.source().image_path("7.png").unwrap(),
        path.parent().unwrap().join("7.png")
    );
    let Metadata::Manifest(manifest) = library.into_metadata() else {
        unreachable!()
    };
    assert_eq!(manifest.len(), 2);
    assert!(manifest.transparent_zero());
    assert_eq!(manifest.palette(), fpg.palette());
    assert_eq!(manifest.sprite(7).unwrap().width, 2);
    for bytes in [
        "{}",
        "{\"kind\":\"graphics-library\"}",
        "fpg\u{1a}",
        "invalid",
    ] {
        std::fs::write(&path, bytes).unwrap();
        for format in [
            None,
            Some(Format::Atlas),
            Some(Format::Aseprite),
            Some(Format::Fpg),
            Some(Format::FpgManifest),
            Some(Format::Png),
        ] {
            assert!(matches!(Library::open(&path, format), Err(Error::Asset(_))));
        }
        std::fs::remove_file(&path).unwrap();
    }
    assert!(matches!(
        Library::open(root.join("tests/fixtures/red.png"), None),
        Err(Error::Asset(_))
    ));
    assert!(matches!(
        Library::open(
            root.join("tests/fixtures/atlas.json"),
            Some(Format::Aseprite)
        ),
        Err(Error::Asset(_))
    ));
    assert!(matches!(Library::open(&path, None), Err(Error::Asset(_))));
    assert!(matches!(
        Library::open("bad\0path", None),
        Err(Error::InvalidArgument)
    ));
}
