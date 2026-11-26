import { existsSync, readFileSync } from 'node:fs';
import { publishExample } from './examples.mjs';
import { rewriteGuideLinks, siteLink } from './guide-migration.mjs';
import { revision } from './site-context.mjs';

const directory = new URL('../../site/learn/', import.meta.url);
const route = (name) => 'docs/learn/' + name;

const lessons = {
  'process-model': 'two-workers',
  'process-lifecycle': 'level-teardown',
  'frames-and-time': 'movement-time',
  'process-communication': 'mailbox',
};

// Older release snapshots still publish the original process guide.
export const processLessons = revision && !existsSync(directory) ? [] :
  Object.entries(lessons).map(([name, example]) => {
    const source = readFileSync(new URL(name + '.md', directory), 'utf8');
    const end = source.indexOf('\n');
    return { slug: route(name), label: source.slice(2, end), body: source.slice(end + 1).trim(), example };
  });

export async function writeProcessLessons(writePage, conceptNames) {
  for (const { slug, label, body, example } of processLessons) {
    const sample = await publishExample(example, slug);
    if (!sample) throw new Error('Missing process lesson example: ' + example);
    const content = rewriteGuideLinks(body, slug, conceptNames)
      .replace(/\]\(([\w-]+)\.md\)/g, (link, name) =>
        Object.hasOwn(lessons, name) ? '](' + siteLink(slug, route(name)) + ')' : link)
      .replace('<!-- example -->', sample.links + '\n\n' + sample.snippet);
    await writePage(slug, label, content);
  }
}
