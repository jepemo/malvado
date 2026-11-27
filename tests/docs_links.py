#!/usr/bin/env python3
"""Verify that documentation checks detect broken paths and heading anchors."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
from pathlib import Path
import sys
import tempfile
import unittest

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from check_docs import check_documents

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "docs" / "tools"))
from check_built_site import check_site


class Links(unittest.TestCase):
    def test_utf8_headings(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            target = root / "guide.md"
            target.write_bytes("# Cámara → Position\n".encode("utf-8"))
            source = root / "README.md"
            source.write_bytes("[Cámara](guide.md#cámara--position)\n".encode("utf-8"))
            self.assertEqual(check_documents([source], root), (1, []))

    def test_paths_anchors_fences_and_references(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            target = root / "some guide.md"
            target.write_text('# Camera `move()`\n## Same\n## Same\n<a id="explicit"></a>\n')
            source = root / "README.md"
            source.write_text('''# Start
[one](<some guide.md#camera-move>)
[two](some%20guide.md#same-1)
[local](#start)
[explicit](some%20guide.md#explicit)
[reference]: some%20guide.md#same
```rust
// [fake](missing.md)
```
~~~text
[fake](missing-again.md)
~~~
[external](https://example.com/not-checked)
''')
            count, errors = check_documents([source], root)
            self.assertEqual(count, 5)
            self.assertEqual(errors, [])
            source.write_text('[bad](missing.md)\n[bad anchor](some%20guide.md#absent)\n')
            _, errors = check_documents([source], root)
            self.assertEqual(len(errors), 2)
            self.assertIn(':1: missing link target', errors[0])
            self.assertIn(':2: missing Markdown anchor', errors[1])

    def test_built_migration_links_and_assets(self):
        with tempfile.TemporaryDirectory() as temporary:
            site = Path(temporary)
            (site / "2.1.0").mkdir()
            (site / "index.html").write_text(
                '<a href="/2.1.0/#install">Migration instructions</a><img src="tile.png">'
            )
            guide = site / "2.1.0" / "index.html"
            guide.write_text('<h2 id="install">Install</h2>')
            (site / "tile.png").write_bytes(b"image fixture")
            self.assertEqual(check_site(site, {"2.1.0"})[1], [])
            guide.write_text('<h2 id="renamed">Install</h2>')
            (site / "tile.png").unlink()
            errors = check_site(site, {"2.1.0"})[1]
            self.assertEqual(len(errors), 2)
            self.assertIn('missing site anchor /2.1.0/#install', errors[0])
            self.assertIn('missing site target tile.png', errors[1])

    def test_repository_links_and_updated_headings(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            target = root / "target.md"
            target.write_text('# Before\n')
            source = root / "README.md"
            source.write_text('[link](https://github.com/jepemo/malvado/blob/main/target.md#before)\n')
            self.assertEqual(check_documents([source], root)[1], [])
            target.write_text('# After\n')
            self.assertIn('missing Markdown anchor', check_documents([source], root)[1][0])


if __name__ == "__main__":
    unittest.main()
