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
from zipfile import ZipFile


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / 'docs' / 'tools' / 'prepare_versioned_site.py'
VERSION = '2.0.0-beta.1'
PAGES = 'https://malvado.dev'
CATCH = 'docs/examples/catch/'
RUST_BASICS = 'docs/examples/rust-basics/'
SOURCE_BUILD = 'docs/contributing/build/'
ROLL_PAGE = 'docs/reference/lua/platform/rand/'
IMAGE_PAGE = 'docs/examples/image/'
PROCESS_LESSONS = {
    'process-model': ('two-workers/main.lua', 'examples/features/processes/lua/main.lua'),
    'process-lifecycle': ('level-teardown/level.lua', 'examples/process-lesson/level.lua'),
    'frames-and-time': ('movement-time/movement.lua', 'examples/process-lesson/movement.lua'),
    'process-communication': ('mailbox/mailbox.lua', 'examples/process-lesson/mailbox.lua'),
}
SPRITE_ARCHIVE = (
    'select-sprites/select-sprites.zip', 'examples/applied-learn/sprites.lua',
    {'assets/sheet.json': 'examples/aseprite/sheet.json',
     'assets/sheet.png': 'examples/aseprite/sheet.png'},
)
APPLIED_LESSONS = {
    'drawing-and-coordinates': ('shape-coordinates/shapes.lua', 'examples/applied-learn/shapes.lua', {}),
    'input-and-movement': ('input-movement/movement.lua', 'examples/applied-learn/movement.lua', {}),
    'graphics-and-sprites': SPRITE_ARCHIVE,
    'animation': ('aseprite-bounce/aseprite-bounce.zip', 'examples/aseprite/main.lua',
                  {'sheet.json': 'examples/aseprite/sheet.json', 'sheet.png': 'examples/aseprite/sheet.png'}),
    'collisions': ('circle-collision/main.lua', 'examples/collision/main.lua', {}),
    'text-and-fonts': ('text-labels/labels.lua', 'examples/applied-learn/labels.lua', {}),
    'sound-and-music': ('audio-tone/audio-tone.zip', 'examples/audio/main.lua',
                       {'tone.wav': 'examples/audio/tone.wav'}),
    'cameras-and-scrolling': ('camera-follow/main.lua', 'examples/camera-follow/main.lua', {}),
    'resources': SPRITE_ARCHIVE,
}
LEGACY = {
    'getting-started/': {
        'install-the-sdk': 'docs/start/installation/#install-the-sdk',
        'uninstall-the-sdk': 'docs/start/installation/#uninstall-the-sdk',
        'create-and-run-a-game': 'docs/start/first-program/#create-and-run-a-game',
        'lua-in-vs-code': 'docs/tools/editor/#lua-in-vs-code',
        'build-from-source-contributors': SOURCE_BUILD + '#build-from-source-contributors',
        'try-more-examples': 'docs/examples/#basics',
    },
    'sdk/': {
        'install-a-local-development-build': SOURCE_BUILD + '#install-a-local-development-build',
        'windows-sdk-prefix': SOURCE_BUILD + '#windows-sdk-prefix',
        'build-and-install-the-sdk': 'docs/contributing/release/#build-and-install-the-sdk',
        'create-and-run-a-game': 'docs/start/project-structure/#create-and-run-a-game',
        'create-a-rust-game': 'docs/start/project-structure/#create-a-rust-game',
        'export-a-game': 'docs/tools/export/#export-a-game',
        'export-for-android': 'docs/tools/export/#export-for-android',
    },
    'api/': {
        'run-and-stop-processes': 'docs/learn/process-model/#run-and-stop-processes',
        'organize-a-level': 'docs/patterns/level-lifecycle/#organize-a-level',
        'choose-a-reference': 'docs/reference/#choose-a-reference',
        'keep-resources-alive': 'docs/learn/resources/#keep-resources-alive',
        'choose-a-runtime-and-target': 'docs/tools/platforms/#choose-a-runtime-and-target',
    },
    'docs/samples/': {
        'lua': CATCH + '#lua',
        'c': CATCH + '#c',
        'rust-experimental': CATCH + '#rust-experimental',
        'hello-world': RUST_BASICS + '#hello-world',
        'moving-balls': RUST_BASICS + '#moving-balls',
        'catch': CATCH + '#catch',
    },
}


class Page(HTMLParser):
    def __init__(self, html):
        super().__init__()
        self.canonical = None
        self.links = []
        self.anchors = set()
        self.indexed = False
        self.feed(html)

    def handle_starttag(self, tag, attributes):
        attrs = dict(attributes)
        if 'data-pagefind-body' in attrs:
            self.indexed = True
        if 'id' in attrs:
            self.anchors.add(attrs['id'])
        if tag == 'link' and attrs.get('rel') == 'canonical':
            self.canonical = attrs['href']
        if tag in ('a', 'link') and 'href' in attrs:
            self.links.append(attrs['href'])
        if tag == 'script' and 'src' in attrs:
            self.links.append(attrs['src'])


class VersionedDocsTests(unittest.TestCase):
    def check_example_downloads(self, site, prefix, *, revision=None):
        def source(path):
            if revision:
                return subprocess.check_output(['git', '-C', str(ROOT), 'show', f'{revision}:{path}'])
            return (ROOT / path).read_bytes()

        def present(path):
            if revision:
                return bool(subprocess.check_output(
                    ['git', '-C', str(ROOT), 'ls-tree', revision, '--', path]))
            return True

        def check_link(route, path):
            page = Page((site / route / 'index.html').read_text(encoding='utf-8'))
            page_url = PAGES + prefix + route
            self.assertIn(PAGES + prefix + path, {urljoin(page_url, href) for href in page.links})

        roll = 'example-downloads/roll/roll.lua'
        if present('examples/rand'):
            check_link(ROLL_PAGE, roll)
            self.assertEqual((site / roll).read_bytes(), source('examples/rand/roll.lua'))
        else:
            self.assertFalse((site / roll).exists())

        image = 'example-downloads/image/image.zip'
        lesson = 'examples/features/image'
        if present(lesson):
            check_link('docs/examples/', IMAGE_PAGE)
            check_link('docs/concepts/images/', IMAGE_PAGE)
            check_link(IMAGE_PAGE, image)
            with ZipFile(site / image) as bundle:
                self.assertCountEqual(bundle.namelist(), ['main.lua', 'tile.png', 'LICENSE', 'NOTICE'])
                self.assertEqual(bundle.read('main.lua'), source(f'{lesson}/lua/main.lua'))
                self.assertEqual(bundle.read('tile.png'), source(f'{lesson}/assets/tile.png'))
                self.assertEqual(bundle.read('LICENSE'), source('LICENSE'))
                self.assertIn(b'Malvado project', bundle.read('NOTICE'))
        else:
            self.assertFalse((site / image).exists())

        for chapter, (download, path) in PROCESS_LESSONS.items():
            route = f'docs/learn/{chapter}/'
            download = f'example-downloads/{download}'
            with self.subTest(chapter=chapter):
                if present('docs/site/learn'):
                    check_link(route, download)
                    self.assertEqual((site / download).read_bytes(), source(path))
                    page = Page((site / route / 'index.html').read_text(encoding='utf-8'))
                    self.assertTrue(page.indexed)
                    check_link('docs/learn/process-model/', route)
                else:
                    self.assertFalse((site / download).exists())
                    if chapter != 'process-model':
                        self.assertFalse((site / route).exists())

        for chapter, (download, code, assets) in APPLIED_LESSONS.items():
            route = f'docs/learn/{chapter}/'
            download = f'example-downloads/{download}'
            with self.subTest(chapter=chapter):
                if not present(f'docs/site/learn/{chapter}.md'):
                    self.assertFalse((site / download).exists())
                    if chapter != 'resources':
                        self.assertFalse((site / route).exists())
                    continue
                check_link(route, download)
                check_link('docs/learn/process-model/', route)
                page = Page((site / route / 'index.html').read_text(encoding='utf-8'))
                self.assertTrue(page.indexed)
                if assets:
                    files = {Path(code).name: code, 'LICENSE': 'LICENSE', **assets}
                    with ZipFile(site / download) as bundle:
                        archive_files = [name for name in bundle.namelist() if not name.endswith('/')]
                        self.assertCountEqual(archive_files, [*files, 'NOTICE'])
                        for name, path in files.items():
                            self.assertEqual(bundle.read(name), source(path))
                        self.assertIn(b'Malvado project', bundle.read('NOTICE'))
                else:
                    self.assertEqual((site / download).read_bytes(), source(code))

    def check_site(self, site, prefix):
        pages = {}
        for path in site.rglob('index.html'):
            directory = path.parent.relative_to(site).as_posix()
            route = '' if directory == '.' else directory + '/'
            page_url = f'{PAGES}{prefix}{route}'
            page = Page(path.read_text(encoding='utf-8'))
            self.assertEqual(page.canonical, page_url)
            pages[page_url] = page

        home = pages[f'{PAGES}{prefix}']
        self.assertIn(f'{prefix}docs/start/first-program/', home.links)
        self.assertIn(f'{prefix}docs/examples/', home.links)
        self.assertIn(f'{prefix}favicon.svg', home.links)
        self.assertTrue((site / 'pagefind' / 'pagefind.js').is_file())
        self.assertFalse((site / 'legacy').exists())

        for route, headings in LEGACY.items():
            url = f'{PAGES}{prefix}{route}'
            page = pages[url]
            self.assertFalse(page.indexed)
            links = {urljoin(url, link) for link in page.links}
            for anchor, destination in headings.items():
                with self.subTest(route=route, anchor=anchor):
                    self.assertIn(anchor, page.anchors)
                    self.assertIn(f'{PAGES}{prefix}{destination}', links)

        for route in (CATCH, RUST_BASICS, 'docs/concepts/rust-processes/',
                      'docs/reference/lua/processes/spawn/', 'docs/tools/cli/'):
            self.assertTrue(pages[f'{PAGES}{prefix}{route}'].indexed)

        for page_url, page in pages.items():
            for href in set(page.links):
                destination = urlsplit(urljoin(page_url, href))
                if destination.netloc != 'malvado.dev':
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
        site = ROOT / 'docs' / 'website' / 'dist'
        self.check_site(site, '/')
        self.check_example_downloads(site, '/')
        self.assertIn('https://github.com/jepemo/malvado/tree/main/examples/catch',
                      Page((site / CATCH / 'index.html').read_text()).links)

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
            self.check_site(site, f'/{VERSION}/')
            self.check_example_downloads(site, f'/{VERSION}/', revision=revision)
            self.assertIn('meteors / main.lua', (site / 'index.html').read_text())
            catch = (site / CATCH / 'index.html').read_text()
            rust_basics = (site / RUST_BASICS / 'index.html').read_text()
            samples = catch + rust_basics
            for source in ('examples/catch/lua/main.lua', 'examples/catch/c/main.c',
                           'bindings/rust/malvado/examples/catch.rs', 'examples/rust/hello.rs',
                           'examples/rust/moving-balls.rs'):
                with self.subTest(source=source):
                    self.assertIn(source, samples)
            self.assertIn(f'https://github.com/jepemo/malvado/tree/{revision}/examples/catch',
                          Page(catch).links)
            self.assertIn(f'https://github.com/jepemo/malvado/tree/{revision}/examples/rust',
                          Page(rust_basics).links)
            for path in site.rglob('index.html'):
                html = path.read_text()
                self.assertIn(f'https://github.com/jepemo/malvado/blob/{revision}/', html)
                for route in ('blob', 'tree', 'edit'):
                    self.assertNotIn(f'github.com/jepemo/malvado/{route}/main/', html)
                    self.assertNotIn(f'github.com/jepemo/malvado/{route}/master/', html)
            self.assertIn(f'git checkout --detach {revision}',
                          (site / SOURCE_BUILD / 'index.html').read_text())

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
