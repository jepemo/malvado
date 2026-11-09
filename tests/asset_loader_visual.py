# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

import os
from pathlib import Path
import subprocess
import sys
import tempfile

cli, visual, script, source = sys.argv[1:]
with tempfile.TemporaryDirectory(prefix="malvado-assets-") as temp:
    subprocess.run([cli, "fpg-export", "--transparent-zero",
                    str(Path(source) / "examples/fpg/graphics.fpg"),
                    str(Path(temp) / "assets")], check=True)
    env = dict(os.environ, MALVADO_ASSET_TEST_DIR=temp, MALVADO_ASSET_SOURCE=source)
    subprocess.run([visual, script], cwd=source, env=env, check=True)
