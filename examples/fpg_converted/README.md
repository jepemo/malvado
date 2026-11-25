# Converted FPG graphics

From this directory, build the `assets` folder once and run the example:

```sh
../../build/malvado fpg-export --transparent-zero ../fpg/graphics.fpg assets
../../build/malvado run
```

After conversion, you can run this directory or its absolute entry script
from any working directory with the installed CLI.

The converter needs a new destination folder. The example calls `load_asset()` for the manifest, then uses `graph()` to
select the original IDs and pivots from the exported PNG images. Press
Escape to exit. The fixture is original and uses the project license. See the
[FPG contract](../../docs/api/fpg.md) for format limits.
