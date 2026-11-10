// Malvado 2; SPDX-License-Identifier: Apache-2.0
#![cfg(feature = "assets")]
use malvado::{
    Engine, Error,
    atlas::{Atlas, Region},
};
const ATLAS: &str = include_str!("../../../../tests/fixtures/atlas.json");
#[test]
fn copied_input_and_borrowed_sprites() {
    let json = ATLAS.to_string();
    let atlas = Atlas::parse(&json, 8, 8).unwrap();
    drop(json);
    assert_eq!(atlas.image(), "red.png");
    assert_eq!(atlas.len(), 2);
    assert!(!atlas.is_empty());
    let green = atlas.sprite(42).unwrap();
    assert_eq!(green.name, "green");
    assert_eq!(
        green.region,
        Region {
            x: 4,
            y: 0,
            width: 4,
            height: 8
        }
    );
    assert_eq!(green.pivot, (2, 4));
    assert_eq!(green.source_size, (4, 8));
    assert_eq!(green.trim, (0, 0));
    assert_eq!(atlas.sprite_named("green").unwrap(), Some(green));
    assert_eq!(atlas.sprite_named("Green").unwrap(), None);
    assert_eq!(atlas.sprite_named("bad\0name"), Err(Error::InvalidArgument));
    assert!(atlas.sprite(0).is_none());
    assert!(atlas.sprite(999).is_none());
}
#[test]
fn invalid_metadata_and_dimensions() {
    for json in [
        "",
        "{}",
        "not json",
        "\0",
        &ATLAS.replace("\"id\": 7", "\"id\": 42"),
        &ATLAS.replace("\"schema\": 1", "\"schema\": 99"),
    ] {
        assert!(matches!(Atlas::parse(json, 8, 8), Err(Error::Asset(_))));
    }
    for (w, h) in [(0, 8), (8, -1), (7, 8)] {
        assert!(Atlas::parse(ATLAS, w, h).is_err());
    }
}
#[test]
fn resource_retains_atlas_after_context_shutdown() {
    let engine = Engine::new().unwrap();
    let atlas = engine.resource(Atlas::parse(ATLAS, 8, 8).unwrap()).unwrap();
    let retained = atlas.try_clone().unwrap();
    drop(atlas);
    drop(engine);
    assert_eq!(retained.sprite(7).unwrap().name, "red");
}
