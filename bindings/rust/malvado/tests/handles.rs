// Malvado 2; SPDX-License-Identifier: Apache-2.0
#![cfg(not(miri))]
use malvado::{EndReason, Engine, Error, Signal, SpawnOptions};
use std::cell::{Cell, RefCell};
use std::rc::Rc;

#[test]
fn canonical_handle_identity_and_properties_are_native() {
    let seen = Rc::new(RefCell::new(None));
    let output = seen.clone();
    let mut engine = Engine::new().unwrap();
    let handle = engine
        .spawn(
            SpawnOptions {
                priority: 3,
                type_id: 12,
                ..Default::default()
            },
            move |p| async move {
                *output.borrow_mut() = Some(p.current());
                let snapshot = p.current().snapshot()?;
                assert_eq!(snapshot.priority, 3);
                assert_eq!(snapshot.type_id, 12);
                p.current().set_priority(7)?;
                p.current().set_type(19)?;
                p.frame().await;
                Ok(())
            },
        )
        .unwrap();
    engine.tick().unwrap();
    assert!(seen.borrow().as_ref().unwrap() == &handle);
    assert_eq!(handle.snapshot().unwrap().priority, 7);
    assert_eq!(handle.snapshot().unwrap().type_id, 19);
    engine.tick().unwrap();
    assert_eq!(handle.snapshot().unwrap().reason, Some(EndReason::Returned));
    assert_eq!(handle.set_type(1), Err(Error::NotFound));
    assert!(!handle.kill().unwrap());
    drop(engine);
    assert_eq!(handle.set_priority(1), Err(Error::Closed));
    assert_eq!(handle.snapshot().unwrap().reason, Some(EndReason::Returned));
}

#[test]
fn foreign_handles_cannot_signal_same_numbered_local_processes() {
    let a = Engine::new().unwrap();
    let b = Engine::new().unwrap();
    let ah = a
        .spawn(SpawnOptions::default(), |_| async { Ok(()) })
        .unwrap();
    let bh = b
        .spawn(SpawnOptions::default(), |_| async { Ok(()) })
        .unwrap();
    assert_eq!(ah.id(), bh.id());
    assert!(ah != bh);
    assert_eq!(
        a.signal(&bh, Signal::Kill, true),
        Err(Error::ForeignContext)
    );
    assert!(ah.snapshot().unwrap().alive);
    assert!(bh.snapshot().unwrap().alive);
}

#[test]
fn invalid_spawn_keeps_owned_state_with_the_caller_until_error_cleanup() {
    struct Guard(Rc<Cell<u32>>);
    impl Drop for Guard {
        fn drop(&mut self) {
            self.0.set(self.0.get() + 1);
        }
    }
    let count = Rc::new(Cell::new(0));
    let guard = Guard(count.clone());
    let engine = Engine::new().unwrap();
    let result = engine.spawn(
        SpawnOptions {
            fps: 61,
            ..Default::default()
        },
        move |_| async move {
            let _guard = guard;
            Ok(())
        },
    );
    assert!(matches!(result, Err(Error::InvalidArgument)));
    assert_eq!(count.get(), 1);
    assert_eq!(engine.process_count(), 0);
}

#[test]
fn priority_changes_apply_on_the_following_native_tick() {
    let order = Rc::new(RefCell::new(Vec::new()));
    let output = order.clone();
    let mut engine = Engine::new().unwrap();
    let low = engine
        .spawn(SpawnOptions::default(), move |p| async move {
            output.borrow_mut().push(1);
            p.frame().await;
            output.borrow_mut().push(1);
            Ok(())
        })
        .unwrap();
    let output = order.clone();
    engine
        .spawn(
            SpawnOptions {
                priority: 1,
                ..Default::default()
            },
            move |p| async move {
                output.borrow_mut().push(2);
                low.set_priority(5)?;
                p.frame().await;
                output.borrow_mut().push(2);
                Ok(())
            },
        )
        .unwrap();
    engine.tick().unwrap();
    engine.tick().unwrap();
    assert_eq!(*order.borrow(), [2, 1, 1, 2]);
}
