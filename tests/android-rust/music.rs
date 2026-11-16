// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error,
    audio::{Audio, Music},
};
use std::rc::Rc;
pub struct Streams {
    wav: Music,
    ogg: Music,
    finite: Music,
}
impl Streams {
    pub fn begin(device: Rc<Audio>) -> Self {
        assert!(Music::load(device.clone(), "missing.wav", false).is_err());
        assert!(Music::load(device.clone(), "empty.wav", false).is_err());
        let wav = Music::load(device.clone(), "silence.wav", true).unwrap();
        let ogg = Music::load(device.clone(), "silence.ogg", true).unwrap();
        for stream in [&wav, &ogg] {
            for volume in [f32::NAN, f32::NEG_INFINITY, -0.1, 1.1] {
                assert_eq!(stream.set_volume(volume), Err(Error::InvalidArgument));
            }
            stream.set_volume(0.0).unwrap();
            stream.set_volume(1.0).unwrap();
            stream.play().unwrap();
            stream.play().unwrap();
            stream.pause().unwrap();
            assert!(stream.paused() && !stream.playing());
        }
        let finite = Music::load(device, "silence.ogg", false).unwrap();
        finite.play().unwrap();
        Self { wav, ogg, finite }
    }
    pub fn tick(&self) {
        for stream in [&self.wav, &self.ogg] {
            assert!(stream.paused() && !stream.playing());
        }
    }
    pub fn finish(self) {
        for stream in [&self.wav, &self.ogg] {
            stream.resume().unwrap();
            assert!(stream.playing());
            stream.stop().unwrap();
            assert!(!stream.playing() && !stream.paused());
        }
        assert!(!self.finite.playing(), "finite OGG did not complete");
        // SAFETY: fixture logging retains no pointer.
        unsafe {
            super::acceptance_log(c"Rust music API passed".as_ptr());
        }
    }
}
