// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Engine, Error,
    audio::{Audio, Music},
};
use std::{
    path::Path,
    rc::Rc,
    time::{Duration, Instant},
};
fn pump(audio: &Audio, duration: Duration) {
    let deadline = Instant::now() + duration;
    while Instant::now() < deadline {
        audio.update().unwrap();
        std::thread::sleep(Duration::from_millis(5));
    }
}
fn main() {
    let path = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../../tests/fixtures/silence.wav");
    let audio = Rc::new(Audio::open().unwrap());
    assert!(Music::load(audio.clone(), path.with_file_name("missing.wav"), false).is_err());
    assert!(Music::load(audio.clone(), path.with_extension("mp3"), false).is_err());
    assert!(matches!(
        Music::load(audio.clone(), "bad\0path", false),
        Err(Error::InvalidArgument)
    ));
    assert!(Music::load(audio.clone(), path.with_file_name("empty.wav"), false).is_err());
    for extension in ["wav", "ogg"] {
        let music = Music::load(audio.clone(), path.with_extension(extension), true).unwrap();
        let other = Music::load(audio.clone(), path.with_extension(extension), false).unwrap();
        for volume in [f32::NAN, f32::NEG_INFINITY, -0.1, 1.1] {
            assert_eq!(music.set_volume(volume), Err(Error::InvalidArgument));
        }
        music.set_volume(0.0).unwrap();
        music.set_volume(1.0).unwrap();
        music.play().unwrap();
        music.play().unwrap(); // Repeated play keeps only one native retention.
        other.play().unwrap();
        other.stop().unwrap();
        assert!(!other.playing());
        pump(&audio, Duration::from_millis(800)); // More than three fixture lengths.
        assert!(music.playing(), "loop stopped after one pass");
        music.pause().unwrap();
        assert!(!music.playing());
        pump(&audio, Duration::from_millis(50));
        assert!(!music.playing());
        music.resume().unwrap();
        assert!(music.playing());
        music.stop().unwrap();
        assert!(!music.playing());
        other.play().unwrap();
        let deadline = Instant::now() + Duration::from_secs(3);
        while other.playing() && Instant::now() < deadline {
            audio.update().unwrap();
            std::thread::sleep(Duration::from_millis(5));
        }
        assert!(!other.playing(), "non-looping stream did not complete");
        audio.update().unwrap();
        // Replay after completion must register another retained use.
        other.play().unwrap();
        other.stop().unwrap();
    }
    let mut engine = Engine::new().unwrap();
    let music = Music::load(audio.clone(), &path, true).unwrap();
    let process = engine
        .spawn(Default::default(), move |p| async move {
            let owned = p.resource(music)?;
            owned.play()?;
            p.frame().await;
            Ok(())
        })
        .unwrap();
    engine.tick().unwrap();
    assert!(process.kill().unwrap());
    drop(engine);
    pump(&audio, Duration::from_millis(50)); // Orphaned streams still receive updates.
    let paused = Music::load(audio.clone(), &path, true).unwrap();
    paused.play().unwrap();
    paused.pause().unwrap();
    drop(paused);
    audio.update().unwrap();
    let weak = Rc::downgrade(&audio);
    drop(audio); // Shutdown stops orphaned looping and paused streams.
    assert!(weak.upgrade().is_none());
    let audio = Rc::new(Audio::open().unwrap());
    let music = Music::load(audio.clone(), &path, false).unwrap();
    let weak = Rc::downgrade(&audio);
    drop(audio);
    assert!(weak.upgrade().is_some());
    music.play().unwrap();
    drop(music);
    assert!(weak.upgrade().is_none());
    drop(Audio::open().unwrap());
}
