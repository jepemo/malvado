# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

cli, visual, script, source = sys.argv[1:]
with tempfile.TemporaryDirectory(prefix="malvado-assets-") as temp:
    fpg = str(Path(source) / "examples/fpg/graphics.fpg")
    subprocess.run([cli, "fpg-export", "--transparent-zero", fpg,
                    str(Path(temp) / "assets")], check=True)
    subprocess.run([cli, "fpg-export", fpg, str(Path(temp) / "opaque")], check=True)
    fixtures = Path(source) / "tests/fixtures"
    for directory, name in (("first", "red.png"), ("second", "odd-red.png")):
        destination = Path(temp) / directory
        destination.mkdir()
        shutil.copyfile(fixtures / name, destination / "palette.png")
    env = dict(os.environ, MALVADO_ASSET_TEST_DIR=temp, MALVADO_ASSET_SOURCE=source)
    subprocess.run([visual, script], cwd=source, env=env, check=True)
