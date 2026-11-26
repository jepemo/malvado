import { readFile, writeFile, mkdir, rm } from 'node:fs/promises';
import { repoLink, revision } from './site-context.mjs';
import { migrateGuides, rewriteGuideLinks, siteLink, writeLegacy } from './guide-migration.mjs';
import { clearExampleDownloads, publishExample } from './examples.mjs';

// The repository guides and examples remain the source of truth.
const output = new URL('../src/content/docs/docs/', import.meta.url);
await mkdir(output, { recursive: true });

// Concept guides from docs/api that become website pages. Every other
// docs/api file stays on GitHub as reference material.
const conceptNames = new Set([
  'lua-processes',
  'lua-instance-methods',
  'lua-language-server',
  'c-contract',
  'rust-processes',
  'process-cleanup',
  'runner-timing',
  'resource-lifetime',
  'drawing',
  'keyboard',
  'images',
  'animations',
  'text',
  'cameras',
  'collision',
  'audio',
  'rust-window',
  'rust-drawing',
  'rust-input',
  'rust-atlas',
  'rust-fpg',
  'rust-camera-animation',
  'rust-font',
  'rust-collision',
  'rust-audio',
]);

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
  if (path.startsWith('../site/'))
    return url;
  if (path.startsWith('../../'))
    return githubLink(path.slice(6), fragment);
  if (path.startsWith('../'))
    return githubLink(`docs/${path.slice(3)}`, fragment);
  return url;
}

function rewriteConceptLinks(body) {
  return body.replace(/\]\(([^)\s]+)\)/g, (_, url) => `](${conceptLink(url)})`);
}

const generated = new URL('generated/', output);
await rm(generated, { recursive: true, force: true });
await mkdir(generated, { recursive: true });
for (const name of ['getting-started', 'sdk', 'api', 'samples']) {
  await rm(new URL(name + '.md', output), { force: true });
}

async function writePage(route, title, body, pagefind = true) {
  const file = new URL(route.replaceAll('/', '-') + '.md', generated);
  await writeFile(file, [
    '---',
    'title: ' + JSON.stringify(title),
    'slug: ' + route,
    'pagefind: ' + pagefind,
    '---',
    '',
    body,
    '',
  ].join('\n'));
}

await clearExampleDownloads();
const imageRoute = 'docs/examples/image';
const image = await publishExample('image', imageRoute);
await migrateGuides(writePage, conceptNames, image ? imageRoute : undefined);
if (image) {
  await writePage(imageRoute, 'Draw a PNG', `Load a PNG after opening a window and draw it at its original size.

## Run the example

Install a [desktop Malvado SDK](../../start/installation/) and use a display.
${image.links}

Extract the ZIP into an empty directory. Open a terminal in that directory,
where \`main.lua\` and \`tile.png\` sit side by side, and run this command.

\`\`\`sh
malvado run main.lua
\`\`\`

The program prints \`image: 32x32\` and shows a red tile at (32, 32) in a
160 × 120 window. It ends after about two seconds. You can close the window
earlier.

The ZIP includes the Apache 2.0 license and credits for the original Malvado tile.

## Explore the code

The game loads the tile during initialization. A process draws the tile once
per frame and keeps the image available until it ends.

${image.snippet}

See the [images guide](../../concepts/images/) for drawing options and the
[shared image lesson](${repoLink('examples/features/image/README.md')}) for the
C and experimental Rust versions.
`);
}

// The same complete game in C, Lua and Rust.
function stripLicenseHeader(code, lang) {
  if (lang === 'c')
    return code.replace(/^\/\*[\s\S]*?\*\/\s*/m, '').trim();
  if (lang === 'rust')
    return code.replace(/^(\/\/[^\n]*\n)+\s*/m, '').trim();
  return code.replace(/^(--[^\n]*\n)+\s*/m, '').trim();
}

const gameDir = '../../../examples/catch/';
const lua = stripLicenseHeader(await readFile(new URL(`${gameDir}lua/main.lua`, import.meta.url), 'utf8'), 'lua');
const c = stripLicenseHeader(await readFile(new URL(`${gameDir}c/main.c`, import.meta.url), 'utf8'), 'c');
const rust = stripLicenseHeader(
  await readFile(new URL('../../../bindings/rust/malvado/examples/catch.rs', import.meta.url), 'utf8'), 'rust');

let rustLessons = 'See the [Rust process guide](../../concepts/rust-processes/) for a managed game and the process API.';
try {
  const hello = stripLicenseHeader(
    await readFile(new URL('../../../examples/rust/hello.rs', import.meta.url), 'utf8'), 'rust');
  const movingBalls = stripLicenseHeader(
    await readFile(new URL('../../../examples/rust/moving-balls.rs', import.meta.url), 'utf8'), 'rust');
  rustLessons = `The managed runner takes care of window events, simulation ticks and rendering.

## Hello World

\`\`\`rust title="examples/rust/hello.rs"
${hello}
\`\`\`

## Moving balls

Each ball runs the same function with its own speed, position and color.

\`\`\`rust title="examples/rust/moving-balls.rs"
${movingBalls}
\`\`\`

See the [first Rust games](${repoLink('examples/rust', 'tree')}) for run commands.

`;
} catch (error) {
  if (!revision || error.code !== 'ENOENT') throw error;
}

await writePage('docs/examples/rust-basics', 'First Rust games (experimental)', rustLessons);
await writePage('docs/examples/catch', 'Catch!', `The three Catch! implementations below open a 640 × 360
window, run the same Catch! game and use the shared core scheduler. The player
and the ball are independent processes:

- The player is a cyan rectangle that moves left and right with the arrow keys.
- The ball is an orange circle that falls from the top.
- Catch the ball with the player to score a point. The ball respawns above the
  screen at a new horizontal position. A miss respawns the ball without scoring.

Press Escape or use the window close control to end the game.

Each repository source starts with its license header; the headers are
omitted here.

## Lua

\`\`\`lua title="examples/catch/lua/main.lua"
${lua}
\`\`\`

## C

\`\`\`c title="examples/catch/c/main.c"
${c}
\`\`\`

## Rust (experimental)

Enable the \`graphics\` feature for this example.

### Catch!

\`\`\`rust title="bindings/rust/malvado/examples/catch.rs"
${rust}
\`\`\`

See the [Catch! game lesson](${repoLink('examples/catch', 'tree')}) for controls
and run commands, and the [Lua processes](../../concepts/lua-processes/), [C
process API](../../concepts/c-contract/) and [Rust processes (experimental)](../../concepts/rust-processes/)
guides.
`);

const catchPage = { route: 'docs/examples/catch', title: 'Catch!' };
const rustBasics = { route: 'docs/examples/rust-basics', title: 'First Rust games (experimental)' };
await writeLegacy(writePage, 'docs/samples', 'C, Lua and Rust examples', [
  ['Lua', catchPage],
  ['C', catchPage],
  ['Rust (experimental)', catchPage],
  ['Hello World', rustBasics, rustLessons.includes('## Hello World') ? '#hello-world' : ''],
  ['Moving balls', rustBasics, rustLessons.includes('## Moving balls') ? '#moving-balls' : ''],
  ['Catch!', catchPage, '#catch'],
]);

// Concept guides from docs/api.
const conceptsOutput = new URL('concepts/', output);
await rm(conceptsOutput, { recursive: true, force: true });
await mkdir(conceptsOutput, { recursive: true });
for (const name of conceptNames) {
  const guide = await readFile(new URL(`../../../docs/api/${name}.md`, import.meta.url), 'utf8');
  const heading = /^# ([^\r\n]+)\r?\n/.exec(guide);
  if (!heading)
    throw new Error(`Missing title in docs/api/${name}.md`);
  let body = rewriteGuideLinks(
    rewriteConceptLinks(guide.slice(heading[0].length).trimStart()),
    'docs/concepts/' + name, conceptNames,
  );
  if (name === 'images' && image) {
    const lesson = siteLink('docs/concepts/images', imageRoute);
    body = `Try the [downloadable PNG lesson](${lesson}) with a complete Lua project.\n\n${body}`;
  }
  const title = name.startsWith('rust-') ? heading[1] + ' (experimental)' : heading[1];
  await writeFile(new URL(`${name}.md`, conceptsOutput),
    `---\ntitle: ${JSON.stringify(title)}\n---\n\n${body}\n`);
}
