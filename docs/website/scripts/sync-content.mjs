import { readFile, writeFile, mkdir, rm } from 'node:fs/promises';
import { repoLink, revision, version } from './site-context.mjs';

// The repository guides and examples remain the source of truth.
const output = new URL('../src/content/docs/docs/', import.meta.url);
await mkdir(output, { recursive: true });

// Concept guides from docs/api that become website pages. Every other
// docs/api file stays on GitHub as reference material.
const conceptGroups = [
  ['Processes', [
    ['lua-processes', 'Lua processes'],
    ['lua-instance-methods', 'Lua instance methods'],
    ['lua-language-server', 'Lua Language Server support'],
    ['c-contract', 'C process API'],
    ['rust-processes', 'Rust processes'],
    ['process-cleanup', 'Process cleanup contract'],
    ['runner-timing', 'Shared runner timing'],
    ['resource-lifetime', 'Resource lifetime contract'],
  ]],
  ['Graphics and sound', [
    ['drawing', 'Draw rectangles and circles'],
    ['keyboard', 'Keyboard state and transitions'],
    ['images', 'Draw a PNG image'],
    ['animations', 'Automatic process animation'],
    ['text', 'Owned text content'],
    ['cameras', '2D cameras'],
    ['collision', 'Rectangle collision'],
    ['audio', 'Play a sound'],
  ]],
  ['Rust API', [
    ['rust-window', 'Rust window ownership'],
    ['rust-drawing', 'Rust immediate drawing and images'],
    ['rust-input', 'Rust mouse, gamepad and text input'],
    ['rust-atlas', 'Rust atlas metadata'],
    ['rust-fpg', 'Rust FPG import and conversion'],
    ['rust-camera-animation', 'Rust camera and animation values'],
    ['rust-font', 'Rust font ownership and Unicode glyphs'],
    ['rust-collision', 'Native collision and static worlds in Rust'],
    ['rust-audio', 'Rust audio'],
  ]],
];
const conceptNames = new Set(conceptGroups.flatMap(([, pages]) => pages.map(([name]) => name)));

function githubLink(path, fragment = '') {
  const kind = /^(?:.*\/)?[^/]*\.md$/.test(path) || path.includes('.') ? 'blob' : 'tree';
  return repoLink(path, kind) + fragment;
}

// Rewrite a relative link found inside a concept guide. Concept pages live
// under /docs/concepts/<name>/, so sibling concept pages are ../<name>/.
function conceptLink(url) {
  const match = /^([^#]*)(#[^#]*)?$/.exec(url);
  const path = match[1];
  const fragment = match[2] ?? '';
  if (!path)
    return url;
  if (/^[a-zA-Z0-9_.-]+\.md$/.test(path)) {
    const name = path.slice(0, -3);
    if (conceptNames.has(name))
      return `../${name}/${fragment}`;
    return githubLink(`docs/api/${path}`, fragment);
  }
  if (path === '../site/sdk.md')
    return `../../../sdk/${fragment}`;
  if (path.startsWith('../../'))
    return githubLink(path.slice(6), fragment);
  if (path.startsWith('../'))
    return githubLink(`docs/${path.slice(3)}`, fragment);
  return url;
}

function rewriteConceptLinks(body) {
  return body.replace(/\]\(([^)\s]+)\)/g, (_, url) => `](${conceptLink(url)})`);
}

// Rewrite GitHub links to synced concept guides in the site guides. The
// prefix is the path from the guide page to the concepts directory.
function rewriteGuideConceptLinks(body, prefix) {
  const pattern = new RegExp(
    `https://github\\.com/jepemo/malvado/(?:blob|tree|edit)/master/docs/api/(${[...conceptNames].join('|')})\\.md(#[^\\s)]+)?`,
    'g',
  );
  return body.replace(pattern, (_, name, fragment = '') => `${prefix}${name}/${fragment}`);
}

async function readGuide(slug) {
  const guide = await readFile(new URL(`../../site/${slug}.md`, import.meta.url), 'utf8');
  const heading = /^# ([^\r\n]+)\r?\n/.exec(guide);
  if (!heading)
    throw new Error(`Missing title in ${slug}.md`);
  return { title: heading[1], body: guide.slice(heading[0].length).trimStart() };
}

// Site guides: getting-started, sdk and api. Their pages live at the site
// root, so concept pages need a ../docs/concepts/ prefix.
const guideConceptPrefixes = { 'getting-started': '../docs/concepts/', 'sdk': '../docs/concepts/', 'api': '../docs/concepts/' };
for (const slug of ['getting-started', 'sdk', 'api']) {
  const { title, body: raw } = await readGuide(slug);
  let body = raw.replace(
    /\]\((getting-started|sdk|api)\.md(#[^)]+)?\)/g,
    (_, page, fragment = '') => `](../${page}/${fragment})`,
  );
  body = rewriteGuideConceptLinks(body, guideConceptPrefixes[slug]);
  if (revision) {
    body = body.replace(
      /https:\/\/github\.com\/jepemo\/malvado\/(blob|tree|edit)\/master\//g,
      (_, kind) => repoLink('', kind),
    );
  }
  if (slug === 'getting-started' && version) {
    body = `This documentation is for Malvado ${version}. When building from Git, run \`git checkout --detach ${revision}\` after entering the cloned directory and before running \`mise install\`.\n\n${body}`;
  }
  await writeFile(new URL(`${slug}.md`, output),
    `---\ntitle: ${JSON.stringify(title)}\nslug: ${slug}\n---\n\n${body}`);
}

// Aligned process example in C, Lua and Rust.
function stripLicenseHeader(code, lang) {
  if (lang === 'c')
    return code.replace(/^\/\*[\s\S]*?\*\/\s*/m, '').trim();
  if (lang === 'rust')
    return code.replace(/^(\/\/[^\n]*\n)+\s*/m, '').trim();
  return code.replace(/^(--[^\n]*\n)+\s*/m, '').trim();
}

const exampleDir = '../../../examples/features/image/';
const lua = stripLicenseHeader(await readFile(new URL(`${exampleDir}lua/main.lua`, import.meta.url), 'utf8'), 'lua');
const c = stripLicenseHeader(await readFile(new URL(`${exampleDir}c/main.c`, import.meta.url), 'utf8'), 'c');
const rust = stripLicenseHeader(await readFile(new URL(`${exampleDir}rust/main.rs`, import.meta.url), 'utf8'), 'rust');

await writeFile(new URL('samples.md', output), `---\ntitle: C, Lua and Rust examples\ndescription: See the same image example in the three languages.\n---

These runnable examples come from the repository. All three open a 160 × 120
window, load the shared \`tile.png\` asset and draw it at \`(32, 32)\`. They use
the same core scheduler, and print the image size:

\`\`\`text
image: 32x32
\`\`\`

Each repository source starts with its license header; the headers are
omitted here.

## Lua

\`\`\`lua title="examples/features/image/lua/main.lua"
${lua}
\`\`\`

## C

\`\`\`c title="examples/features/image/c/main.c"
${c}
\`\`\`

## Rust (experimental)

Rust gameplay support is experimental in Malvado 2.0.

\`\`\`rust title="examples/features/image/rust/main.rs"
${rust}
\`\`\`

Copy the shared asset as described in the [lesson instructions](${repoLink('examples/features/image', 'tree')}). See the [Draw a PNG image](../concepts/images/) guide, and the [Lua processes](../concepts/lua-processes/), [C process API](../concepts/c-contract/) and [Rust processes](../concepts/rust-processes/) guides.
`);

// Concept guides from docs/api.
const conceptsOutput = new URL('concepts/', output);
await rm(conceptsOutput, { recursive: true, force: true });
await mkdir(conceptsOutput, { recursive: true });
for (const [, pages] of conceptGroups) {
  for (const [name] of pages) {
    const guide = await readFile(new URL(`../../../docs/api/${name}.md`, import.meta.url), 'utf8');
    const heading = /^# ([^\r\n]+)\r?\n/.exec(guide);
    if (!heading)
      throw new Error(`Missing title in docs/api/${name}.md`);
    const body = rewriteConceptLinks(guide.slice(heading[0].length).trimStart());
    await writeFile(new URL(`${name}.md`, conceptsOutput),
      `---\ntitle: ${JSON.stringify(heading[1])}\n---\n\n${body}\n`);
  }
}
