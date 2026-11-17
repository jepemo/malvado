// Malvado 2; SPDX-License-Identifier: Apache-2.0
package dev.malvado.test;

/** Test-only entry that lets the host recreate one live native Activity. */
public final class RecreateActivity extends android.app.NativeActivity {
    private static final class Receiver extends android.content.BroadcastReceiver {
        private final RecreateActivity activity;
        Receiver(RecreateActivity activity) { this.activity = activity; }
        @Override
        public void onReceive(android.content.Context context, android.content.Intent intent) {
            activity.recreate();
        }
    }
    private final Receiver receiver = new Receiver(this);

    @Override
    protected void onCreate(android.os.Bundle state) {
        super.onCreate(state);
        registerReceiver(receiver, new android.content.IntentFilter("dev.malvado.test.RECREATE"),
                android.content.Context.RECEIVER_EXPORTED);
    }

    @Override
    protected void onDestroy() {
        unregisterReceiver(receiver);
        super.onDestroy();
    }
}
