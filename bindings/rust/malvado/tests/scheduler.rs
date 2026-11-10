// Malvado 2; SPDX-License-Identifier: Apache-2.0
#![cfg(not(miri))]
use malvado::{EndReason, Engine, Error, Signal, SpawnOptions, Status};
use std::cell::{Cell, RefCell};
use std::future::{pending, poll_fn};
use std::rc::Rc;
use std::task::Poll;

struct Guard(Rc<Cell<u32>>);
impl Drop for Guard {
    fn drop(&mut self) {
        self.0.set(self.0.get() + 1);
    }
}

#[test]
fn golden_native_trace_matches_c_and_lua() {
    let tick = Rc::new(Cell::new(0));
    let output = Rc::new(RefCell::new(String::new()));
    let drops = Rc::new(Cell::new(0));
    let mut engine = Engine::new().unwrap();
    let (t, out, d) = (tick.clone(), output.clone(), drops.clone());
    engine
        .spawn(SpawnOptions::default(), move |process| async move {
            let _guard = Guard(d.clone());
            let mut value = 40;
            out.borrow_mut()
                .push_str(&format!("{} parent {value}\n", t.get()));
            let (t2, out2) = (t.clone(), out.clone());
            process.spawn(
                SpawnOptions {
                    attached: false,
                    ..Default::default()
                },
                move |child| async move {
                    let _guard = Guard(d);
                    let mut value = 7;
                    out2.borrow_mut()
                        .push_str(&format!("{} child {value}\n", t2.get()));
                    child.frame().await;
                    value += 1;
                    out2.borrow_mut()
                        .push_str(&format!("{} child {value}\n", t2.get()));
                    Ok(())
                },
            )?;
            process.frame().await;
            value += 1;
            out.borrow_mut()
                .push_str(&format!("{} parent {value}\n", t.get()));
            Ok(())
        })
        .unwrap();
    let (t, out, d) = (tick.clone(), output.clone(), drops.clone());
    engine
        .spawn(
            SpawnOptions {
                fps: 15,
                ..Default::default()
            },
            move |process| async move {
                let _guard = Guard(d);
                for value in 1..=3 {
                    out.borrow_mut()
                        .push_str(&format!("{} slow {value}\n", t.get()));
                    if value < 3 {
                        process.frame().await;
                    }
                }
                Ok(())
            },
        )
        .unwrap();
    assert_eq!(output.borrow().as_str(), "");
    for i in 1..=9 {
        tick.set(i);
        engine.tick().unwrap();
    }
    assert_eq!(engine.process_count(), 0);
    assert_eq!(drops.get(), 3);
    output
        .borrow_mut()
        .push_str(&format!("closed {}\n", drops.get()));
    assert_eq!(
        output.borrow().as_str(),
        include_str!("fixtures/alpha7_baseline.txt")
    );
}

#[test]
fn all_native_cadences_match_the_lua_baseline() {
    for fps in 0..=60 {
        let count = Rc::new(Cell::new(0));
        let output = count.clone();
        let mut engine = Engine::new().unwrap();
        engine
            .spawn(
                SpawnOptions {
                    fps,
                    ..Default::default()
                },
                move |p| async move {
                    loop {
                        output.set(output.get() + 1);
                        p.frame().await;
                    }
                },
            )
            .unwrap();
        for _ in 0..=60 {
            engine.tick().unwrap();
        }
        assert_eq!(
            count.get(),
            if fps == 0 { 61 } else { fps + 1 },
            "fps {fps}"
        );
    }
}

#[test]
fn sleeping_and_freezing_preserve_pending_locals_until_wakeup() {
    for signal in [Signal::Sleep, Signal::Freeze] {
        let count = Rc::new(Cell::new(0));
        let output = count.clone();
        let mut engine = Engine::new().unwrap();
        let h = engine
            .spawn(SpawnOptions::default(), move |p| async move {
                let mut local = 5;
                output.set(local);
                p.frame().await;
                local += 1;
                output.set(local);
                Ok(())
            })
            .unwrap();
        engine.tick().unwrap();
        engine.signal(&h, signal).unwrap();
        assert_eq!(
            h.snapshot().unwrap().status,
            Some(if signal == Signal::Sleep {
                Status::Sleeping
            } else {
                Status::Frozen
            })
        );
        for _ in 0..20 {
            engine.tick().unwrap();
        }
        assert_eq!(count.get(), 5);
        engine.signal(&h, Signal::Wake).unwrap();
        engine.tick().unwrap();
        assert_eq!(count.get(), 6);
        assert_eq!(h.snapshot().unwrap().reason, Some(EndReason::Returned));
    }
}

#[test]
fn unsupported_pending_future_is_not_busy_polled() {
    let polls = Rc::new(Cell::new(0));
    let count = polls.clone();
    let mut engine = Engine::new().unwrap();
    engine
        .spawn(SpawnOptions::default(), move |_| {
            poll_fn(move |_| {
                count.set(count.get() + 1);
                Poll::Pending
            })
        })
        .unwrap();
    for _ in 0..100 {
        engine.tick().unwrap();
    }
    assert_eq!(polls.get(), 1);
}

#[test]
fn self_kill_and_external_cancel_drop_only_at_native_safe_points() {
    let drops = Rc::new(Cell::new(0));
    let d = drops.clone();
    let mut engine = Engine::new().unwrap();
    let h = engine
        .spawn(SpawnOptions::default(), move |p| async move {
            let _guard = Guard(d.clone());
            assert!(p.current().kill()?);
            assert_eq!(d.get(), 0);
            p.frame().await;
            panic!("a killed future must never resume");
        })
        .unwrap();
    engine.tick().unwrap();
    assert_eq!(drops.get(), 1);
    assert_eq!(h.snapshot().unwrap().reason, Some(EndReason::Killed));
    let guard = Guard(drops.clone());
    let h = engine
        .spawn(SpawnOptions::default(), move |_| async move {
            let _guard = guard;
            pending::<()>().await;
            Ok(())
        })
        .unwrap();
    assert!(h.kill().unwrap());
    assert_eq!(drops.get(), 1);
    engine.tick().unwrap();
    assert_eq!(drops.get(), 2);
    assert!(!h.kill().unwrap());
}

#[test]
fn attached_and_detached_children_keep_native_creator_and_owner() {
    let children = Rc::new(RefCell::new(Vec::new()));
    let output = children.clone();
    let mut engine = Engine::new().unwrap();
    let parent = engine
        .spawn(SpawnOptions::default(), move |p| async move {
            for attached in [true, false] {
                output.borrow_mut().push(p.spawn(
                    SpawnOptions {
                        attached,
                        ..Default::default()
                    },
                    |_| async {
                        pending::<()>().await;
                        Ok(())
                    },
                )?);
            }
            p.frame().await;
            Ok(())
        })
        .unwrap();
    engine.tick().unwrap();
    let children = children.borrow();
    for (i, child) in children.iter().enumerate() {
        let snapshot = child.snapshot().unwrap();
        assert_eq!(snapshot.creator_id, parent.id());
        assert_eq!(snapshot.owner_id, if i == 0 { parent.id() } else { 0 });
    }
    engine.tick().unwrap();
    assert_eq!(
        children[0].snapshot().unwrap().reason,
        Some(EndReason::Killed)
    );
    assert!(children[1].snapshot().unwrap().alive);
    assert_eq!(engine.process_count(), 1);
    drop(engine);
    assert_eq!(
        children[1].snapshot().unwrap().reason,
        Some(EndReason::Killed)
    );
    assert_eq!(children[1].kill(), Err(Error::Closed));
}

#[test]
fn panic_and_async_error_have_native_error_reason_and_safe_cleanup() {
    for panic in [false, true] {
        let drops = Rc::new(Cell::new(0));
        let d = drops.clone();
        let mut engine = Engine::new().unwrap();
        let h = engine
            .spawn(SpawnOptions::default(), move |_| async move {
                let _guard = Guard(d);
                if panic {
                    panic!("contained future panic");
                }
                Err(Error::Closed)
            })
            .unwrap();
        assert_eq!(engine.tick(), Err(Error::Process));
        assert_eq!(drops.get(), 1);
        assert_eq!(h.snapshot().unwrap().reason, Some(EndReason::Error));
        assert_eq!(engine.process_count(), 0);
    }
}

#[test]
fn destructor_panics_do_not_unwind_across_native_cleanup() {
    struct BadDrop;
    impl Drop for BadDrop {
        fn drop(&mut self) {
            panic!("destructor failure");
        }
    }
    let mut engine = Engine::new().unwrap();
    let value = BadDrop;
    let h = engine
        .spawn(SpawnOptions::default(), move |_| async move {
            let _value = value;
            pending::<()>().await;
            Ok(())
        })
        .unwrap();
    h.kill().unwrap();
    engine.tick().unwrap();
    assert_eq!(h.snapshot().unwrap().reason, Some(EndReason::Killed));
    let value = BadDrop;
    engine
        .spawn(SpawnOptions::default(), move |_| async move {
            let _v = value;
            pending::<()>().await;
            Ok(())
        })
        .unwrap();
    drop(engine);
}

#[test]
fn wake_after_shutdown_and_saved_process_tokens_are_safe() {
    let saved_waker = Rc::new(RefCell::new(None));
    let saved_context = Rc::new(RefCell::new(None));
    let (waker_out, context_out) = (saved_waker.clone(), saved_context.clone());
    let drops = Rc::new(Cell::new(0));
    let guard = Guard(drops.clone());
    let mut engine = Engine::new().unwrap();
    let h = engine
        .spawn(SpawnOptions::default(), move |p| {
            *context_out.borrow_mut() = Some(p);
            let guard = guard;
            poll_fn(move |cx| {
                let _keep_guard = &guard;
                *waker_out.borrow_mut() = Some(cx.waker().clone());
                Poll::Pending
            })
        })
        .unwrap();
    engine.tick().unwrap();
    drop(engine);
    assert_eq!(drops.get(), 1);
    let waker = saved_waker.borrow().as_ref().unwrap().clone();
    std::thread::spawn(move || waker.wake()).join().unwrap();
    let process = saved_context.borrow();
    let process = process.as_ref().unwrap();
    assert_eq!(process.signal(&h, Signal::Wake), Err(Error::Closed));
    assert!(matches!(
        process.spawn(SpawnOptions::default(), |_| async { Ok(()) }),
        Err(Error::Closed)
    ));
    assert_eq!(h.snapshot().unwrap().reason, Some(EndReason::Killed));
}
