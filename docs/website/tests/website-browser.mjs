import assert from 'node:assert/strict';
import { once } from 'node:events';
import { readFile } from 'node:fs/promises';
import { createServer } from 'node:http';
import { extname } from 'node:path';
import { after, before } from 'node:test';
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

export function serveWebsite(requiredPage) {
  let browser, origin;
  before(async () => {
    await readFile(new URL(requiredPage, dist));
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
      await run(page, origin);
      assert.deepEqual(errors, []);
    } finally {
      await context.close();
    }
  }

  return withPage;
}
