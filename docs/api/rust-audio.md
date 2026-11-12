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
// Regularly in the host loop, including frames without a simulation tick:
audio.update()?;
# Ok::<(), Error>(())
```

## Device and ownership

Open one `Audio` on the application main thread. A second safe owner returns
`Busy`. Device failure returns `Platform`. The device is separate from `Window`
and `Engine`. It does not tick processes, poll events, or install a scheduler.
Do not open or close the same native device through another owner while Rust
owns it. Same-thread native borrowing shares pause state and refill; it does not
create another device owner. Mixed-language device ownership is not supported.

Keep an `Rc<Audio>` in the host. Loaded sounds and music keep the service alive. Playing
and paused uses retain native instances even if a process releases its resource.
`update()` releases completed sound uses. `stop()` releases that instance's
playback retention immediately. Paused uses stay retained until stop, resume
and completion, or service shutdown. Repeated play does not add another
retention. Sound playback restarts when played again.

Dropping the final service owner stops and destroys outstanding instances before
closing the device. Shutdown does not wait for a sound to finish. Retained native
uses hold the device, but do not retain the service itself; there is no ownership
cycle. All device, sound, music, control, and cleanup operations stay on the owner thread.

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

## Music streams

`Music::load(audio, path, looping)` opens a WAV or OGG stream. The Boolean loop
setting is fixed at load time. Each load owns its decoder and playback state;
streams cannot be cloned. Controls and volume follow the sound contract.
`play()` starts native stream playback. To restart from the beginning, call
`stop()` before `play()`; play alone does not rewind the decoder. Use `resume()`
to keep a paused position. `playing()` is false while paused. `paused()` reads
the native pause state for sounds and music; idle instances return false.

Call `Audio::update()` regularly on the owner thread. It refills active streams
through the shared native device pass and releases completed Rust uses. It also
refills native streams loaded outside the Rust root vector. Paused uses stay
retained without refill. The host must continue updates even after the process
that loaded a playing stream ends. Looping streams keep playing until stopped
or service shutdown. Updates return `Platform` if a native update fails.
There is no automatic integration with window frames or process ticks.

```rust,no_run
use malvado::{Error, audio::{Audio, Music}};
use std::rc::Rc;
let audio = Rc::new(Audio::open()?);
let music = Music::load(audio.clone(), "theme.ogg", true)?;
music.play()?;
// Call regularly in the host loop, including while the music has no process owner.
audio.update()?;
music.stop()?;
# Ok::<(), Error>(())
```

## Verification

`mise run test-rust-graphics` runs the main-thread audio fixture in Debug and
Release, checks all used installed C signatures, and runs a relocated offline
consumer. It checks invalid paths and volume, restart, pause/resume/stop,
independent clone ownership, completion, process resource cleanup, final device
release and reopen. Linux uses the existing ALSA null configuration. macOS uses
Core Audio. `mise run test-rust-graphics-sanitize` checks native ASan/UBSan paths;
its existing host leak-detection limits still apply.

The music fixture checks both WAV and OGG, loop continuation beyond three
fixture lengths, non-looping completion and replay, independent loads,
pause/update/resume/stop, process owner removal, orphaned stream updates,
paused/looping shutdown, device retention and reopen. The relocated consumer
also loads and controls a stream from its own directory.

A private ownership test also checks exact Rc retention counts, paused survival,
completion/stop release and device teardown without a reference cycle.

Silent fixtures verify API and ownership behavior. These tests do not certify
physical output, latency, device hotplug, or subjective sound quality.

Audio acceptance: full host CI passes on implementation commit `8b76938`.
The [Linux graphics job](https://github.com/jepemo/malvado/actions/runs/42984841966/job/130403450713)
passes Debug/Release, null-device playback, the relocated consumer and native
ASan/UBSan with leak detection enabled. This includes the WAV cleanup regression
below. The existing graphics-provider filters are unchanged. Local macOS checks
also pass; its existing leak-detection limitation remains explicit.

Linux sanitizer regression: the raylib 5.5 WAV decoder allocation survived
`UnloadMusicStream`. The native adapter now frees that context through raylib's
allocator after decoder shutdown, including wrapper allocation and invalid-stream
failure paths. OGG cleanup stays in raylib. A version assertion requires review
on a provider upgrade. Empty WAV rejection is covered by C and Rust fixtures.
No third-party source or leak suppression was changed.

The [shared maintenance contract](audio-maintenance.md#rust-service) defines native
pause queries, refill, binding playback roots and device ownership.
