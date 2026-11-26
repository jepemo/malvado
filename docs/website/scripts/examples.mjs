import { execFile } from 'node:child_process';
import { copyFile, mkdir, mkdtemp, readFile, rm, stat, writeFile } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { basename, dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { promisify } from 'node:util';
import { siteLink } from './guide-migration.mjs';
import { repoLink, revision } from './site-context.mjs';

const repo = new URL('../../../', import.meta.url);
const downloads = new URL('../public/example-downloads/', import.meta.url);
const run = promisify(execFile);

const examples = {
  hello: { directory: 'examples/helloworld/', source: 'main.lua' },
  meteors: { directory: 'examples/meteors/', source: 'main.lua' },
  'blinking-text': { directory: 'examples/blinking-text/', source: 'main.lua' },
  'world-wall': { directory: 'examples/world-collision/', source: 'main.lua' },
  'space-asteroids': {
    directory: 'examples/space_asteroids/',
    source: 'main.lua',
    assets: { 'asteroid.png': 'asteroid.png', 'spaceship.png': 'spaceship.png' },
    credit: 'The ship and asteroid artwork are original Malvado project assets.',
  },
  'catch-lua': { directory: 'examples/catch/', source: 'lua/main.lua' },
  'catch-c': { directory: 'examples/catch/', source: 'c/main.c' },
  'catch-rust': { directory: 'bindings/rust/malvado/examples/', source: 'catch.rs' },
  ...Object.fromEntries([
    '01-start', '02-player', '03-movement', '04-enemy',
    '05-spawn', '06-collision', '07-score', 'main',
  ].map((name) => [`dodge-${name}`, { directory: 'examples/dodge/', source: name + '.lua' }])),
  'two-workers-c': { directory: 'examples/features/processes/', source: 'c/main.c' },
  'two-workers-rust': { directory: 'examples/features/processes/', source: 'rust/main.rs' },
  'two-workers': { directory: 'examples/features/processes/', source: 'lua/main.lua' },
  'level-teardown': { directory: 'examples/process-lesson/', source: 'level.lua' },
  'movement-time': { directory: 'examples/process-lesson/', source: 'movement.lua' },
  mailbox: { directory: 'examples/process-lesson/', source: 'mailbox.lua' },
  'shape-coordinates': { directory: 'examples/applied-learn/', source: 'shapes.lua' },
  'input-movement': { directory: 'examples/applied-learn/', source: 'movement.lua' },
  'text-labels': { directory: 'examples/applied-learn/', source: 'labels.lua' },
  'select-sprites': {
    directory: 'examples/applied-learn/',
    source: 'sprites.lua',
    assets: { 'assets/sheet.json': '../aseprite/sheet.json', 'assets/sheet.png': '../aseprite/sheet.png' },
    credit: 'The atlas and sprite artwork are original Malvado project assets.',
  },
  'aseprite-bounce': {
    directory: 'examples/aseprite/',
    source: 'main.lua',
    assets: { 'sheet.json': 'sheet.json', 'sheet.png': 'sheet.png' },
    credit: 'The atlas and sprite artwork are original Malvado project assets.',
  },
  'circle-collision': { directory: 'examples/collision/', source: 'main.lua' },
  'audio-tone': {
    directory: 'examples/audio/',
    source: 'main.lua',
    assets: { 'tone.wav': 'tone.wav' },
    credit: 'The Malvado project generated the 440 Hz tone for this example.',
  },
  'camera-follow': { directory: 'examples/camera-follow/', source: 'main.lua' },
  roll: {
    directory: 'examples/rand/',
    source: 'roll.lua',
  },
  image: {
    directory: 'examples/features/image/',
    source: 'lua/main.lua',
    assets: { 'tile.png': 'assets/tile.png' },
    credit: 'The red tile is original artwork by the Malvado project.',
  },
};

for (const [language, extension] of [['c', 'c'], ['rust', 'rs']]) {
  examples[`image-${language}`] = {
    ...examples.image,
    source: `${language}/main.${extension}`,
  };
}

export async function clearExampleDownloads() {
  await rm(downloads, { recursive: true, force: true });
}

async function writeBundle(archive, files, credit) {
  const staging = await mkdtemp(join(tmpdir(), 'malvado-doc-example-'));
  try {
    for (const [name, source] of Object.entries(files)) {
      const destination = join(staging, name);
      await mkdir(dirname(destination), { recursive: true });
      await copyFile(source, destination);
    }
    await writeFile(join(staging, 'NOTICE'), credit + '\n');
    await run('python3', ['-m', 'zipfile', '-c', fileURLToPath(archive), '.'], { cwd: staging });
  } finally {
    await rm(staging, { recursive: true, force: true });
  }
}

export async function publishExample(id, route, language = 'lua') {
  const example = examples[id];
  if (!example) throw new Error(`Unknown documentation example: ${id}`);

  const directory = new URL(example.directory, repo);
  if (revision) {
    // Older snapshots can predate a lesson. Existing lessons must be complete.
    try {
      await stat(directory);
    } catch (error) {
      if (error.code === 'ENOENT') return null;
      throw error;
    }
  }
  const sourcePath = new URL(example.source, directory);
  const source = await readFile(sourcePath);
  const filename = basename(example.source);
  const output = new URL(`${id}/`, downloads);
  await mkdir(output, { recursive: true });

  const url = (name) => siteLink(route, `example-downloads/${id}`) + name;
  const sourceUrl = repoLink(example.directory + example.source);
  const code = source.toString('utf8');
  const snippet = [`\`\`\`${language} title="${filename}"`, code.trimEnd(), '\`\`\`'].join('\n');
  let downloadUrl = url(filename);
  let downloadLabel = `Download ${filename}`;
  if (example.assets) {
    const files = {
      [filename]: sourcePath,
      LICENSE: new URL('LICENSE', repo),
    };
    for (const [name, path] of Object.entries(example.assets))
      files[name] = new URL(path, directory);
    const archive = `${id}.zip`;
    await writeBundle(new URL(archive, output), files, example.credit);
    const languageLabel = { lua: 'Lua', c: 'C', rust: 'Rust' }[language];
    downloadUrl = url(archive);
    downloadLabel = `Download the ${languageLabel} project ZIP`;
  } else {
    await writeFile(new URL(filename, output), source);
  }
  const links = `[${downloadLabel}](${downloadUrl}) · [View source on GitHub](${sourceUrl})`;
  return { language, filename, code, sourceUrl, downloadUrl, downloadLabel, snippet, links };
}
