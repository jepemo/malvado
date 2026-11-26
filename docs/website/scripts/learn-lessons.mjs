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
  'drawing-and-coordinates': 'shape-coordinates',
  'input-and-movement': 'input-movement',
  'graphics-and-sprites': 'select-sprites',
  animation: 'aseprite-bounce',
  collisions: 'circle-collision',
  'text-and-fonts': 'text-labels',
  'sound-and-music': 'audio-tone',
  'cameras-and-scrolling': 'camera-follow',
  resources: 'select-sprites',
};

const playgrounds = {
  'process-model': 'Scheduler',
  collisions: 'Collision',
  'cameras-and-scrolling': 'Camera',
  animation: 'Animation',
};
const playgroundMarker = '<!-- playground -->';

// Release snapshots may predate some or all of these lessons.
export const learnLessons = Object.entries(lessons).flatMap(([name, example]) => {
  const path = new URL(name + '.md', directory);
  if (revision && !existsSync(path)) return [];
  const source = readFileSync(path, 'utf8');
  const end = source.indexOf('\n');
  const body = source.slice(end + 1).trim();
  const playground = playgrounds[name];
  if (playground && !revision && !body.includes(playgroundMarker))
    throw new Error('Missing playground marker in ' + name);
  return [{ name, slug: route(name), label: source.slice(2, end), body, example,
    playground: body.includes(playgroundMarker) ? playground : undefined }];
});

export async function writeLearnLessons(writePage, conceptNames) {
  for (const { slug, label, body, example, playground } of learnLessons) {
    const sample = await publishExample(example, slug);
    if (!sample) throw new Error('Missing Learn example: ' + example);
    const content = rewriteGuideLinks(body, slug, conceptNames)
      .replace(/\]\(([\w-]+)\.md\)/g, (link, name) =>
        learnLessons.some((lesson) => lesson.name === name) ? '](' + siteLink(slug, route(name)) + ')' : link)
      .replace('<!-- example -->', sample.links + '\n\n' + sample.snippet);
    if (playground) {
      await writePage(slug, label, [
        `import ${playground} from '../../../../components/playgrounds/${playground}.astro';`,
        content.replace(playgroundMarker, `<${playground} />`),
      ].join('\n\n'), true, 'mdx');
    } else {
      await writePage(slug, label, content);
    }
  }
}
