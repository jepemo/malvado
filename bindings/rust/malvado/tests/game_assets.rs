// Malvado 2; SPDX-License-Identifier: Apache-2.0

#[cfg(feature = "assets")]
mod common;

use malvado::{Error, Game};
use std::path::Path;

#[test]
fn game_directories_are_independent() -> Result<(), Error> {
    let root = std::env::temp_dir().join("game assets ñ");
    let mut first = Game::with_game_dir(&root)?;
    let second = Game::with_game_dir(&root)?;
    assert_eq!(first.game_dir()?, root);
    first.set_assets_dir("one")?;
    first.set_assets_dir("two")?;
    assert_eq!(second.assets_dir()?, root);
    assert_eq!(first.set_assets_dir(""), Err(Error::InvalidArgument));
    assert_eq!(
        first.set_assets_dir("bad\0path"),
        Err(Error::InvalidArgument)
    );
    assert_eq!(
        first.resolve_asset_path("sprite.png")?,
        root.join("two/sprite.png")
    );
    assert_eq!(first.resolve_asset_path(&root)?, root);
    #[cfg(unix)]
    {
        use std::os::unix::ffi::OsStringExt;
        let filename = std::ffi::OsString::from_vec(b"tile-\xff.png".to_vec());
        assert_eq!(
            first.resolve_asset_path(&filename)?,
            root.join("two").join(filename)
        );
    }
    first.reset_assets_dir()?;
    assert_eq!(first.assets_dir()?, root);
    assert!(Path::new(&Game::new()?.game_dir()?).is_absolute());
    Ok(())
}

#[cfg(feature = "assets")]
#[test]
fn metadata_loads_follow_the_game_directory() -> Result<(), Error> {
    let source_dir = common::root().join("examples/aseprite");
    let game = Game::with_game_dir(&source_dir)?;
    let library = game.load_asset("sheet.json", None)?;
    assert_eq!(library.source().format(), malvado::asset::Format::Aseprite);
    assert_eq!(library.source().path(), source_dir.join("sheet.json"));
    Ok(())
}
