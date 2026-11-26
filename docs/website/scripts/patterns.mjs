import { existsSync, readFileSync } from 'node:fs';
import { publishExample, recipeNames } from './examples.mjs';
import { rewriteGuideLinks } from './guide-migration.mjs';
import { revision } from './site-context.mjs';

const directory = new URL('../../site/patterns/', import.meta.url);

// Release snapshots may predate the recipes.
export const patterns = recipeNames.flatMap((name) => {
  const path = new URL(name + '.md', directory);
  if (revision && !existsSync(path)) return [];
  const source = readFileSync(path, 'utf8');
  const heading = /^# (.+)\r?\n/.exec(source);
  if (!heading) throw new Error('Missing title in ' + path.pathname);
  return [{ name, slug: 'docs/patterns/' + name, label: heading[1],
    body: source.slice(heading[0].length).trim() }];
});

export async function writePatterns(writePage, conceptNames) {
  const marker = '<!-- example -->';
  for (const { name, slug, label, body } of patterns) {
    if (!body.includes(marker)) throw new Error('Missing recipe example marker: ' + name);
    const sample = await publishExample('recipe-' + name, slug);
    if (!sample) throw new Error('Missing recipe example: ' + name);
    const content = rewriteGuideLinks(body, slug, conceptNames)
      .replace(marker, sample.links + '\n\n' + sample.snippet);
    await writePage(slug, label, content);
  }
}
