import { posix } from 'node:path';
import { readFile } from 'node:fs/promises';
import { repoLink, revision, version } from './site-context.mjs';

const guides = {
  'getting-started': {
    'Install the SDK': 'docs/start/installation',
    'Uninstall the SDK': 'docs/start/installation',
    'Create and run a game': 'docs/start/first-program',
    'Lua in VS Code': 'docs/tools/editor',
    'Build from source (contributors)': 'docs/contributing/build',
    'Try more examples': 'docs/examples',
    'Prepare your computer': 'docs/contributing/build',
    'Run Hello World': 'docs/contributing/build',
    'Create a project': 'docs/start/first-program',
  },
  sdk: {
    'Install a local development build': 'docs/contributing/build',
    'Windows SDK prefix': 'docs/contributing/build',
    'Build and install the SDK': 'docs/contributing/release',
    'Create and run a game': 'docs/start/project-structure',
    'Create a Rust game': 'docs/start/project-structure',
    'Export a game': 'docs/tools/export',
    'Export for Android': 'docs/tools/export',
  },
  api: {
    'Run and stop processes': 'docs/learn/process-model',
    'Organize a level': 'docs/patterns/level-lifecycle',
    'Choose a reference': 'docs/reference',
    'Keep resources alive': 'docs/learn/resources',
    'Choose a runtime and target': 'docs/tools/platforms',
  },
};

const guideHomes = {
  'getting-started': 'docs/start/first-program',
  sdk: 'docs/start/installation',
  api: 'docs/learn/process-model',
};

export function siteLink(from, to, hash = '') {
  const relative = posix.relative(from, to);
  return (relative ? relative + '/' : './') + hash;
}

function anchor(heading) {
  return heading.toLowerCase().replace(/[()!]/g, '').replaceAll(' ', '-');
}

function guideLink(from, page, hash) {
  const heading = Object.keys(guides[page]).find((label) => '#' + anchor(label) === hash);
  if (!heading) return siteLink(from, guideHomes[page]);
  return siteLink(from, guides[page][heading], heading === 'Try more examples' ? '#basics' : hash);
}

export function rewriteGuideLinks(body, from, conceptNames) {
  return body
    .replace(/\[(?:SDK and projects|SDK guide)\](?=\((?:\.\.\/site\/)?sdk\.md#create-a-rust-game\))/g,
      '[Rust project setup]')
    .replace('[SDK and projects](sdk.md#export-for-android)', '[Android export](sdk.md#export-for-android)')
    .replace('[SDK and projects](sdk.md)', '[Installation](sdk.md)')
    .replace('[gameplay API guides](api.md)', '[process guide](api.md)')
    .replace(/\]\((?:(?:\.\.\/site\/)?(getting-started|sdk|api))\.md(#[^)]+)?\)/g,
      (_, page, hash = '') => '](' + guideLink(from, page, hash) + ')')
    .replace(/https:\/\/github\.com\/jepemo\/malvado\/(blob|tree|edit)\/(?:main|master)\/([^\s)#]+)(#[^\s)]+)?/g,
      (_, kind, path, hash = '') => {
        const name = /^docs\/api\/(.+)\.md$/.exec(path)?.[1];
        return conceptNames.has(name)
          ? siteLink(from, 'docs/concepts/' + name, hash)
          : repoLink(path, kind) + hash;
      });
}

export async function writeLegacy(writePage, route, title, destinations) {
  const links = destinations.map(([heading, destination, hash = '#' + anchor(heading)]) =>
    '## ' + heading + '\n\n[Continue to ' + destination.title + '](' +
    siteLink(route, destination.route, hash) + ').');
  await writePage(route, title, 'Find the current location of each section.\n\n' +
    links.join('\n\n'), false);
}

function splitAt(text, marker) {
  const index = text.indexOf(marker);
  if (index < 0) throw new Error('Missing migration section: ' + marker.trim());
  return [text.slice(0, index).trimEnd(), text.slice(index).trimStart()];
}

export async function migrateGuides(writePage, conceptNames, imageRoute) {
  const sources = {};
  for (const [page, headings] of Object.entries(guides)) {
    const source = await readFile(new URL('../../site/' + page + '.md', import.meta.url), 'utf8');
    const [intro, ...parts] = source.trim().split(/^## /m);
    const sections = new Map();
    for (const part of parts) {
      const end = part.indexOf('\n');
      const heading = part.slice(0, end).trim();
      if (!headings[heading]) throw new Error('Unmapped heading in ' + page + ': ' + heading);
      sections.set(heading, '## ' + part.trimEnd());
    }
    sources[page] = { intro, sections };
  }
  // The gameplay guide uses reference-style links shared across sections.
  const apiSections = sources.api.sections;
  const runtimeHeading = 'Choose a runtime and target';
  const [runtime, definitions] = splitAt(apiSections.get(runtimeHeading), '\n[reference]:');
  apiSections.set(runtimeHeading, runtime);

  const sdkSections = sources.sdk.sections;
  const packageHeading = 'Build and install the SDK';
  const [packaging, archive] = splitAt(sdkSections.get(packageHeading), 'Check the archive before you extract it.');
  sdkSections.set(packageHeading, packaging
    .replace('[Get started](getting-started.md)', '[Build Malvado from source](../build/)') +
    '\n\n[Check and install the archive](../../start/installation/#install-from-an-archive).');

  const windowsHeading = 'Windows SDK prefix';
  const [windows, windowsPlayers] = splitAt(sdkSections.get(windowsHeading), 'Windows folder exports');
  sdkSections.set(windowsHeading, windows +
    '\n\nSee [Windows players](../../tools/platforms/#windows-players) for game requirements.');

  sdkSections.set('Install from an archive', '## Install from an archive\n\n' +
    'Download your platform archive and its checksum from [GitHub Releases](https://github.com/jepemo/malvado/releases). ' +
    'On Windows, extract the matching archive and add its bin directory to your PATH. ' +
    'Run the doctor command to check the installed tools.\n\n' +
    'For a macOS or Linux archive, use these commands.\n\n' + archive);
  sdkSections.set('Windows players', '## Windows players\n\n' + windowsPlayers);

  const page = async (route, title, intro, selection) => {
    const sections = selection.flatMap(([source, ...headings]) => headings.map((heading) => {
      const section = sources[source].sections.get(heading);
      if (section && source === 'api') return section + '\n\n' + definitions;
      return section;
    })).filter(Boolean);
    let body = [intro, ...sections].filter(Boolean).join('\n\n');
    if (route === 'docs/contributing/build' && version) {
      body = body.replace('\ncd malvado\n', '\ncd malvado\ngit checkout --detach ' + revision + '\n');
    }
    await writePage(route, title, rewriteGuideLinks(body, route, conceptNames));
  };

  const titles = {
    'docs/start/installation': 'Installation',
    'docs/start/first-program': 'Your first Malvado program',
    'docs/start/project-structure': 'Project structure',
    'docs/tools/editor': 'Editor setup',
    'docs/tools/export': 'Build and export games',
    'docs/tools/platforms': 'Platform support',
    'docs/learn/process-model': 'Processes across languages',
    'docs/learn/resources': 'Resource ownership in games',
    'docs/patterns/level-lifecycle': 'Organize a level',
    'docs/reference': 'Choose an API reference',
    'docs/contributing/build': 'Build Malvado from source',
    'docs/contributing/release': 'Package the SDK',
    'docs/contributing/testing': 'Run the engine tests',
    'docs/examples': 'Browse examples',
  };

  const publish = (route, intro, ...selection) => page(route, titles[route], intro, selection);
  const local = (from, to, label = titles[to]) => '[' + label + '](' + siteLink(from, to) + ')';
  await publish('docs/start/installation',
    'Install a native SDK to get the Malvado command, C libraries, and experimental Rust crate. ' +
    'The prebuilt SDK can run Lua projects without a compiler. Run `malvado --version` to verify the installation. Contributors can ' +
    local('docs/start/installation', 'docs/contributing/build', 'build a development SDK from source') + '.',
    ['getting-started', 'Install the SDK', 'Uninstall the SDK'],
    ['sdk', 'Install from an archive']);
  await publish('docs/start/first-program',
    'Create a small Lua game, run it, and edit its greeting. ' +
    local('docs/start/first-program', 'docs/start/installation', 'Install the SDK') + ' first. ' +
    'After the game runs, see [what the generator created](../project-structure/).',
    ['getting-started', 'Create and run a game', 'Create a project']);
  await publish('docs/start/project-structure',
    'A generated project has a language-specific entry point. Run project commands from the project directory. ' +
    'See the [asset paths guide](' + repoLink('docs/api/asset-sources.md') + '#game-asset-paths) for file lookup.',
    ['sdk', 'Create and run a game', 'Create a Rust game']);
  await publish('docs/tools/editor',
    'Use [Lua Language Server](' + siteLink('docs/tools/editor', 'docs/concepts/lua-language-server') +
    ') for completion and type checking in Lua projects.',
    ['getting-started', 'Lua in VS Code']);
  await publish('docs/tools/export',
    'Use the build command to compile a native C or experimental Rust game. Export from the same project directory to share the game.',
    ['sdk', 'Export a game', 'Export for Android']);
  await publish('docs/tools/platforms',
    'Check the [platform matrix](' + repoLink('docs/development/platforms.md') +
    ') for supported SDKs, export targets and graphics requirements.',
    ['api', 'Choose a runtime and target'], ['sdk', 'Windows players']);
  await publish('docs/learn/process-model',
    sources.api.intro.replace(/^# .+\n/, '').trim() + '\n\n' +
    'Start with [Lua processes](../../concepts/lua-processes/), then see how to ' +
    '[start and stop them](../../concepts/lua-processes/#start-and-stop), ' +
    '[wait between activations](../../concepts/lua-processes/#process-timing), ' +
    '[manage children](../../concepts/lua-processes/#children-and-ownership), and ' +
    '[send messages](../../concepts/lua-processes/#send-and-receive-messages).',
    ['api', 'Run and stop processes']);
  await publish('docs/learn/resources',
    'Use the resource handles provided by your language. See the [images guide](' +
    siteLink('docs/learn/resources', 'docs/concepts/images') + ') and [asset paths](' +
    repoLink('docs/api/asset-sources.md') + '#game-asset-paths) for loading assets.',
    ['api', 'Keep resources alive']);
  if (apiSections.has('Organize a level')) {
    await publish('docs/patterns/level-lifecycle',
      'Use process ownership to keep the lifetime of the active level independent of the game score.',
      ['api', 'Organize a level']);
  } else {
    await publish('docs/patterns/level-lifecycle',
      'Use [attached children](' + siteLink('docs/patterns/level-lifecycle', 'docs/concepts/lua-processes', '#children-and-ownership') +
      ') for processes that should end when their owner ends. See the [process tree example](' +
      repoLink('examples/process-tree', 'tree') + ') for a group of processes with a shared lifetime.');
  }
  await publish('docs/reference',
    'Find exact names, types and behavior for each language. Use the [Lua API](./lua/), the ' +
    '[C reference](' + repoLink('docs/api/current-c-api.md') + '), and the ' +
    '[Rust support policy (experimental)](' + repoLink('docs/api/rust-support.md') + '). ' +
    'See the [language matrix](' + repoLink('docs/api/language-parity.md') + ') for feature coverage.',
    ['api', 'Choose a reference']);
  await publish('docs/contributing/build',
    'Build the engine to work on the source or install a local development SDK.',
    ['getting-started', 'Build from source (contributors)', 'Prepare your computer', 'Run Hello World'],
    ['sdk', 'Install a local development build', 'Windows SDK prefix']);
  await publish('docs/contributing/release',
    'For the release workflow and documentation snapshots, see [publishing](' +
    repoLink('docs/development/publishing.md') + ') and [distribution](' + repoLink('docs/development/distribution.md') + ').',
    ['sdk', 'Build and install the SDK']);
  const more = sources['getting-started'].sections.get('Try more examples');
  const [, testing, ...featureLessons] = more.split(/\n\n/);
  await publish('docs/contributing/testing',
    testing + '\n\nSee [verification and CI](' + repoLink('docs/development/ci.md') + ') for platform and release checks.');
  await publish('docs/examples', [
    '## Basics',
    featureLessons.join('\n\n'),
    '- [Meteors](' + repoLink('examples/meteors', 'tree') + ') shows three independent Lua processes.',
    '- [First Rust games (experimental)](./rust-basics/) shows Hello World and moving balls.',
    '## Graphics and effects',
    '- [Draw a PNG](' + (imageRoute ? siteLink('docs/examples', imageRoute) : '../concepts/images/') +
      ') and [animate a process](../concepts/animations/).',
    '- [Blinking text](' + repoLink('examples/blinking-text', 'tree') + ') changes the color of Lua text.',
    '## Gameplay mechanics',
    '- [Move](' + repoLink('examples/move', 'tree') + ') reads the arrow keys in Lua.',
    '- [World collision](' + repoLink('examples/world-collision', 'tree') + ') uses collision against static geometry in Lua.',
    '## Complete games',
    '- [Catch!](./catch/) is the same game in Lua, C, and experimental Rust.',
    '- [Space Asteroids](' + repoLink('examples/space_asteroids', 'tree') + ') is a Lua game.',
    'See the [repository examples index](' + repoLink('examples/README.md') + ') for more.',
  ].join('\n\n'));

  for (const [route, headings] of Object.entries(guides)) {
    const destinations = Object.entries(headings).map(([heading, target]) => {
      let hash = '#' + anchor(heading);
      if (heading === 'Try more examples') hash = '#basics';
      if (!sources[route].sections.has(heading)) hash = '';
      return [heading, { route: target, title: titles[target] }, hash];
    });
    await writeLegacy(writePage, route, sources[route].intro.split('\n')[0].slice(2), destinations);
  }
}
