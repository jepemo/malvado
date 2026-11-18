# Android App Bundle export

The installed CLI can build a native base-module AAB with the existing game
runtime and assets. AAB uses a protobuf manifest and resource table. It is a
separate format from APK; renaming an APK does not create a bundle.

Install the pinned authoring tools in a new directory:

```sh
python3 tools/android_bundle.py --output build-android-bundle-tools
```

The installer supports macOS arm64/x86_64 and Linux x86_64. It verifies
SHA-256 values before extraction
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


## Signing and execution checks

The same command supports C and Rust projects with their full Release target
SDKs. C requires the pinned NDK/CMake compiler path; Rust also requires its
normal staticlib bridge and target ABI probe runner. All language paths use the
same base-module builder and asset index.

Add the existing external signing options to sign an AAB:

```sh
malvado export --target android-arm64 --format aab --release \
  --runtime-sdk /absolute/path/release-runtime --android-sdk "$ANDROID_SDK_ROOT" \
  --bundle-tools /absolute/path/build-android-bundle-tools \
  --output ../dist/game.aab --id org.example.game \
  --keystore /absolute/path/keys/game.p12 --key-alias game \
  --store-password-env GAME_STORE_PASSWORD
```

AAB uses `jarsigner`, not `apksigner`. Password values stay in their named
nonempty environment variables; the key must be outside the project. Strict
verification uses the selected keystore and alias and requires an explicit
verified-signature result. It rejects unsigned results even when jarsigner
returns status zero. Without a keystore, the AAB remains unsigned. Signing and
validation do not publish the bundle or establish production key custody.

For local automated execution, add `--format aab --bundle-tools PATH --release`
to `test-android-export`, `test-android-c-export` or `test-android-rust-export`.
The C/Rust verifiers also need disposable external signing credentials. The Lua
verifier creates its own disposable key. Each verifier checks the AAB, generates
a universal APK using bundletool and runs the ordinary language/lifetime checks.
Temporary password files have mode 0600 and are removed with their owned staging.
No password values are passed in command arguments. Release APKs must remain
non-debuggable and every installed package must be removed.

C, both Lua entries and Rust passed this arm64/API 34 path with assets, pixels,
process errors, contained Rust panic and actual cleanup evidence. Lua also
checked unsigned AABs separately. Compiler/entry failures remain errors before
publication. Linux tool archives are pinned and verified, but Linux authoring
and hosted/candidate execution still need acceptance. Other Android export ABIs
remain unavailable. Physical devices and store eligibility remain separate.
