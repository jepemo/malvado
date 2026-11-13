// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error, Game,
    animation::{Animation, Frame},
};
const FRAMES: [Frame; 2] = [
    Frame {
        graph_id: 3,
        duration_us: 100_000,
    },
    Frame {
        graph_id: 9,
        duration_us: 200_000,
    },
];
#[test]
fn boundaries_looping_and_held_end() {
    for looping in [false, true] {
        let mut frames = FRAMES;
        let animation = Animation::new(&frames, looping).unwrap();
        assert!(!animation.is_empty());
        assert_eq!(animation.len(), FRAMES.len());
        frames[0].graph_id = 100;
        assert_eq!(animation.sample(0).graph_id, 3);
        assert_ne!(animation.sample(0).graph_id, frames[0].graph_id);
        assert_eq!(animation.sample(99_999).index, 0);
        assert_eq!(animation.sample(100_000).graph_id, 9);
        assert_eq!(animation.sample(299_999).index, 1);
        assert_eq!(animation.sample(300_000).index, if looping { 0 } else { 1 });
        assert_eq!(
            animation.sample(u64::MAX).index,
            if looping && u64::MAX % 300_000 < 100_000 {
                0
            } else {
                1
            }
        );
    }
}
#[test]
fn invalid_definitions() {
    assert!(matches!(
        Animation::new(&[], false),
        Err(Error::InvalidArgument)
    ));
    for (id, duration) in [(0, 1), (-1, 1), (1, 0), (1, 3_600_000_001)] {
        assert!(matches!(
            Animation::new(
                &[Frame {
                    graph_id: id,
                    duration_us: duration
                }],
                true
            ),
            Err(Error::InvalidArgument)
        ));
    }
    assert!(Animation::new(&vec![FRAMES[0]; 8193], true).is_err());
    assert!(Animation::new(&vec![FRAMES[0]; 8192], true).is_ok());
}
#[test]
fn fixed_ticks_rate_reset_and_independent_playback() {
    let animation = Animation::new(
        &[Frame {
            graph_id: 1,
            duration_us: 2_000_003,
        }],
        true,
    )
    .unwrap();
    let mut playback = animation.playback();
    let other = animation.playback();
    for _ in 0..60 {
        playback.advance(60).unwrap();
    }
    assert_eq!(playback.elapsed_us(), 1_000_000);
    for rate in [0, 1_000_001, 30] {
        assert_eq!(playback.advance(rate), Err(Error::InvalidArgument));
    }
    assert_eq!(playback.elapsed_us(), 1_000_000);
    assert_eq!(other.elapsed_us(), 0);
    playback.reset();
    playback.advance(30).unwrap();
    assert_eq!(playback.elapsed_us(), 33_333);
}
#[test]
fn stopped_and_looping_playback_and_native_resource_ownership() {
    let game = Game::new().unwrap();
    let animation = game
        .resource(Animation::new(&FRAMES, false).unwrap())
        .unwrap();
    drop(game);
    let mut playback = animation.playback();
    for _ in 0..100 {
        playback.advance(60).unwrap();
    }
    assert_eq!(playback.elapsed_us(), 300_000);
    assert_eq!(playback.sample().graph_id, 9);
    let looping = Animation::new(&FRAMES, true).unwrap();
    let mut playback = looping.playback();
    for _ in 0..60 {
        playback.advance(60).unwrap();
    }
    assert_eq!(playback.elapsed_us(), 100_000);
}
