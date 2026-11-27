import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { test } from 'node:test';
import { serveWebsite } from './website-browser.mjs';

const withPage = serveWebsite('docs/examples/index.html');

async function open(page, origin, route = '') {
  const response = await page.goto(`${origin}/docs/examples/${route}`);
  assert.equal(response.status(), 200);
}

async function selected(page, name) {
  const tab = page.getByRole('tab', { name, exact: true, selected: true });
  await tab.waitFor();
  const panel = page.getByRole('tabpanel', { name, exact: true });
  await panel.waitFor();
  assert.equal(await page.getByRole('tabpanel').count(), 1);
  assert.equal(await tab.getAttribute('aria-controls'), await panel.getAttribute('id'));
  assert.equal(await panel.getAttribute('aria-labelledby'), await tab.getAttribute('id'));
  assert.equal(await tab.getAttribute('tabindex'), '0');
  return tab;
}

test('keyboard selection, exact copy and preference across examples', async () => {
  await withPage({ permissions: ['clipboard-read', 'clipboard-write'] }, async (page, origin) => {
    await open(page, origin, 'catch/');
    const lua = await selected(page, 'Lua');
    await lua.focus();
    await page.keyboard.press('ArrowLeft');
    const rust = await selected(page, 'Rust (experimental)');
    assert(await rust.evaluate((element) => document.activeElement === element));
    await page.keyboard.press('Home');
    await selected(page, 'Lua');
    await page.keyboard.press('ArrowRight');
    const c = await selected(page, 'C');
    assert(await c.evaluate((element) => document.activeElement === element));
    assert.equal(await page.getByRole('button', { name: 'Copy code', exact: true }).count(), 1);
    await page.getByRole('button', { name: 'Copy code', exact: true }).click();
    await page.getByText('Code copied.', { exact: true }).waitFor();
    const source = await readFile(new URL('../../../examples/catch/c/main.c', import.meta.url), 'utf8');
    assert.equal(await page.evaluate(() => navigator.clipboard.readText()), source);
    const download = await page.getByRole('link', { name: 'Download main.c' }).getAttribute('href');
    assert.equal(await (await page.request.get(new URL(download, page.url()).href)).text(), source);
    await page.getByRole('tab', { name: 'C', exact: true }).press('End');
    await selected(page, 'Rust (experimental)');
    await open(page, origin, 'image/');
    await selected(page, 'Rust (experimental)');
    await open(page, origin, 'shapes/');
    assert.equal(await page.getByRole('tab').count(), 0);
    await page.getByRole('heading', { name: 'Lua', exact: true }).waitFor();
    await open(page, origin, 'catch/');
    await selected(page, 'Rust (experimental)');
    await page.reload();
    await selected(page, 'Rust (experimental)');
  });
});

test('blocked storage and clipboard leave the examples usable', async () => {
  await withPage({}, async (page, origin) => {
    await page.addInitScript(() => {
      for (const name of ['getItem', 'setItem']) {
        const original = Storage.prototype[name];
        Storage.prototype[name] = function (key, ...args) {
          if (key === 'malvado-example-language') throw new DOMException('Storage blocked', 'SecurityError');
          return original.call(this, key, ...args);
        };
      }
      navigator.clipboard.writeText = () => Promise.reject(new DOMException('Clipboard blocked', 'NotAllowedError'));
    });
    await open(page, origin, 'catch/');
    await selected(page, 'Lua');
    await page.getByRole('tab', { name: 'C', exact: true }).click();
    await selected(page, 'C');
    await page.getByRole('button', { name: 'Copy code' }).click();
    await page.getByText('Copy unavailable. Select the code to copy it manually.').waitFor();
    await open(page, origin, 'image/');
    await selected(page, 'Lua');
  });
});

test('the gallery and each language can be read without JavaScript', async () => {
  await withPage({ javaScriptEnabled: false }, async (page, origin) => {
    await open(page, origin);
    await page.getByRole('link', { name: 'Catch!', exact: true }).last().click();
    assert.equal(await page.getByRole('tablist').count(), 0);
    assert.equal(await page.getByRole('button', { name: 'Copy code' }).count(), 0);
    for (const language of ['Lua', 'C', 'Rust (experimental)']) {
      await page.getByRole('heading', { name: language, exact: true }).waitFor();
    }
    assert.equal(await page.locator('.language-examples .expressive-code pre:visible').count(), 3);
    for (const fragment of ['lua', 'c', 'rust-experimental', 'catch']) {
      assert(await page.locator('#' + fragment).count());
    }
  });
});

test('gallery filter, legacy links and site search', async () => {
  await withPage({}, async (page, origin) => {
    await open(page, origin);
    const filter = page.getByRole('searchbox', { name: 'Find an example' });
    await filter.fill('spawn fragments');
    assert.equal(await page.locator('[data-example-card]:visible').count(), 1);
    await page.getByRole('link', { name: 'Space Asteroids', exact: true }).click();
    assert(page.url().endsWith('/space-asteroids/'));
    await open(page, origin);
    await filter.fill('zzunmatched');
    await page.getByText('0 examples', { exact: true }).waitFor();
    await filter.fill('');
    assert(await page.locator('[data-example-card]:visible').count() >= 14);
    await page.locator('site-search button').first().click();
    await page.locator('site-search dialog input').fill('spawn fragments');
    await page.locator('site-search dialog a[href*="space-asteroids"]').first().waitFor();
    await page.goto(`${origin}/docs/samples/#rust-experimental`);
    await page.locator('main a[href$="/catch/#rust-experimental"]').click();
    await selected(page, 'Rust (experimental)');
  });
});

test('mobile layout and highlighting follow the site theme', async () => {
  await withPage({ viewport: { width: 320, height: 700 }, isMobile: true, hasTouch: true }, async (page, origin) => {
    await open(page, origin);
    assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
    await open(page, origin, 'catch/');
    const preview = page.getByRole('img', { name: /falling orange ball/ });
    await preview.scrollIntoViewIfNeeded();
    await page.waitForFunction(() => document.querySelector('img[src$="/gallery-previews/catch.png"]')?.naturalWidth > 0);
    await page.getByRole('tab', { name: 'Rust (experimental)' }).click();
    await selected(page, 'Rust (experimental)');
    const code = page.getByRole('tabpanel').locator('.expressive-code pre');
    const backgrounds = [];
    for (const theme of ['light', 'dark']) {
      await page.locator('html').evaluate((html, theme) => { html.dataset.theme = theme; }, theme);
      assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
      backgrounds.push(await code.evaluate((pre) => getComputedStyle(pre).backgroundColor));
    }
    assert.notEqual(backgrounds[0], backgrounds[1]);
  });
});

test('reference search reaches a runnable example on mobile', async () => {
  await withPage({ viewport: { width: 320, height: 740 }, isMobile: true }, async (page, origin) => {
    await page.goto(`${origin}/docs/reference/lua/`);
    await page.getByRole('button', { name: 'Search', exact: true }).click();
    const dialog = page.getByRole('dialog');
    await dialog.locator('input').fill('camera_move');
    await dialog.locator('a[href*="/cameras/camera_move/"]').first().click();
    const main = page.getByRole('main');
    await main.getByRole('heading', { name: 'camera_move', exact: true }).waitFor();
    await main.getByRole('heading', { name: 'Run the example', exact: true }).waitFor();
    assert.match(await main.getByRole('link', { name: 'Download cameras.lua' }).getAttribute('href'),
      /reference-cameras\/cameras\.lua$/);
    assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
  });
});
