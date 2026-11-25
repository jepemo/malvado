// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{EndReason, prelude::*, window::Window};
use std::{cell::Cell, rc::Rc, time::Duration};

unsafe extern "C" {
    fn mv_keyboard_sample(key: i32, pressed: i32, released: i32);
    fn malvado_test_frame_hash(light: *mut i32) -> u64;
}

fn press(key: Key) {
    // SAFETY: inject a buffered observation on the live window owner thread.
    unsafe { mv_keyboard_sample(key as i32, 1, 0) };
}

fn frame_hash() -> u64 {
    let mut light = 0;
    // SAFETY: read back the live main-thread frame into an initialized output.
    let hash = unsafe { malvado_test_frame_hash(&mut light) };
    assert!(light > 0);
    hash
}

#[allow(
    dead_code,
    reason = "exercise the real example processes from the test host"
)]
mod catch {
    include!("../examples/catch.rs");

    pub fn verify() -> Result<(), Error> {
        use malvado::{
            collision::Transform,
            graphic::{Geometry, Pivot},
            window::Window,
        };

        let window = Window::open(WIDTH as i32, HEIGHT as i32, "Catch check")?;
        let mut game = Game::new()?;
        let player = game.spawn(player)?;
        let target = player.clone();
        let actor = game.spawn(move |p| ball(p, target))?;

        fn check_frame(
            window: &Window,
            game: &Game,
            ball: (f64, f64),
            score: &str,
        ) -> Result<(), Error> {
            let actual = {
                let frame = window.frame(BLACK)?;
                frame.process_graphics(game)?;
                super::frame_hash()
            };
            let reference = window.frame(BLACK)?;
            reference.graphic(
                Geometry::rectangle(80.0, 16.0, Pivot::Center)?,
                Transform {
                    x: 320.0,
                    y: 320.0,
                    ..Default::default()
                },
                CYAN,
            )?;
            reference.graphic(
                Geometry::circle(12.0)?,
                Transform {
                    x: ball.0,
                    y: ball.1,
                    ..Default::default()
                },
                ORANGE,
            )?;
            reference.text(score, (20, 20), 24, WHITE)?;
            assert_eq!(actual, super::frame_hash(), "{score} at {ball:?}");
            Ok(())
        }

        assert!(actor.graphic()?.is_none());
        game.tick()?;
        assert_eq!(player.position()?, (320.0, 320.0));
        assert_eq!(actor.position()?, (80.0, 0.0));
        check_frame(&window, &game, (80.0, 0.0), "Score: 0")?;

        // Only the position after movement overlaps the player.
        actor.set_position(320.0, 298.0)?;
        assert!(!actor.collides_with(&player)?);
        game.tick()?;
        assert_eq!(actor.position()?, (480.0, -12.0));
        check_frame(&window, &game, (480.0, -12.0), "Score: 1")?;
        game.tick()?;
        assert_eq!(actor.position()?, (480.0, -9.0));
        check_frame(&window, &game, (480.0, -9.0), "Score: 1")?;

        actor.set_position(480.0, 372.0)?;
        game.tick()?;
        assert_eq!(actor.position()?, (0.0, -12.0));
        check_frame(&window, &game, (0.0, -12.0), "Score: 1")?;
        Ok(())
    }
}

#[allow(dead_code, reason = "exercise the lesson with a bounded managed host")]
mod hello {
    include!("../../../../examples/rust/hello.rs");

    pub fn verify() -> Result<(), Error> {
        let game = Game::windowed(800, 600, "Hello world")?;
        let actor = game.spawn(hello)?;
        game.spawn(move |p| async move {
            p.frame().await;
            assert_eq!(actor.draw_count()?, 1);
            p.quit()
        })?;
        game.run()
    }
}

#[allow(dead_code, reason = "exercise the lesson with a bounded managed host")]
mod moving_balls {
    include!("../../../../examples/rust/moving-balls.rs");

    pub fn verify() -> Result<(), Error> {
        let game = Game::windowed(WIDTH, HEIGHT, "Moving balls")?;
        let cyan = game.spawn(|p| ball(p, 90.0, 120.0, CYAN))?;
        let orange = game.spawn(|p| ball(p, 180.0, 240.0, ORANGE))?;
        game.spawn(move |p| async move {
            p.frame().await;
            assert_eq!(cyan.position()?, (RADIUS + 2.0, 90.0));
            assert_eq!(orange.position()?, (RADIUS + 4.0, 180.0));
            p.quit()
        })?;
        game.run()
    }
}

struct MarkDrop(Rc<Cell<bool>>);
impl Drop for MarkDrop {
    fn drop(&mut self) {
        self.0.set(true);
    }
}

fn exits_and_input() -> Result<(), Error> {
    let game = Game::windowed(160, 120, "Managed input")?;
    let dropped = Rc::new(Cell::new(false));
    let guard = MarkDrop(dropped.clone());
    let monitor = game.spawn(move |p| async move {
        let _guard = guard;
        assert!(!p.key_pressed(Key::Space));
        press(Key::Space);
        // Force the next render to have multiple logical ticks due.
        std::thread::sleep(Duration::from_millis(65));
        p.frame().await;
        assert!(p.key_pressed(Key::Space) && p.key_pressed(Key::Space));
        p.frame().await;
        assert!(!p.key_pressed(Key::Space));
        press(Key::Escape);
        for _ in 0..10 {
            p.frame().await;
            if p.key_pressed(Key::Escape) {
                panic!("the runner must consume its exit policy before polling processes");
            }
        }
        panic!("the runner did not observe Escape");
    })?;
    game.spawn(|p| async move {
        p.frame().await;
        assert!(p.key_pressed(Key::Space));
        Ok(())
    })?;
    game.run()?;
    assert!(dropped.get());
    assert_eq!(monitor.snapshot()?.reason, Some(EndReason::Killed));

    let mut game = Game::windowed(160, 120, "Finish")?;
    let audio = game.audio()?;
    assert!(Rc::ptr_eq(&audio, &game.audio()?));
    let service = Rc::downgrade(&audio);
    drop(audio);
    let finished = game.spawn(|p| async move {
        p.frame().await;
        Ok(())
    })?;
    game.run()?;
    assert_eq!(finished.snapshot()?.reason, Some(EndReason::Returned));
    assert!(service.upgrade().is_none());

    let game = Game::windowed(160, 120, "Process failure")?;
    let failure = game.spawn(|_| async { Err(Error::InvalidArgument) })?;
    assert_eq!(game.run(), Err(Error::Process));
    assert_eq!(failure.snapshot()?.reason, Some(EndReason::Error));
    drop(Window::open(160, 120, "Window lease released")?);
    Ok(())
}

fn main() -> Result<(), Error> {
    exits_and_input()?;
    catch::verify()?;
    hello::verify()?;
    moving_balls::verify()
}
