// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error,
    audio::{Audio, Sound},
};
use std::rc::Rc;
pub struct Case {
    device: Rc<Audio>,
    sound: Sound,
    finite: Sound,
    streams: super::music::Streams,
}
impl Case {
    pub fn begin() -> Self {
        assert!(
            std::thread::spawn(|| matches!(Audio::open(), Err(Error::Platform(_))))
                .join()
                .unwrap()
        );
        let device = Rc::new(Audio::open().unwrap());
        assert!(matches!(Audio::open(), Err(Error::Busy)));
        assert!(Sound::load(device.clone(), "missing.wav").is_err());
        assert!(Sound::load(device.clone(), "empty.wav").is_err());
        let original = Sound::load(device.clone(), "silence.wav").unwrap();
        for volume in [f32::NAN, f32::INFINITY, -0.1, 1.1] {
            assert_eq!(original.set_volume(volume), Err(Error::InvalidArgument));
        }
        original.set_volume(0.0).unwrap();
        original.set_volume(1.0).unwrap();
        let sound = original.try_clone().unwrap();
        drop(original);
        sound.play().unwrap();
        sound.pause().unwrap();
        assert!(sound.paused() && !sound.playing());
        let finite = Sound::load(device.clone(), "silence.wav").unwrap();
        finite.play().unwrap();
        let streams = super::music::Streams::begin(device.clone());
        Self {
            streams,
            device,
            sound,
            finite,
        }
    }
    pub fn tick(&self) {
        self.device.update().unwrap();
        self.streams.tick();
        assert!(self.sound.paused() && !self.sound.playing());
    }
    pub fn finish(self) {
        self.sound.resume().unwrap();
        assert!(self.sound.playing());
        self.sound.stop().unwrap();
        assert!(!self.sound.playing() && !self.sound.paused());
        assert!(!self.finite.playing(), "finite WAV did not complete");
        let weak = Rc::downgrade(&self.device);
        let Self {
            device,
            sound,
            finite,
            streams,
        } = self;
        streams.finish();
        drop(device);
        assert!(weak.upgrade().is_some());
        drop(sound);
        drop(finite);
        assert!(weak.upgrade().is_none());
        // SAFETY: fixture logging retains no pointer.
        unsafe {
            super::acceptance_log(c"audio API passed".as_ptr());
        }
    }
}
