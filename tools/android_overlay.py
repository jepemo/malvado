#!/usr/bin/env python3
"""Build an owned runtime resource overlay for the Android acceptance APK."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
from pathlib import Path
import re


def resources(work, overlay=False):
    folder = Path(work) / ('overlay-res' if overlay else 'game-res') / 'values'
    folder.mkdir(parents=True, exist_ok=True)
    (folder / 'strings.xml').write_text('<resources><string name="app_name">' +
                                      ('Overlay active' if overlay else 'Malvado acceptance') +
                                      '</string></resources>')
    if not overlay:
        (folder / 'overlayable.xml').write_text('''<resources>
<overlayable name="Acceptance"><policy type="public">
<item type="string" name="app_name"/>
</policy></overlayable></resources>''')
    return folder.parent


def link(log, run, tools, sdk, manifest, res, output):
    compiled = Path(str(output) + '.resources.zip')
    run(log, [tools / 'aapt2', 'compile', '--dir', res, '-o', compiled])
    run(log, [tools / 'aapt2', 'link', '-o', output, '--manifest', manifest,
              '-I', sdk / 'platforms/android-34/android.jar', compiled])


def build(log, run, tools, sdk, work, package, key):
    name = package + '.overlay'
    manifest = work / 'OverlayManifest.xml'
    manifest.write_text(f'''<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="{name}" android:versionCode="1" android:versionName="test">
<uses-sdk android:minSdkVersion="28" android:targetSdkVersion="34"/>
<overlay android:targetPackage="{package}" android:targetName="Acceptance" android:isStatic="false"/>
<application android:hasCode="false" android:label="Acceptance overlay"/>
</manifest>''')
    unsigned, aligned, apk = [work / path for path in ('overlay-unsigned.apk', 'overlay-aligned.apk', 'overlay.apk')]
    link(log, run, tools, sdk, manifest, resources(work, overlay=True), unsigned)
    run(log, [tools / 'zipalign', '-f', '4', unsigned, aligned])
    run(log, [tools / 'apksigner', 'sign', '--ks', key, '--ks-pass', 'pass:android', '--out', apk, aligned])
    run(log, [tools / 'apksigner', 'verify', apk])
    return name, apk


def is_enabled(output, package, enabled):
    marker = 'x' if enabled else ' '
    return any(re.fullmatch(r'\[' + re.escape(marker) + r'\]\s+' + re.escape(package), line.strip())
               for line in output.splitlines())
