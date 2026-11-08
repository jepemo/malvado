# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

from pathlib import Path
import subprocess
import sys
import tempfile

cli, visual_test, source, script = sys.argv[1:]
with tempfile.TemporaryDirectory(prefix="malvado-fpg-visual-") as temp:
    output = Path(temp) / "assets"
    subprocess.run([cli, "fpg-export", "--transparent-zero", source, str(output)], check=True)
    subprocess.run([visual_test, script], cwd=temp, check=True)
