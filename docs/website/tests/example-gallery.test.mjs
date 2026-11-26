import assert from 'node:assert/strict';
import { once } from 'node:events';
import { readFile } from 'node:fs/promises';
import { createServer } from 'node:http';
import { extname } from 'node:path';
import { after, before, test } from 'node:test';
import { chromium } from 'playwright';

const dist = new URL('../dist/', import.meta.url);
const mime = {
  '.html': 'text/html', '.js': 'application/javascript', '.css': 'text/css',
  '.wasm': 'application/wasm', '.png': 'image/png',
};
const server = createServer(async (request, response) => {
  const name = new URL(request.url, 'http://localhost').pathname;
  const file = new URL('.' + name + (name.endsWith('/') ? 'index.html' : ''), dist);
  try {
    const bytes = await readFile(file);
    response.setHeader('content-type', mime[extname(file.pathname)] ?? 'application/octet-stream');
    response.end(bytes);
  } catch {
    response.writeHead(404).end();
  }
});

let browser, origin;
before(async () => {
  await readFile(new URL('docs/examples/index.html', dist));
  server.listen(0, '127.0.0.1');
  await once(server, 'listening');
  origin = `http://127.0.0.1:${server.address().port}`;
  browser = await chromium.launch();
});

after(async () => {
  await browser?.close();
  server.close();
});

async function withPage(options, run) {
  const context = await browser.newContext(options);
  const page = await context.newPage();
  page.setDefaultTimeout(8000);
  const errors = [];
  page.on('pageerror', (error) => errors.push(error.message));
  page.on('console', (message) => {
    if (message.type() === 'error') errors.push(message.text());
  });
  try {
    await run(page);
    assert.deepEqual(errors, []);
  } finally {
    await context.close();
  }
}

async function open(page, route = '') {
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
  await withPage({ permissions: ['clipboard-read', 'clipboard-write'] }, async (page) => {
    await open(page, 'catch/');
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
    await open(page, 'image/');
    await selected(page, 'Rust (experimental)');
    await open(page, 'shapes/');
    assert.equal(await page.getByRole('tab').count(), 0);
    await page.getByRole('heading', { name: 'Lua', exact: true }).waitFor();
    await open(page, 'catch/');
    await selected(page, 'Rust (experimental)');
    await page.reload();
    await selected(page, 'Rust (experimental)');
  });
});

test('blocked storage and clipboard leave the examples usable', async () => {
  await withPage({}, async (page) => {
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
    await open(page, 'catch/');
    await selected(page, 'Lua');
    await page.getByRole('tab', { name: 'C', exact: true }).click();
    await selected(page, 'C');
    await page.getByRole('button', { name: 'Copy code' }).click();
    await page.getByText('Copy unavailable. Select the code to copy it manually.').waitFor();
    await open(page, 'image/');
    await selected(page, 'Lua');
  });
});

test('the gallery and each language can be read without JavaScript', async () => {
  await withPage({ javaScriptEnabled: false }, async (page) => {
    await open(page);
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
  await withPage({}, async (page) => {
    await open(page);
    const filter = page.getByRole('searchbox', { name: 'Find an example' });
    await filter.fill('spawn fragments');
    assert.equal(await page.locator('[data-example-card]:visible').count(), 1);
    await page.getByRole('link', { name: 'Space Asteroids', exact: true }).click();
    assert(page.url().endsWith('/space-asteroids/'));
    await open(page);
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
  await withPage({ viewport: { width: 320, height: 700 }, isMobile: true, hasTouch: true }, async (page) => {
    await open(page);
    assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
    await open(page, 'catch/');
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
