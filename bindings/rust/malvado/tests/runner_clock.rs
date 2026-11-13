// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Error, Game};
use std::{cell::Cell, rc::Rc};
#[test]
fn native_time_plans_do_not_poll_processes() {
    let mut game = Game::new().unwrap();
    let other = Game::new().unwrap();
    let count = Rc::new(Cell::new(0));
    let seen = count.clone();
    let _actor = game
        .spawn(Default::default(), move |p| async move {
            loop {
                seen.set(seen.get() + 1);
                p.frame().await;
            }
        })
        .unwrap();
    let fixture = include_str!("../../../../tests/fixtures/runner-clock.txt");
    let mut executed = 0;
    for row in fixture.lines() {
        let values: Vec<_> = row.split_whitespace().collect();
        let elapsed = values[0].parse().unwrap();
        let steps: u32 = values[1].parse().unwrap();
        let planned: u64 = values[2].parse().unwrap();
        let dropped: f64 = values[3].parse().unwrap();
        assert_eq!(game.advance_time(elapsed).unwrap(), steps);
        assert_eq!(count.get(), executed);
        assert_eq!(game.planned_ticks().unwrap(), planned);
        assert!((game.dropped_seconds().unwrap() - dropped).abs() < 1e-9);
        for _ in 0..steps {
            game.tick().unwrap();
            executed += 1;
        }
        assert_eq!(count.get(), executed);
    }
    assert_eq!(other.planned_ticks().unwrap(), 0);
    let before = (
        game.planned_ticks().unwrap(),
        game.dropped_seconds().unwrap(),
    );
    for invalid in [-1.0, f64::NAN, f64::INFINITY] {
        assert_eq!(game.advance_time(invalid), Err(Error::InvalidArgument));
        assert_eq!(
            (
                game.planned_ticks().unwrap(),
                game.dropped_seconds().unwrap()
            ),
            before
        );
    }
    game.tick().unwrap(); // Manual logical ticks remain independent of wall time.
    assert_eq!(game.planned_ticks().unwrap(), before.0);
    assert_eq!(count.get(), executed + 1);
}
#[test]
fn overflow_preserves_the_existing_plan() {
    let mut game = Game::new().unwrap();
    assert_eq!(game.advance_time(f64::MAX).unwrap(), 5);
    let before = (
        game.planned_ticks().unwrap(),
        game.dropped_seconds().unwrap(),
    );
    assert_eq!(game.advance_time(f64::MAX), Err(Error::Overflow));
    assert_eq!(
        (
            game.planned_ticks().unwrap(),
            game.dropped_seconds().unwrap()
        ),
        before
    );
}
