# Play a sound

The [audio inventory](audio-inventory.md) maps the C and Lua APIs to examples and tests.

Malvado plays short WAV sound effects and streams WAV or OGG music.

## Lua

Load a sample once and play it when you need it.

```lua
local mv = require("malvado")
local jump = mv.load_sound("jump.wav")

mv.play_sound(jump)
local voice = mv.play_sound(jump, {volume = 0.5})
if voice then
    mv.pause_sound(voice)
    mv.sound_volume(voice, 0.4)
    mv.resume_sound(voice)
end
```

Paths are relative to the working directory. Loading raises an error for an
invalid WAV file, a device that cannot open, or headless mode.

Each successful `play_sound(sound, options?)` call returns a new Voice and
starts it immediately. Voices overlap and share the decoded sample data.
`options.volume` sets the initial volume for that voice. It must be a finite
number from 0 to 1 and defaults to 1. An empty options table uses the default.
Unknown options, invalid arguments, and playback failures raise errors.

The runner supports up to 32 playing or paused voices at once. When all slots
are in use, `play_sound` returns `nil` and existing voices continue. A stopped
or completed voice frees its slot.

Pass a Voice to `stop_sound`, `pause_sound`, `resume_sound`, `sound_playing`,
and `sound_volume`. Each function acts on that voice alone. `sound_playing`
returns `false` while paused and after stop or completion. Repeating
`stop_sound` is safe. Calling `pause_sound`, `resume_sound`, or
`sound_volume` after stop or completion raises an error.

You can ignore the returned Voice. The runner retains it and its sample data
while it plays or is paused, even if the creating process exits. Stop or
completion releases the native playback resources. A retained Lua handle
remains safe to query. See the [resource lifetime contract](resource-lifetime.md).

When updating older Lua code, reuse the same sample in successive
`play_sound` calls to overlap effects. Save the returned Voice to control one
playback. Pass the initial volume in the options table.

## C

Include `malvado/platform.h` and link the raylib platform library. Open the audio device before loading sounds. Destroy all sounds before closing the device:

```c
if (mv_audio_open()) {
    MvSound *sound = mv_sound_load("jump.wav");
    if (sound) {
        mv_sound_set_volume(sound, 0.5f);
        mv_sound_play(sound);
        /* Keep the sound alive until playback ends. */
        mv_sound_stop(sound);
        mv_sound_destroy(sound);
    }
    mv_audio_close();
}
```

`mv_audio_open` returns 1 on success. It is safe to call it again while the device is open. `mv_sound_load` returns `NULL` on failure. `mv_sound_play`, `mv_sound_stop`, `mv_sound_pause`, `mv_sound_resume`, and `mv_sound_set_volume` return 1 on success or 0 for invalid arguments or a closed device. Pause and resume keep the current playback position. `mv_sound_playing` returns 1 while the sound is playing and 0 while paused or stopped. `mv_sound_destroy(NULL)` and `mv_audio_close()` on a closed device are safe. Use audio functions on the device owner thread.

`mv_sound_clone(sound)` returns an owned playback instance or `NULL`.
Clones share the same decoded data. Destroy each instance with
`mv_sound_destroy`.

## Stream music

Load a WAV or OGG stream. The runner updates active streams automatically, including while process execution is paused. This example loops the track:

```lua
local music = mv.load_music("music.ogg", {loop = true})
mv.play_music(music)
```

`load_music(path, options?)` accepts only the optional boolean field
`loop`, which defaults to `false`. Replace an old positional loop argument
with `{loop = true}` or omit the options for a single play.
Use `mv.pause_music`, `mv.resume_music`, `mv.stop_music`,
`mv.music_playing`, and `mv.music_volume` to control the stream.
Volume is from zero to one. `mv.update_music(music)` remains available
for explicit refill, including scripts that use audio outside `mv.start`.

Playing and paused music retains its stream after the creating process ends. The runner releases playback references when playback stops, completes, or the runner closes.

In C, `mv_music_load(path, loop)` returns an owned stream or `NULL`. Open the audio device first. Call `mv_audio_update()` regularly to refill all active native streams, including when the rest of the game is idle. Explicit `mv_music_update` remains available. Use `mv_music_play`, `mv_music_pause`, `mv_music_resume`, `mv_music_stop`, `mv_music_playing`, and `mv_music_set_volume` to control it. The operations return 0 for invalid arguments or a closed device; `mv_music_playing` also returns 0 when paused or stopped. Destroy the stream with `mv_music_destroy` before closing the device.

Audio uses one process-wide device. Destroy every sound and stream before closing it.

Exported Android games request audio focus while their Activity is resumed
and the device is open. Focus loss mutes the mixer. Transient gain restores its
previous output level. Home releases the focus request, and returning to the
game requests focus again. A permanent loss keeps output muted until a new
request. Focus changes preserve each sound and stream's game-controlled
pause state. Playback positions can advance while output is muted.
Closing the audio device releases focus. These checks measure mixer state
and Android focus ownership. Audible behavior is verified on physical devices.

For a local device check, build the visual tests and run `MALVADO_REAL_AUDIO=1 ./build-visual/audio-visual-test tests/audio_visual.lua` from the repository root. The test uses silent WAV and OGG fixtures. It checks that both loops are still playing after 0.75 seconds; it does not check audible output.

See [shared audio maintenance](audio-maintenance.md) for native pause state, runner timing and playback roots.
