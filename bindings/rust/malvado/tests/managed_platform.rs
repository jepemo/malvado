// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Real platform objects exercise the shared adoption API and native cleanup.
use malvado::{audio::Audio, window::Window};
use malvado_sys::{self as sys, graphics as ffi};
use std::{ffi::CString, ptr, rc::Rc};
unsafe extern "C" fn idle(_: *mut sys::MvContext, _: u64, _: *mut std::ffi::c_void) -> i32 {
    0
}
unsafe extern "C" fn owner_released(data: *mut std::ffi::c_void) {
    // SAFETY: test stack counter remains live through native context cleanup.
    unsafe {
        *data.cast::<usize>() += 1;
    }
}
fn main() {
    let window = Rc::new(Window::open(64, 64, "Managed native resources").unwrap());
    let audio = Audio::open().unwrap();
    let root = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../../tests/fixtures");
    let path = |name: &str| CString::new(root.join(name).to_str().unwrap()).unwrap();
    // SAFETY: every successful load transfers exactly one native reference.
    // The window/audio stay live until all native owners and retained uses end.
    unsafe {
        let ctx = sys::mv_context_create(60.0, 4);
        assert!(!ctx.is_null());
        let mut id = 0;
        assert_eq!(
            sys::mv_process_spawn(ctx, 0, 0, Some(idle), None, ptr::null_mut(), &mut id),
            0
        );
        let mut owner = sys::MvProcessRef::default();
        assert_eq!(sys::mv_process_ref_create(ctx, id, &mut owner), 0);
        let image = ffi::mv_image_load(path("red.png").as_ptr());
        let cached = ffi::mv_image_load(path("red.png").as_ptr());
        assert!(!image.is_null() && image == cached);
        assert_eq!(ffi::mv_image_retain(ptr::null_mut()), 0);
        assert_eq!(ffi::mv_image_retain(image), 1);
        ffi::mv_image_destroy(image);
        assert_eq!(ffi::mv_image_width(cached), 8);
        let graphic = ffi::MvImageGraphic {
            image,
            region: ffi::MvImageRegion {
                x: 0,
                y: 0,
                width: 8,
                height: 8,
            },
            pivot_x: 4,
            pivot_y: 4,
        };
        let mut released = 0usize;
        assert_eq!(
            ffi::mv_process_image_set_owned(
                ctx,
                owner,
                graphic,
                (&mut released as *mut usize).cast(),
                Some(owner_released)
            ),
            0
        );
        let mut copied = graphic;
        let mut present = 0;
        assert_eq!(
            ffi::mv_process_image_get(ctx, owner, &mut copied, &mut present),
            0
        );
        assert_eq!(present, 1);
        assert_eq!(copied.image, image);
        let font = ffi::mv_font_load(path("fonts/AnonymousPro-Bold.ttf").as_ptr(), 24);
        let sound = ffi::mv_sound_load(path("silence.wav").as_ptr());
        let clone = ffi::mv_sound_clone(sound);
        let music = ffi::mv_music_load(path("silence.wav").as_ptr(), 0);
        assert!(!font.is_null() && !sound.is_null() && !clone.is_null() && !music.is_null());
        let (mut images, mut fonts, mut sounds, mut clones, mut streams, mut cached_owner) = (
            ptr::null_mut(),
            ptr::null_mut(),
            ptr::null_mut(),
            ptr::null_mut(),
            ptr::null_mut(),
            ptr::null_mut(),
        );
        assert_eq!(
            ffi::mv_resource_adopt_image(ctx, &owner, image, &mut images),
            0
        );
        assert_eq!(
            ffi::mv_resource_adopt_image(ctx, ptr::null(), cached, &mut cached_owner),
            0
        );
        assert_eq!(
            ffi::mv_resource_adopt_font(ctx, &owner, font, &mut fonts),
            0
        );
        assert_eq!(
            ffi::mv_resource_adopt_sound(ctx, &owner, sound, &mut sounds),
            0
        );
        assert_eq!(
            ffi::mv_resource_adopt_sound(ctx, ptr::null(), clone, &mut clones),
            0
        );
        assert_eq!(
            ffi::mv_resource_adopt_music(ctx, &owner, music, &mut streams),
            0
        );
        for resource in [images, fonts, streams] {
            assert_eq!(sys::mv_resource_retain(resource), 0);
        }
        let mut killed = 0;
        assert_eq!(sys::mv_process_kill(ctx, owner, &mut killed), 0);
        assert_eq!(killed, 1);
        assert_eq!(sys::mv_process_tick(ctx), 0);
        assert_eq!(released, 1);
        assert_eq!(ffi::mv_image_width(image), 8);
        assert!(ffi::mv_font_text_width(font, c"Test".as_ptr(), 24) > 0);
        assert_eq!(ffi::mv_sound_set_volume(clone, 0.0), 1);
        assert_eq!(ffi::mv_music_set_volume(music, 0.0), 1);
        assert_eq!(sys::mv_resource_release(images), 0);
        // The cached load still owns its independent native image reference.
        assert_eq!(ffi::mv_image_width(cached), 8);
        sys::mv_context_destroy(ctx);
        assert!(ffi::mv_font_text_width(font, c"After context".as_ptr(), 24) > 0);
        assert_eq!(ffi::mv_music_stop(music), 1);
        assert_eq!(sys::mv_resource_release(fonts), 0);
        assert_eq!(sys::mv_resource_release(streams), 0);
    }
    drop(audio);
    drop(window);
}
