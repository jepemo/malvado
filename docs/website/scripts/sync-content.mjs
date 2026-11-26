import { readFile, writeFile, mkdir, rm } from 'node:fs/promises';
import { repoLink, revision } from './site-context.mjs';
import { migrateGuides, rewriteGuideLinks, siteLink, writeLegacy } from './guide-migration.mjs';
import { clearExampleDownloads } from './examples.mjs';
import { writeExampleGallery } from './example-gallery.mjs';
import { learnLessons, writeLearnLessons } from './learn-lessons.mjs';
import { dodgeTutorial, writeTutorials } from './tutorials.mjs';
import { patterns, writePatterns } from './patterns.mjs';
import { writeChangelog } from './changelog.mjs';

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

async function writePage(route, title, body, pagefind = true, extension = 'md') {
  const file = new URL(route.replaceAll('/', '-') + '.' + extension, generated);
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

await writeChangelog(writePage);
await clearExampleDownloads();
const imageRoute = 'docs/examples/image';
const gallery = await writeExampleGallery(writePage);
const processModel = learnLessons.find(({ name }) => name === 'process-model');
const resources = learnLessons.find(({ name }) => name === 'resources');
const hasLevelRecipe = patterns.some(({ name }) => name === 'level-lifecycle');
await migrateGuides(writePage, conceptNames, processModel?.label, resources?.label,
  dodgeTutorial?.route, hasLevelRecipe);
await writeLearnLessons(writePage, conceptNames);
await writeTutorials(writePage, conceptNames);
await writePatterns(writePage, conceptNames);

function stripLicenseHeader(code) {
  return code.replace(/^(\/\/[^\n]*\n)+\s*/m, '').trim();
}

let rustLessons = 'See the [Rust process guide](../../concepts/rust-processes/) for a managed game and the process API.';
try {
  const hello = stripLicenseHeader(
    await readFile(new URL('../../../examples/rust/hello.rs', import.meta.url), 'utf8'));
  const movingBalls = stripLicenseHeader(
    await readFile(new URL('../../../examples/rust/moving-balls.rs', import.meta.url), 'utf8'));
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
  if (name === 'images' && gallery.has('image')) {
    const lesson = siteLink('docs/concepts/images', imageRoute);
    body = `Try the [downloadable PNG lesson](${lesson}) with complete runnable programs.\n\n${body}`;
  }
  const title = name.startsWith('rust-') ? heading[1] + ' (experimental)' : heading[1];
  await writeFile(new URL(`${name}.md`, conceptsOutput),
    `---\ntitle: ${JSON.stringify(title)}\n---\n\n${body}\n`);
}
