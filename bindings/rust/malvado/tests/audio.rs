// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error, Game,
    audio::{Audio, Sound},
};
use std::{
    cell::RefCell,
    path::Path,
    rc::Rc,
    time::{Duration, Instant},
};
fn main() {
    assert!(
        std::thread::spawn(|| matches!(Audio::open(), Err(Error::Platform(_))))
            .join()
            .unwrap()
    );
    let path = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../../tests/fixtures/silence.wav");
    let audio = Rc::new(Audio::open().unwrap());
    assert!(matches!(Audio::open(), Err(Error::Busy)));
    assert!(Sound::load(audio.clone(), path.with_file_name("missing.wav")).is_err());
    assert!(Sound::load(audio.clone(), path.with_file_name("silence.mp3")).is_err());
    assert!(matches!(
        Sound::load(audio.clone(), "bad\0path"),
        Err(Error::InvalidArgument)
    ));
    let sound = Sound::load(audio.clone(), &path).unwrap();
    for volume in [f32::NAN, f32::INFINITY, -0.1, 1.1] {
        assert_eq!(sound.set_volume(volume), Err(Error::InvalidArgument));
    }
    sound.set_volume(0.0).unwrap();
    sound.set_volume(1.0).unwrap();
    sound.play().unwrap();
    sound.pause().unwrap();
    assert!(sound.paused());
    assert!(!sound.playing());
    audio.update().unwrap(); // Paused uses must survive maintenance.
    sound.resume().unwrap();
    sound.stop().unwrap();
    assert!(!sound.playing());
    sound.play().unwrap();
    sound.play().unwrap(); // Restart does not add a second retention.
    let copy = sound.try_clone().unwrap();
    copy.play().unwrap();
    sound.stop().unwrap();
    drop(sound);
    copy.pause().unwrap();
    assert!(!copy.playing());
    copy.resume().unwrap();
    let second = copy.try_clone().unwrap();
    drop(copy);
    second.play().unwrap();
    second.stop().unwrap();
    drop(second);
    // A process can release its last resource handle during playback.
    let mut game = Game::new().unwrap();
    let slot = Rc::new(RefCell::new(None));
    let out = slot.clone();
    let sound = Sound::load(audio.clone(), &path).unwrap();
    let process = game
        .spawn(Default::default(), move |p| async move {
            let resource = p.resource(sound)?;
            resource.play()?;
            resource.pause()?;
            *out.borrow_mut() = Some(resource);
            p.frame().await;
            Ok(())
        })
        .unwrap();
    game.tick().unwrap();
    drop(slot.borrow_mut().take());
    assert!(process.kill().unwrap());
    drop(game);
    audio.update().unwrap();
    // Shutdown releases paused and orphaned instances before closing the device.
    let weak = Rc::downgrade(&audio);
    drop(audio);
    assert!(weak.upgrade().is_none());
    let audio = Rc::new(Audio::open().unwrap());
    let sound = Sound::load(audio.clone(), &path).unwrap();
    sound.play().unwrap();
    let deadline = Instant::now() + Duration::from_secs(3);
    while sound.playing() && Instant::now() < deadline {
        audio.update().unwrap();
        std::thread::sleep(Duration::from_millis(5));
    }
    assert!(!sound.playing(), "WAV playback did not complete");
    audio.update().unwrap();
    let weak = Rc::downgrade(&audio);
    drop(audio);
    assert!(weak.upgrade().is_some()); // Loaded resources keep the device live.
    drop(sound);
    assert!(weak.upgrade().is_none());
    drop(Audio::open().unwrap());
}
