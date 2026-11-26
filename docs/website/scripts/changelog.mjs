import { execFileSync } from 'node:child_process';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { version } from './site-context.mjs';

const route = 'docs/changelog';
const publicIndex = 'https://malvado.dev/docs/changelog/';

export async function writeChangelog(writePage) {
  if (version) {
    const candidate = process.env.MALVADO_DOCS_RELEASE_NOTES;
    if (candidate) {
      const markdown = await readFile(candidate, 'utf8');
      const body = markdown.slice(markdown.indexOf('\n') + 1).trim();
      await writePage(route, `Malvado ${version} release notes`,
        'These notes describe the source revision used for this documentation. ' +
        `See the [published changelog](${publicIndex}) for release availability and downloads.\n\n${body}`);
    }
    return;
  }

  const tool = fileURLToPath(new URL('../../../tools/release_notes.py', import.meta.url));
  const { intro, releases } = JSON.parse(execFileSync('python3', [tool, 'website'], {
    encoding: 'utf8', maxBuffer: 16 * 1024 * 1024,
  }));
  const rows = releases.map((release) => {
    const path = `./${release.version}/`;
    const status = release.status === 'stable' ? 'Stable' : 'Prerelease';
    const upgrade = release.breaking ? ` · [Breaking changes](${path}#breaking-changes)` : '';
    return `- [${release.version}](${path}) · ${release.published} · ${status}${upgrade}`;
  });
  await writePage(route, 'Changelog', [
    intro, '', '## Releases', '', ...rows,
  ].join('\n'));

  for (const release of releases) {
    const status = release.status === 'stable' ? 'Stable release' : 'Prerelease';
    const documentation = {
      site: 'Versioned documentation',
      bundle: 'Versioned documentation in the release bundle',
      source: 'Documentation in the tagged source',
    }[release.docs];
    const links = [
      `[Release and downloads](${release.release})`,
      ...(documentation ? [`[${documentation}](${release.documentation})`] : []),
    ];
    await writePage(`${route}/${release.version}`, `Malvado ${release.version}`, [
      `${status} published on ${release.published} (UTC).`, '',
      links.join(' · '), '', release.body, '',
      '[All releases](../)',
    ].join('\n'));
  }
}
