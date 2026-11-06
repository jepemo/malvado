import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

export default defineConfig({
  site: 'https://jepemo.github.io',
  base: '/malvado',
  integrations: [starlight({
    title: 'Malvado',
    description: 'Guides for the Malvado game engine.',
    customCss: ['./src/styles/docs.css'],
    social: [{ icon: 'github', label: 'GitHub', href: 'https://github.com/jepemo/malvado' }],
    expressiveCode: { themes: ['github-light', 'github-dark'] },
    sidebar: [
      { label: 'Getting started', items: [
        { label: 'Introduction', slug: 'docs' },
        { label: 'Build and run', slug: 'docs/getting-started' },
        { label: 'Lua and C examples', slug: 'docs/samples' },
      ] },
      { label: 'Current guides on GitHub', collapsed: true, items: [
        { label: 'Processes', link: 'https://github.com/jepemo/malvado/blob/master/docs/api/lua-processes.md' },
        { label: 'Graphics and sprites', link: 'https://github.com/jepemo/malvado/blob/master/docs/api/images.md' },
        { label: 'Collision', link: 'https://github.com/jepemo/malvado/blob/master/docs/api/collision.md' },
        { label: 'C API', link: 'https://github.com/jepemo/malvado/blob/master/docs/api/c-contract.md' },
        { label: 'SDK and projects', link: 'https://github.com/jepemo/malvado/blob/master/docs/site/sdk.md' },
      ] },
    ],
  })],
});
