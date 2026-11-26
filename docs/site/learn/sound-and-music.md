# Sound and music

Play two copies of the same sound at different volumes, then start a
repeating stream. This shows why a game can load a sound once and still
play several independent instances.

## Run the example

Install the [desktop SDK](../../start/installation/). Use a display and an
audio device. Extract the ZIP into an empty directory, keep `main.lua`
and `tone.wav` together, and run this command there.

```sh
malvado run main.lua
```

Press **1** and **2** close together to hear the tones overlap. Press
**Space** to start or stop the low-volume loop. Press **Escape** or close
the window to exit. The archive includes the Apache 2.0 license and
credits for Malvado's generated 440 Hz tone.

<!-- example -->

## Load once, then play

`load_sound` loads a WAV sample. Each successful `play_sound` call
starts a new voice with its own volume. Keep the returned Voice handle
if you need to pause, resume or stop that particular sound. You can ignore
the handle for a short effect. The runner keeps active voices and their
sample data alive even if the process that started them ends. If all
32 voice slots are busy, `play_sound` returns `nil`.

`load_music` opens a WAV or OGG stream. Use it for a track that should
play over time. `play_music` and `stop_music` act on that stream's
handle. The Lua runner refills active streams. This example uses the
same file for both paths so you can compare their behavior.

Try setting the stream's `loop` option to `false`. The stream now ends
on its own. The example's on-screen switch tracks what Space requested.
Use `music_playing(music)` when you need to check whether the stream is
actually playing.

Use a press edge for one-shot effects. With a held-key check, a process
can start another voice every tick and run out of slots. See the
[Lua audio reference](../../reference/lua/audio/), the [C and Lua audio
guide](../../concepts/audio/) and the [experimental Rust audio
guide](../../concepts/rust-audio/).
