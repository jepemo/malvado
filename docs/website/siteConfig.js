/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/* List of projects/orgs using your project for the users page */
const users = [];
/*
const users = [
  {
    caption: 'User1',
    image: '/test-site/img/docusaurus.svg',
    infoLink: 'https://github.com/jepemo/malvado',
    pinned: true,
  },
];
*/

const siteConfig = {
  title: 'malvado' /* title for your website */,
  tagline: 'A game programming library with "DIV Game Studio"-style processes for Lua/Love2D ',
  url: 'https://github.com/jepemo/malvado' /* your website url */,
  baseUrl: '/malvado/' /* base url for your project */,
  projectName: 'malvado',
  headerLinks: [
    {doc: 'installation', label: 'Docs'},
    { href: "https://github.com/jepemo/malvado", label: "GitHub" },
    //{doc: 'doc4', label: 'API'},
    //{page: 'help', label: 'Help'},
    //{blog: false, label: 'Blog'},
  ],
  users,
  /* path to images for header/footer */
  headerIcon: 'img/docusaurus.svg',
  footerIcon: 'img/docusaurus.svg',
  favicon: 'img/favicon.png',
  /* colors for website */
  colors: {
    primaryColor:  '#b21313',//'#2E8555',
    secondaryColor: '#ff7700',//'#205C3B',
  },
  /* custom fonts for website */
  /*fonts: {
    myFont: [
      "Times New Roman",
      "Serif"
    ],
    myOtherFont: [
      "-apple-system",
      "system-ui"
    ]
  },*/
  // This copyright info is used in /core/Footer.js and blog rss/atom feeds.
  copyright:
    'Copyright © ' +
    new Date().getFullYear() +
    '',
  // organizationName: 'deltice', // or set an env variable ORGANIZATION_NAME
  // projectName: 'test-site', // or set an env variable PROJECT_NAME
  highlight: {
    // Highlight.js theme to use for syntax highlighting in code blocks
    theme: 'default',
  },
  scripts: ['https://buttons.github.io/buttons.js'],
  // You may provide arbitrary config keys to be used as needed by your template.
  repoUrl: 'https://github.com/jepemo/malvado',
};

module.exports = siteConfig;
