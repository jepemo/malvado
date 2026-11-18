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

This run covers the terminal workflow on the recorded Mac. Linux and Windows
manual authoring runs, clean-system runs and physical input and audio checks
remain in the [beta.1 checklist](../versions/2.0.0-beta.1.md).
