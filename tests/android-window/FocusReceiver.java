// Malvado 2; SPDX-License-Identifier: Apache-2.0
package dev.malvado.test;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.AudioFocusRequest;
import android.media.AudioManager;

/** A second package that requests focus during the export test. */
public final class FocusReceiver extends BroadcastReceiver {
    private static AudioFocusRequest request;

    @Override
    public void onReceive(Context context, Intent intent) {
        AudioManager audio = context.getSystemService(AudioManager.class);
        int gain = intent.getIntExtra("gain", 0);
        if (gain != 0) {
            request = new AudioFocusRequest.Builder(gain)
                .setOnAudioFocusChangeListener(change -> {})
                .build();
            setResultCode(audio.requestAudioFocus(request));
        } else if (request != null) {
            setResultCode(audio.abandonAudioFocusRequest(request));
            request = null;
        }
    }
}
