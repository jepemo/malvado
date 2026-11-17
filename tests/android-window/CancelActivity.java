// Malvado 2; SPDX-License-Identifier: Apache-2.0
package dev.malvado.test;

/** Destroy the Activity before Android can create its first native surface. */
public final class CancelActivity extends android.app.NativeActivity {
    @Override
    protected void onCreate(android.os.Bundle state) {
        super.onCreate(state);
        finish();
    }
}
