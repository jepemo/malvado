import { readFile, writeFile, mkdir } from 'node:fs/promises';
import { repoLink, revision, version } from './site-context.mjs';

// The repository guides and examples remain the source of truth.
const output = new URL('../src/content/docs/docs/', import.meta.url);
await mkdir(output, { recursive: true });
for (const slug of ['getting-started', 'sdk', 'api']) {
  const guide = await readFile(new URL(`../../site/${slug}.md`, import.meta.url), 'utf8');
  const heading = /^# ([^\r\n]+)\r?\n/.exec(guide);
  if (!heading) throw new Error(`Missing title in ${slug}.md`);
  let body = guide.slice(heading[0].length).trimStart().replace(
    /\]\((getting-started|sdk|api)\.md(#[^)]+)?\)/g,
    (_, page, fragment = '') => `](../${page}/${fragment})`,
  );
  if (revision) {
    body = body.replace(
      /https:\/\/github\.com\/jepemo\/malvado\/(blob|tree|edit)\/master\//g,
      (_, kind) => repoLink('', kind),
    );
  }
  if (slug === 'getting-started' && version) {
    body = `This documentation is for Malvado ${version}. When building from Git, run \`git checkout --detach ${revision}\` after entering the cloned directory and before running \`mise install\`.\n\n${body}`;
  }
  await writeFile(new URL(`${slug}.md`, output),
    `---\ntitle: ${JSON.stringify(heading[1])}\nslug: ${slug}\n---\n\n${body}`);
}
const lua = await readFile(new URL('../../../examples/c/processes/main.lua', import.meta.url), 'utf8');
const c = await readFile(new URL('../../../examples/c/processes/main.c', import.meta.url), 'utf8');
await writeFile(new URL('samples.md', output), `---\ntitle: Lua and C examples\ndescription: See a process in each language.\n---\n\nThese runnable examples come from the repository. Both use the same core scheduler.\n\n## Lua\n\n\`\`\`lua title="examples/c/processes/main.lua"\n${lua.trim()}\n\`\`\`\n\n## C\n\n\`\`\`c title="examples/c/processes/main.c"\n${c.trim()}\n\`\`\`\n\nSee the [example instructions](${repoLink('examples/c/processes', 'tree')}) and [C contract](${repoLink('docs/api/c-contract.md')}).\n`);
