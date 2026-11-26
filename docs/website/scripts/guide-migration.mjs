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

export async function migrateGuides(writePage, conceptNames, imageRoute, processModelTitle, resourcesTitle) {
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
    'To install manually on macOS or Linux, download the archive for your operating system and architecture ' +
    'and its matching checksum from [GitHub Releases](https://github.com/jepemo/malvado/releases). ' +
    'Open a terminal in the download directory. Use only one matching SDK archive for the commands below.\n\n' + archive);
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
    'docs/tools/cli': 'CLI reference',
    'docs/tools/editor': 'Editor setup',
    'docs/tools/export': 'Build and export games',
    'docs/tools/platforms': 'Platform support',
    'docs/learn/process-model': processModelTitle ?? 'Processes across languages',
    'docs/learn/resources': resourcesTitle ?? 'Resource ownership in games',
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
    'Install the desktop SDK for your computer so you can ' +
    local('docs/start/installation', 'docs/start/first-program', 'run your first Lua game') + '. ' +
    'Check the ' + local('docs/start/installation', 'docs/tools/platforms', 'supported platforms') +
    ' or follow the contributor guide to ' +
    local('docs/start/installation', 'docs/contributing/build', 'build Malvado from source') + '.',
    ['getting-started', 'Install the SDK', 'Uninstall the SDK'],
    ['sdk', 'Install from an archive']);
  await publish('docs/start/first-program',
    'Open a window that says **Hello, Malvado!**, then put your own message in it. ' +
    local('docs/start/first-program', 'docs/start/installation', 'Install the SDK') +
    ' and use a desktop session with a display. After the game runs, see ' +
    '[what the generator created](../project-structure/) or try ' +
    '[the first gameplay examples](../../examples/#basics).',
    ['getting-started', 'Create and run a game', 'Create a project']);
  await publish('docs/start/project-structure',
    'Start with the files from [your first Lua game](../first-program/). ' +
    'Run project commands from the project directory. Use [editor setup](../../tools/editor/) ' +
    'for Lua completion or [build and export](../../tools/export/) to share the game.',
    ['sdk', 'Create and run a game', 'Create a Rust game']);
  await publish('docs/tools/cli', `Use \`malvado\` from a terminal after [installing the SDK](../../start/installation/).
Run project commands from the project directory unless you supply a path.

| Command | What it does |
| --- | --- |
| \`malvado --help\` | Lists commands supported by your installed CLI. |
| \`malvado --version\` | Prints the CLI version. |
| \`malvado doctor\` | Reports installed capabilities and tools. |
| \`malvado new [--lang lua\\|c\\|rust] PATH\` | Creates a project in a new directory. Lua is the default. |
| \`malvado check [PATH]\` | Checks Lua syntax, builds a C project, or runs Cargo's check for Rust. |
| \`malvado run [PATH]\` | Starts a Lua script or builds and starts a C or Rust game. |
| \`malvado run --headless [PATH]\` | Runs a Lua script without opening a window. |
| \`malvado build [PATH]\` | Compiles a C or Rust game. |
| \`malvado export [PATH]\` | Exports from the current project to a new destination. |
| \`malvado fpg-export [--transparent-zero] INPUT OUTPUT\` | Converts supported FPG sprites to PNG files and a JSON manifest. |

For Lua, pass a script or a project directory to \`run\`. Without a path,
the command runs \`main.lua\` in the current directory. A headless script
must provide its own way to finish.

Use \`doctor\` to check for graphics support, CMake, Rust tools, and
export components. A successful report can still list missing optional
components. The command reports availability without testing the graphics driver.

See the [detailed CLI reference](${repoLink('docs/development/cli.md')}) for
exit codes, file rules, and platform-specific options. Follow
[build and export games](../export/) for a distribution workflow.`);
  await publish('docs/tools/editor',
    'Use [Lua Language Server](' + siteLink('docs/tools/editor', 'docs/concepts/lua-language-server') +
    ') for completion and type checking in Lua projects.',
    ['getting-started', 'Lua in VS Code']);
  await publish('docs/tools/export',
    'Prepare a game folder or package that someone else can run. ' +
    'Start with a game that runs in your [installed SDK](../../start/installation/). ' +
    'Check [platform support](../platforms/) before choosing a target. ' +
    'See the [CLI reference](../cli/) for command options.',
    ['sdk', 'Export a game', 'Export for Android']);
  await publish('docs/tools/platforms',
    'Use the desktop SDK that matches your computer. Choose an export format ' +
    'for the computer or device that will run your game.\n\n' +
    '| Platform | Install and author games | Share games |\n' +
    '| --- | --- | --- |\n' +
    '| macOS arm64 | Released SDK and shell installer | Native folder or macOS app bundle |\n' +
    '| Linux x86_64 | Released SDK and shell installer | Native folder |\n' +
    '| Windows x64 MSVC (beta) | Released SDK archive | Native folder with a Windows executable |\n' +
    '| Android arm64 (beta) | Author on macOS arm64 or Linux x86_64 with a separate target SDK and Android tools | APK or Android App Bundle |\n\n' +
    'Follow [installation](../../start/installation/) for a desktop SDK or ' +
    '[build and export games](../export/) for a player package. Check the ' +
    '[platform matrix](' + repoLink('docs/development/platforms.md') +
    ') for graphics requirements, automated coverage, and Windows and Android hardware verification limits.',
    ['api', 'Choose a runtime and target'], ['sdk', 'Windows players']);
  if (!processModelTitle) {
    await publish('docs/learn/process-model',
      sources.api.intro.replace(/^# .+\n/, '').trim() + '\n\n' +
      'Start with [Lua processes](../../concepts/lua-processes/), then see how to ' +
      '[start and stop them](../../concepts/lua-processes/#start-and-stop), ' +
      '[wait between activations](../../concepts/lua-processes/#process-timing), ' +
      '[manage children](../../concepts/lua-processes/#children-and-ownership), and ' +
      '[send messages](../../concepts/lua-processes/#send-and-receive-messages).',
      ['api', 'Run and stop processes']);
  }
  if (!resourcesTitle) {
    await publish('docs/learn/resources',
      'Use the resource handles provided by your language. See the [images guide](' +
      siteLink('docs/learn/resources', 'docs/concepts/images') + ') and [asset paths](' +
      repoLink('docs/api/asset-sources.md') + '#game-asset-paths) for loading assets.',
      ['api', 'Keep resources alive']);
  }
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
