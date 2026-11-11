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

const ANIMATED: &str = include_str!("../../../../tests/fixtures/animated-atlas.json");
const PINGPONG: &str = include_str!("../../../../tests/fixtures/aseprite/pingpong.json");
const PIVOT: &str = include_str!("../../../../tests/fixtures/aseprite/pivot.json");
#[test]
fn schema_two_sequences_and_independent_native_conversion() {
    let atlas = Atlas::parse(ANIMATED, 8, 8).unwrap();
    let clip = atlas.animation_named("flash").unwrap().unwrap();
    assert_eq!(clip.len(), 2);
    assert!(!clip.is_empty());
    assert_eq!(
        clip.frame(1),
        Some(malvado::atlas::ClipFrame {
            sprite_id: 42,
            duration_ms: 200
        })
    );
    assert!(clip.frame(2).is_none());
    assert!(clip.frame(usize::MAX).is_none());
    for (time, id) in [(0, 7), (99, 7), (100, 42), (299, 42), (300, 7)] {
        assert_eq!(clip.sample(time, true), id);
    }
    assert_eq!(clip.sample(u64::MAX, false), 42);
    assert_eq!(clip.color(), None);
    assert_eq!(clip.repeats(), None);
    assert!(atlas.animation_named("unknown").unwrap().is_none());
    assert!(matches!(
        atlas.animation_named("bad\0name"),
        Err(Error::InvalidArgument)
    ));
    let animation = clip.to_animation(true).unwrap();
    drop(atlas);
    let mut playback = animation.playback();
    for _ in 0..6 {
        playback.advance(60).unwrap();
    }
    assert_eq!(playback.sample().graph_id, 42);
    assert_eq!(animation.sample(300_000).graph_id, 7);
}
#[test]
fn aseprite_directions_colors_and_repeat_metadata() {
    let atlas = Atlas::parse_aseprite(PINGPONG, 8, 8).unwrap();
    for (name, ids) in [
        ("bounce", vec![1, 2, 3, 2]),
        ("backbounce", vec![3, 2, 1, 2]),
        ("single", vec![2]),
        ("pair", vec![2, 1]),
    ] {
        let clip = atlas.animation_named(name).unwrap().unwrap();
        assert_eq!(
            (0..clip.len())
                .map(|i| clip.frame(i).unwrap().sprite_id)
                .collect::<Vec<_>>(),
            ids
        );
    }
    let clip = atlas.animation_named("bounce").unwrap().unwrap();
    assert_eq!(clip.color(), Some([0x12, 0xab, 0xcd, 0x80]));
    assert_eq!(clip.repeats(), Some(3));
    assert_eq!(clip.sample(800, true), 1);
    assert_eq!(clip.sample(u64::MAX, false), 2);
    assert_eq!(
        atlas.animation_named("single").unwrap().unwrap().repeats(),
        Some(0)
    );
    assert_eq!(
        atlas
            .animation_named("backbounce")
            .unwrap()
            .unwrap()
            .repeats(),
        None
    );
}
#[test]
fn aseprite_trim_pivot_and_rejected_metadata() {
    let atlas = Atlas::parse_aseprite(PIVOT, 8, 8).unwrap();
    let sprite = atlas.sprite(1).unwrap();
    assert_eq!(sprite.source_size, (8, 8));
    assert_eq!(sprite.trim, (2, 3));
    assert_eq!(sprite.pivot, (2, 3));
    assert_eq!(atlas.pivot_color(), Some([0x12, 0x34, 0x56, 0x78]));
    for json in [
        PIVOT.replace("\"rotated\":false", "\"rotated\":true"),
        PIVOT.replace("\"duration\":120", "\"duration\":0"),
        PIVOT.replace("\"w\":8,\"h\":8", "\"w\":7,\"h\":8"),
    ] {
        assert!(Atlas::parse_aseprite(&json, 8, 8).is_err());
    }
    assert!(Atlas::parse_aseprite("{}", 8, 8).is_err());
}

#[test]
fn native_metadata_adoption_and_retained_lookup() {
    use malvado_sys::{self as sys, assets as ffi};
    use std::ptr;
    // SAFETY: copied JSON, unique caller-owned atlas and live owner-thread
    // context. The independent resource reference keeps metadata after shutdown.
    unsafe {
        let atlas = ffi::mv_atlas_parse(ATLAS.as_ptr().cast(), ATLAS.len(), 8, 8, ptr::null_mut());
        assert!(!atlas.is_null());
        let ctx = sys::mv_context_create(60.0, 4);
        assert!(!ctx.is_null());
        let mut resource = ptr::null_mut();
        assert_eq!(
            ffi::mv_resource_adopt_atlas(ptr::null_mut(), ptr::null(), atlas, &mut resource),
            sys::MV_INVALID_ARGUMENT
        );
        assert!(resource.is_null());
        assert_eq!(
            ffi::mv_resource_adopt_atlas(ctx, ptr::null(), atlas, &mut resource),
            sys::MV_OK
        );
        assert_eq!(sys::mv_resource_retain(resource), sys::MV_OK);
        sys::mv_context_destroy(ctx);
        assert_eq!(sys::mv_resource_data(resource), atlas.cast());
        assert_eq!(ffi::mv_atlas_count(atlas), 2);
        let mut sprite = std::mem::MaybeUninit::uninit();
        assert_eq!(ffi::mv_atlas_find(atlas, 42, sprite.as_mut_ptr()), 1);
        assert_eq!(sprite.assume_init().pivot_x, 2);
        assert_eq!(sys::mv_resource_release(resource), sys::MV_OK);
    }
}
