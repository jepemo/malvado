#!/usr/bin/env python3
"""Extract declaration metadata from pinned BennuGD headers, offline, using stdlib.
This is a lexical inventory, not a C preprocessor or a semantic API translator.
"""
import hashlib
import json
import re
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / 'api/upstream-manifest.json'


def uncomment(text):
    # Keep line numbers and quoted strings; don't interpret comments inside strings.
    pattern = r'"(?:\\.|[^"\\])*"|/\*[\s\S]*?\*/|//[^\n]*'
    return re.sub(pattern, lambda m: m[0] if m[0].startswith('"') else ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)


def generate():
    manifest = json.loads(MANIFEST.read_text())
    records, summaries = [], []
    for source in manifest['sources']:
        path = ROOT / source['local']
        blob = path.read_bytes()
        assert hashlib.sha256(blob).hexdigest() == source['sha256'], path
        if not path.name.endswith('_exports.h'):
            continue
        text = uncomment(blob.decode('utf-8', errors='replace'))
        conditions, by_line = [], {}
        for number, line in enumerate(text.splitlines(), 1):
            directive = re.match(r'\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)', line)
            if directive:
                op, expr = directive.groups()
                if op in ('if', 'ifdef', 'ifndef'):
                    conditions.append([f'#{op} {expr.strip()}'])
                elif op in ('else', 'elif') and conditions:
                    conditions[-1].append(f'#{op} {expr.strip()}'.strip())
                elif op == 'endif' and conditions:
                    conditions.pop()
            by_line[number] = [list(branches) for branches in conditions]
        def base(match, kind):
            number = text.count('\n', 0, match.start()) + 1
            return dict(engine=source['engine'], module=path.stem.removesuffix('_exports'), kind=kind,
                        source=source['local'], line=number, url=source['url'] + f'#L{number}',
                        preprocessor_context=by_line.get(number, []), status='not_assessed')
        # A declaration per overload. Sentinel FUNC(0,...) intentionally omitted.
        function_pattern = r'\bFUNC\(\s*"([^"\n]+)"\s*,\s*"([^"\n]*)"\s*,\s*(TYPE_\w+)\s*,\s*(\w+(?:\([^()]*\))?)\s*\)'
        count = Counter()
        for match in re.finditer(function_pattern, text):
            record = base(match, 'function')
            record.update(zip(('name', 'signature', 'return_type', 'implementation'), match.groups()))
            records.append(record)
            count['functions'] += 1
        assert len(re.findall(r'\bFUNC\(\s*"', text)) == count['functions'], f'Unparsed FUNC: {path}'
        for match in re.finditer(r'\{\s*"([^"\n]+)"\s*,\s*(TYPE_\w+)\s*,\s*([^{}]*?)\s*\}', text):
            record = base(match, 'constant')
            record.update(zip(('name', 'type', 'expression'), match.groups()))
            records.append(record)
            count['constants'] += 1
        for match in re.finditer(r'char\s*\*\s*__bgdexport\(\s*\w+\s*,\s*(globals_def|locals_def)\s*\)\s*=([\s\S]*?);', text):
            # Semicolons occur inside quoted declarations: use lexical string tokens
            # up to the next unquoted semicolon instead of the tentative regex end.
            start = text.index('=', match.start()) + 1
            pos, chunks = start, []
            token = re.compile(r'\s*("(?:\\.|[^"\\])*"|;|#[^\n]*|\S+)', re.S)
            while (part := token.match(text, pos)):
                value = part[1]
                pos = part.end()
                if value == ';':
                    break
                if value.startswith('"'):
                    chunks.append(json.loads(value))
            record = base(match, match[1])
            record.update(name=match[1], declaration_string_fragments=''.join(chunks), raw_initializer=text[start:pos-1].strip())
            records.append(record)
            count['declaration_blocks'] += 1
        summaries.append(dict(engine=source['engine'], module=path.stem.removesuffix('_exports'), **count))
    data = dict(schema_version=1, scope='lexical module exports; all conditional branches; not a preprocessed build', modules=summaries, records=records)
    (ROOT/'api/catalog.json').write_text(json.dumps(data, ensure_ascii=False, indent=2)+'\n')
    lines = ['# Inventario de funciones de referencia', '',
             'Generado por `python3 docs/tools/catalog_api.py`. Una fila por sobrecarga declarada; incluye ramas condicionales y alias. No equivale a funciones implementadas en Malvado.', '',
             'Las constantes y bloques globales/locales se consultan en [catalog.json](catalog.json) y en los originales de [upstream](upstream.md). Las firmas son las cadenas originales del registro C; no son firmas Lua.', '',
             '| Referencia | Módulo | Sobrecargas | Constantes | Bloques global/local |', '| --- | --- | ---: | ---: | ---: |']
    for item in summaries:
        lines.append(f'| {item["engine"]} | {item["module"]} | {item.get("functions",0)} | {item.get("constants",0)} | {item.get("declaration_blocks",0)} |')
    for engine in ('bennugd2','bennugd1'):
        for item in (x for x in summaries if x['engine']==engine and x.get('functions')):
            lines += ['', f'## {engine} / {item["module"]}', '', '| Nombre | Firma original | Retorno | Fuente |', '| --- | --- | --- | --- |']
            for r in records:
                if r['engine']==engine and r['module']==item['module'] and r['kind']=='function':
                    lines.append(f'| `{r["name"]}` | `{r["signature"]}` | `{r["return_type"]}` | [L{r["line"]}]({r["url"]}) |')
    (ROOT/'api/inventory.md').write_text('\n'.join(lines)+'\n')
    for engine in ('bennugd1','bennugd2'):
        subset = [r for r in records if r['engine']==engine]
        print(engine, dict(Counter(r['kind'] for r in subset)))


if __name__ == '__main__':
    generate()
