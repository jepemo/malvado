// Malvado 2; SPDX-License-Identifier: Apache-2.0
#![cfg(not(miri))]
use malvado::{EndReason, Error, Game, ProcessContext, Relation, Signal, SpawnOptions, Status};
use std::cell::{Cell, RefCell};
use std::future::pending;
use std::rc::Rc;

#[derive(Clone)]
struct Count(Rc<Cell<usize>>);
impl Drop for Count {
    fn drop(&mut self) {
        self.0.set(self.0.get() + 1);
    }
}

#[test]
fn queries_and_signals_preserve_ownership_and_creation_relations() {
    let children = Rc::new(RefCell::new(Vec::new()));
    let out = children.clone();
    let mut game = Game::new().unwrap();
    let root = game
        .spawn(SpawnOptions::default(), move |p| async move {
            for attached in [true, false] {
                out.borrow_mut().push(p.spawn(
                    SpawnOptions {
                        attached,
                        type_id: 7,
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
    game.tick().unwrap();
    let children = children.borrow();
    assert_eq!(game.find(7).unwrap().len(), 2);
    assert!(root.children(Relation::Ownership, 0).unwrap()[0] == children[0]);
    assert_eq!(root.children(Relation::Ownership, 8).unwrap().len(), 0);
    let created = root.descendants(Relation::Creation, 7).unwrap();
    assert!(created[0] == children[0] && created[1] == children[1]);
    game.signal_tree(&root, Signal::Freeze, Relation::Ownership)
        .unwrap();
    assert_eq!(children[0].snapshot().unwrap().status, Some(Status::Frozen));
    assert_eq!(
        children[1].snapshot().unwrap().status,
        Some(Status::Running)
    );
    game.signal_tree(&root, Signal::Wake, Relation::Ownership)
        .unwrap();
    game.tick().unwrap();
    assert_eq!(root.snapshot().unwrap().reason, Some(EndReason::Returned));
    assert_eq!(
        children[0].snapshot().unwrap().reason,
        Some(EndReason::Killed)
    );
    assert_eq!(
        root.children(Relation::Ownership, 0).err(),
        Some(Error::NotFound)
    );
    assert!(root.children(Relation::Creation, 0).unwrap()[0] == children[1]);
    let lineage = game.lineage().unwrap();
    assert_eq!(lineage.len(), 2);
    assert_eq!((lineage[0].id, lineage[0].alive), (root.id(), false));
    assert_eq!(lineage[1].creator_id, root.id());
    game.signal_tree(&root, Signal::Sleep, Relation::Creation)
        .unwrap();
    assert_eq!(
        children[1].snapshot().unwrap().status,
        Some(Status::Sleeping)
    );
    assert_eq!(game.find(7).unwrap().len(), 1);
    game.signal_tree(&root, Signal::Kill, Relation::Creation)
        .unwrap();
    game.tick().unwrap();
    assert_eq!(game.process_count(), 0);
    assert!(game.lineage().unwrap().is_empty());
    assert!(root.descendants(Relation::Creation, 0).unwrap().is_empty());
}

#[test]
fn nested_creation_and_ownership_trees_have_distinct_kill_boundaries() {
    let records = Rc::new(RefCell::new(Vec::new()));
    let output = records.clone();
    let mut game = Game::new().unwrap();
    let root = game
        .spawn(SpawnOptions::default(), move |p| async move {
            let child = p.spawn(
                SpawnOptions {
                    attached: false,
                    ..Default::default()
                },
                move |p| async move {
                    output
                        .borrow_mut()
                        .push(p.spawn(SpawnOptions::default(), |_| async {
                            pending::<()>().await;
                            Ok(())
                        })?);
                    pending::<()>().await;
                    Ok(())
                },
            )?;
            let _keep_child = child;
            pending::<()>().await;
            Ok(())
        })
        .unwrap();
    game.tick().unwrap();
    game.tick().unwrap();
    assert_eq!(root.descendants(Relation::Ownership, 0).unwrap().len(), 0);
    assert_eq!(root.descendants(Relation::Creation, 0).unwrap().len(), 2);
    assert_eq!(root.children(Relation::Creation, 0).unwrap().len(), 1);
    game.signal_tree(&root, Signal::Kill, Relation::Ownership)
        .unwrap();
    game.tick().unwrap();
    assert_eq!(game.process_count(), 2);
    game.signal_tree(&root, Signal::Kill, Relation::Creation)
        .unwrap();
    game.tick().unwrap();
    assert_eq!(game.process_count(), 0);
    assert_eq!(
        records.borrow()[0].snapshot().unwrap().reason,
        Some(EndReason::Killed)
    );
}

#[test]
fn owner_and_retained_resource_references_are_independent() {
    let count = Rc::new(Cell::new(0));
    let game = Game::new().unwrap();
    let resource = game.resource(Count(count.clone())).unwrap();
    let retained = resource.try_clone().unwrap();
    drop(resource);
    assert_eq!(count.get(), 0);
    drop(game);
    assert_eq!(count.get(), 0);
    let later = retained.try_clone().unwrap();
    assert_eq!(retained.0.get(), 0);
    assert_eq!(retained.transfer_to(None), Err(Error::Closed));
    drop(retained);
    assert_eq!(count.get(), 0);
    drop(later);
    assert_eq!(count.get(), 1);
}

#[test]
fn process_resources_release_after_poll_and_can_outlive_the_process() {
    let count = Rc::new(Cell::new(0));
    let retained = Rc::new(RefCell::new(None));
    let (out, drops) = (retained.clone(), count.clone());
    let mut game = Game::new().unwrap();
    game.spawn(SpawnOptions::default(), move |p| async move {
        let resource = p.resource(Count(drops.clone()))?;
        *out.borrow_mut() = Some(resource.try_clone()?);
        assert!(p.current().kill()?);
        assert_eq!(drops.get(), 0);
        p.frame().await;
        unreachable!();
    })
    .unwrap();
    game.tick().unwrap();
    assert_eq!(count.get(), 0);
    drop(retained.borrow_mut().take());
    assert_eq!(count.get(), 1);
    let drops = count.clone();
    game.spawn(SpawnOptions::default(), move |p| async move {
        drop(p.resource(Count(drops.clone()))?);
        assert_eq!(drops.get(), 1); // owner reference still owns the value
        Ok(())
    })
    .unwrap();
    game.tick().unwrap();
    assert_eq!(count.get(), 2);
}

#[test]
fn transfer_rejects_foreign_stale_and_closed_owners_without_mutation() {
    let count = Rc::new(Cell::new(0));
    let mut game = Game::new().unwrap();
    let foreign = Game::new().unwrap();
    let dead = game
        .spawn(SpawnOptions::default(), |_| async { Ok(()) })
        .unwrap();
    let owner = game
        .spawn(SpawnOptions::default(), |_| async {
            pending::<()>().await;
            Ok(())
        })
        .unwrap();
    let other = foreign
        .spawn(SpawnOptions::default(), |_| async { Ok(()) })
        .unwrap();
    let resource = game.resource(Count(count.clone())).unwrap();
    resource.transfer_to(Some(&owner)).unwrap();
    assert_eq!(
        resource.transfer_to(Some(&other)),
        Err(Error::ForeignContext)
    );
    game.tick().unwrap();
    assert_eq!(resource.transfer_to(Some(&dead)), Err(Error::NotFound));
    drop(resource);
    assert_eq!(count.get(), 0);
    owner.kill().unwrap();
    game.tick().unwrap();
    assert_eq!(count.get(), 1);
}

#[test]
fn promoting_resource_to_context_preserves_it_after_original_owner_ends() {
    let count = Rc::new(Cell::new(0));
    let drops = count.clone();
    let mut game = Game::new().unwrap();
    game.spawn(SpawnOptions::default(), move |p| async move {
        let resource = p.resource(Count(drops))?;
        resource.transfer_to(None)?;
        Ok(())
    })
    .unwrap();
    game.tick().unwrap();
    assert_eq!(count.get(), 0);
    drop(game);
    assert_eq!(count.get(), 1);
}

#[test]
fn detached_spawn_from_destructor_keeps_the_native_historical_creator() {
    struct SpawnOnDrop {
        process: ProcessContext,
        output: Rc<RefCell<Option<malvado::Handle>>>,
    }
    impl Drop for SpawnOnDrop {
        fn drop(&mut self) {
            *self.output.borrow_mut() = Some(
                self.process
                    .spawn(
                        SpawnOptions {
                            attached: false,
                            ..Default::default()
                        },
                        |_| async { Ok(()) },
                    )
                    .unwrap(),
            );
        }
    }
    let spawned = Rc::new(RefCell::new(None));
    let out = spawned.clone();
    let mut game = Game::new().unwrap();
    let root = game
        .spawn(SpawnOptions::default(), move |p| async move {
            let _cleanup = SpawnOnDrop {
                process: p.clone(),
                output: out,
            };
            pending::<()>().await;
            Ok(())
        })
        .unwrap();
    game.tick().unwrap();
    assert!(root.kill().unwrap());
    game.tick().unwrap();
    assert_eq!(root.snapshot().unwrap().reason, Some(EndReason::Killed));
    let child = spawned.borrow();
    let child = child.as_ref().unwrap();
    assert_eq!(child.snapshot().unwrap().creator_id, root.id());
    assert_eq!(child.snapshot().unwrap().owner_id, 0);
    assert!(child.snapshot().unwrap().alive);
    game.tick().unwrap();
    assert_eq!(child.snapshot().unwrap().reason, Some(EndReason::Returned));
}

#[test]
fn resource_destructor_panics_are_contained_at_process_and_context_cleanup() {
    struct BadDrop(Rc<Cell<usize>>);
    impl Drop for BadDrop {
        fn drop(&mut self) {
            self.0.set(self.0.get() + 1);
            panic!("resource destructor");
        }
    }
    let count = Rc::new(Cell::new(0));
    let drops = count.clone();
    let mut game = Game::new().unwrap();
    game.spawn(SpawnOptions::default(), move |p| async move {
        drop(p.resource(BadDrop(drops))?);
        Ok(())
    })
    .unwrap();
    game.tick().unwrap();
    assert_eq!(count.get(), 1);
    drop(game.resource(BadDrop(count.clone())).unwrap());
    drop(game);
    assert_eq!(count.get(), 2);
}

#[test]
fn factory_panic_is_contained_and_native_state_remains_usable() {
    let mut game = Game::new().unwrap();
    let handle = game
        .spawn(
            SpawnOptions::default(),
            |_| -> std::future::Ready<Result<(), Error>> { panic!("factory") },
        )
        .unwrap();
    assert_eq!(game.tick(), Err(Error::Process));
    assert_eq!(handle.snapshot().unwrap().reason, Some(EndReason::Error));
    game.spawn(SpawnOptions::default(), |_| async { Ok(()) })
        .unwrap();
    game.tick().unwrap();
    assert_eq!(game.process_count(), 0);
}

#[test]
fn repeated_spawn_cancel_and_shutdown_drop_every_state_and_resource_once() {
    let count = Rc::new(Cell::new(0));
    let resources = Rc::new(Cell::new(0));
    for _ in 0..10 {
        let mut game = Game::new().unwrap();
        let mut handles = Vec::new();
        for index in 0..256 {
            let guard = Count(count.clone());
            let drops = resources.clone();
            handles.push(
                game.spawn(SpawnOptions::default(), move |p| async move {
                    let _guard = guard;
                    let _resource = p.resource(Count(drops))?;
                    if index % 2 == 0 {
                        p.current().kill()?;
                    }
                    p.frame().await;
                    Ok(())
                })
                .unwrap(),
            );
        }
        game.tick().unwrap();
        for handle in &handles {
            let _ = handle.kill().unwrap();
        }
        drop(game);
        assert!(handles.iter().all(|h| !h.snapshot().unwrap().alive));
    }
    assert_eq!(count.get(), 2560);
    assert_eq!(resources.get(), 2560);
}

#[test]
fn lifecycle_trace_matches_c_and_lua() {
    let children = Rc::new(RefCell::new(Vec::new()));
    let out = children.clone();
    let mut game = Game::new().unwrap();
    let root = game
        .spawn(SpawnOptions::default(), move |p| async move {
            for attached in [true, false] {
                out.borrow_mut().push(p.spawn(
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
    game.tick().unwrap();
    let mut trace = format!(
        "owned={} created={}\n",
        root.children(Relation::Ownership, 0).unwrap().len(),
        root.children(Relation::Creation, 0).unwrap().len()
    );
    let children = children.borrow();
    game.signal_tree(&root, Signal::Freeze, Relation::Ownership)
        .unwrap();
    assert_eq!(children[0].snapshot().unwrap().status, Some(Status::Frozen));
    assert_eq!(
        children[1].snapshot().unwrap().status,
        Some(Status::Running)
    );
    trace.push_str("ownership: frozen running\n");
    game.signal_tree(&root, Signal::Wake, Relation::Creation)
        .unwrap();
    game.tick().unwrap();
    assert_eq!(root.snapshot().unwrap().reason, Some(EndReason::Returned));
    assert_eq!(
        children[0].snapshot().unwrap().reason,
        Some(EndReason::Killed)
    );
    trace.push_str("parent: returned attached: killed\n");
    trace.push_str(&format!(
        "dead creator: {} descendant\n",
        root.descendants(Relation::Creation, 0).unwrap().len()
    ));
    game.signal_tree(&root, Signal::Freeze, Relation::Creation)
        .unwrap();
    assert_eq!(children[1].snapshot().unwrap().status, Some(Status::Frozen));
    trace.push_str("creation: frozen\n");
    game.signal_tree(&root, Signal::Kill, Relation::Creation)
        .unwrap();
    game.tick().unwrap();
    trace.push_str(&format!("remaining={}\n", game.process_count()));
    assert_eq!(trace, include_str!("fixtures/lifecycle.txt"));
}

#[test]
fn retained_resource_can_acquire_a_new_owner_after_the_old_one_ends() {
    let count = Rc::new(Cell::new(0));
    let retained = Rc::new(RefCell::new(None));
    let (out, drops) = (retained.clone(), count.clone());
    let mut game = Game::new().unwrap();
    game.spawn(SpawnOptions::default(), move |p| async move {
        *out.borrow_mut() = Some(p.resource(Count(drops))?);
        Ok(())
    })
    .unwrap();
    let next = game
        .spawn(SpawnOptions::default(), |_| async {
            pending::<()>().await;
            Ok(())
        })
        .unwrap();
    game.tick().unwrap();
    let resource = retained.borrow_mut().take().unwrap();
    resource.transfer_to(Some(&next)).unwrap();
    drop(resource);
    assert_eq!(count.get(), 0);
    next.kill().unwrap();
    game.tick().unwrap();
    assert_eq!(count.get(), 1);
}

#[test]
fn native_resources_release_once_for_return_error_panic_and_cancellation() {
    for mode in 0..3 {
        let count = Rc::new(Cell::new(0));
        let drops = count.clone();
        let mut game = Game::new().unwrap();
        game.spawn(SpawnOptions::default(), move |p| async move {
            let _value = p.resource(Count(drops))?;
            match mode {
                0 => Err(Error::Process),
                1 => panic!("process with native resource"),
                _ => {
                    p.current().kill()?;
                    p.frame().await;
                    Ok(())
                }
            }
        })
        .unwrap();
        let result = game.tick();
        assert_eq!(
            result,
            if mode == 2 {
                Ok(())
            } else {
                Err(Error::Process)
            }
        );
        assert_eq!(count.get(), 1);
        drop(game);
        assert_eq!(count.get(), 1);
    }
}

#[test]
fn failed_registration_to_a_dead_process_releases_the_untransferred_value() {
    let saved = Rc::new(RefCell::new(None));
    let out = saved.clone();
    let mut game = Game::new().unwrap();
    game.spawn(SpawnOptions::default(), move |p| async move {
        *out.borrow_mut() = Some(p);
        Ok(())
    })
    .unwrap();
    game.tick().unwrap();
    let count = Rc::new(Cell::new(0));
    let token = saved.borrow();
    assert!(matches!(
        token.as_ref().unwrap().resource(Count(count.clone())),
        Err(Error::NotFound)
    ));
    assert_eq!(count.get(), 1);
    drop(game);
    assert_eq!(count.get(), 1);
}
