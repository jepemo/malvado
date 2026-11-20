import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import { repoLink, repository, version } from './scripts/site-context.mjs';

export default defineConfig({
  site: 'https://jepemo.github.io',
  base: version ? `/malvado/${version}` : '/malvado',
  integrations: [starlight({
    title: version ? `Malvado ${version}` : 'Malvado',
    description: 'Guides for the Malvado game engine.',
    customCss: ['./src/styles/docs.css'],
    social: [{ icon: 'github', label: 'GitHub', href: repository }],
    expressiveCode: { themes: ['github-light', 'github-dark'] },
    sidebar: [
      { label: 'Getting started', items: [
        { label: 'Introduction', slug: 'docs' },
        { label: 'Get started', slug: 'getting-started' },
        { label: 'SDK and projects', slug: 'sdk' },
        { label: 'Gameplay API guides', slug: 'api' },
        { label: 'Lua and C examples', slug: 'docs/samples' },
      ] },
      { label: 'API reference on GitHub', collapsed: true, items: [
        { label: 'Processes', link: repoLink('docs/api/lua-processes.md') },
        { label: 'Graphics and sprites', link: repoLink('docs/api/images.md') },
        { label: 'Collision', link: repoLink('docs/api/collision.md') },
        { label: 'C API', link: repoLink('docs/api/c-contract.md') },
      ] },
    ],
  })],
});
