// Malvado 2; SPDX-License-Identifier: Apache-2.0
package dev.malvado.test;

import android.inputmethodservice.InputMethodService;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.TextView;

/** Exercise Android's real input-method connection with deterministic composition. */
public final class TestInputMethod extends InputMethodService {
    private boolean sent;

    @Override
    public View onCreateInputView() {
        TextView keys = new TextView(this);
        keys.setText("Malvado composition test");
        keys.setMinHeight(160);
        return keys;
    }

    @Override
    public void onStartInputView(EditorInfo info, boolean restarting) {
        super.onStartInputView(info, restarting);
        if (!info.packageName.startsWith("dev.malvado.") || sent) return;
        sent = true;
        InputConnection connection = getCurrentInputConnection();
        connection.replaceText(0, 0, "ignored replacement", 1, null);
        connection.finishComposingText();
        connection.setComposingText("draft", 1);
        android.os.Handler timer = new android.os.Handler(getMainLooper());
        timer.postDelayed(() -> {
            connection.setComposingText("e", 1);
            connection.commitText("\u00e9", 1);
            connection.setComposingText("\ud83d\ude00", 1);
            connection.finishComposingText();
            timer.postDelayed(() -> connection.commitText("late", 1), 400);
        }, 1600);
    }

    @Override
    public void onFinishInput() {
        sent = false;
        super.onFinishInput();
    }
}
