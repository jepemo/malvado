// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error,
    image::Image,
    window::{Color, Window},
};
use std::rc::Rc;

fn main() -> Result<(), Error> {
    let window = Rc::new(Window::open(160, 120, "Malvado Image")?);
    // The image keeps its window alive until the image is dropped.
    let image = Image::load(window.clone(), "tile.png")?;
    let (width, height) = image.size();
    println!("image: {width}x{height}");
    for _ in 0..120 {
        if window.should_close() {
            break;
        }
        let frame = window.frame(Color {
            r: 0,
            g: 0,
            b: 0,
            a: 255,
        })?;
        frame.image(&image, (32, 32))?;
    }
    Ok(())
}
