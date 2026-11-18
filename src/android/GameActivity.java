// Malvado 2; SPDX-License-Identifier: Apache-2.0
package dev.malvado.player;

import android.app.NativeActivity;
import android.media.AudioAttributes;
import android.media.AudioFocusRequest;
import android.media.AudioManager;

/** Android entry shared by exported C, Lua and Rust games. */
public final class GameActivity extends NativeActivity {
    private boolean resumed;
    private boolean destroyed;
    private long device;
    private long focusGeneration;
    private AudioFocusRequest request;

    private static native void audioFocusChanged(long device, boolean focused);

    // Called on the native game thread. Never wait for Android's main thread.
    public void malvadoSetAudioDevice(long value) {
        runOnUiThread(() -> {
            if (destroyed || device == value) return;
            releaseFocus();
            device = value;
            requestFocus();
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        resumed = true;
        requestFocus();
    }

    @Override
    protected void onPause() {
        resumed = false;
        releaseFocus();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        destroyed = true;
        releaseFocus();
        device = 0;
        super.onDestroy();
    }

    private void requestFocus() {
        if (!resumed || device == 0 || request != null) return;
        AudioManager audio = getSystemService(AudioManager.class);
        if (audio == null) return;
        long generation = ++focusGeneration;
        AudioAttributes attributes = new AudioAttributes.Builder()
            .setUsage(AudioAttributes.USAGE_GAME)
            .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
            .build();
        request = new AudioFocusRequest.Builder(AudioManager.AUDIOFOCUS_GAIN)
            .setAudioAttributes(attributes)
            .setAcceptsDelayedFocusGain(true)
            .setWillPauseWhenDucked(true)
            .setOnAudioFocusChangeListener(change -> {
                if (generation == focusGeneration) {
                    if (change == AudioManager.AUDIOFOCUS_LOSS) {
                        releaseFocus();
                    } else {
                        audioFocusChanged(device, change == AudioManager.AUDIOFOCUS_GAIN);
                    }
                }
            })
            .build();
        int result = audio.requestAudioFocus(request);
        audioFocusChanged(device, result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED);
        if (result == AudioManager.AUDIOFOCUS_REQUEST_FAILED) request = null;
    }

    private void releaseFocus() {
        ++focusGeneration;
        if (device != 0) audioFocusChanged(device, false);
        if (request != null) {
            getSystemService(AudioManager.class).abandonAudioFocusRequest(request);
            request = null;
        }
    }
}
