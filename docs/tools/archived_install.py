#!/usr/bin/env python3
"""Pin installer commands in the Pages copies of the two original snapshots."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import argparse
from pathlib import Path
import re

INSTALL = 'curl -fsSL https://malvado.dev/install.sh | bash'
RELEASES = 'https://github.com/jepemo/malvado/releases'
CHANGELOG = 'https://malvado.dev/docs/changelog/'


def replace_one(text, pattern, replacement):
    updated, count = re.subn(pattern, replacement, text, flags=re.S)
    if count != 1:
        raise ValueError(f'Expected one archived install section matching {pattern!r}')
    return updated


def pin_install(snapshot, version):
    if version not in {'2.0.0', '2.1.0'}:
        return
    command = f'{INSTALL} -s -- --version {version}'
    homepage = snapshot / 'index.html'
    landing = homepage.read_text(encoding='utf-8')
    landing = replace_one(
        landing, r'(<div class="code-window terminal" data-copy-block>.*?<pre\b[^>]*><code>).*?(</code></pre></div>)',
        lambda match: f'{match[1]}{command}{match[2]}',
    )
    landing = replace_one(
        landing, r'(</code></pre></div>)<p class="section-note">.*?</p>',
        lambda match: f'{match[1]}<p class="section-note">This installs Malvado {version} '
        'on macOS arm64 or Linux x86_64. For Windows, download the '
        f'<a href="{RELEASES}/tag/v{version}">matching SDK</a>. '
        f'See the <a href="{CHANGELOG}{version}/">release notes</a>.</p>',
    )
    homepage.write_text(landing, encoding='utf-8')
    if version == '2.1.0':
        guide = snapshot / 'getting-started' / 'index.html'
        html = guide.read_text(encoding='utf-8')
        html = replace_one(
            html, r'(<span style="[^"]*">bash</span>)(</div></div></code></pre>[^>]*<div class="copy">.*?data-code=")'
            + re.escape(INSTALL) + '"',
            lambda match: f'{match[1]} -s -- --version {version}{match[2]}{command}"',
        )
        guide.write_text(html, encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    parser.add_argument('version')
    args = parser.parse_args()
    pin_install(args.directory, args.version)


if __name__ == '__main__':
    main()
