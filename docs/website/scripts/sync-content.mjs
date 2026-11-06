import { readFile, writeFile, mkdir } from 'node:fs/promises';

// The repository guides and examples remain the source of truth.
const output = new URL('../src/content/docs/docs/', import.meta.url);
await mkdir(output, { recursive: true });
const guide = await readFile(new URL('../../site/getting-started.md', import.meta.url), 'utf8');
if (!guide.startsWith('# Get started\n')) throw new Error('The getting-started guide title changed.');
await writeFile(new URL('getting-started.md', output), `---\ntitle: Build and run\ndescription: Build Malvado from source and run Hello World.\n---\n\n${guide.slice('# Get started\n'.length).trimStart()}`);
const lua = await readFile(new URL('../../../examples/c/processes/main.lua', import.meta.url), 'utf8');
const c = await readFile(new URL('../../../examples/c/processes/main.c', import.meta.url), 'utf8');
await writeFile(new URL('samples.md', output), `---\ntitle: Lua and C examples\ndescription: See a process in each language.\n---\n\nThese runnable examples come from the repository. Both use the same core scheduler.\n\n## Lua\n\n\`\`\`lua title="examples/c/processes/main.lua"\n${lua.trim()}\n\`\`\`\n\n## C\n\n\`\`\`c title="examples/c/processes/main.c"\n${c.trim()}\n\`\`\`\n\nSee the [example instructions](https://github.com/jepemo/malvado/tree/master/examples/c/processes) and [C contract](https://github.com/jepemo/malvado/blob/master/docs/api/c-contract.md).\n`);
