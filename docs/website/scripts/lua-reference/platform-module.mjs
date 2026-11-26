import { referenceExample, colorParameter } from './common.mjs';

const startup = referenceExample('startup',
  'The program prints a virtual size of 320 × 240, resizes it to 640 × 360, then prints the two process cadences (30 and 20) and the elapsed wall time. It exits when both processes finish.');

const fades = referenceExample('fades',
  'The program prints the initial transparent overlay, observes the opaque blue stage, then prints the final transparent overlay and “Fade queue finished”. It exits on its own.');

const fadeDuration = 'Optional finite number in milliseconds. Omission or nil selects 250. Use 0 for an immediate transition when the request starts. Positive values round up to a whole simulation tick, with a maximum of 1,000,000 ticks.';

const fadeReturns = 'No values. Invalid input or a full queue raises a Lua error and leaves previous work unchanged.';

const fadeNotes = 'The game advances the queue on simulation ticks, including in headless mode. An idle queue starts immediately. A busy queue holds up to 16 pending transitions and starts them in call order.';

const fields = (prefix, definitions) =>
  definitions.map(([name, text]) => [`${prefix}.${name}`, text]);

const fpgIndex = 'Required Lua number representable as an integer. Enumeration starts at 1, in file order.';

export default {
  'platform/start': {
    description: 'Initialize the game and run its processes. Call this from the entry script when you want to start the game explicitly.',
    signatures: ['mv.start(init, options?)', 'start(init, options?)'],
    params: [
      ['init', 'Required function called once without arguments, before the first tick. Its return values are discarded.'],
      ['options', 'Optional table with the following fields and no metatable. Omit it or pass nil to use all defaults.'],
      ['options.width', 'Optional positive Lua integer in C int range, in logical pixels. Defaults to 800.'],
      ['options.height', 'Optional positive Lua integer in C int range, in logical pixels. Defaults to 600.'],
      ['options.title', 'Optional Lua string without zero bytes. Defaults to `"Malvado"`.'],
      ['options.fps', 'Optional Lua integer from 0 to 60. Defaults to 0, which activates a process on every eligible tick. Sets the game default for spawned processes.'],
    ],
    returns: 'No values. Invalid options, a second startup, window creation failure, an initializer error or a process error raises a Lua error.',
    ...startup,
    notes: 'The loop ends when the window closes, a process fails or no processes remain. With `--headless`, the processes must end for the loop to exit normally. For declarative startup, return a game table from the entry script. Use one startup form per run.',
    related: ['process', 'set_fps', 'window_size'],
  },
  'platform/set_fps': {
    description: 'Set the default activation cadence for future process spawns in this game.',
    signatures: ['mv.set_fps(fps)', 'set_fps(fps)'],
    params: [['fps', 'Required Lua integer from 0 to 60, in activations per 60 eligible logical ticks. Both 0 and 60 run on every eligible tick.']],
    returns: 'No values. Invalid arguments or calls outside a running game raise a Lua error.',
    ...startup,
    notes: 'Instance fields and process definition defaults take precedence. Existing processes keep their cadence. Call from the initializer or while the game runs.',
    related: ['start', 'frame'],
  },
  'platform/set_title': {
    description: 'Change the open window title. Use it to name the current level or mode.',
    signatures: ['mv.set_title(title)', 'set_title(title)'],
    params: [['title', 'Required Lua string without zero bytes.']],
    returns: 'No values. Invalid arguments, an unavailable window or a backend failure raise a Lua error. Headless runs have no window.',
    ...referenceExample('title', 'The window opens with “Level 2” in its title bar and closes after two seconds. You can also close the window.', true),
    related: ['window_size', 'start'],
  },
  'platform/set_window_size': {
    description: 'Request a new size for the existing game window. In headless mode, change the virtual size immediately.',
    signatures: ['mv.set_window_size(width, height)', 'set_window_size(width, height)'],
    params: [['width', 'Required positive Lua integer in C int range, in logical pixels.'],
      ['height', 'Required positive Lua integer in C int range, in logical pixels.']],
    returns: 'No values. Invalid arguments, calls outside a running game or a failed resize raise a Lua error.',
    ...startup,
    notes: 'Call from the initializer or a running process. Desktop window systems can apply the change later or constrain the size. Query the applied dimensions with `window_size()`. Android and other backends without resize support raise an error.',
    related: ['start', 'window_size'],
  },
  'platform/window_size': {
    description: 'Read the current logical window size. Use it to place elements relative to the window edges.',
    signatures: ['mv.window_size()', 'window_size()'],
    params: [],
    returns: 'Two Lua integers in logical pixels, width then height. Headless mode returns the virtual size. Extra arguments, calls before startup or an unavailable window raise a Lua error.',
    ...startup,
    related: ['start', 'set_window_size', 'set_title'],
  },
  'platform/time_seconds': {
    description: 'Read a monotonic clock. Subtract two readings to measure elapsed wall time.',
    signatures: ['mv.time_seconds()', 'time_seconds()'],
    params: [],
    returns: 'One Lua number in seconds. Extra arguments or an unavailable clock raise a Lua error.',
    ...startup,
    notes: 'Use differences between readings. Use `self.delta` for logical simulation time between process activations. The clock is available in headless mode.',
    related: ['frame'],
  },
  'platform/fade_to': {
    description: 'Queue a full-screen fade to an RGBA color. Use a colored overlay for a transition or a brief effect.',
    signatures: ['mv.fade_to(options)', 'fade_to(options)'],
    params: [['options', 'Required table with the following fields.'],
      ['options.color', colorParameter],
      ['options.duration_ms', fadeDuration]],
    returns: fadeReturns,
    ...fades,
    notes: fadeNotes,
    related: ['fade_off', 'fade_on', 'fade_active', 'fade_color'],
  },
  'platform/fade_off': {
    description: 'Queue a full-screen fade to opaque black. Use it to cover the scene before a transition.',
    signatures: ['mv.fade_off(duration_ms?)', 'fade_off(duration_ms?)'],
    params: [['duration_ms', fadeDuration]],
    returns: fadeReturns,
    ...fades,
    notes: fadeNotes + ' The separate legacy profile uses tick durations and a 16-tick default.',
    related: ['fade_on', 'fade_to', 'fade_active'],
  },
  'platform/fade_on': {
    description: 'Queue a fade to transparent black so that the scene becomes visible again.',
    signatures: ['mv.fade_on(duration_ms?)', 'fade_on(duration_ms?)'],
    params: [['duration_ms', fadeDuration]],
    returns: fadeReturns,
    ...fades,
    notes: fadeNotes + ' The separate legacy profile uses tick durations and a 16-tick default.',
    related: ['fade_off', 'fade_to', 'fade_active'],
  },
  'platform/fade_active': {
    description: 'Check whether the game still has a running or queued fade. Use this to wait for a transition to finish.',
    signatures: ['mv.fade_active()', 'fade_active()'],
    params: [],
    returns: 'One boolean. True means a fade is active or queued. False means the queue is idle. Extra arguments raise a Lua error.',
    ...fades,
    related: ['fade_to', 'fade_color'],
  },
  'platform/fade_color': {
    description: 'Read the current color of the full-screen fade overlay.',
    signatures: ['mv.fade_color()', 'fade_color()'],
    params: [],
    returns: 'One new table `{r, g, b, a}` with four positional integer channels in 0 through 255. The overlay starts at `{0, 0, 0, 0}`. Extra arguments raise a Lua error.',
    ...fades,
    related: ['fade_active', 'fade_to'],
  },
  'platform/rand': {
    description: 'Return a pseudorandom integer between two inclusive bounds. Use it to roll a die or pick a loot table entry.',
    signatures: ['mv.rand(min, max)', 'rand(min, max)'],
    params: [
      ['min', 'Required. The lowest value, as a Lua number that can be represented exactly as a Lua integer.'],
      ['max', 'Required. The highest value, with the same type as `min` and a value of at least `min`.'],
    ],
    returns: 'One Lua integer from `min` through `max`. A wrong argument count, an invalid bound, or `min > max` raises a Lua error.',
    exampleId: 'roll',
    exampleSetup: 'With the Malvado SDK installed, save `roll.lua` in an empty directory. Open a terminal there and run `malvado run --headless roll.lua`.',
    exampleResult: 'The program prints `Dice roll:` followed by an integer from 1 to 6, then exits. Try `mv.rand(6, 6)` to see what happens when both bounds are equal.',
    notes: 'Calls to `rand` and `math.random` advance the same Lua generator. An invalid call leaves the generator state unchanged. See [pseudorandom numbers](../../../../concepts/lua-processes/#pseudorandom-numbers) for repeatable sequences.',
    related: ['rand_seed'],
  },
  'platform/rand_seed': {
    description: 'Reseed the shared Lua pseudorandom generator. Set a known seed to reproduce a sequence while testing a game.',
    signatures: ['mv.rand_seed()', 'mv.rand_seed(seed)', 'rand_seed()', 'rand_seed(seed)'],
    params: [['seed', 'Optional Lua integer, including zero or a negative integer. Omit it to ask Lua to generate a fresh seed.']],
    returns: 'No values. Explicit nil, other types and extra arguments raise a Lua error before the generator state changes.',
    ...referenceExample('random-seed', 'The program checks that resetting to the same seed repeats the first roll, prints it, then exits.'),
    notes: 'The seed also affects `math.random` in the same Lua state. Lua seeds the generator at startup. Use the generator for ordinary gameplay.',
    related: ['rand'],
  },

  'module/import_globals': {
    description: 'Install the supported Malvado functions and constants in the Lua global environment. Use this at the top of a game script to call operations by their short names.',
    signatures: ['mv.import_globals()'],
    params: [],
    returns: 'No values. Extra arguments, an existing conflicting global or an installation failure raise a Lua error.',
    ...referenceExample('import-globals', 'The program imports twice, checks that the global function and constant match their module forms, prints “Malvado globals installed” and exits.'),
    notes: 'The import checks for conflicts before it writes. It rolls back additions if installation fails. Calling it again succeeds if the installed values are unchanged. Lua locals can shadow the imported names.',
    related: ['start', 'process'],
  },
  'module/legacy_fpg': {
    description: 'Load PNG graphics for the legacy process drawing profile. Use it when a migrated game still uses `self.fpg`.',
    signatures: ['mv.legacy_fpg(path)', 'mv.legacy_fpg(path, "directory")', 'mv.legacy_fpg(path, "image", rows, columns)'],
    params: [
      ['path', 'Required nonempty path without zero bytes. Relative paths use the game asset directory. Supply a directory for directory mode or a PNG file for image mode.'],
      ['kind', 'Optional string `"directory"` or `"image"`. Omission or nil selects directory mode. Directory mode reads up to 4096 PNG files in byte-sorted name order.'],
      ['rows', 'Required in image mode. Positive integer in C int range that divides the PNG height.'],
      ['columns', 'Required in image mode. Positive integer in C int range that divides the PNG width.'],
    ],
    returns: 'One legacy sheet userdata that retains the loaded images. Invalid arguments, a missing window, an unsupported directory mode, an empty directory or an image load failure raises a Lua error.',
    exampleId: 'reference-legacy-sheet',
    exampleSetup: 'With the Malvado SDK installed and a desktop display, extract the ZIP into an empty directory. Open a terminal there and run `malvado run legacy-sheet.lua`. Keep `tile.png` next to the script. The tile is original Malvado artwork under Apache 2.0.',
    exampleResult: 'A red tile appears with its upper-left corner at (20, 24). The program closes after two seconds. You can also close the window.',
    notes: 'Directory mode is available on macOS and Linux. The legacy `frame()` draws `self.fpg` at zero-based `self.fpgIndex`. Large indices wrap and a missing or negative index hides the sheet. Image sheets use the upper-left pivot. Directory images use a center pivot. The explicit legacy profile imports the same loader as `fpg`.',
    related: ['load_asset'],
  },
};
