# Shared audio maintenance

The native platform owns playback control state and stream refill. Language
bindings keep their language-specific lifetime roots. The service uses the
existing audio device; it does not run processes or create another scheduler.

## Native pause state

`mv_sound_paused(sound)` and `mv_music_paused(music)` report explicit native pause
state. Backend `playing` queries remain false while paused. Play, stop and resume
clear pause state. Repeated pause preserves it. Pausing an idle/completed
instance does not mark it paused. Sound clones start unpaused and have independent
controls. NULL, a closed device or headless input returns 0. Read and control
instances on the device owner thread.

## Native stream update

Call `mv_audio_update()` regularly on the device owner thread. It refills every
started, unpaused native music stream, including streams created by another
binding on the same device. It skips loaded-but-idle, stopped and paused streams.
Completion ends automatic refill; it does not restart a completed stream. Explicit
`mv_music_update(music)` remains available.

The adapter's stream index is borrowed. It adds no ownership or extra allocation
on load. Destroying a music instance removes it before freeing its decoder. The
update pass allocates nothing in the adapter; the backend can allocate its decode
buffer. The pass returns 1 for an open device, including an empty index, or 0 for
no device/headless.

```c
if (mv_audio_open()) {
    MvMusic *music = mv_music_load("music.ogg", 1);
    if (music) {
        mv_music_play(music);
        /* Call regularly from the existing host loop. */
        mv_audio_update();
        mv_music_stop(music);
        mv_music_destroy(music);
    }
    mv_audio_close();
}
```

The index does not retain resources or the device. C callers own each loaded
instance and destroy it before device close. Destroy still stops/releases an
instance immediately. Lua registry roots and Rust Rc roots keep active playback
alive when their process owner ends. These roots belong to the bindings; native
state and refill do not depend on their representation. Device ownership remains
exclusive to one host. Borrowing native operations on that same owner thread does
not create another device owner.

Native controls, paused/idle/repeated-pause boundaries, stop without restart,
loop continuation, index removal and headless failure are automated. Silent
fixtures and backend state do not establish audible output or physical-device
quality; those checks remain in beta.

## Lua runner

The runner calls native maintenance after scheduler cleanup and on each host-loop
iteration. Streams therefore continue to refill on frames with no simulation tick
and while the debugger pauses process execution. This does not advance the
scheduler. `mv.update_music(music)` remains available for explicit use, including
scripts that operate audio without starting the runner.

Playing/paused userdata stay rooted in Lua's playback registry. The sweep reads
native pause/playing queries instead of maintaining duplicate flags. Stop removes
the root immediately; completion removes it during maintenance. Process owner
termination does not end playback while that root remains. Native controls and
refill are shared, while registry references remain a Lua binding responsibility.
Lua's existing controls and playing queries remain its public interface; pause
state queries are internal to retention.

The resource fixture starts a finite stream, lets its creator return, and proves
that maintenance releases the playback root without explicit Lua stream updates.
The native fixture also creates a stream during a paused Lua frame and checks
completion while the scheduler tick counter stays unchanged. Both run with native
ASan/UBSan and silent audio fixtures.

## Rust service

`Audio::update()` calls the native device pass, then releases completed Rust
playback roots. It no longer refills only the streams in its Rust root vector.
`Sound::paused()` and `Music::paused()` read native state. There is no separate
Cell cache of pause state. A same-thread native pause is therefore visible to
Rust retention, and paused instances stay alive after their original owner drops.

Rust still owns its Rc playback roots and device token. Stop removes that
instance's root; completion is collected by update; paused roots survive until
stop, resumed completion or service shutdown. Dropping the final service owner
releases outstanding instances before closing the device. Native C instances
created on the borrowed device stay caller-owned and must be destroyed before
that device closes.

Tests change pause state through raw native operations and verify Rust retention.
They also run a looping native stream absent from Rust's root vector, destroy it,
and update the device again. Existing completion, owner-death, pause/resume/stop,
exactly-once release and device reopen checks remain active in Debug, Release
and native graphical sanitizer runs. Installed consumers exercise both pause
queries and shared device maintenance.
