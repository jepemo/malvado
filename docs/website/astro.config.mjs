import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import { repoLink, repository, version } from './scripts/site-context.mjs';
import { learnLessons } from './scripts/learn-lessons.mjs';
import { dodgeTutorial } from './scripts/tutorials.mjs';

export default defineConfig({
  site: 'https://malvado.dev',
  base: version ? `/${version}` : '/',
  integrations: [starlight({
    title: version ? `Malvado ${version}` : 'Malvado',
    description: 'Guides for the Malvado game engine.',
    logo: { src: './src/assets/brand-mark.svg', alt: 'Malvado' },
    customCss: ['./src/styles/docs.css'],
    social: [{ icon: 'github', label: 'GitHub', href: repository }],
    expressiveCode: { themes: ['github-light', 'github-dark'] },
    sidebar: [
      { label: 'Getting started', items: [
        { label: 'Introduction', slug: 'docs' },
        { label: 'Installation', slug: 'docs/start/installation' },
        { label: 'Your first Malvado program', slug: 'docs/start/first-program' },
        { label: 'Project structure', slug: 'docs/start/project-structure' },
      ] },
      { label: 'Learn Malvado', collapsed: true, items: [
        ...learnLessons.map(({ label, slug }) => ({ label, slug })),
        ...(!learnLessons.some(({ name }) => name === 'process-model') ? [
          { label: 'Processes across languages', slug: 'docs/learn/process-model' },
        ] : []),
        { label: 'Lua process guide', slug: 'docs/concepts/lua-processes' },
        ...(!learnLessons.some(({ name }) => name === 'resources') ? [
          { label: 'Drawing and coordinates', slug: 'docs/concepts/drawing' },
          { label: 'Keyboard input', slug: 'docs/concepts/keyboard' },
          { label: 'Images and sprites', slug: 'docs/concepts/images' },
          { label: 'Animation', slug: 'docs/concepts/animations' },
          { label: 'Collisions', slug: 'docs/concepts/collision' },
          { label: 'Text and fonts', slug: 'docs/concepts/text' },
          { label: 'Sound and music', slug: 'docs/concepts/audio' },
          { label: '2D cameras', slug: 'docs/concepts/cameras' },
          { label: 'Resource ownership in games', slug: 'docs/learn/resources' },
        ] : []),
      ] },
      ...(dodgeTutorial ? [{ label: 'Tutorials', items: [
        { label: dodgeTutorial.title, slug: dodgeTutorial.route },
      ] }] : []),
      { label: 'Game programming patterns', collapsed: true, items: [
        { label: 'Organize a level', slug: 'docs/patterns/level-lifecycle' },
      ] },
      { label: 'Examples', collapsed: true, items: [
        { label: 'Browse examples', slug: 'docs/examples' },
        { label: 'First Rust games (experimental)', slug: 'docs/examples/rust-basics' },
        { label: 'Catch!', slug: 'docs/examples/catch' },
      ] },
      { label: 'Tools and deployment', collapsed: true, items: [
        { label: 'CLI reference', slug: 'docs/tools/cli' },
        { label: 'Editor setup', slug: 'docs/tools/editor' },
        { label: 'Build and export games', slug: 'docs/tools/export' },
        { label: 'Platform support', slug: 'docs/tools/platforms' },
      ] },
      { label: 'Lua API', collapsed: true, items: [
        { label: 'Introduction', slug: 'docs/reference/lua' },
        { label: 'Instance methods', slug: 'docs/concepts/lua-instance-methods' },
        { label: 'Processes', slug: 'docs/reference/lua/processes' },
        { label: 'Input', slug: 'docs/reference/lua/input' },
        { label: 'Graphics', slug: 'docs/reference/lua/graphics' },
        { label: 'Drawing', slug: 'docs/reference/lua/drawing' },
        { label: 'Images and sprites', slug: 'docs/reference/lua/images' },
        { label: 'Animations', slug: 'docs/reference/lua/animations' },
        { label: 'Text', slug: 'docs/reference/lua/text' },
        { label: 'Audio', slug: 'docs/reference/lua/audio' },
        { label: 'Cameras', slug: 'docs/reference/lua/cameras' },
        { label: 'Collision', slug: 'docs/reference/lua/collision' },
        { label: 'Worlds', slug: 'docs/reference/lua/world' },
        { label: 'FPG', slug: 'docs/reference/lua/fpg' },
        { label: 'Platform', slug: 'docs/reference/lua/platform' },
        { label: 'Module', slug: 'docs/reference/lua/module' },
        { label: 'Global exports', link: repoLink('docs/api/lua-globals.md') },
        { label: 'Namespaces', link: repoLink('docs/api/lua-namespaces.md') },
      ] },
      { label: 'Rust API (experimental)', collapsed: true, items: [
        { label: 'Support policy', link: repoLink('docs/api/rust-support.md') },
        { label: 'Processes', slug: 'docs/concepts/rust-processes' },
        { label: 'Window ownership', slug: 'docs/concepts/rust-window' },
        { label: 'Drawing and images', slug: 'docs/concepts/rust-drawing' },
        { label: 'Mouse, gamepad and text input', slug: 'docs/concepts/rust-input' },
        { label: 'Atlas metadata', slug: 'docs/concepts/rust-atlas' },
        { label: 'FPG import and conversion', slug: 'docs/concepts/rust-fpg' },
        { label: 'Cameras and animation', slug: 'docs/concepts/rust-camera-animation' },
        { label: 'Fonts and Unicode glyphs', slug: 'docs/concepts/rust-font' },
        { label: 'Collision and static worlds', slug: 'docs/concepts/rust-collision' },
        { label: 'Audio', slug: 'docs/concepts/rust-audio' },
      ] },
      { label: 'C API', collapsed: true, items: [
        { label: 'C process API', slug: 'docs/concepts/c-contract' },
        { label: 'C API reference', link: repoLink('docs/api/current-c-api.md') },
      ] },
      { label: 'Contributing', collapsed: true, items: [
        { label: 'Build Malvado from source', slug: 'docs/contributing/build' },
        { label: 'Architecture', link: repoLink('docs/development/architecture.md') },
        { label: 'Technical contracts', items: [
          { label: 'Process cleanup', slug: 'docs/concepts/process-cleanup' },
          { label: 'Shared runner timing', slug: 'docs/concepts/runner-timing' },
          { label: 'Resource lifetime', slug: 'docs/concepts/resource-lifetime' },
          { label: 'C and Lua boundaries', link: repoLink('docs/development/architecture.md') + '#process-adapters' },
          { label: 'Rust FFI and ownership', link: repoLink('bindings/rust/README.md') },
        ] },
        { label: 'Run the engine tests', slug: 'docs/contributing/testing' },
        { label: 'Documentation guidelines', link: repoLink('docs/development/api-documentation.md') },
        { label: 'Release engineering', slug: 'docs/contributing/release' },
        { label: 'Roadmap', link: repoLink('docs/roadmap/README.md') },
      ] },
    ],
  })],
});
