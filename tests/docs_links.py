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


class Links(unittest.TestCase):
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

    def test_repository_links_and_updated_headings(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            target = root / "target.md"
            target.write_text('# Before\n')
            source = root / "README.md"
            source.write_text('[link](https://github.com/jepemo/malvado/blob/master/target.md#before)\n')
            self.assertEqual(check_documents([source], root)[1], [])
            target.write_text('# After\n')
            self.assertIn('missing Markdown anchor', check_documents([source], root)[1][0])


if __name__ == "__main__":
    unittest.main()
