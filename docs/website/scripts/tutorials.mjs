import { existsSync, readFileSync } from 'node:fs';
import { publishExample } from './examples.mjs';
import { rewriteGuideLinks } from './guide-migration.mjs';
import { revision } from './site-context.mjs';

const source = new URL('../../site/tutorials/dodge.md', import.meta.url);
const route = 'docs/tutorials/dodge';

function readDodgeTutorial() {
  // Release snapshots can predate the tutorial.
  if (revision && !existsSync(source)) return null;
  const content = readFileSync(source, 'utf8');
  const heading = /^# (.+)\r?\n/.exec(content);
  if (!heading) throw new Error('Missing title in docs/site/tutorials/dodge.md');
  return { route, title: heading[1], body: content.slice(heading[0].length).trim() };
}

export const dodgeTutorial = readDodgeTutorial();

export async function writeTutorials(writePage, conceptNames) {
  if (!dodgeTutorial) return;
  let body = rewriteGuideLinks(dodgeTutorial.body, route, conceptNames);
  for (const [marker, name] of body.matchAll(/^<!-- checkpoint: ([\w-]+) -->$/gm)) {
    const sample = await publishExample(`dodge-${name}`, route);
    if (!sample) throw new Error('Missing Dodge! checkpoint: ' + name);
    body = body.replace(marker, sample.links);
  }
  await writePage(route, dodgeTutorial.title, body);
}
