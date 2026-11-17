# Android App Bundle export

The installed CLI can build a native base-module AAB with the existing game
runtime and assets. AAB uses a protobuf manifest and resource table. It is a
separate format from APK; renaming an APK does not create a bundle.

Install the pinned authoring tools in a new directory:

```sh
python3 tools/android_bundle.py --output build-android-bundle-tools
```

The installer selects macOS or Linux, verifies SHA-256 values before extraction
or execution, preserves existing destinations and removes its owned output on
failure. It installs bundletool 1.18.3 and Maven AAPT2 8.2.2-10154469 with the
AAPT2 notice and provenance. Export rechecks both archives and the extracted
AAPT2 executable. Tool download is explicit; export does not download tools.

From a Lua project, generate an unsigned release bundle:

```sh
malvado export --target android-arm64 --format aab --release \
  --runtime-sdk /absolute/path/release-runtime \
  --android-sdk "$ANDROID_SDK_ROOT" \
  --bundle-tools /absolute/path/build-android-bundle-tools \
  --output ../dist/game.aab --id org.example.game
```

The existing Release runtime checks and manifest flags apply. AAB requires
Java and the selected bundle tools. It compiles the manifest with AAPT2's
`--proto-format`, writes the native library and assets into a base module,
builds with bundletool and runs `bundletool validate` before exclusive output
publication. A failed operation publishes no bundle. Existing outputs remain
unchanged. Debug AAB generation is unavailable.

This follows Android's [command-line bundle build process](https://developer.android.com/build/building-cmdline)
and uses the official [bundletool release](https://github.com/google/bundletool/releases/tag/1.18.3).
No store publication is part of export. Format validation does not establish
store eligibility or device behavior; APKs derived from the bundle need their
own execution checks.
