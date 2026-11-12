// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Engine, Error, Signal, SpawnOptions,
    animation::{Animation, Frame},
};
use std::{cell::Cell, rc::Rc};
fn definition(looping: bool) -> Animation {
    Animation::new(
        &[
            Frame {
                graph_id: 7,
                duration_us: 50000,
            },
            Frame {
                graph_id: 42,
                duration_us: 50000,
            },
        ],
        looping,
    )
    .unwrap()
}
#[test]
fn shared_clock_trace_and_retained_definition() {
    let mut engine = Engine::new().unwrap();
    let actor = engine
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    assert_eq!(actor.animation_clock().unwrap(), None);
    let animation = definition(true);
    actor.set_animation_clock(&animation).unwrap();
    drop(animation);
    let alias = actor.clone();
    let lines: Vec<_> = include_str!("../../../../tests/fixtures/process-animation.txt")
        .lines()
        .collect();
    for (i, line) in lines.iter().enumerate() {
        let values: Vec<u64> = line
            .split_whitespace()
            .map(|x| x.parse().unwrap())
            .collect();
        let clock = alias.animation_clock().unwrap().unwrap();
        assert_eq!(clock.elapsed_us, values[0]);
        assert_eq!(clock.sample.graph_id as u64, values[1]);
        if i + 1 < lines.len() {
            engine.tick().unwrap();
        }
    }
    for signal in [Signal::Sleep, Signal::Freeze] {
        engine.signal(&actor, signal).unwrap();
        engine.tick().unwrap();
        assert_eq!(actor.animation_clock().unwrap().unwrap().elapsed_us, 0);
    }
    engine.signal(&actor, Signal::Wake).unwrap();
    engine.tick().unwrap();
    assert_eq!(actor.animation_clock().unwrap().unwrap().elapsed_us, 16666);
    actor.clear_animation_clock().unwrap();
    actor.clear_animation_clock().unwrap();
    assert!(alias.animation_clock().unwrap().is_none());
    let once = definition(false);
    actor.set_animation_clock(&once).unwrap();
    drop(once);
    for _ in 0..10 {
        engine.tick().unwrap();
    }
    assert_eq!(actor.animation_clock().unwrap().unwrap().elapsed_us, 100000);
    assert_eq!(
        actor.animation_clock().unwrap().unwrap().sample.graph_id,
        42
    );
    actor.kill().unwrap();
    engine.tick().unwrap();
    assert_eq!(actor.animation_clock(), Err(Error::NotFound));
    assert_eq!(
        actor.set_animation_clock(&definition(true)),
        Err(Error::NotFound)
    );
    drop(engine);
    assert_eq!(actor.animation_clock(), Err(Error::Closed));
    assert_eq!(actor.clear_animation_clock(), Err(Error::Closed));
}
#[test]
fn cadence_and_in_tick_assignment_use_one_native_tick() {
    let mut engine = Engine::new().unwrap();
    let polls = Rc::new(Cell::new(0));
    let count = polls.clone();
    let slow = engine
        .spawn(
            SpawnOptions {
                fps: 15,
                ..Default::default()
            },
            move |p| async move {
                loop {
                    count.set(count.get() + 1);
                    p.frame().await;
                }
            },
        )
        .unwrap();
    let animation = definition(true);
    slow.set_animation_clock(&animation).unwrap();
    let mut expected = animation.playback();
    for _ in 0..9 {
        engine.tick().unwrap();
        let sample = expected.advance(60).unwrap();
        let clock = slow.animation_clock().unwrap().unwrap();
        assert_eq!(clock.elapsed_us, expected.elapsed_us());
        assert_eq!(clock.sample, sample);
    }
    assert_eq!(polls.get(), 3);
    let during = definition(true);
    let actor = engine
        .spawn(Default::default(), move |p| async move {
            p.current().set_animation_clock(&during)?;
            drop(during);
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    engine.tick().unwrap();
    assert_eq!(actor.animation_clock().unwrap().unwrap().elapsed_us, 0);
    engine.tick().unwrap();
    assert_eq!(actor.animation_clock().unwrap().unwrap().elapsed_us, 16666);
    let ended = engine
        .spawn(Default::default(), |_| async { Ok(()) })
        .unwrap();
    ended.set_animation_clock(&definition(true)).unwrap();
    engine.tick().unwrap();
    assert_eq!(ended.animation_clock(), Err(Error::NotFound));
    drop(engine);
    assert_eq!(actor.animation_clock(), Err(Error::Closed));
}
