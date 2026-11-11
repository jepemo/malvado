// Malvado 2; SPDX-License-Identifier: Apache-2.0
#![cfg(feature = "graphics")]
use malvado::{drawing::DrawState, window::Color};
#[test]
fn defaults_channels_and_independent_copies() {
    let mut state = DrawState::new().unwrap();
    assert_eq!(
        state.color(),
        Color {
            r: 255,
            g: 255,
            b: 255,
            a: 255
        }
    );
    assert_eq!(
        state.background(),
        Color {
            r: 0,
            g: 0,
            b: 0,
            a: 255
        }
    );
    let original = state;
    for r in 0..=255 {
        let color = Color {
            r,
            g: 255 - r,
            b: 17,
            a: r,
        };
        state.set_color(color);
        state.set_background((r, 255 - r, 17));
        assert_eq!(state.color(), color);
        assert_eq!(state.background(), Color { a: 255, ..color });
    }
    assert_eq!(original.color().g, 255);
    assert_eq!(original.background().r, 0);
}
