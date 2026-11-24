// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error,
    window::{Color, Window},
};

fn show_window(window: &Window) -> Result<(), Error> {
    for _ in 0..60 {
        if window.should_close() {
            break;
        }
        drop(window.frame(Color {
            r: 20,
            g: 30,
            b: 40,
            a: 255,
        })?);
    }
    Ok(())
}

fn print_size(window: &Window) {
    let (width, height) = window.size();
    println!("window: {width}x{height}");
}

fn main() -> Result<(), Error> {
    let window = Window::open(320, 240, "Malvado Window Resize")?;
    print_size(&window);
    show_window(&window)?;
    if !window.should_close() {
        // Each frame guard has ended before the request.
        window.set_size(640, 360)?;
        show_window(&window)?;
        print_size(&window);
    }
    Ok(())
}
