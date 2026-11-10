// Malvado 2; SPDX-License-Identifier: Apache-2.0
#![cfg(feature = "assets")]
use malvado::{
    Engine, Error,
    fpg::{Fpg, Point},
};
const DATA: &[u8] = include_bytes!("../../../../tests/fixtures/fpg/bennugd-8.fpg");
#[test]
fn metadata_indexes_and_borrowed_data() {
    let data = DATA.to_vec();
    let fpg = Fpg::parse(&data).unwrap();
    drop(data);
    assert_eq!(fpg.len(), 2);
    assert!(!fpg.is_empty());
    assert_eq!(fpg.palette()[3], 63);
    assert_eq!(fpg.palette()[7], 63);
    assert_eq!(fpg.gamma(), &[0; 576]);
    assert_eq!(fpg.at(0).unwrap().info.id, 7);
    assert_eq!(fpg.at(1).unwrap().info.id, 42);
    assert!(fpg.at(2).is_none());
    assert!(fpg.at(usize::MAX).is_none());
    assert!(fpg.sprite(999).is_none());
    assert!(fpg.sprite(-1).is_none());
    let sprite = fpg.sprite(7).unwrap();
    assert_eq!(
        (
            sprite.info.width,
            sprite.info.height,
            sprite.info.point_count
        ),
        (2, 2, 2)
    );
    assert_eq!(&sprite.info.name[..4], b"red\0");
    assert_eq!(sprite.info.filename, [0; 12]);
    assert_eq!(sprite.pixels, [0, 1, 1, 0]);
    assert_eq!(
        fpg.point(7, 0),
        Some(Point {
            x: 1,
            y: 0,
            defined: true
        })
    );
    assert_eq!(
        fpg.point(7, 1),
        Some(Point {
            x: -1,
            y: -1,
            defined: false
        })
    );
    assert_eq!(fpg.point(7, 2), None);
    assert_eq!(fpg.point(999, 0), None);
}
#[test]
fn malformed_variants_and_complete_record_boundaries() {
    assert!(matches!(Fpg::parse(b"bad"), Err(Error::Asset(_))));
    // The boundary after a complete first record is itself a valid library.
    let first_end = 8 + 768 + 576 + 64 + 8 + 4;
    for end in 0..DATA.len() {
        let parsed = Fpg::parse(&DATA[..end]);
        assert_eq!(parsed.is_ok(), end == first_end, "boundary {end}");
    }
    for (offset, value) in [(0, b'x'), (8, 64), (8 + 768 + 576 + 4, 1)] {
        let mut invalid = DATA.to_vec();
        invalid[offset] = value;
        assert!(Fpg::parse(&invalid).is_err());
    }
    let mut duplicate = DATA.to_vec();
    duplicate[first_end..first_end + 4].copy_from_slice(&7_i32.to_le_bytes());
    assert!(Fpg::parse(&duplicate).is_err());
}
#[test]
fn preserve_raw_names_gamma_and_native_resource_retention() {
    let mut data = DATA.to_vec();
    let name = 8 + 768 + 576 + 8;
    data[name..name + 32].fill(0xff);
    data[8 + 768] = 23;
    let engine = Engine::new().unwrap();
    let fpg = engine.resource(Fpg::parse(&data).unwrap()).unwrap();
    let retained = fpg.try_clone().unwrap();
    drop(fpg);
    drop(engine);
    assert_eq!(retained.sprite(7).unwrap().info.name, [0xff; 32]);
    assert_eq!(retained.gamma()[0], 23);
    assert_eq!(retained.sprite(42).unwrap().pixels, [2, 1]);
}

#[test]
fn rgba_and_png_conversion_policy_and_missing_ids() {
    let fpg = Fpg::parse(DATA).unwrap();
    assert_eq!(
        fpg.rgba(7, true).unwrap(),
        [0, 0, 0, 0, 252, 0, 0, 255, 252, 0, 0, 255, 0, 0, 0, 0]
    );
    assert_eq!(&fpg.rgba(7, false).unwrap()[..4], [0, 0, 0, 255]);
    assert_eq!(
        fpg.rgba(42, false).unwrap(),
        [0, 252, 0, 255, 252, 0, 0, 255]
    );
    assert_eq!(fpg.rgba(999, true), Err(Error::NotFound));
    assert_eq!(fpg.png(-1, false), Err(Error::NotFound));
    for transparent in [false, true] {
        let png = fpg.png(7, transparent).unwrap();
        assert_eq!(&png[..8], b"\x89PNG\r\n\x1a\n");
        assert_eq!(u32::from_be_bytes(png[16..20].try_into().unwrap()), 2);
        assert_eq!(u32::from_be_bytes(png[20..24].try_into().unwrap()), 2);
        assert_eq!((png[24], png[25]), (8, 3)); // indexed 8-bit output
        assert_eq!(png.windows(4).any(|w| w == b"tRNS"), transparent);
    }
}
#[test]
fn metadata_round_trip_preserves_original_bytes_and_points() {
    use malvado::fpg::Manifest;
    let mut data = DATA.to_vec();
    data[8 + 768] = 23;
    let name = 8 + 768 + 576 + 8;
    data[name..name + 32].fill(0xff);
    let fpg = Fpg::parse(&data).unwrap();
    assert_eq!(
        fpg.rgba(42, false).unwrap(),
        [0, 252, 0, 255, 252, 0, 0, 255]
    );
    for transparent in [false, true] {
        let json = fpg.json(transparent).unwrap();
        assert!(!json.as_bytes().contains(&0));
        let manifest = Manifest::parse(&json).unwrap();
        assert_eq!(manifest.len(), 2);
        assert!(!manifest.is_empty());
        assert_eq!(manifest.transparent_zero(), transparent);
        assert_eq!(manifest.palette(), fpg.palette());
        assert_eq!(manifest.gamma(), fpg.gamma());
        for id in [7, 42] {
            assert_eq!(manifest.sprite(id), Some(fpg.sprite(id).unwrap().info));
            for index in 0..3 {
                assert_eq!(manifest.point(id, index), fpg.point(id, index));
            }
        }
        assert_eq!(manifest.at(0).unwrap().id, 7);
        assert_eq!(manifest.at(1).unwrap().id, 42);
        assert_eq!(manifest.sprite(-1), None);
        assert_eq!(manifest.at(usize::MAX), None);
        assert_eq!(manifest.point(999, 0), None);
    }
    let json = fpg.json(true).unwrap();
    drop(fpg);
    let engine = Engine::new().unwrap();
    let manifest = engine.resource(Manifest::parse(&json).unwrap()).unwrap();
    drop(json);
    drop(engine);
    assert_eq!(manifest.sprite(7).unwrap().name, [0xff; 32]);
    assert_eq!(manifest.gamma()[0], 23);
    assert_eq!(
        manifest.point(7, 1),
        Some(Point {
            x: -1,
            y: -1,
            defined: false
        })
    );
}
#[test]
fn manifest_rejects_bad_structure_and_embedded_zero() {
    use malvado::fpg::Manifest;
    for json in ["", "{}", "null", "\0", "not json"] {
        assert!(matches!(Manifest::parse(json), Err(Error::Asset(_))));
    }
    let json = Fpg::parse(DATA).unwrap().json(true).unwrap();
    for invalid in [
        json.replace("\"schema\":1", "\"schema\":99"),
        json.replace("\"id\":7", "\"id\":42"),
    ] {
        assert!(Manifest::parse(&invalid).is_err());
    }
}

#[test]
fn independent_thread_owners_can_import_json_assets() {
    let workers: Vec<_> = (0..8)
        .map(|_| {
            std::thread::spawn(|| {
                for _ in 0..20 {
                    let fpg = Fpg::parse(DATA).unwrap();
                    let json = fpg.json(true).unwrap();
                    let manifest = malvado::fpg::Manifest::parse(&json).unwrap();
                    assert_eq!(manifest.sprite(7).unwrap().width, 2);
                    assert!(malvado::fpg::Manifest::parse("bad").is_err());
                    let atlas = malvado::atlas::Atlas::parse(
                        include_str!("../../../../tests/fixtures/atlas.json"),
                        8,
                        8,
                    )
                    .unwrap();
                    assert_eq!(atlas.sprite(42).unwrap().name, "green");
                }
            })
        })
        .collect();
    for worker in workers {
        worker.join().unwrap();
    }
}
