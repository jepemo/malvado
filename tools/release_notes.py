#!/usr/bin/env python3
"""Validate canonical notes and render the public changelog and release body."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0

import argparse
from datetime import date
import json
from pathlib import Path
import posixpath
import re
import subprocess
from urllib.parse import quote, unquote, urlsplit

from check_docs import LINK, ROOT, markdown_anchors, prose

NOTES = 'docs/changelog'
REPOSITORY = 'https://github.com/jepemo/malvado'
WEBSITE = 'https://malvado.dev'
VERSION = r'2\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)(?:-(?:alpha|beta|rc)\.[1-9][0-9]*)?'
REVISION = r'[0-9a-f]{40}'
BACKFILLED = {'1.0', '2.0.0-alpha.7', '2.0.0-beta.1', '2.0.0', '2.1.0'}


def read_tree(root, path, ref=None):
    if ref is None:
        return (root / path).read_bytes()
    result = subprocess.run(
        ['git', '-C', str(root), 'show', f'{ref}:{path}'],
        stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, check=False,
    )
    if result.returncode:
        raise FileNotFoundError(f'{path} at {ref}')
    return result.stdout


def publications(read):
    entries = json.loads(read(f'{NOTES}/publications.json'))
    if not isinstance(entries, list):
        raise ValueError('Publication metadata must be a list')
    seen = set()
    previous = date.max
    for entry in entries:
        required = {'version', 'published', 'status', 'docs'}
        if (not isinstance(entry, dict) or entry.keys() != required
                or not all(isinstance(entry[key], str) for key in required)):
            raise ValueError('Invalid publication metadata fields')
        version = entry['version']
        published = date.fromisoformat(entry['published'])
        expected_status = 'prerelease' if '-' in version else 'stable'
        if ((version != '1.0' and not re.fullmatch(VERSION, version))
                or version in seen or entry['published'] != published.isoformat()
                or published > previous or entry['status'] != expected_status
                or entry['docs'] not in {'site', 'bundle', 'source', 'none'}):
            raise ValueError(f'Invalid or out-of-order publication metadata for {version}')
        seen.add(version)
        previous = published
    return entries


def notes_body(read, version):
    text = read(f'{NOTES}/{version}.md').decode('utf-8')
    title, _, body = text.partition('\n')
    body = body.strip()
    highlights = re.match(r'## Highlights\n(?:[ \t]*\n)*[ \t]*[^#\s]', prose(body))
    if (title != f'# Malvado {version}' or not highlights
            or re.search(r'\b(?:TODO|TBD)\b|<version>', prose(body))):
        raise ValueError(f'Release notes for {version} must have a matching title and completed Highlights')
    return body


def render_links(body, ref, read=None):
    anchors = markdown_anchors(body)

    def render(match):
        group = 1 if match[1] else 2
        raw = match[group]
        url = raw.strip('<>')
        target = urlsplit(url)
        if target.scheme == 'https' and target.netloc:
            if re.match(re.escape(REPOSITORY) + r'/(?:blob|raw|tree)/(?:main|master)/', url):
                raise ValueError(f'Use a relative path for a versioned repository link: {url}')
            return match[0]
        if target.scheme or target.netloc or target.path.startswith('/') or target.query:
            raise ValueError(f'Use a relative repository path or HTTPS URL: {url}')
        fragment = unquote(target.fragment)
        if not target.path:
            if fragment not in anchors:
                raise ValueError(f'Missing release note anchor: {url}')
            return match[0]
        path = posixpath.normpath(posixpath.join(NOTES, unquote(target.path)))
        if path == '..' or path.startswith('../') or path.startswith('/'):
            raise ValueError(f'Release note link escapes the repository: {url}')
        if read is not None:
            data = read(path)
            if fragment and path.endswith('.md') and fragment not in markdown_anchors(data.decode('utf-8')):
                raise ValueError(f'Missing Markdown anchor: {url}')
        # Markdown images need the raw bytes. Ordinary links open the repository file.
        image = bool(re.search(r'!\[[^\]]*$', match.string[:match.start()]))
        kind = 'raw' if image else 'blob'
        absolute = f'{REPOSITORY}/{kind}/{ref}/{quote(path)}'
        if target.fragment:
            absolute += '#' + target.fragment
        start = match.start(group) - match.start()
        end = match.end(group) - match.start()
        return match[0][:start] + absolute + match[0][end:]

    return '\n'.join(LINK.sub(render, line) if visible.strip() else line
                     for line, visible in zip(body.split('\n'), prose(body).split('\n'), strict=True))


def candidate(read, version, revision):
    if not re.fullmatch(VERSION, version) or not re.fullmatch(REVISION, revision):
        raise ValueError('Expected a Malvado 2 version and full lowercase Git commit ID')
    if any(entry['version'] == version for entry in publications(read)):
        raise ValueError(f'{version} was already published at the candidate revision')
    body = render_links(notes_body(read, version), revision, read)
    return f'# Malvado {version}\n\n{body}\n'


def public_notes(root, check_tags=False):
    def working(path):
        return read_tree(root, path)

    entries = publications(working)
    pages = []
    for entry in entries:
        version = entry['version']
        tag = f'v{version}'
        body = notes_body(working, version)
        ref = tag
        if check_tags:
            def from_release(path):
                return read_tree(root, path, tag)

            if version not in BACKFILLED and notes_body(from_release, version) != body:
                raise ValueError(f'Published release notes differ from {tag}')
            render_links(body, tag, from_release)
        # A published tag and its verified candidate SHA name the same source.
        rendered = render_links(body, ref)
        release = f'{REPOSITORY}/releases/tag/{tag}'
        docs = {
            'site': f'{WEBSITE}/{version}/',
            'bundle': f'{REPOSITORY}/releases/download/{tag}/malvado-{version}-release-bundle.tar.gz',
            'source': f'{REPOSITORY}/tree/{tag}/' + ('doc' if version == '1.0' else 'docs'),
            'none': None,
        }[entry['docs']]
        pages.append(dict(entry, body=rendered, release=release, documentation=docs,
                          breaking='breaking-changes' in markdown_anchors(body)))
    return pages


def index(pages):
    lines = [
        '# Changelog', '',
        '<!-- Generated by tools/release_notes.py. Edit docs/changelog/ instead. -->', '',
        f'Read the [website changelog]({WEBSITE}/docs/changelog/) for publication dates,',
        'downloads and versioned documentation.', '',
        '| Version | Published (UTC) | Status | Upgrade |',
        '| --- | --- | --- | --- |',
    ]
    for page in pages:
        version = page['version']
        path = f'{NOTES}/{version}.md'
        upgrade = f'[Breaking changes]({path}#breaking-changes)' if page['breaking'] else '—'
        lines.append(f'| [{version}]({path}) | {page["published"]} | {page["status"].title()} | {upgrade} |')
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    for command in ('index', 'check', 'website', 'snapshots'):
        commands.add_parser(command)
    commands.choices['check'].add_argument('--verify-tags', action='store_true')
    release = commands.add_parser('candidate')
    release.add_argument('version')
    release.add_argument('revision')
    args = parser.parse_args()
    try:
        if args.command == 'candidate':
            print(candidate(lambda path: read_tree(ROOT, path, args.revision),
                            args.version, args.revision), end='')
            return
        pages = public_notes(ROOT, getattr(args, 'verify_tags', False))
        if args.command == 'snapshots':
            for page in pages:
                if page['docs'] == 'site':
                    print(page['version'])
            return
        if args.command == 'website':
            intro = (ROOT / NOTES / 'README.md').read_text(encoding='utf-8')
            print(json.dumps({'intro': intro.split('\n', 1)[1].strip(), 'releases': pages}))
            return
        expected = index(pages)
        path = ROOT / 'CHANGELOG.md'
        if args.command == 'index':
            path.write_text(expected, encoding='utf-8')
        elif path.read_text(encoding='utf-8') != expected:
            raise ValueError('CHANGELOG.md is stale. Run python3 tools/release_notes.py index.')
    except (OSError, ValueError) as error:
        parser.exit(1, f'{error}\n')


if __name__ == '__main__':
    main()
