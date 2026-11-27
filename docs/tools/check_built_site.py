#!/usr/bin/env python3
"""Check local page targets, fragments and assets in the built website."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
from html.parser import HTMLParser
from pathlib import Path
import sys
from urllib.parse import unquote, urljoin, urlsplit
from zipfile import BadZipFile, ZipFile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
from release_notes import publications

ORIGIN = 'https://malvado.dev'


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
        if tag == 'a' and 'name' in attrs:
            self.anchors.add(attrs['name'])
        if tag == 'link' and attrs.get('rel') == 'canonical':
            self.canonical = attrs['href']
        for key in ('href', 'src', 'poster'):
            if attrs.get(key):
                self.links.append(attrs[key])
        srcset = attrs.get('srcset', '')
        if srcset and not srcset.startswith('data:'):
            self.links.extend(candidate.strip().split()[0] for candidate in srcset.split(',') if candidate.strip())


def check_site(site, snapshots=(), current_only=False):
    pages = {}
    for file in sorted(site.rglob('*.html')):
        path = file.relative_to(site)
        if path.parts[0] not in snapshots:
            pages[path.as_posix()] = Page(file.read_text(encoding='utf-8'))
    snapshot_pages = {}
    errors = []
    downloads = set()
    if 'index.html' not in pages:
        errors.append('Missing site index.html')
    deferred = set()
    for path, page in pages.items():
        route = path.removesuffix('index.html')
        for link in sorted(set(page.links)):
            url = urlsplit(urljoin(f'{ORIGIN}/{route}', link))
            if url.scheme not in ('http', 'https') or url.netloc != 'malvado.dev':
                continue
            if path == '404.html' and link == page.canonical:
                continue  # Astro emits 404.html for the host's error handler.
            target = unquote(url.path).lstrip('/')
            if '..' in Path(target).parts:
                errors.append(f'{path}: site link escapes the build: {link}')
                continue
            if current_only and target.split('/')[0] in snapshots:
                deferred.add(link)
                continue
            if not target or target.endswith('/') or (site / target).is_dir():
                target = f'{target.rstrip("/")}/index.html' if target else 'index.html'
            if not (site / target).is_file():
                errors.append(f'{path}: missing site target {link}')
                continue
            if url.fragment and target.endswith('.html'):
                page_target = pages.get(target) or snapshot_pages.get(target)
                if page_target is None:
                    # Archived pages are immutable. Inspect them only as link targets.
                    page_target = Page((site / target).read_text(encoding='utf-8'))
                    snapshot_pages[target] = page_target
                if unquote(url.fragment) not in page_target.anchors:
                    errors.append(f'{path}: missing site anchor {link}')
            if target.startswith('example-downloads/'):
                downloads.add(target)
    for path in sorted((site / 'example-downloads').glob('*/*')):
        relative = path.relative_to(site).as_posix()
        if relative not in downloads:
            errors.append(f'{relative}: download has no page link')
        elif path.stat().st_size == 0:
            errors.append(f'{relative}: empty download')
        elif path.suffix == '.zip':
            try:
                with ZipFile(path) as archive:
                    entries = archive.namelist()
                    if len(entries) != len(set(entries)) or not {'NOTICE', 'LICENSE'} <= set(entries):
                        raise ValueError('missing notices or duplicate bundle paths')
                    if archive.testzip() is not None:
                        raise ValueError('invalid bundle checksum')
            except (OSError, ValueError, BadZipFile) as error:
                errors.append(f'{relative}: {error}')
    return len(pages), errors, deferred


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('site', type=Path)
    parser.add_argument('--current-only', action='store_true',
                        help='check a local build before published snapshots are attached')
    args = parser.parse_args()
    snapshots = {entry['version'] for entry in publications(lambda path: (ROOT / path).read_bytes())
                 if entry['docs'] == 'site'}
    count, errors, deferred = check_site(args.site, snapshots, args.current_only)
    if errors:
        raise SystemExit('\n'.join(errors))
    if deferred:
        print('Published snapshot links for the full check: ' + ', '.join(sorted(deferred)))
    print(f'PASS: local links, fragments and assets across {count} built pages')


if __name__ == '__main__':
    main()
