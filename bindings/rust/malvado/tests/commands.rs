// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Engine, Error, SpawnOptions, graphic::Color};
use std::{
    cell::{Cell, RefCell},
    future::poll_fn,
    rc::Rc,
    task::{Poll, Waker},
};
const WHITE: Color = Color {
    r: 255,
    g: 255,
    b: 255,
    a: 255,
};

#[test]
fn copied_commands_validate_and_clear_only_on_actual_low_cadence_activation() {
    let mut engine = Engine::new().unwrap();
    let activations = Rc::new(Cell::new(0));
    let observed = activations.clone();
    let actor = engine
        .spawn(
            SpawnOptions {
                fps: 5,
                ..Default::default()
            },
            move |p| async move {
                loop {
                    let actor = p.current();
                    assert_eq!(actor.draw_count()?, 0);
                    actor.draw_box((0, 0), (4, 4), true, WHITE)?;
                    actor.draw_circle((10, 10), 2, false, WHITE)?;
                    observed.set(observed.get() + 1);
                    p.frame().await;
                }
            },
        )
        .unwrap();
    actor
        .draw_text("before activation", (0, 0), 20, WHITE)
        .unwrap();
    assert_eq!(actor.draw_count().unwrap(), 1);
    engine.tick().unwrap();
    assert_eq!(activations.get(), 1);
    assert_eq!(actor.draw_count().unwrap(), 2);
    actor
        .draw_text(&String::from("copied"), (0, 0), -1, WHITE)
        .unwrap();
    for invalid in [
        actor.draw_box((i32::MAX, 0), (1, 1), true, WHITE),
        actor.draw_box((0, 0), (0, 1), true, WHITE),
        actor.draw_circle((i32::MIN, 0), 1, true, WHITE),
        actor.draw_text("bad\0text", (0, 0), 20, WHITE),
    ] {
        assert_eq!(invalid, Err(Error::InvalidArgument));
    }
    for _ in 0..11 {
        engine.tick().unwrap();
        assert_eq!(actor.draw_count().unwrap(), 3);
    }
    engine.tick().unwrap();
    assert_eq!(activations.get(), 2);
    assert_eq!(actor.draw_count().unwrap(), 2);
    actor.clear_draw_commands().unwrap();
    actor.clear_draw_commands().unwrap();
    assert_eq!(actor.draw_count().unwrap(), 0);
    actor.kill().unwrap();
    assert_eq!(actor.draw_count(), Err(Error::NotFound));
    assert_eq!(
        actor.draw_box((0, 0), (1, 1), true, WHITE),
        Err(Error::NotFound)
    );
}

#[test]
fn pending_without_wake_keeps_commands_until_the_next_real_poll() {
    let mut engine = Engine::new().unwrap();
    let saved = Rc::new(RefCell::new(None::<Waker>));
    let output = saved.clone();
    let actor = engine
        .spawn(Default::default(), move |p| async move {
            let actor = p.current();
            let mut first = true;
            poll_fn(move |cx| {
                assert_eq!(actor.draw_count().unwrap(), 0);
                if first {
                    first = false;
                    actor.draw_box((1, 1), (2, 2), true, WHITE).unwrap();
                    *output.borrow_mut() = Some(cx.waker().clone());
                    Poll::Pending
                } else {
                    Poll::Ready(Ok(()))
                }
            })
            .await
        })
        .unwrap();
    engine.tick().unwrap();
    actor.draw_text("retained", (0, 0), 20, WHITE).unwrap();
    for _ in 0..5 {
        engine.tick().unwrap();
        assert_eq!(actor.draw_count().unwrap(), 2);
    }
    saved.borrow().as_ref().unwrap().wake_by_ref();
    engine.tick().unwrap();
    assert_eq!(actor.draw_count(), Err(Error::NotFound));
}
