# Rust audio

Enable `graphics` and install the native graphics SDK. This feature supplies
raylib platform services, including audio. Audio does not require an open window.
Default core-only builds do not link audio or raylib.

```rust,no_run
use malvado::{Error, audio::{Audio, Sound}};
use std::rc::Rc;

let audio = Rc::new(Audio::open()?);
let sound = Sound::load(audio.clone(), "jump.wav")?;
sound.set_volume(0.5)?;
sound.play()?;
// In the host loop, after each simulation step:
audio.update();
# Ok::<(), Error>(())
```

## Device and ownership

Open one `Audio` on the application main thread. A second safe owner returns
`Busy`. Device failure returns `Platform`. The device is separate from `Window`
and `Engine`. It does not tick processes, poll events, or install a scheduler.
Do not open, close, or operate the same native device through another host while
Rust owns it. Mixed-language device ownership is not supported.

Keep an `Rc<Audio>` in the host. Loaded sounds keep the service alive. Playing
and paused uses retain native instances even if a process releases its resource.
`update()` releases completed sound uses. `stop()` releases that instance's
playback retention immediately. Paused uses stay retained until stop, resume
and completion, or service shutdown. Repeated play restarts one instance and
does not add another retention.

Dropping the final service owner stops and destroys outstanding instances before
closing the device. Shutdown does not wait for a sound to finish. Retained native
uses hold the device, but do not retain the service itself; there is no ownership
cycle. All device, sound, control, and cleanup operations stay on the owner thread.

## Sounds

`Sound::load` accepts WAV files. Each load owns a native instance.
`try_clone()` shares decoded sample data with an independent native playback
instance. It can fail. Sound has no `Clone` implementation that silently shares
playback controls. A clone can outlive the original.

`play()` starts or restarts; `pause()`, `resume()`, and `stop()` control that
instance. `playing()` is false for paused and stopped sounds. These operations
use native behavior; they do not expose a second playback state machine.
Volume is finite and in `0..=1`; invalid values return `InvalidArgument` without
changing volume. NUL in a path is invalid. Missing files, unsupported formats,
load/clone failures and native action failures return `Platform`. Native load
failures do not distinguish allocation errors from file errors.

## Verification

`mise run test-rust-graphics` runs the main-thread audio fixture in Debug and
Release, checks all used installed C signatures, and runs a relocated offline
consumer. It checks invalid paths and volume, restart, pause/resume/stop,
independent clone ownership, completion, process resource cleanup, final device
release and reopen. Linux uses the existing ALSA null configuration. macOS uses
Core Audio. `mise run test-rust-graphics-sanitize` checks native ASan/UBSan paths;
its existing host leak-detection limits still apply.

Silent fixtures verify API and ownership behavior. These tests do not certify
physical output, latency, device hotplug, or subjective sound quality.

Sound acceptance: Debug/Release, installed consumer and native ASan/UBSan
pass on macOS. Linux null-device execution is included in the graphics CI job;
its result must be recorded before final A7.6 closure.
