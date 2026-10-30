# Invalid legacy graphics fixture

`a.png` is a valid generated image. `b.png` is intentionally invalid. The directory loader must report an error after it encounters `b.png` and release the image it already loaded.
