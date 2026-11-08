# Audio API baseline for alpha.6

This inventory records the delivered sound and music API. It is an input to the
complete 2.0 inventory. The [audio guide](audio.md) defines formats, ownership,
errors, examples, and device requirements. Audio remains experimental.

| Area | C | Lua | Examples and automated evidence |
| --- | --- | --- | --- |
| Audio device | `mv_audio_open`, `mv_audio_close` | Opened by `mv.load_sound` or `mv.load_music`; closed by the runner | [audio demo](../../examples/audio/README.md), `input`, `audio_headless`, `visual_audio` |
| WAV sounds | `mv_sound_load`, `mv_sound_clone`, `mv_sound_destroy` | `mv.load_sound`, `mv.clone_sound`; managed sound | [audio demo](../../examples/audio/README.md), `audio_headless`, `visual_audio`, `visual_audio_resource` |
| Sound playback | `mv_sound_play`, `mv_sound_stop`, `mv_sound_pause`, `mv_sound_resume`, `mv_sound_playing`, `mv_sound_set_volume` | `mv.play_sound`, `mv.stop_sound`, `mv.pause_sound`, `mv.resume_sound`, `mv.sound_playing`, `mv.sound_volume` | [audio demo](../../examples/audio/README.md), `audio_headless`, `visual_audio`, `visual_audio_resource` |
| Music streams | `mv_music_load`, `mv_music_destroy`, `mv_music_play`, `mv_music_update`, `mv_music_stop`, `mv_music_pause`, `mv_music_resume`, `mv_music_playing`, `mv_music_set_volume` | `mv.load_music`, `mv.play_music`, `mv.update_music`, `mv.stop_music`, `mv.pause_music`, `mv.resume_music`, `mv.music_playing`, `mv.music_volume`; managed music | [audio demo](../../examples/audio/README.md), `audio_headless`, `visual_audio`, `visual_audio_resource` |

C callers open the process-wide device and destroy resources before closing it.
Lua manages the device and resource references. Both use the same platform
adapter. Audio is unavailable in headless mode. Lua functions are also available
after `import_globals()`; see the [global export table](lua-globals.md).

`visual_audio` and `visual_audio_resource` are registered on Linux with an ALSA
null device; they are not registered by the normal macOS visual suite. These
tests do not establish audible output or physical-device behavior. The optional
local playback check in the audio guide is separate. This baseline does not
select additional formats, effects, or mixer APIs for 2.0.
