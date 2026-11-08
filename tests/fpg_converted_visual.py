# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

cli, visual_test, source, script = sys.argv[1:]
with tempfile.TemporaryDirectory(prefix="malvado-fpg-visual-") as temp:
    output = Path(temp) / "converted"
    subprocess.run([cli, "fpg-export", "--transparent-zero", source, str(output)], check=True)
    manifest = json.loads((output / "manifest.json").read_text())
    sprite = next(item for item in manifest["sprites"] if item["id"] == 7)
    point = sprite["points"][0]
    assert point["defined"]
    env = dict(os.environ, MALVADO_TEST_FPG_DIR=str(output),
               MALVADO_TEST_PIVOT_X=str(point["x"]), MALVADO_TEST_PIVOT_Y=str(point["y"]))
    subprocess.run([visual_test, script], env=env, check=True)
