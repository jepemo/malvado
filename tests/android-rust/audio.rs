// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error, Game,
    audio::{Audio, Sound},
};
use std::{cell::Cell, rc::Rc};
pub struct Case {
    device: Rc<Audio>,
    sound: Sound,
    finite: Sound,
    streams: super::music::Streams,
    drops: Rc<Cell<usize>>,
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
            drops: Rc::new(Cell::new(0)),
            streams,
            device,
            sound,
            finite,
        }
    }
    pub fn attach(&self, game: &mut Game) {
        let sound = self.sound.try_clone().unwrap();
        sound.play().unwrap();
        sound.pause().unwrap();
        let guard = super::Guard(self.drops.clone());
        game.spawn(move |process| async move {
            let _guard = guard;
            let sound = process.resource(sound)?;
            loop {
                assert!(sound.paused() && !sound.playing());
                process.frame().await;
            }
        })
        .unwrap();
        assert_eq!(self.drops.get(), 0);
    }
    pub fn tick(&self) {
        self.device.update().unwrap();
        self.streams.tick();
        assert!(self.sound.paused() && !self.sound.playing());
    }
    pub fn finish(self) {
        assert_eq!(self.drops.get(), 1);
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
            drops: _,
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
