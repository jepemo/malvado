import { readFile, writeFile, mkdir, rm } from 'node:fs/promises';
import { repoLink, revision } from './site-context.mjs';
import { publishExample } from './examples.mjs';
import { learnLessons } from './learn-lessons.mjs';
import { siteLink } from './guide-migration.mjs';
import content from './lua-api-content.mjs';
import { checkLuaReference } from './check-lua-reference.mjs';

// Generate the Lua API reference from the repository inventories:
// docs/api/lua-namespaces.md and docs/api/lua-globals.md.
const repo = new URL('../../../', import.meta.url);
const read = (path) => readFile(new URL(path, repo), 'utf8');

// One page per function, grouped by namespace. The `ns` field matches a
// `mv.<ns>.` prefix in the namespace table. `flat` lists the global aliases
// installed by import_globals(). `guide` is a concept page, or a repository
// path when the guide is not part of the website.
const groups = [
  {
    id: 'processes', label: 'Processes', ns: 'process', guide: 'lua-processes',
    summary: 'Start, control and inspect processes.',
    flat: ['process', 'spawn', 'find', 'children', 'descendants', 'snapshot', 'frame',
      'signal', 'signal_tree', 'set_priority', 'get_priority', 'kill', 'alive',
      'kill_tree', 'status', 'send', 'recv'],
    constants: ['SIGNAL_SLEEP', 'SIGNAL_FREEZE', 'SIGNAL_WAKEUP', 'SIGNAL_KILL',
      'STATUS_RUNNING', 'STATUS_SLEEPING', 'STATUS_FROZEN', 'STATUS_DEAD'],
  },
  {
    id: 'input', label: 'Input', ns: 'input', guide: 'keyboard',
    summary: 'Read the keyboard, mouse, controllers and typed text.',
    flat: ['key', 'key_pressed', 'key_released', 'mouse_position', 'mouse_button',
      'mouse_pressed', 'mouse_released', 'pad_connected', 'pad_button', 'pad_pressed',
      'pad_released', 'pad_axis', 'text_input_next', 'text_input_set_keyboard'],
    constants: ['KEY_A to KEY_Z', 'KEY_0 to KEY_9', 'KEY_ESCAPE', 'KEY_SPACE',
      'KEY_LEFT', 'KEY_RIGHT', 'KEY_UP', 'KEY_DOWN', 'KEY_ENTER', 'KEY_TAB',
      'KEY_BACKSPACE', 'MOUSE_LEFT', 'MOUSE_MIDDLE', 'MOUSE_RIGHT',
      'PAD_A', 'PAD_B', 'PAD_X', 'PAD_Y', 'PAD_UP', 'PAD_DOWN', 'PAD_LEFT', 'PAD_RIGHT',
      'PAD_LB', 'PAD_RB', 'PAD_BACK', 'PAD_START', 'PAD_LEFT_X', 'PAD_LEFT_Y',
      'PAD_RIGHT_X', 'PAD_RIGHT_Y', 'PAD_LEFT_TRIGGER', 'PAD_RIGHT_TRIGGER'],
  },
  {
    id: 'graphics', label: 'Graphics', ns: 'graphics', guide: 'drawing',
    summary: 'Create reusable process graphics, regions and scrolls.',
    flat: ['rect', 'circle', 'region', 'scroll', 'animation', 'graph', 'animation_elapsed', 'animation_finished'],
  },
  {
    id: 'drawing', label: 'Drawing', guide: 'drawing',
    summary: 'Draw shapes and text, and set the drawing state.',
    flat: ['draw_rect', 'draw_circle', 'draw_line', 'draw_text',
      'set_color', 'clear_screen'],
  },
  {
    id: 'images', label: 'Images and sprites', guide: 'images',
    summary: 'Load image and atlas assets and draw them.',
    flat: ['set_assets_dir', 'get_assets_dir', 'load_asset', 'load_image', 'image_size', 'draw_image',
      'parse_atlas', 'parse_aseprite', 'draw_sprite', 'atlas_sprite_info', 'atlas_sprite_id',
      'atlas_animation_frame', 'atlas_animation_color', 'atlas_animation_repeats', 'atlas_pivot_color'],
  },
  {
    id: 'animations', label: 'Animations', guide: 'animations',
    summary: 'Sample a frame from a reusable animation.',
    flat: ['animation_sample'],
  },
  {
    id: 'text', label: 'Text', guide: 'text',
    summary: 'Create persistent text and load fonts.',
    flat: ['font', 'load_font', 'write', 'change_text', 'delete_text', 'text_size'],
    constants: ['TEXT_ALIGN_LEFT', 'TEXT_ALIGN_CENTER', 'TEXT_ALIGN_RIGHT'],
  },
  {
    id: 'audio', label: 'Audio', guide: 'audio',
    summary: 'Load and play sounds and music.',
    flat: ['load_sound', 'play_sound', 'stop_sound', 'pause_sound',
      'resume_sound', 'sound_playing', 'sound_volume', 'load_music', 'play_music',
      'update_music', 'stop_music', 'pause_music', 'resume_music', 'music_playing',
      'music_volume'],
  },
  {
    id: 'cameras', label: 'Cameras', guide: 'cameras',
    summary: 'Create and move 2D cameras.',
    flat: ['camera_follow', 'camera_create', 'camera_set', 'camera_move', 'camera_position', 'camera_destroy'],
  },
  {
    id: 'collision', label: 'Collision', ns: 'collision', guide: 'collision',
    summary: 'Build shape colliders and query collisions.',
    flat: ['collision', 'rect_overlaps', 'rect_contains'],
  },
  {
    id: 'world', label: 'Worlds', guide: 'collision',
    summary: 'Move processes through a static collision world.',
    flat: ['world_create', 'world_add_rect', 'world_overlap', 'world_sweep', 'world_collision', 'world_move'],
  },
  {
    id: 'fpg', label: 'FPG', ns: 'fpg', guide: 'docs/api/fpg.md',
    summary: 'Parse FPG graphics libraries and read their sprite metadata. Requires asset support.',
    flat: ['fpg_count', 'fpg_id_at', 'fpg_palette', 'fpg_gamma', 'fpg_sprite', 'fpg_point',
      'fpg_rgba', 'fpg_image', 'fpg_png', 'fpg_json', 'manifest_count', 'manifest_id_at',
      'manifest_palette', 'manifest_gamma', 'manifest_sprite', 'manifest_point', 'manifest_transparent_zero'],
  },
  {
    id: 'platform', label: 'Platform', guide: 'runner-timing',
    summary: 'Start the game, change the window and read the clock.',
    flat: ['start', 'set_fps', 'set_title', 'set_window_size', 'window_size', 'time_seconds', 'fade_to', 'fade_off',
      'fade_on', 'fade_active', 'fade_color', 'rand', 'rand_seed'],
  },
  {
    id: 'module', label: 'Module', guide: 'lua-processes',
    summary: 'Functions on the module table itself.',
    module: [
      { name: 'import_globals', summary: 'Install the flat global functions, with conflict checks.' },
      { name: 'legacy_fpg', summary: 'Load a legacy image sheet or directory.' },
    ],
  },
];

// Short summaries for flat functions with no namespace entry. The namespace
// table already describes the rest.
const flatSummaries = {
  process: 'Define a process. Call `process(fn, options)`; `mv.process` also holds the namespace functions.',
  collision: 'Find colliding processes. Call `collision(target)` to query the current process; `mv.collision` also holds the shape constructors.',
  draw_rect: 'Draw a filled or outlined rectangle.',
  draw_circle: 'Draw a filled or outlined circle.',
  draw_line: 'Draw a line between two points.',
  draw_text: 'Queue text with an optional font and horizontal alignment.',
  set_color: 'Set the color that later drawing calls use.',
  clear_screen: 'Clear the window to a color.',
  set_assets_dir: 'Set the directory for subsequent asset loads.',
  get_assets_dir: 'Return the absolute asset directory.',
  load_asset: 'Load an image or atlas asset by path.',
  load_image: 'Load a PNG image.',
  image_size: 'Read the width and height of an image.',
  draw_image: 'Queue a full image or source rectangle.',
  parse_atlas: 'Parse Malvado atlas metadata in memory.',
  parse_aseprite: 'Parse Aseprite atlas metadata in memory.',
  draw_sprite: 'Draw an atlas sprite.',
  atlas_sprite_info: 'Read sprite metadata.',
  atlas_sprite_id: 'Find a sprite id by name.',
  atlas_animation_frame: 'Find the sprite ID at a time in a named atlas animation.',
  atlas_animation_color: 'Read the imported color of an animation tag.',
  atlas_pivot_color: 'Read the pivot color of an atlas sprite.',
  atlas_animation_repeats: 'Read the imported repeat count of an animation tag.',
  font: 'Create a built-in font descriptor with a default size.',
  load_font: 'Load a TTF or OTF font.',
  write: 'Create persistent text owned by the current process.',
  change_text: 'Change one persistent text with a partial options table.',
  delete_text: 'Remove persistent text immediately.',
  text_size: 'Measure text in pixels without drawing it.',
  load_sound: 'Load a reusable WAV sample.',
  play_sound: 'Start an independent voice.',
  stop_sound: 'Stop a voice.',
  pause_sound: 'Pause a voice.',
  resume_sound: 'Resume a paused voice.',
  sound_playing: 'Check whether a voice is playing.',
  sound_volume: 'Set a voice volume.',
  load_music: 'Load a music stream.',
  play_music: 'Play a music stream.',
  update_music: 'Update the music stream once.',
  stop_music: 'Stop the music.',
  pause_music: 'Pause the music.',
  resume_music: 'Resume paused music.',
  music_playing: 'Check whether music is playing.',
  music_volume: 'Set the music volume.',
  start: 'Start the game loop.',
  set_fps: 'Set the default cadence for future Lua processes.',
  set_title: 'Change the window title.',
  set_window_size: 'Resize the current game window.',
  window_size: 'Read the window size.',
  time_seconds: 'Read the monotonic clock.',
  fade_to: 'Fade the screen to a color.',
  fade_off: 'Fade to opaque black.',
  fade_on: 'Fade back to transparent.',
  fade_active: 'Check whether a fade is running.',
  fade_color: 'Read the current fade color.',
  rand: 'Return a random integer in an inclusive range.',
  rand_seed: 'Reseed the shared Lua random generator.',
  rect_overlaps: 'Test whether two rectangles overlap.',
  rect_contains: 'Test whether a rectangle contains a point.',
};

function parseNamespaceFunctions(markdown) {
  const functions = [];
  for (const row of markdown.matchAll(/^\| `mv\.([a-z_]+)\.([a-z_]+)` \| ([^|]+?) \|/gm))
    functions.push({ ns: row[1], name: row[2], summary: row[3].trim() });
  return functions;
}

function parseGlobals(markdown) {
  const globals = [];
  for (const row of markdown.matchAll(/^\| `([A-Za-z0-9_]+)` \| `mv\.([A-Za-z0-9_.]+)` \|/gm))
    globals.push({ name: row[1], field: row[2] });
  return globals;
}

const namespaceFunctions = parseNamespaceFunctions(await read('docs/api/lua-namespaces.md'));
const globalInventory = await read('docs/api/lua-globals.md');
const globals = parseGlobals(globalInventory);
if (namespaceFunctions.length < 40 || globals.length < 100)
  throw new Error('Unexpected Lua inventory shape; check docs/api/lua-*.md');

// One entry per function: a namespaced function, a flat alias, or both.
for (const group of groups) {
  const entries = new Map();
  for (const fn of namespaceFunctions.filter((item) => item.ns === group.ns))
    entries.set(fn.name, { name: fn.name, title: `mv.${group.ns}.${fn.name}`, summary: fn.summary, flat: [] });
  for (const global of group.flat ?? []) {
    const alias = [...entries.values()].find((entry) =>
      entry.name === global || entry.name === global.replace(`${group.ns}_`, ''));
    if (alias) {
      alias.title = global;
      alias.flat.push(global);
    } else {
      const summary = flatSummaries[global] ?? content[`${group.id}/${global}`]?.description;
      if (!summary)
        throw new Error(`Missing summary for flat function: ${global}`);
      entries.set(global, { name: global, title: global, summary, flat: [global] });
    }
  }
  for (const fn of group.module ?? [])
    entries.set(fn.name, { name: fn.name, title: fn.name, summary: fn.summary, flat: [] });
  group.entries = [...entries.values()].sort((a, b) => a.title.localeCompare(b.title));
  for (const entry of group.entries)
    entry.slug = entry.flat[0] ?? entry.name;
}

// Public functions and their aliases must land in a page.
const allEntries = groups.flatMap((group) => group.entries.map((entry) => ({ ...entry, group: group.id })));
const constants = ({ name, field }) => field.startsWith('colors.') || /^[A-Z][A-Z0-9_]*$/.test(name);
const palette = globals.filter((entry) => entry.field.startsWith('colors.'));
groups.find((group) => group.id === 'drawing').constants = palette.map((entry) => entry.name);

if (!revision) {
  const globalNames = new Set(globals.filter((entry) => !constants(entry)).map(({ name }) => name));
  const withImport = (field) => globalNames.has(field.split('.')[0]) ? [`mv.${field}`, field] : [`mv.${field}`];
  const publicForms = new Set([
    ...globals.filter((entry) => !constants(entry)).flatMap(({ name, field }) => [name, `mv.${field}`]),
    ...namespaceFunctions.flatMap(({ ns, name }) => withImport(`${ns}.${name}`)),
    ...Array.from(globalInventory.matchAll(/^- \`(mv\.[a-z_0-9]+)\`:/gm), (row) => row[1]),
  ]);
  const methods = Array.from((await read('docs/api/lua-instance-methods.md'))
    .matchAll(/^\| \`([^\`]+:[^\`]+)\` \| \`([^\`]+)\` \|/gm), (row) => row.slice(1));
  const entries = groups.flatMap((group) => group.entries.map((entry) => ({
    key: `${group.id}/${entry.slug}`,
    forms: [
      ...entry.flat.flatMap(withImport),
      ...namespaceFunctions.filter(({ ns, name }) => ns === group.ns && name === entry.name)
        .flatMap(({ ns, name }) => withImport(`${ns}.${name}`)),
      ...(group.module?.some(({ name }) => name === entry.name) ? [`mv.${entry.name}`] : []),
    ],
  })));
  checkLuaReference(entries, content, publicForms, methods);
}

const output = new URL('../src/content/docs/docs/reference/lua/', import.meta.url);
await rm(output, { recursive: true, force: true });
await mkdir(output, { recursive: true });

const familyDetails = {
  processes: 'See [process properties](../../../concepts/lua-processes/#process-data), [signal and status values](../../../concepts/lua-processes/#control-processes), and [instance methods](../../../concepts/lua-instance-methods/#process-handles).',
  input: 'See the [input conventions](../../../concepts/lua-processes/#lua-api-conventions) and the [input inventory](' + repoLink('docs/api/input-inventory.md') + ') for the names and types used by each device.',
  graphics: 'See [process graphics](../../../concepts/drawing/#process-graphics), [clipping regions](../../../concepts/drawing/#clip-a-lua-process-to-a-region), and [scrolling](../../../concepts/drawing/#scroll-a-lua-process).',
  drawing: 'See the [palette values and color channels](../../../concepts/drawing/#lua-colors).',
  text: 'See [font descriptors](../../../concepts/text/#lua-fonts-and-styles) and [alignment values](../../../concepts/text/#lua-process-text).',
  audio: 'The family uses sound samples, per-playback voice handles and music stream handles. See [audio instance methods](../../../concepts/lua-instance-methods/#sound-voices-and-music).',
  cameras: 'A managed camera can use the full window or a screen-space viewport. See [camera associations and coordinates](../../../concepts/cameras/).',
  collision: 'See [reusable colliders and transforms](../../../concepts/collision/#reusable-shapes-in-lua).',
  world: 'See [world and process types](../../../concepts/collision/#contextual-process-and-world-calls), and [sweep results](../../../concepts/collision/#sweeping-a-shape).',
};

function guideLink(guide, depth) {
  if (guide.startsWith('docs/'))
    return repoLink(guide);
  return `${'../'.repeat(depth)}concepts/${guide}/`;
}

function guideLabel(guide) {
  return guide.startsWith('docs/') ? 'FPG' : guide.replace(/-/g, ' ');
}

function front(title, description, body) {
  return `---\ntitle: ${JSON.stringify(title)}\ndescription: ${JSON.stringify(description)}\n---\n\n${body}\n`;
}

await writeFile(new URL('index.md', output), front('Lua API',
  'Lua functions and constants, grouped by namespace.',
  `Load the module with \`local mv = require("malvado")\`. Use
\`mv.import_globals()\` to install the [supported global exports](${repoLink('docs/api/lua-globals.md')}).
These include functions such as \`frame\` and constants such as \`KEY_ESCAPE\`,
\`SIGNAL_KILL\` and \`RED\`. Each entry lists its supported call forms.

See [Lua API conventions](../../concepts/lua-processes/#lua-api-conventions)
for units, defaults and validation. See [instance methods](../../concepts/lua-instance-methods/)
for optional handle syntax, and the [LuaLS definitions](${repoLink('types/lib', 'tree')})
for the public types and their fields.

Try the [drawing program](./drawing/draw_rect/#run-the-example) to see filled
and outlined shapes. Use the [two workers program](./processes/frame/#run-the-example)
to see how processes take turns.

Choose a family in the sidebar to find individual functions. See the
[Lua process guide](../../concepts/lua-processes/) for the gameplay model.`));

for (const group of groups) {
  const dir = new URL(`${group.id}/`, output);
  await mkdir(dir, { recursive: true });
  const table = group.entries
    .map((entry) => `| [\`${entry.title}\`](./${entry.slug}/) | ${entry.summary} |`)
    .join('\n');
  const constants = group.constants
    ? `\nConstants: ${group.constants.map((name) => `\`${name}\``).join(', ')}.\n`
    : '';
  await writeFile(new URL('index.md', dir), front(group.label, group.summary,
    `${group.summary} See the [${guideLabel(group.guide)}](${guideLink(group.guide, 3)}) guide.

${familyDetails[group.id] ?? ''}${constants}
| Function | Summary |
| --- | --- |
${table}`));
  for (const entry of group.entries) {
    const detail = content[`${group.id}/${entry.slug}`];
    if (!detail && revision) {
      const namespaceForm = namespaceFunctions.some((fn) => fn.ns === group.ns && fn.name === entry.name)
        ? ` or \`mv.${group.ns}.${entry.name}\``
        : '';
      const call = entry.flat.length
        ? `Use \`mv.${entry.flat[0]}\`${namespaceForm}, or import globals to call \`${entry.flat[0]}\`.`
        : `Call as \`${entry.title}\`.`;
      const sentence = entry.summary.endsWith('.') ? entry.summary : `${entry.summary}.`;
      await writeFile(new URL(`${entry.slug}.md`, dir), front(entry.title, entry.summary,
        `${sentence}

${call}

See the [${guideLabel(group.guide)}](${guideLink(group.guide, 4)}) guide for arguments, errors and examples.`));
      continue;
    }
    if (!detail?.description || !detail.signatures?.length || !Array.isArray(detail.params) ||
        !detail.returns || !detail.exampleId || !detail.exampleSetup || !detail.exampleResult)
      throw new Error(`Incomplete Lua reference entry: ${group.id}/${entry.slug}`);
    const parts = [detail.description, '', '## Signatures', '',
      '```lua', ...detail.signatures, '```'];
    if (entry.flat.length || detail.signatures.some((call) => /^(process|collision)\./.test(call)))
      parts.push('', 'Call the unqualified forms after `mv.import_globals()`.');
    parts.push('');
    if (detail.params.length) {
      parts.push('## Parameters', '');
      parts.push(...detail.params.map(([name, text]) => `- \`${name}\` — ${text}`));
    } else {
      parts.push('Takes no arguments.');
    }
    parts.push('');
    parts.push('## Returns', '', detail.returns, '');
    const route = `docs/reference/lua/${group.id}/${entry.slug}`;
    const runnable = await publishExample(detail.exampleId, route);
    if (runnable) {
      parts.push('## Run the example', '', runnable.links, '', detail.exampleSetup, '',
        runnable.snippet, '', detail.exampleResult, '');
    } else {
      parts.push('## Run the example', '',
        `This example was added after this SDK snapshot. See the [current reference](https://malvado.dev/${route}/#run-the-example) for the runnable program.`, '');
    }
    if (detail.notes)
      parts.push('## Things to know', '', detail.notes, '');
    const related = (detail.related ?? [])
      .map((name) => {
        const target = allEntries.find((item) => item.slug === name && item.group === group.id) ??
          allEntries.find((item) => item.slug === name);
        if (!target) {
          if (!revision) throw new Error(`Unknown related Lua function on ${route}: ${name}`);
          return null;
        }
        const path = target.group === group.id ? `../${target.slug}/` : `../../${target.group}/${target.slug}/`;
        return `[\`${target.title}\`](${path})`;
      })
      .filter(Boolean);
    parts.push('## Related', '');
    if (related.length)
      parts.push(`See also ${related.join(', ')}.`, '');
    if (detail.playground) {
      const [name, label] = detail.playground;
      const lesson = learnLessons.find((item) => item.name === name && item.playground);
      if (lesson) {
        parts.push(`[${label}](${siteLink(route, lesson.slug, '#playground')}).`, '');
      } else if (!revision) {
        throw new Error(`Unknown Lua reference playground on ${route}: ${name}`);
      }
    }
    parts.push(`See the [${guideLabel(group.guide)}](${guideLink(group.guide, 4)}) guide for more examples and limits.`);
    await writeFile(new URL(`${entry.slug}.md`, dir), front(entry.title, entry.summary, parts.join('\n')));
  }
}

console.log(`Lua API: ${groups.reduce((sum, group) => sum + group.entries.length, 0)} function pages in ${groups.length} groups`);
for (const group of groups) {
  const documented = group.entries.filter(({ slug }) => {
    const entry = content[`${group.id}/${slug}`];
    return entry?.signatures?.length && entry.returns &&
      entry.exampleId && entry.exampleSetup && entry.exampleResult;
  }).length;
  console.log(`  ${group.label}: ${documented}/${group.entries.length} entries with signatures, returns and canonical examples`);
}
