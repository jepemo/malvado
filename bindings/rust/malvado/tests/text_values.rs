// Malvado 2; SPDX-License-Identifier: Apache-2.0
// Native FFI belongs in integration tests; --lib remains executable under Miri.
use malvado::{
    Error,
    graphic::TextAlignment,
    text::{Snapshot, Text},
};

#[test]
fn native_alignment_boundaries() {
    assert_eq!(
        TextAlignment::Left.aligned_x(i32::MAX, i32::MAX),
        Ok(i32::MAX)
    );
    assert_eq!(TextAlignment::Center.aligned_x(10, 5), Ok(8));
    assert_eq!(TextAlignment::Right.aligned_x(10, 5), Ok(5));
    assert_eq!(
        TextAlignment::Right.aligned_x(i32::MIN, i32::MAX),
        Ok(i32::MIN)
    );
    assert_eq!(TextAlignment::Center.aligned_x(i32::MIN, 0), Ok(i32::MIN));
    assert_eq!(
        TextAlignment::Right.aligned_x(1, -1),
        Err(Error::InvalidArgument)
    );
}

#[test]
fn copied_content_and_atomic_updates() {
    let input = String::from("Hola 🦀");
    let mut text = Text::new(&input, (i32::MIN, i32::MAX), TextAlignment::Center).unwrap();
    drop(input);
    assert_eq!(text.snapshot().value, "Hola 🦀");
    assert!(matches!(
        text.update("bad\0value", (0, 0), TextAlignment::Left),
        Err(Error::InvalidArgument)
    ));
    assert_eq!(text.snapshot().position, (i32::MIN, i32::MAX));
    assert_eq!(text.snapshot().alignment, TextAlignment::Center);
    text.update("", (1, 2), TextAlignment::Right).unwrap();
    assert_eq!(
        text.snapshot(),
        Snapshot {
            value: "",
            position: (1, 2),
            alignment: TextAlignment::Right
        }
    );
    assert!(matches!(
        Text::new("\0", (0, 0), TextAlignment::Left),
        Err(Error::InvalidArgument)
    ));
}
