# Beta.1 authoring verification

## macOS arm64

The local command-line walkthrough passed on macOS 26.6.2 (25G83) with
AppleClang 21.0.0, CMake 3.31.6 and Rust 1.99.1. The beta.1 source revision
was `0f3633b9495654948c0f43167217887da0e06493`. The old SDK was built
from the accepted alpha.8 revision `2a20b7ae9b8cc03a21a92bd2aee29e185789f6f1`.

| Step | Verified result |
| --- | --- |
| Install alpha.8 in a new managed prefix with spaces | The client reported `2.0.0-alpha.8`. |
| Create C, Lua and Rust projects with the installed CLI | All three projects passed `malvado check` with the sources from the [process lesson](../../examples/features/processes/README.md). |
| Install beta.1 over the same prefix | The client reported `2.0.0-beta.1`, and `doctor` found the graphics backend, export launchers and required notices. |
| Check, build, run and export | All three projects passed `check`, and C and Rust passed `build`. The three runs produced the same process trace. Folder and `.app` export succeeded for each language. The app bundles preserved the dependency notice. |
| Uninstall the managed SDK | The prefix was removed. The project files stayed intact. All six exported players produced the same trace with the SDK absent and authoring tools off PATH. |

### Interactive playback

On macOS 26.6.2 (25G83) arm64, the C, Lua and Rust starter games were
exported as app bundles with a local SDK built from
`0301d5defd1f5ffe4b37621a8992b9a48c157efd`. The SDK was moved away from
the original prefix before launch. Each game opened, displayed
“Hello, Malvado!” and exited with Escape.

The exported Lua demo opened the sprites page when clicked and rendered
the character and cat. Escape returned to the menu. Down and Return opened
the keyboard page, which displayed the typed character. The exported audio
demo showed the loop state changing on and off with Space.

These checks used the development Mac. A clean macOS system, physical
controllers and hotplug, and audible sound and music remain unverified.

## Linux x86_64

The command-line walkthrough passed in an Ubuntu 24.04.5 LTS x86_64 authoring
container on macOS arm64. It used GCC 13.3.0, CMake 3.28.3, Rust 1.99.1
and Xvfb with software OpenGL. The beta.1 SDK came from the
[Linux package job](https://github.com/jepemo/malvado/actions/runs/44320665075)
at revision `d502125ece681074e9a302798c9793a68fdca603`.

| Step | Verified result |
| --- | --- |
| Verify the checksums and extract alpha.8 and beta.1 into separate prefixes with spaces | Both archives matched their checksums. The CLI reported the expected version from each prefix. |
| Create C, Lua and Rust games with alpha.8 | All three projects passed `check` with the process lesson sources. |
| Switch to the beta.1 SDK and update the Rust manifest path | `doctor` found graphics, launchers and notices. The original projects passed `check` and `run`. C and Rust passed `build`. |
| Export the process and image lessons | All six exports succeeded on the container's local filesystem. The three image exports preserved the shared PNG. |
| Remove both SDK prefixes | The CLI was absent from PATH and the projects were still present. |
| Run the six exports in a separate Ubuntu container | C, Lua and Rust produced the same process trace. The image lessons opened under Xvfb and reported `image: 32x32`. The player container had no SDK, C compiler, CMake, Lua executable, Cargo or rustc. |

The Docker Desktop bind mount rejected the exporter's exclusive rename.
Publishing to the container's local filesystem and copying completed
exports to the mount worked.

The Linux run used a virtual display and an emulated CPU.
[2.0.0](../versions/2.0.0.md) tracks the remaining clean-system and physical
input and audio checks for macOS and Linux. The manual Windows authoring
walkthrough moves to [2.1.0](../versions/2.1.0.md).
