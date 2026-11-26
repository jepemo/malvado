import { referenceExample, collisionExample } from './common.mjs';

const fields = (prefix, definitions) =>
  definitions.map(([name, text]) => [`${prefix}.${name}`, text]);

const inputExample = referenceExample('input',
  'Press and release Space, click the left mouse button, and type into the window. The display shows held states, pointer position and confirmed text. On a controller in slot 0, try the A button and the left stick. The terminal reports transitions. The program exits after 15 seconds or when you close the window.', true);

const inputSignatures = (call) => [`mv.${call}`, `mv.input.${call}`, call];

const keyName = 'Required case-sensitive key string. Use a `KEY_*` constant, a lowercase letter a–z, a digit 0–9, or escape, space, left, right, up, down, enter, tab or backspace.';

const mouseName = 'Required case-sensitive button string. Use `MOUSE_LEFT`, `MOUSE_MIDDLE`, `MOUSE_RIGHT`, or the strings left, middle or right.';

const padSlot = 'Required whole Lua number from 0 to 3.';

const padButton = 'Required case-sensitive button string. Use a button constant or one of a, b, x, y, up, down, left, right, lb, rb, back or start.';

const inputBoolean = 'One boolean. In headless mode it is false. An invalid argument count or invalid input raises a Lua error.';

const padBoolean = inputBoolean + ' A disconnected slot returns false.';

const edgeNotes = 'Every read sees the same edge snapshot for the current logical tick. Reading does not consume it. A slow, sleeping or frozen process can miss the tick.';

const padEdgeNotes = edgeNotes + ' A new connection establishes the held baseline. Disconnecting clears the slot without reporting a release.';

const androidExport = '[Android export setup](../../../../tools/export/#export-for-android)';

const processExample = referenceExample('process-control',
  'The program finds two workers in the creation tree and one in the ownership tree. It wakes the guard, then stops the level and both workers and exits.');

const mailboxExample = {
  ...referenceExample('mailbox', 'The scoreboard prints scores of 10 and 15, then a final score of 15 and exits.'),
  exampleId: 'mailbox',
};

const frameExample = {
  ...referenceExample('main', 'The program prints first: 1, second: 1, first: 2 and second: 2 on separate lines, then exits.'),
  exampleId: 'two-workers',
};

const processSignatures = (...calls) => calls.flatMap((call) => [
  `mv.${call}`, `mv.process.${call}`, call, `process.${call}`,
]);

const processHandle = 'Required process handle from the current runtime.';

const liveProcess = 'Required live process handle from the current runtime.';

const constructor = 'Required constructor returned by `mv.process` or `mv.process.define`.';

const definitionOptions = [
  ['body', 'Required Lua function. It receives the process handle as its only argument. The process ends when the body returns.'],
  ['options', 'Optional table. Omission or nil selects the defaults.'],
  ['options.defaults', 'Optional table of initial fields. Instance fields override it. Without this option the definition adds no default fields.'],
  ['options.children_attached', 'Optional boolean. Defaults to true. It provides the initial `self.children_attached`, which governs later spawns from this process.'],
];

const definitionReturns = 'One constructor function. Call it to spawn an instance and receive its canonical process handle. An invalid body, unknown options, invalid option types or exhausted type IDs raise a Lua error.';

const definitionNotes = 'Definitions can share managed resources. Each spawn deep-copies plain default tables, including shared references and cycles, up to 256 levels. Invalid default values and read-only process fields fail when an instance is spawned. See [process fields](../../../../concepts/lua-processes/#process-data) for the built-in fields and their defaults.';

const hierarchyOptions = [
  ['options', 'Optional table. Omission or nil selects the defaults.'],
  ['options.relation', 'Optional string. Use "ownership" (default) or "creation".'],
  ['options.type', 'Optional process constructor. Omit it to include all process types.'],
];

const hierarchyNotes = 'The root is excluded. Sleeping and frozen processes are included. Ownership queries need a live root. Creation queries can use a retained creator while its lineage is still known.';

const treeOption = [
  ['options', 'Optional table. Omission or nil selects the default relation.'],
  ['options.relation', 'Optional string. Use "creation" (default) or "ownership".'],
];

const signalValue = 'Required case-sensitive signal string. Use `SIGNAL_SLEEP` ("sleep"), `SIGNAL_FREEZE` ("freeze"), `SIGNAL_WAKEUP` ("wakeup") or `SIGNAL_KILL` ("kill").';

const treeReturn = 'No values. Invalid arguments, an unavailable root for the selected relation or a failure to apply the signal raise a Lua error. If this action kills the running process, execution stops at the call.';

const processCollisionNotes = 'The query excludes the source and includes sleeping and frozen processes. A dead target does not match. An invalid explicitly selected live target raises an error. A type query skips instances with invalid colliders or transforms.';

export default {
  'processes/process': {
    description: 'Define a process type. Use the returned constructor to create individual players, enemies or other actors with the same behavior.',
    signatures: ['mv.process(body, options?)', 'process(body, options?)', 'constructor(fields?, spawn_options?)'],
    params: [...definitionOptions,
      ['fields', 'Optional table of initial instance fields. Omission or nil uses the definition and engine defaults.'],
      ['spawn_options', 'Optional table for the constructor call. Omission or nil uses automatic attachment.'],
      ['spawn_options.attached', 'Optional boolean. True attaches to an active owner. False spawns independently. The default follows the creator’s children_attached field, or creates a root outside a process.']],
    returns: definitionReturns,
    ...processExample,
    notes: definitionNotes,
    related: ['define', 'spawn', 'frame', 'start'],
  },
  'processes/define': {
    description: 'Define a process type through the process namespace. It uses the same rules as calling the namespace itself.',
    signatures: ['mv.process.define(body, options?)', 'process.define(body, options?)', 'constructor(fields?, spawn_options?)'],
    params: [...definitionOptions,
      ['fields', 'Optional table of initial instance fields. Omission or nil uses the definition and engine defaults.'],
      ['spawn_options', 'Optional table for the constructor call. Omission or nil uses automatic attachment.'],
      ['spawn_options.attached', 'Optional boolean. True attaches to an active owner. False spawns independently. The default follows the creator’s children_attached field, or creates a root outside a process.']],
    returns: definitionReturns,
    ...processExample,
    notes: definitionNotes,
    related: ['process', 'spawn'],
  },
  'processes/spawn': {
    description: 'Start one process from a definition. Use this form to choose explicitly whether the process follows its creator’s lifetime.',
    signatures: processSignatures('spawn(constructor, fields?, options?)'),
    params: [['constructor', constructor],
      ['fields', 'Optional table of starting fields that override the definition defaults. Omission or nil leaves the defaults in effect.'],
      ['options', 'Optional table. Omit it or pass nil for automatic attachment.'],
      ['options.attached', 'Optional boolean. True attaches to the active owner. False creates an independent process. Omit it to use the creator’s children_attached field, or create a root when there is no creator.']],
    returns: 'One canonical process handle. Invalid fields or options, an attachment without an owner, shutdown or a failed allocation raise a Lua error. A failed call leaves no process.',
    ...processExample,
    notes: 'Instance fields are copied into the new process, with table values shared by reference. A terminating process can spawn from a close callback only with explicit `attached = false` during deferred cleanup. The creator ID is preserved even for detached children.',
    related: ['process', 'define', 'children'],
  },
  'processes/find': {
    description: 'Find live instances of a process type. Use it to inspect a group, such as all enemies in a level.',
    signatures: processSignatures('find(constructor)'),
    params: [['constructor', constructor]],
    returns: 'A new array of canonical handles in creation order, or an empty array if none match. Invalid arguments or an inspection failure raise a Lua error.',
    ...processExample,
    notes: 'The results include sleeping and frozen processes and native processes that share the type ID.',
    related: ['children', 'descendants', 'snapshot'],
  },
  'processes/children': {
    description: 'List the live children one level below a process. Use it to inspect direct members of an ownership or creation tree.',
    signatures: [...processSignatures('children(parent, options?)'), 'handle:children(options?)'],
    params: [['parent', processHandle], ...hierarchyOptions],
    returns: 'A new array of canonical handles in creation order, or an empty array if none match. Invalid arguments or an unavailable root for the chosen relation raise a Lua error.',
    ...processExample,
    notes: hierarchyNotes,
    related: ['descendants', 'find'],
  },
  'processes/descendants': {
    description: 'List the live descendants at any depth below a process. Use this to inspect all actors created by a level.',
    signatures: [...processSignatures('descendants(parent, options?)'), 'handle:descendants(options?)'],
    params: [['parent', processHandle], ...hierarchyOptions],
    returns: 'A new array of canonical handles in creation order, or an empty array if none match. Invalid arguments or an unavailable root for the chosen relation raise a Lua error.',
    ...processExample,
    notes: hierarchyNotes + ' Creation queries can follow relationships through dead creators.',
    related: ['children', 'find', 'signal_tree'],
  },
  'processes/snapshot': {
    description: 'Copy the current scheduler state. Use this for an inspection display or to diagnose a level teardown.',
    signatures: processSignatures('snapshot()'),
    params: [],
    returns: 'One table with a stable hexadecimal runtime string (48 characters), integer tick, array of live processes and array of lineage records. Extra arguments or a capture failure raise a Lua error. See [all snapshot fields](../../../../concepts/lua-processes/#inspect-the-scheduler).',
    ...processExample,
    notes: 'Records use numeric inspection IDs and include native and Lua processes. The copy contains no game fields or message payloads. Editing it leaves the scheduler unchanged.',
    related: ['find', 'status'],
  },
  'processes/frame': {
    playground: ['process-model', 'Step through two cooperative workers'],
    description: 'Yield the running process until its next activation. Call it inside a loop to let other processes and the engine run.',
    signatures: processSignatures('frame()'),
    params: [],
    returns: 'No values on resumption. Arguments, calls outside the scheduled process coroutine or calls in a non-yieldable context raise a Lua error.',
    ...frameExample,
    notes: 'The process cadence controls when it resumes. Read `self.delta` for logical seconds since its previous activation. It is 0 on the first activation.',
    related: ['process', 'start', 'time_seconds'],
  },
  'processes/signal': {
    description: 'Signal one process. Use it to pause, resume or end that process.',
    signatures: [...processSignatures('signal(handle, signal)'), 'handle:signal(signal)'],
    params: [['handle', processHandle], ['signal', signalValue]],
    returns: 'A kill signal returns true for a live target or false for a dead target. Other successful signals return no values. If the call kills the running process, execution stops at the call. Invalid inputs or a non-kill signal to a dead process raise a Lua error.',
    ...processExample,
    notes: 'Sleep hides the last process drawing. Freeze keeps it visible. Both pause execution. Wakeup allows activation again. Killing an owner also ends its attached processes.',
    related: ['signal_tree', 'kill', 'status'],
  },
  'processes/signal_tree': {
    description: 'Signal a process and its live lineage. Use this to pause or resume a group together.',
    signatures: [...processSignatures('signal_tree(handle, signal, options?)'), 'handle:signal_tree(signal, options?)'],
    params: [['handle', processHandle], ['signal', signalValue], ...treeOption],
    returns: treeReturn,
    ...processExample,
    notes: 'The creation relation includes detached descendants and can pass through dead creators. The ownership relation uses the current tree and requires a live root. The signal applies to existing processes.',
    related: ['signal', 'kill_tree', 'descendants'],
  },
  'processes/set_priority': {
    description: 'Change a live process’s scheduling priority. Use it when one process needs to read freshly updated state from another.',
    signatures: [...processSignatures('set_priority(priority)', 'set_priority(handle, priority)'), 'handle:set_priority(priority)'],
    params: [['handle', liveProcess + ' The one-argument form selects the scheduled process.'],
      ['priority', 'Required Lua number that fits a Lua integer exactly. The initial priority is 0.']],
    returns: 'No values. Invalid priority, invalid or dead handles, and an implicit call without the scheduled process context raise a Lua error.',
    ...processExample,
    notes: 'Higher values run first. Equal values use creation order. Changes made during a tick affect ordering starting on the next tick.',
    related: ['get_priority', 'frame'],
  },
  'processes/get_priority': {
    description: 'Read the current scheduler priority of a live process.',
    signatures: processSignatures('get_priority(handle)'),
    params: [['handle', liveProcess]],
    returns: 'One Lua integer, equal to the live process’s `priority` property. Invalid arguments or dead handles raise a Lua error.',
    ...processExample,
    related: ['set_priority'],
  },
  'processes/kill': {
    description: 'End the scheduled process or an explicitly selected process with reason "killed".',
    signatures: [...processSignatures('kill()', 'kill(handle)'), 'handle:kill()'],
    params: [['handle', processHandle + ' Omit it to end the scheduled process.']],
    returns: 'The explicit form returns true for a live target or false for a dead target. If the call kills the running process, it stops immediately, including inside pcall or xpcall. Invalid arguments or a self-kill in a non-yieldable context raise a Lua error.',
    ...processExample,
    notes: 'Killing an owner ends attached descendants. The scheduler runs close callbacks and releases process resources and pending messages during cleanup.',
    related: ['signal', 'kill_tree', 'alive'],
  },
  'processes/alive': {
    description: 'Check whether a process handle still refers to a live process. Use it before continuing work with a saved handle.',
    signatures: [...processSignatures('alive(handle)'), 'handle:alive()'],
    params: [['handle', processHandle]],
    returns: 'One boolean. Sleeping and frozen processes are alive. An ended process gives false. Invalid arguments raise a Lua error.',
    ...processExample,
    related: ['kill', 'status'],
  },
  'processes/kill_tree': {
    description: 'End a process and its live lineage. Use this to tear down a level and its detached children.',
    signatures: [...processSignatures('kill_tree(handle, options?)'), 'handle:kill_tree(options?)'],
    params: [['handle', processHandle], ...treeOption],
    returns: treeReturn,
    ...processExample,
    notes: 'This is the kill signal applied to a tree. A creation tree includes detached descendants. An ownership tree uses the current owner relationships.',
    related: ['kill', 'signal_tree'],
  },
  'processes/status': {
    description: 'Read a process’s current status during the run. Use it to distinguish sleeping, frozen and ended processes.',
    signatures: processSignatures('status(handle)'),
    params: [['handle', processHandle]],
    returns: 'One string equal to `handle.status`. It is `STATUS_RUNNING` ("running"), `STATUS_SLEEPING` ("sleeping"), `STATUS_FROZEN` ("frozen") or `STATUS_DEAD` ("dead"). Invalid arguments raise a Lua error.',
    ...processExample,
    notes: 'A retained handle still exposes its status after termination. During runtime shutdown, read the handle property.',
    related: ['alive', 'snapshot'],
  },
  'processes/send': {
    description: 'Send a value to a live process. Use the mailbox to coordinate game actors.',
    signatures: [...processSignatures('send(handle, value)'), 'handle:send(value)'],
    params: [['handle', liveProcess], ['value', 'Required non-nil Lua value. Native recipients accept booleans, numbers and binary strings.']],
    returns: 'No values. An invalid recipient or value, a full mailbox or a failed send raises a Lua error.',
    ...mailboxExample,
    notes: 'Each recipient has a FIFO mailbox of up to 1,024 entries. Binary strings are copied. Other Lua objects retain their identity when sent to Lua processes. Pending messages are released when the process ends.',
    related: ['recv'],
  },
  'processes/recv': {
    description: 'Remove the oldest message from a mailbox. Use it to check for new work on a process activation.',
    signatures: [...processSignatures('recv()', 'recv(handle)'), 'handle:recv()'],
    params: [['handle', liveProcess + ' Omit it to read from the scheduled process.']],
    returns: 'One message value, or nil if the queue is empty. A message can be false. Explicit nil, invalid or dead handles, or a missing scheduled context for an implicit call raise a Lua error.',
    ...mailboxExample,
    notes: 'This call returns immediately, including when the mailbox is empty. The explicit form can read another process’s mailbox, including from game setup.',
    related: ['send'],
  },
  'processes/collision': {
    description: 'Find the first process that overlaps an explicit source process. Use it when checking contact with a pickup or enemy.',
    signatures: ['mv.process.collision(source, target)', 'process.collision(source, target)', 'handle:collision(target)'],
    params: [['source', liveProcess], ['target', 'Required process constructor or process handle.']],
    returns: 'The first canonical matching handle in creation order, or nil if there is no match. Invalid handles, invalid source geometry or a dead source raise a Lua error.',
    ...collisionExample,
    notes: processCollisionNotes,
    related: ['collisions'],
  },
  'processes/collisions': {
    description: 'Find all processes that overlap an explicit source process. Use it for effects that act on every target in an area.',
    signatures: ['mv.process.collisions(source, target)', 'process.collisions(source, target)', 'handle:collisions(target)'],
    params: [['source', liveProcess], ['target', 'Required process constructor or process handle.']],
    returns: 'A new array of canonical matching handles in creation order, or an empty array if none match. Invalid handles, invalid source geometry or a dead source raise a Lua error.',
    ...collisionExample,
    notes: processCollisionNotes,
    related: ['collision'],
  },

  'input/key': {
    description: 'Check whether a key is held. Use the held state for continuous movement.',
    signatures: inputSignatures('key(name)'),
    params: [['name', keyName]],
    returns: inputBoolean,
    ...inputExample,
    related: ['key_pressed', 'key_released'],
  },
  'input/key_pressed': {
    description: 'Check for a key press in the current logical tick. Use it for an action such as jumping once per press.',
    signatures: inputSignatures('key_pressed(name)'),
    params: [['name', keyName]],
    returns: inputBoolean,
    ...inputExample,
    notes: edgeNotes,
    related: ['key', 'key_released'],
  },
  'input/key_released': {
    description: 'Check for a key release in the current logical tick. Use it to finish an action such as charging a jump.',
    signatures: inputSignatures('key_released(name)'),
    params: [['name', keyName]],
    returns: inputBoolean,
    ...inputExample,
    notes: edgeNotes,
    related: ['key', 'key_pressed'],
  },
  'input/mouse_position': {
    description: 'Read the pointer position. Use it to place a cursor or test a screen-space button.',
    signatures: inputSignatures('mouse_position()'),
    params: [],
    returns: 'Two Lua integers in logical window pixels, x then y. The origin is the upper-left corner. A windowless or headless run returns 0, 0. Extra arguments raise a Lua error.',
    ...inputExample,
    notes: 'Android maps the primary touch contact to the pointer and left button.',
    related: ['mouse_button', 'mouse_pressed'],
  },
  'input/mouse_button': {
    description: 'Check whether a mouse button is held. Use it while dragging an item.',
    signatures: inputSignatures('mouse_button(button)'),
    params: [['button', mouseName]],
    returns: inputBoolean,
    ...inputExample,
    notes: 'Android maps the primary touch contact to the left button.',
    related: ['mouse_position', 'mouse_pressed', 'mouse_released'],
  },
  'input/mouse_pressed': {
    description: 'Check for a mouse press in the current logical tick. Use it to start a drag or select an item.',
    signatures: inputSignatures('mouse_pressed(button)'),
    params: [['button', mouseName]],
    returns: inputBoolean,
    ...inputExample,
    notes: edgeNotes + ' Android maps a primary touch press to the left button.',
    related: ['mouse_button', 'mouse_released'],
  },
  'input/mouse_released': {
    description: 'Check for a mouse release in the current logical tick. Use it to finish a drag.',
    signatures: inputSignatures('mouse_released(button)'),
    params: [['button', mouseName]],
    returns: inputBoolean,
    ...inputExample,
    notes: edgeNotes + ' Android maps the end of a primary touch contact to a left button release.',
    related: ['mouse_button', 'mouse_pressed'],
  },
  'input/pad_connected': {
    description: 'Check whether a slot has a controller. Use it to show the available controls for a player.',
    signatures: inputSignatures('pad_connected(slot)'),
    params: [['slot', padSlot]],
    returns: 'One boolean. It is true for a connected slot and false for a disconnected slot or in headless mode. Invalid arguments raise a Lua error.',
    ...inputExample,
    related: ['pad_button', 'pad_axis'],
  },
  'input/pad_button': {
    description: 'Check whether a controller button is held. Use it for continuous actions.',
    signatures: inputSignatures('pad_button(slot, button)'),
    params: [['slot', padSlot], ['button', padButton]],
    returns: padBoolean,
    ...inputExample,
    related: ['pad_connected', 'pad_pressed', 'pad_released', 'pad_axis'],
  },
  'input/pad_pressed': {
    description: 'Check for a controller button press in the current logical tick. Use it to trigger an action once.',
    signatures: inputSignatures('pad_pressed(slot, button)'),
    params: [['slot', padSlot], ['button', padButton]],
    returns: padBoolean,
    ...inputExample,
    notes: padEdgeNotes,
    related: ['pad_button', 'pad_released'],
  },
  'input/pad_released': {
    description: 'Check for a controller button release in the current logical tick.',
    signatures: inputSignatures('pad_released(slot, button)'),
    params: [['slot', padSlot], ['button', padButton]],
    returns: padBoolean,
    ...inputExample,
    notes: padEdgeNotes,
    related: ['pad_button', 'pad_pressed'],
  },
  'input/pad_axis': {
    description: 'Read an analog controller axis. Use it for proportional movement.',
    signatures: inputSignatures('pad_axis(slot, axis)'),
    params: [['slot', padSlot],
      ['axis', 'Required case-sensitive axis string. Use `PAD_LEFT_X`, `PAD_LEFT_Y`, `PAD_RIGHT_X`, `PAD_RIGHT_Y`, `PAD_LEFT_TRIGGER`, `PAD_RIGHT_TRIGGER`, or their lowercase names without the PAD_ prefix.']],
    returns: 'One Lua number. Sticks range from −1 to 1. Triggers range from 0 to 1. An unavailable axis, disconnected slot or headless run gives 0. Invalid slot, name or argument count raises a Lua error.',
    ...inputExample,
    related: ['pad_connected', 'pad_button'],
  },
  'input/text_input_next': {
    description: 'Read one confirmed character from the shared text input queue. Use it when entering names or other text.',
    signatures: inputSignatures('text_input_next()'),
    params: [],
    returns: 'One UTF-8 Lua string containing a Unicode character, or nil if the queue is empty or the game is headless. Extra arguments or an invalid queued character raise a Lua error.',
    ...inputExample,
    notes: 'A successful read removes the oldest character. Use a single reader if several processes need entered text. The game owns any editing and display of the accumulated string.',
    related: ['key', 'text_input_set_keyboard'],
  },
  'input/text_input_set_keyboard': {
    description: 'Request or hide the Android on-screen keyboard. Request it when the player selects a text field.',
    signatures: inputSignatures('text_input_set_keyboard(visible)'),
    params: [['visible', 'Required boolean. True requests the keyboard. False hides it.']],
    returns: 'One boolean. True means the request was submitted to Android. False means the service is unavailable, including on desktop and in headless mode. Invalid arguments raise a Lua error.',
    ...inputExample,
    exampleSetup: inputExample.exampleSetup + ` On desktop, press Enter to see the false result. For Android, create a Lua project with \`malvado new input-monitor\`, save the download as \`main.lua\` in that project, then export and install it using the ${androidExport}.`,
    exampleResult: 'Tap the Typed label in the exported game to request the keyboard. Confirm characters to append them to the label. Tap the label again to hide it. Use Android Back to exit early.',
    notes: 'Android decides whether to show an input method. A hide request or backgrounding discards unfinished composition. The confirmed characters remain in the shared queue until read.',
    related: ['text_input_next'],
  },
};
