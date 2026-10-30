# Play a sound

Malvado can load short WAV sound effects. This API is experimental. It plays each sound once. Playing the same sound again restarts it.

## Lua

Load a sound, then play it when you need it:

```lua
local mv = require("malvado")
local sound = mv.load_sound("jump.wav")

mv.sound_volume(sound, 0.5)
mv.play_sound(sound)
if mv.sound_playing(sound) then
    mv.stop_sound(sound)
end
```

Paths are relative to the working directory. Volume ranges from zero to one. Loading fails if the file is not a valid WAV or the audio device cannot open. Audio is not available in headless mode. Keep a reference to a sound while it plays. Lua releases sounds when they are no longer used or when the runner closes.

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

`mv_audio_open` returns 1 on success. It is safe to call it again while the device is open. `mv_sound_load` returns `NULL` on failure. `mv_sound_play`, `mv_sound_stop`, and `mv_sound_set_volume` return 1 on success or 0 for invalid arguments or a closed device. `mv_sound_playing` returns 1 while the sound is playing and 0 otherwise. `mv_sound_destroy(NULL)` and `mv_audio_close()` on a closed device are safe. Use audio functions on the device owner thread.

Audio uses one process-wide device. Destroy every sound before closing it. Music streaming, looping, and multiple simultaneous instances of the same sound are not supported yet.
