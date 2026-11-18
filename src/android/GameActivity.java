// Malvado 2; SPDX-License-Identifier: Apache-2.0
package dev.malvado.player;

import android.app.NativeActivity;
import android.media.AudioAttributes;
import android.media.AudioFocusRequest;
import android.media.AudioManager;
import android.text.Editable;
import android.text.InputType;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

/** Android entry shared by exported C, Lua and Rust games. */
public final class GameActivity extends NativeActivity {
    private boolean resumed;
    private boolean destroyed;
    private long device;
    private long focusGeneration;
    private AudioFocusRequest request;
    private TextEditor editor;

    static native void keyboardTextCommitted(long session, String text);

    public void malvadoSetKeyboard(long session) {
        runOnUiThread(() -> {
            if (destroyed) return;
            if (session == 0) {
                closeEditor();
            } else if (resumed) {
                if (editor == null || editor.session != session) {
                    closeEditor();
                    editor = new TextEditor(this, session);
                    addContentView(editor, new ViewGroup.LayoutParams(1, 1));
                    editor.requestFocus();
                }
                editor.show();
            }
        });
    }

    private void closeEditor() {
        TextEditor previous = editor;
        if (previous == null) return;
        editor = null;
        previous.deactivate();
        getSystemService(InputMethodManager.class).hideSoftInputFromWindow(previous.getWindowToken(), 0);
        ViewGroup parent = (ViewGroup) previous.getParent();
        if (parent != null) parent.removeView(previous);
    }

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
        closeEditor();
        releaseFocus();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        destroyed = true;
        closeEditor();
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

/** A focusable editor with no visible text field. The game draws its own text. */
final class TextEditor extends View {
    final long session;
    private TextConnection connection;
    private boolean active = true;
    private boolean showPending;

    TextEditor(GameActivity activity, long session) {
        super(activity);
        this.session = session;
        setFocusableInTouchMode(true);
        setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_NO);
    }

    void deactivate() {
        active = false;
        connection = null;
    }

    boolean acceptsConnection(TextConnection candidate) {
        return active && connection == candidate;
    }

    void connectionClosed(TextConnection candidate) {
        if (connection == candidate) connection = null;
    }

    void show() {
        showPending = true;
        post(this::showWhenFocused);
    }

    private void showWhenFocused() {
        if (active && showPending && hasWindowFocus() && isFocused()) {
            showPending = false;
            getContext().getSystemService(InputMethodManager.class)
                .showSoftInput(this, InputMethodManager.SHOW_IMPLICIT);
        }
    }

    @Override
    public void onWindowFocusChanged(boolean focused) {
        super.onWindowFocusChanged(focused);
        if (focused) showWhenFocused();
    }

    @Override
    public boolean onCheckIsTextEditor() {
        return active;
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo info) {
        if (!active) return null;
        info.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
        info.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI | EditorInfo.IME_FLAG_NO_PERSONALIZED_LEARNING;
        info.initialSelStart = info.initialSelEnd = 0;
        connection = new TextConnection(this);
        return connection;
    }
}

final class TextConnection extends BaseInputConnection {
    private final TextEditor editor;

    TextConnection(TextEditor editor) {
        super(editor, true);
        this.editor = editor;
    }

    @Override
    public boolean commitText(CharSequence text, int cursor) {
        if (!editor.acceptsConnection(this) || text == null) return false;
        String confirmed = text.toString();
        removeComposingSpans(getEditable());
        getEditable().clear();
        GameActivity.keyboardTextCommitted(editor.session, confirmed);
        return true;
    }

    @Override
    public boolean finishComposingText() {
        if (!editor.acceptsConnection(this)) return false;
        Editable draft = getEditable();
        int start = getComposingSpanStart(draft);
        int end = getComposingSpanEnd(draft);
        if (start >= 0 && end > start) return commitText(draft.subSequence(start, end), 1);
        draft.clear();
        return true;
    }

    @Override
    public void closeConnection() {
        editor.connectionClosed(this);
        getEditable().clear();
        super.closeConnection();
    }
}
