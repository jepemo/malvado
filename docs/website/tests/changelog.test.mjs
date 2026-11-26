import assert from 'node:assert/strict';
import { test } from 'node:test';
import { serveWebsite } from './website-browser.mjs';

const withPage = serveWebsite('docs/changelog/index.html');

test('mobile navigation, prerelease labeling and migration links', async () => {
  await withPage({ viewport: { width: 320, height: 780 }, colorScheme: 'dark' }, async (page, origin) => {
    await page.goto(origin);
    await page.getByRole('navigation', { name: 'Main navigation' })
      .getByRole('link', { name: 'Changelog' }).click();
    const main = page.getByRole('main');
    await main.getByRole('heading', { name: 'Changelog', exact: true }).waitFor();
    assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
    assert.equal(await page.locator('html').getAttribute('data-theme'), 'dark');
    await main.getByRole('link', { name: '2.0.0-beta.1', exact: true }).click();
    await main.getByText('Prerelease published on 2026-11-19 (UTC).', { exact: true }).waitFor();
    await main.getByRole('link', { name: 'All releases' }).click();
    await main.getByRole('link', { name: 'Breaking changes' }).first().click();
    assert.equal(new URL(page.url()).hash, '#breaking-changes');
    await main.getByRole('heading', { name: 'Breaking changes', exact: true }).waitFor();
    assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
    const unavailable = await page.request.get(`${origin}/docs/changelog/unreleased/`);
    assert.equal(unavailable.status(), 404);
  });
});

test('Pagefind finds release pages', async () => {
  await withPage({ colorScheme: 'light' }, async (page, origin) => {
    await page.goto(`${origin}/docs/changelog/`);
    assert.equal(await page.locator('html').getAttribute('data-theme'), 'light');
    await page.getByRole('button', { name: 'Search', exact: true }).click();
    await page.locator('dialog input').fill('half-second fade');
    await page.locator('dialog').getByRole('link', { name: /Malvado 2\.1\.0/ }).first().waitFor();
  });
});
