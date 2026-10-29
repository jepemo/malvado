#!/usr/bin/env python3
"""Download the pinned API reference files and verify their SHA-256 hashes."""
import hashlib
import json
import sys
from pathlib import Path
from urllib.error import URLError
from urllib.parse import urlsplit
from urllib.request import urlopen

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / 'api/upstream-manifest.json'
DESTINATION = (ROOT / 'api/upstream').resolve()


def fetch():
    manifest = json.loads(MANIFEST.read_text(encoding='utf-8'))
    for source in manifest['sources']:
        path = (ROOT / source['local']).resolve()
        if not path.is_relative_to(DESTINATION):
            raise ValueError(f'Invalid destination: {path}')
        expected = source['sha256']
        if path.is_file() and hashlib.sha256(path.read_bytes()).hexdigest() == expected:
            continue
        url = urlsplit(source['url'])
        parts = url.path.strip('/').split('/', 4)
        if url.scheme != 'https' or url.netloc != 'github.com' or len(parts) != 5 or parts[2] != 'blob' or parts[3] != source['commit']:
            raise ValueError(f'Invalid pinned URL: {source["url"]}')
        raw_url = f'https://raw.githubusercontent.com/{parts[0]}/{parts[1]}/{parts[3]}/{parts[4]}'
        print(f'Downloading {source["local"]}', flush=True)
        with urlopen(raw_url, timeout=30) as response:
            blob = response.read()
        if hashlib.sha256(blob).hexdigest() != expected:
            raise ValueError(f'SHA-256 mismatch: {source["local"]}')
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(blob)
    print(f'Verified {len(manifest["sources"])} pinned files.')


if __name__ == '__main__':
    try:
        fetch()
    except (OSError, URLError, ValueError) as error:
        sys.exit(f'Unable to fetch API sources: {error}')
