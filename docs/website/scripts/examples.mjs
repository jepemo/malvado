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
  'two-workers': { directory: 'examples/features/processes/', source: 'lua/main.lua' },
  'level-teardown': { directory: 'examples/process-lesson/', source: 'level.lua' },
  'movement-time': { directory: 'examples/process-lesson/', source: 'movement.lua' },
  mailbox: { directory: 'examples/process-lesson/', source: 'mailbox.lua' },
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

export async function publishExample(id, route) {
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
  const sourceLink = `[View source on GitHub](${repoLink(example.directory + example.source)})`;
  const code = source.toString('utf8').trimEnd();
  const snippet = ['```lua title="' + filename + '"', code, '```'].join('\n');
  const links = [sourceLink];
  if (example.assets) {
    const files = {
      [filename]: sourcePath,
      LICENSE: new URL('LICENSE', repo),
    };
    for (const [name, path] of Object.entries(example.assets))
      files[name] = new URL(path, directory);
    const archive = `${id}.zip`;
    await writeBundle(new URL(archive, output), files, example.credit);
    links.unshift(`[Download the Lua project ZIP](${url(archive)})`);
  } else {
    await writeFile(new URL(filename, output), source);
    links.unshift(`[Download ${filename}](${url(filename)})`);
  }
  return { snippet, links: links.join(' · ') };
}
