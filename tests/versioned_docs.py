#!/usr/bin/env python3
"""Check the public build and the candidate site built by the Pages command."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

from html.parser import HTMLParser
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from urllib.parse import unquote, urljoin, urlsplit


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / 'docs' / 'tools' / 'prepare_versioned_site.py'
VERSION = '2.0.0-beta.1'
PAGES = 'https://jepemo.github.io'
ROUTES = ('', 'api/', 'getting-started/', 'sdk/', 'docs/', 'docs/samples/')


class Page(HTMLParser):
    def __init__(self, html):
        super().__init__()
        self.canonical = None
        self.links = []
        self.anchors = set()
        self.feed(html)

    def handle_starttag(self, tag, attributes):
        attrs = dict(attributes)
        if 'id' in attrs:
            self.anchors.add(attrs['id'])
        if tag == 'link' and attrs.get('rel') == 'canonical':
            self.canonical = attrs['href']
        if tag in ('a', 'link') and 'href' in attrs:
            self.links.append(attrs['href'])
        if tag == 'script' and 'src' in attrs:
            self.links.append(attrs['src'])


class VersionedDocsTests(unittest.TestCase):
    def check_site(self, site, prefix):
        pages = {}
        for route in ROUTES:
            page_url = f'{PAGES}{prefix}{route}'
            page = Page((site / route / 'index.html').read_text(encoding='utf-8'))
            self.assertEqual(page.canonical, page_url)
            pages[page_url] = page

        home = pages[f'{PAGES}{prefix}']
        self.assertIn(f'{prefix}getting-started/', home.links)
        self.assertIn(f'{prefix}favicon.svg', home.links)
        self.assertIn('export-for-android', pages[f'{PAGES}{prefix}sdk/'].anchors)
        self.assertTrue((site / 'pagefind' / 'pagefind.js').is_file())
        self.assertFalse((site / 'legacy').exists())

        for page_url, page in pages.items():
            for href in page.links:
                destination = urlsplit(urljoin(page_url, href))
                if destination.netloc != 'jepemo.github.io':
                    continue
                with self.subTest(source=page_url, link=href):
                    self.assertTrue(destination.path.startswith(prefix), destination.path)
                    path = site / unquote(destination.path.removeprefix(prefix))
                    if destination.path.endswith('/'):
                        path /= 'index.html'
                    self.assertTrue(path.is_file(), path)
                    if destination.fragment and path.suffix == '.html':
                        target_url = f'{PAGES}{destination.path}'
                        self.assertIn(unquote(destination.fragment), pages[target_url].anchors)

    def test_public_site_preserves_live_routes(self):
        self.check_site(ROOT / 'docs' / 'website' / 'dist', '/malvado/')

    def test_candidate_has_versioned_urls_and_locked_source_links(self):
        revision = subprocess.check_output(
            ['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip()
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / 'candidate docs'
            command = [sys.executable, SCRIPT, VERSION, revision, output]
            subprocess.run(command, check=True, timeout=240)
            metadata = json.loads((output / 'candidate.json').read_text())
            self.assertEqual(metadata, {
                'version': VERSION, 'revision': revision, 'path': f'{VERSION}/',
            })
            site = output / VERSION
            self.check_site(site, f'/malvado/{VERSION}/')
            for page in ROUTES:
                html = (site / page / 'index.html').read_text()
                self.assertIn(f'https://github.com/jepemo/malvado/blob/{revision}/', html)
                for route in ('blob', 'tree', 'edit'):
                    self.assertNotIn(f'github.com/jepemo/malvado/{route}/master/', html)
            self.assertIn(f'git checkout --detach {revision}',
                          (site / 'getting-started' / 'index.html').read_text())

            repeat = subprocess.run(command, capture_output=True, timeout=30)
            self.assertNotEqual(repeat.returncode, 0)
            self.assertTrue((site / 'index.html').is_file())

            invalid = output.parent / 'invalid'
            wrong = subprocess.run([sys.executable, SCRIPT, VERSION, 'HEAD', invalid],
                                   capture_output=True, timeout=30)
            self.assertNotEqual(wrong.returncode, 0)
            self.assertFalse(invalid.exists())


if __name__ == '__main__':
    unittest.main()
