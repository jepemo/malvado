// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Relation, prelude::*};
use std::{cell::Cell, rc::Rc};

async fn cadence(p: ProcessContext, tick: Rc<Cell<u32>>, expected: &[u32]) -> Result<(), Error> {
    let mut previous = None;
    for (index, &number) in expected.iter().enumerate() {
        assert_eq!(tick.get(), number);
        let delta = previous.map_or(0.0, |last| f64::from(number - last) / 60.0);
        assert_eq!(p.delta(), delta);
        previous = Some(number);
        if index + 1 < expected.len() {
            p.frame().await;
        }
    }
    Ok(())
}

#[test]
fn cadence_matches_lua_and_defaults_apply_at_each_spawn() -> Result<(), Error> {
    let mut game = Game::new()?;
    let tick = Rc::new(Cell::new(0));
    let clock = || tick.clone();

    // Same activation sequences as tests/cadence.lua and tests/default_fps.lua.
    let time = clock();
    game.spawn(move |p| cadence(p, time, &[1, 2]))?;
    game.set_fps(30)?;
    assert_eq!(game.set_fps(61), Err(Error::InvalidArgument));
    let time = clock();
    game.spawn(move |p| cadence(p, time, &[1, 3, 5, 7]))?;
    let time = clock();
    let sleeping = game.spawn(move |p| cadence(p, time, &[1, 6, 8]))?;
    let time = clock();
    let frozen = game.spawn(move |p| cadence(p, time, &[1, 6, 8]))?;
    game.set_fps(15)?;
    let time = clock();
    game.spawn(move |p| cadence(p, time, &[1, 5, 9, 13, 17]))?;
    let time = clock();
    let parent = game.spawn(move |p| async move {
        let child = p.spawn(move |c| cadence(c, time, &[2, 6, 10]))?;
        let parent = p.current();
        assert_eq!(parent.children(Relation::Ownership, 0)?[0].id(), child.id());
        while child.snapshot()?.alive {
            p.frame().await;
        }
        Ok(())
    })?;
    assert!(parent.children(Relation::Creation, 0)?.is_empty());

    for (fps, expected) in [(0, &[1, 2][..]), (17, &[1, 5, 9, 12, 16])] {
        let time = clock();
        game.spawn_with(
            SpawnOptions {
                fps: Some(fps),
                ..Default::default()
            },
            move |p| cadence(p, time, expected),
        )?;
    }
    let invalid = game.spawn_with(
        SpawnOptions {
            fps: Some(61),
            ..Default::default()
        },
        |_| async { panic!("an invalid spawn must not run") },
    );
    assert!(matches!(invalid, Err(Error::InvalidArgument)));

    let mut other = Game::new()?;
    async fn worker(p: ProcessContext) -> Result<(), Error> {
        assert_eq!(p.delta(), 0.0);
        p.frame().await;
        assert_eq!(p.delta(), 1.0 / 60.0);
        Ok(())
    }
    other.spawn(worker)?;
    // The native clock can plan five catch-up ticks and drop the rest.
    assert_eq!(other.advance_time(2.0)?, 5);
    assert!(other.dropped_seconds()? > 0.0);
    assert_eq!(other.process_count(), 1);
    other.tick()?;
    other.tick()?;
    assert_eq!(other.process_count(), 0);

    for number in 1..=17 {
        tick.set(number);
        if number == 3 {
            game.signal(&sleeping, Signal::Sleep)?;
            game.signal(&frozen, Signal::Freeze)?;
        }
        if number == 6 {
            game.signal(&sleeping, Signal::Wake)?;
            game.signal(&frozen, Signal::Wake)?;
        }
        game.tick()?;
    }
    assert_eq!(game.process_count(), 0);
    Ok(())
}

#[test]
fn context_operations_use_checked_native_state() -> Result<(), Error> {
    let mut game = Game::new()?;
    let target = game.spawn(|p| async move {
        p.set_graphic(rect(10.0, 4.0).color(CYAN))?;
        p.set_position(10.0, 10.0)?;
        p.frame().await;
        Ok(())
    })?;
    let actor = game.spawn(move |p| async move {
        p.set_graphic(circle(2.0))?;
        assert_eq!(p.graphic()?.unwrap().color, WHITE);
        assert_eq!(p.set_graphic(rect(0.0, 2.0)), Err(Error::InvalidArgument));
        assert_eq!(p.set_graphic(circle(f64::NAN)), Err(Error::InvalidArgument));
        assert_eq!(p.graphic()?.unwrap().color, WHITE);
        p.set_position(10.0, 4.0)?;
        p.set_y(5.0)?;
        assert!(!p.collides_with(&target)?);
        p.set_y(6.25)?;
        assert!(p.collides_with(&target)?);
        assert_eq!(p.first_collision(0)?.unwrap().id(), target.id());
        p.set_z(-2.0)?;
        p.set_x(9.0)?;
        p.set_angle(42.0)?;
        p.set_scale(1.5)?;
        assert_eq!(p.position()?, (9.0, 6.25));
        assert_eq!((p.x()?, p.y()?, p.z()?), (9.0, 6.25, -2.0));
        assert_eq!((p.angle()?, p.scale()?), (42.0, 1.5));
        assert_eq!(p.set_x(f64::INFINITY), Err(Error::InvalidArgument));
        assert_eq!(p.position()?, (9.0, 6.25));
        p.draw_text("Hello", (3, 4), 12, Color::rgba(255, 255, 255, 128))?;
        #[cfg(feature = "graphics")]
        {
            assert!(!p.key_down(Key::Left));
            assert!(!p.key_pressed(Key::Left));
            assert!(!p.key_released(Key::Left));
            assert_eq!(p.mouse_position(), (0, 0));
            assert!(!p.pad_connected(0));
            assert_eq!(p.text_input_next(), None);
            assert_eq!(p.quit(), Err(Error::Platform("no managed window")));
        }
        p.frame().await;
        Ok(())
    })?;
    game.tick()?;
    assert_eq!(actor.draw_count()?, 1);
    game.tick()?;
    assert_eq!(game.process_count(), 0);
    Ok(())
}

#[test]
fn delta_counts_ticks_since_the_last_ready_poll() -> Result<(), Error> {
    use std::{cell::RefCell, future::poll_fn, task::Poll};

    let mut game = Game::new()?;
    let wake = Rc::new(RefCell::new(None));
    let output = wake.clone();
    game.spawn(move |p| async move {
        assert_eq!(p.delta(), 0.0);
        let mut first = true;
        poll_fn(|cx| {
            if std::mem::take(&mut first) {
                output.replace(Some(cx.waker().clone()));
                Poll::Pending
            } else {
                Poll::Ready(())
            }
        })
        .await;
        assert_eq!(p.delta(), 5.0 / 60.0);
        Ok(())
    })?;

    game.tick()?;
    for _ in 0..4 {
        game.tick()?;
    }
    wake.take().unwrap().wake();
    game.tick()?;
    assert_eq!(game.process_count(), 0);
    Ok(())
}
