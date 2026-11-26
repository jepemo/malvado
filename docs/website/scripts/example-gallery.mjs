import { publishExample } from './examples.mjs';
import { siteLink } from './guide-migration.mjs';
import { learnLessons } from './learn-lessons.mjs';
import { repoLink, revision } from './site-context.mjs';

const galleryRoute = 'docs/examples';
const groups = [
  { id: 'basics', title: 'Basics' },
  { id: 'graphics-and-effects', title: 'Graphics and effects' },
  { id: 'gameplay-mechanics', title: 'Gameplay mechanics' },
  { id: 'complete-games', title: 'Complete games' },
];
const entries = [
  {
    id: 'hello-world', group: 'basics', title: 'Hello World', lua: 'hello',
    summary: 'Open a window and draw a greeting from a process.',
    result: 'A white “Hello World” greeting appears near the top left. Press Escape or close the window to exit.',
    reference: ['Your first Malvado program', 'docs/start/first-program'],
  },
  {
    id: 'shapes', group: 'basics', title: 'Draw shapes', lua: 'shape-coordinates', file: 'shapes.lua',
    summary: 'Place a rectangle, line and circle with screen coordinates.',
    result: 'A line connects the lower left of a blue rectangle to the center of an orange circle. Press Escape or close the window to exit.',
    preview: { width: 260, height: 140, alt: 'A white line joins a blue rectangle and an orange circle on a black background.' },
    reference: ['Drawing and coordinates', 'docs/learn/drawing-and-coordinates'],
  },
  {
    id: 'movement', group: 'basics', title: 'Move with input', lua: 'input-movement', file: 'movement.lua',
    summary: 'Read the keyboard and mouse to move a circle and place a marker.',
    result: 'Move the circle with the arrow keys. Press Space to center it. Click to move the marker. Press Escape or close the window to exit.',
    reference: ['Keyboard input API', 'docs/reference/lua/input'],
  },
  {
    id: 'two-workers', group: 'basics', title: 'Run two processes', lua: 'two-workers',
    c: 'two-workers-c', rust: 'two-workers-rust', headless: true,
    summary: 'Give two independent workers a turn on every tick.',
    result: 'The workers finish after this trace. The window stays closed.\n\n```text\nfirst: 1\nsecond: 1\nfirst: 2\nsecond: 2\n```',
    reference: ['Thinking in processes', 'docs/learn/process-model'],
  },
  {
    id: 'meteors', group: 'basics', title: 'Move three meteors', lua: 'meteors',
    summary: 'Give each process its own speed and position.',
    result: 'Three circles fall at different speeds. Each wraps to the top after leaving the screen. Press Escape or close the window to exit.',
    reference: ['Frames and time', 'docs/learn/frames-and-time'],
  },
  {
    id: 'image', group: 'graphics-and-effects', title: 'Draw a PNG', lua: 'image', c: 'image-c', rust: 'image-rust',
    assets: ['tile.png'],
    credit: 'The red tile is original Malvado artwork. The ZIP includes the Apache 2.0 license and credit.',
    summary: 'Load a PNG after opening a window and draw it at its original size.',
    result: 'A red tile appears at (32, 32) in a 160 × 120 window. The terminal prints `image: 32x32`. The window closes about two seconds later. You can close it earlier.',
    preview: { width: 160, height: 120, alt: 'One red tile on a black background.' },
    reference: ['Images and sprites API', 'docs/reference/lua/images'],
  },
  {
    id: 'sprite-animation', group: 'graphics-and-effects', title: 'Animate a sprite', lua: 'aseprite-bounce',
    assets: ['sheet.json', 'sheet.png'],
    credit: 'The atlas and sprite artwork are original Malvado assets. The ZIP includes the Apache 2.0 license and credit.',
    summary: 'Play an animation from an Aseprite atlas with per-frame timing.',
    result: 'The sprite loops through the “bounce” animation. The atlas defines how long each frame appears and keeps frames aligned around a shared pivot. Press Escape or close the window to exit.',
    reference: ['Animations API', 'docs/reference/lua/animations'],
  },
  {
    id: 'blinking-text', group: 'graphics-and-effects', title: 'Fade text', lua: 'blinking-text',
    summary: 'Animate the opacity of several labels that share a font.',
    result: 'Three copies of a centered greeting share a built-in font. Their colors fade and brighten at different times. Close the window to exit.',
    requirement: 'Use a [development SDK](../../contributing/build/#install-a-local-development-build) with text alignment support.',
    reference: ['Text API', 'docs/reference/lua/text'],
  },
  {
    id: 'collision', group: 'gameplay-mechanics', title: 'Detect an overlap', lua: 'circle-collision',
    summary: 'Check for a collision between two process graphics.',
    result: 'Move the circle with the arrow keys. A message appears while the circle overlaps the square. Press Escape or close the window to exit.',
    reference: ['Collision API', 'docs/reference/lua/collision'],
  },
  {
    id: 'world-collision', group: 'gameplay-mechanics', title: 'Stop at a wall', lua: 'world-wall',
    summary: 'Move a player against static collision geometry.',
    result: 'Use the arrow keys to move the circle. The wall stops it. Press Escape or close the window to exit.',
    reference: ['World API', 'docs/reference/lua/world'],
  },
  {
    id: 'audio', group: 'gameplay-mechanics', title: 'Play and loop audio', lua: 'audio-tone',
    assets: ['tone.wav'],
    credit: 'Malvado generated the 440 Hz tone. The ZIP includes the Apache 2.0 license and credit.',
    requirement: 'This example also needs an audio output device.',
    summary: 'Overlap short sounds and toggle a repeating audio stream.',
    result: 'Press 1 and 2 close together to play the same tone at two volumes. Press Space to toggle the loop. Press Escape or close the window to exit.',
    reference: ['Audio API', 'docs/reference/lua/audio'],
  },
  {
    id: 'camera-follow', group: 'gameplay-mechanics', title: 'Follow a player', lua: 'camera-follow',
    summary: 'Keep a moving player centered while the HUD stays in screen space.',
    result: 'Move the blue player with the arrow keys. The orange wall scrolls past while the on-screen instructions stay fixed. Press Escape or close the window to exit.',
    reference: ['Cameras API', 'docs/reference/lua/cameras'],
  },
  {
    id: 'catch', group: 'complete-games', title: 'Catch!', lua: 'catch-lua', c: 'catch-c', rust: 'catch-rust',
    summary: 'Move a paddle, catch a falling ball and keep score.',
    result: 'Use Left and Right to move the cyan paddle. The orange ball falls and returns to the top after a catch or a miss. Each catch adds a point. The paddle and ball run as independent processes in all three versions. Press Escape or close the window to exit.',
    preview: { width: 640, height: 360, alt: 'A falling orange ball, a cyan paddle and Score: 0 in the upper left.' },
    reference: ['Thinking in processes', 'docs/learn/process-model'],
    guide: 'examples/catch',
  },
  {
    id: 'space-asteroids', group: 'complete-games', title: 'Space Asteroids', lua: 'space-asteroids',
    assets: ['asteroid.png', 'spaceship.png'],
    credit: 'The ship and asteroid are original Malvado artwork. The ZIP includes the Apache 2.0 license and credit.',
    summary: 'Shoot asteroids, spawn fragments on impact and survive the round.',
    result: 'Turn with Left and Right, thrust with Up and press Space to fire. A hit splits a large asteroid into three smaller ones. Destroy all 20 targets to win. A collision with your ship ends the round. Press Escape to exit.',
    reference: ['Process creation API', 'docs/reference/lua/processes/spawn'],
  },
];

const labels = { lua: 'Lua', c: 'C', rust: 'Rust (experimental)' };

function runInstructions(entry, variant) {
  const { language, filename } = variant;
  if (language === 'lua') {
    const setup = entry.assets
      ? `Extract the ZIP into an empty directory. Open a terminal there with ${filename} and ${entry.assets.join(' and ')}.`
      : `Download ${filename} into an empty directory and open a terminal there.`;
    return { setup, commands: `malvado run ${entry.headless ? '--headless ' : ''}${filename}` };
  }

  const starterFile = language === 'c' ? 'main.c' : 'main.rs';
  const setup = entry.assets
    ? `Extract the ZIP into an empty directory and open a terminal there. Create a project with the SDK, then copy the source and ${entry.assets.join(' and ')} into it.`
    : `Download ${filename} into an empty directory and open a terminal there. Create a project with the SDK and replace the starter source.`;
  const commands = [
    `malvado new --lang ${language} lesson`,
    `cp ${filename} lesson/${starterFile}`,
    ...(entry.assets ?? []).map((asset) => `cp ${asset} lesson/`),
    'cd lesson',
    'malvado run',
  ].join('\n');
  return { setup, commands };
}

export async function writeExampleGallery(writePage) {
  const published = [];
  for (const entry of entries) {
    const route = `${galleryRoute}/${entry.id}`;
    const variants = [];
    for (const language of Object.keys(labels)) {
      if (!entry[language]) continue;
      const variant = await publishExample(entry[language], route, language);
      if (!variant) continue;
      const { code, filename, sourceUrl, downloadUrl, downloadLabel } = variant;
      variants.push({ language, code, filename, sourceUrl, downloadUrl, downloadLabel,
        ...runInstructions(entry, variant) });
    }
    if (!variants.length) continue;

    const tools = variants.some(({ language }) => language !== 'lua')
      ? ' C and Rust need CMake and a C compiler. Rust also needs Cargo and rustc.' : '';
    const [referenceLabel, referenceRoute] = entry.id === 'meteors' &&
        !learnLessons.some(({ name }) => name === 'frames-and-time')
      ? ['Lua processes', 'docs/concepts/lua-processes'] : entry.reference;
    const guide = entry.guide ? ` See the [shared example README](${repoLink(entry.guide, 'tree')}).` : '';
    // Screenshots show the current sources. Snapshots show the result in words.
    const preview = entry.preview && !revision
      ? `<img src="${siteLink(route, 'gallery-previews')}${entry.id}.png" alt=${JSON.stringify(entry.preview.alt)} width={${entry.preview.width}} height={${entry.preview.height}} loading="lazy" />`
      : '';
    await writePage(route, entry.title, [
      "import LanguageTabs from '../../../../components/LanguageTabs.astro';",
      entry.summary,
      entry.id === 'catch' ? '<span id="catch"></span>' : '',
      '## What you will see',
      entry.result,
      preview,
      '## Run and read the code',
      `Install a [desktop Malvado SDK](${siteLink(route, 'docs/start/installation')}).${entry.headless ? '' : ' Use a display.'}${tools}${entry.requirement ? ' ' + entry.requirement : ''}`,
      `<LanguageTabs id=${JSON.stringify(entry.id)} variants={${JSON.stringify(variants)}} />`,
      entry.credit,
      `See [${referenceLabel}](${siteLink(route, referenceRoute)}).${guide} [Browse all examples](../).`,
    ].filter(Boolean).join('\n\n'), true, 'mdx');

    published.push({
      id: entry.id, group: entry.group, title: entry.title, href: siteLink(galleryRoute, route), summary: entry.summary,
      difficulty: entry.group === 'basics' || entry.id === 'image' ? 'Beginner' : 'Intermediate',
      languages: variants.map(({ language }) => labels[language]).join(', '),
    });
  }

  const sections = groups.map((group) => ({
    ...group,
    entries: published.filter(({ group: id }) => id === group.id),
  })).filter(({ entries }) => entries.length);

  await writePage(galleryRoute, 'Browse examples', [
    "import ExampleGallery from '../../../../components/ExampleGallery.astro';",
    'Start with a window and a few shapes. Then try input, graphics and complete games.',
    `<ExampleGallery groups={${JSON.stringify(sections)}} />`,
    'Try the [first Rust games (experimental)](./rust-basics/) or explore the ' +
      `[aligned SDK lessons](${repoLink('examples/features/README.md')}) for startup and window resizing. ` +
      `See the [repository examples](${repoLink('examples/README.md')}) for more.`,
  ].join('\n\n'), true, 'mdx');

  return new Set(published.map(({ id }) => id));
}
