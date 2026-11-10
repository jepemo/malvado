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
