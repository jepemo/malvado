import assert from 'node:assert/strict';
import { test } from 'node:test';
import { serveWebsite } from './website-browser.mjs';

const withPage = serveWebsite('docs/learn/process-model/index.html');
const lessons = [
  ['process-model', 'scheduler', 'processes/frame', 'Step through two cooperative workers'],
  ['collisions', 'collision', 'collision/rect_overlaps', 'Move two rectangles and test for overlap'],
  ['cameras-and-scrolling', 'camera', 'cameras/camera_set', 'Explore camera position and the viewport'],
  ['animation', 'animation', 'graphics/animation', 'Try the animation timeline'],
];

async function open(page, origin, [lesson, id]) {
  const response = await page.goto(`${origin}/docs/learn/${lesson}/#playground`);
  assert.equal(response.status(), 200);
  assert.equal(await page.locator('h2#playground').count(), 1);
  return page.locator(`[data-playground="${id}"]`);
}

const readout = (root) => root.locator('[data-summary]');
const reset = (root) => root.getByRole('button', { name: 'Reset', exact: true });

test('scheduler steps one worker or the rest of a tick, then resets', async () => {
  await withPage({}, async (page, origin) => {
    const root = await open(page, origin, lessons[0]);
    const next = root.getByRole('button', { name: 'Next turn' });
    const tick = root.getByRole('button', { name: 'Finish tick' });
    const output = root.locator('[data-console]');
    const first = root.locator('[data-worker="first"]');
    const second = root.locator('[data-worker="second"]');
    assert.match(await readout(root).innerText(), /Ready for tick 1/);

    await next.focus();
    await page.keyboard.press('Enter');
    assert.equal(await output.innerText(), 'first: 1');
    assert.equal(await first.innerText(), 'Yielded at frame()');
    assert.equal(await second.innerText(), 'Ready');

    await tick.click();
    assert.equal(await output.textContent(), 'first: 1\nsecond: 1');
    assert.match(await readout(root).innerText(), /Next turn starts tick 2/);
    await next.click();
    assert.equal(await first.innerText(), 'Returned');
    assert.equal(await second.innerText(), 'Yielded at frame()');
    await tick.click();
    assert.equal(await output.textContent(), 'first: 1\nsecond: 1\nfirst: 2\nsecond: 2');
    assert.equal(await second.innerText(), 'Returned');
    assert.ok(await next.isDisabled());
    assert.ok(await tick.isDisabled());

    await reset(root).click();
    assert.match(await output.innerText(), /Waiting/);
    assert.equal(await first.innerText(), 'Ready');
    assert.equal(await second.innerText(), 'Ready');
    await tick.click();
    assert.equal(await output.textContent(), 'first: 1\nsecond: 1');
  });
});

test('both collision rectangles move and edge contact needs positive overlap on both axes', async () => {
  await withPage({}, async (page, origin) => {
    const root = await open(page, origin, lessons[1]);
    const slider = (coordinate) => root.getByRole('slider', { name: `Rectangle ${coordinate}` });
    const overlap = root.locator('[data-overlap]');
    await slider('A X').fill('50');
    assert.match(await readout(root).innerText(), /^No overlap/);
    assert.equal(await overlap.getAttribute('width'), '0');
    await slider('A X').press('ArrowRight');
    assert.match(await readout(root).innerText(), /^Overlap of 1 × 30/);
    assert.equal(await overlap.getAttribute('width'), '1');
    await slider('A Y').fill('90');
    assert.match(await readout(root).innerText(), /^No overlap/);
    await slider('B Y').press('ArrowRight');
    assert.match(await readout(root).innerText(), /^Overlap of 1 × 1/);
    await slider('B X').press('ArrowRight');
    assert.match(await readout(root).innerText(), /^No overlap/);

    await reset(root).click();
    assert.match(await readout(root).innerText(), /A starts at \(20, 20\) and B at \(110, 40\)/);
    for (const [name, value] of [['A X', '20'], ['A Y', '20'], ['B X', '110'], ['B Y', '40']])
      assert.equal(await slider(name).inputValue(), value);
  });
});

test('camera moves the viewport in the world and projects into a fixed screen viewport', async () => {
  await withPage({}, async (page, origin) => {
    const root = await open(page, origin, lessons[2]);
    const x = root.getByRole('slider', { name: 'Camera X' });
    const y = root.getByRole('slider', { name: 'Camera Y' });
    const world = root.locator('[data-world-view]');
    const screen = root.locator('[data-screen-view]');
    const point = root.locator('[data-screen-point]');
    assert.match(await readout(root).innerText(), /screen \(60, 45\) inside/);
    await x.fill('120');
    assert.match(await readout(root).innerText(), /screen \(40, 45\) inside/);
    await x.press('ArrowRight');
    await y.press('ArrowRight');
    assert.match(await readout(root).innerText(), /screen \(39, 44\) outside/);
    assert.equal(await world.getAttribute('x'), '121');
    assert.equal(await world.getAttribute('y'), '51');
    assert.equal(await point.getAttribute('cx'), '-1');
    assert.equal(await point.getAttribute('cy'), '14');
    assert.equal(await screen.getAttribute('x'), '40');
    assert.equal(await screen.getAttribute('y'), '30');
    assert.equal(await screen.evaluate((svg) => getComputedStyle(svg).overflow), 'hidden');
    assert.equal(await screen.evaluate((svg) => svg.width.baseVal.value), 100);

    await reset(root).click();
    assert.match(await readout(root).innerText(), /Camera \(100, 50\).*screen \(60, 45\) inside/);
  });
});

test('animation starts paused, plays at the selected rate, loops, pauses and holds frame 3', async () => {
  await withPage({}, async (page, origin) => {
    await page.clock.install();
    const root = await open(page, origin, lessons[3]);
    const frame = () => root.locator('[aria-current="step"]').getAttribute('data-frame');
    const play = root.locator('[data-play]');
    const next = root.getByRole('button', { name: 'Next frame' });
    const rate = root.getByRole('combobox', { name: 'Frames per second' });
    const loop = root.getByRole('checkbox', { name: 'Loop' });

    await page.clock.runFor(1500);
    assert.equal(await frame(), '1');
    await rate.selectOption('2');
    assert.match(await readout(root).innerText(), /500 ms per frame, 1500 ms per loop/);
    await play.click();
    assert.equal(await play.innerText(), 'Pause');
    await page.clock.runFor(250);
    assert.equal(await frame(), '1');
    await page.clock.runFor(300);
    assert.equal(await frame(), '2');
    await page.clock.runFor(1000);
    assert.equal(await frame(), '1');
    await play.click();
    await page.clock.runFor(1600);
    assert.equal(await frame(), '1');

    await loop.uncheck();
    await next.press('Enter');
    await next.click();
    assert.match(await readout(root).innerText(), /Frame 3 of 3. Paused/);
    await next.click();
    assert.match(await readout(root).innerText(), /Frame 3 of 3. Finished/);
    assert.equal(await frame(), '3');
    assert.ok(await next.isDisabled());
    await reset(root).click();
    assert.equal(await rate.inputValue(), '8');
    assert.ok(await loop.isChecked());
    assert.match(await readout(root).innerText(), /Frame 1 of 3. Paused. 125 ms per frame/);
  });
});

test('reduced motion pauses continuous playback and preserves frame stepping', async () => {
  await withPage({ reducedMotion: 'reduce' }, async (page, origin) => {
    const root = await open(page, origin, lessons[3]);
    const play = root.locator('[data-play]');
    const next = root.getByRole('button', { name: 'Next frame' });
    assert.ok(await root.getByText(/Reduced motion is active/).isVisible());
    assert.ok(await play.isDisabled());
    await next.click();
    assert.match(await readout(root).innerText(), /Frame 2 of 3. Paused/);
    await page.emulateMedia({ reducedMotion: 'no-preference' });
    await play.click();
    await page.emulateMedia({ reducedMotion: 'reduce' });
    await root.getByRole('button', { name: 'Play', exact: true }).waitFor();
    assert.ok(await play.isDisabled());
    assert.match(await readout(root).innerText(), /Paused/);
  });
});

test('each Learn page has a static fallback without JavaScript', async () => {
  await withPage({ javaScriptEnabled: false }, async (page, origin) => {
    for (const lesson of lessons) {
      const root = await open(page, origin, lesson);
      assert.ok(await root.getByText('Conceptual simulation').isVisible());
      assert.ok(await readout(root).isVisible());
      assert.equal(await root.getByRole('button').count(), 0);
      assert.equal(await root.getByRole('slider').count(), 0);
      assert.equal(await root.getByRole('combobox').count(), 0);
      assert.equal(await root.getByRole('checkbox').count(), 0);
      if (lesson[1] === 'scheduler')
        assert.ok(await page.getByRole('table').first().getByText(/yields at/).isVisible());
      else
        assert.ok(await root.locator('svg').first().isVisible());
    }
  });
});

test('API links lead to the four playgrounds and their layout follows the site theme on mobile', async () => {
  await withPage({ viewport: { width: 320, height: 740 }, isMobile: true, hasTouch: true }, async (page, origin) => {
    for (const [lesson, id, api, link] of lessons) {
      const response = await page.goto(`${origin}/docs/reference/lua/${api}/`);
      assert.equal(response.status(), 200);
      await page.getByRole('link', { name: link, exact: true }).click();
      assert.equal(new URL(page.url()).pathname, `/docs/learn/${lesson}/`);
      assert.equal(new URL(page.url()).hash, '#playground');
      const root = page.locator(`[data-playground="${id}"]`);
      const backgrounds = [];
      for (const theme of ['light', 'dark']) {
        await page.locator('html').evaluate((html, selected) => { html.dataset.theme = selected; }, theme);
        const styles = await root.evaluate((node) => ({
          background: getComputedStyle(node).backgroundColor,
          fitsViewport: node.getBoundingClientRect().right <= window.innerWidth,
          noOverflow: node.scrollWidth <= node.clientWidth + 1,
        }));
        assert.ok(styles.fitsViewport && styles.noOverflow, `${id} overflow in ${theme}`);
        backgrounds.push(styles.background);
      }
      assert.notEqual(backgrounds[0], backgrounds[1]);
    }
  });
});
