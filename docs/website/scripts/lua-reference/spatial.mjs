import { referenceExample, collisionExample } from './common.mjs';

const shapes = referenceExample('shapes',
  'Look for a filled and outlined blue rectangle, a filled and outlined orange circle, a blue line, and a centered white label on a dark background. The program closes after two seconds. You can also close the window.', true);

const cameraExample = referenceExample('cameras',
  'The program moves an explicit camera to (15, 15), then tracks a process and reports (70, 40) after one tick. It destroys the camera and exits.');

const worldExample = referenceExample('world',
  'The program builds a wall and first sweeps a test shape toward it. It reports first contact at 92 pixels, moves a process to the wall and exits.');

const cameraArgument = 'A live camera from `camera_create`. Use the form without a camera to select the main camera.';

const cameraCoordinate = 'Finite Lua number in world pixels, with magnitude at most `FLT_MAX / 64`.';

const cameraReturns = 'No values. Invalid arguments or a destroyed camera raise a Lua error.';

const rectangle = [
  ['x', 'Required finite number. Left edge.'],
  ['y', 'Required finite number. Top edge.'],
  ['width', 'Required finite positive number.'],
  ['height', 'Required finite positive number.'],
];

const fields = (prefix, definitions) =>
  definitions.map(([name, text]) => [`${prefix}.${name}`, text]);

const rectangleNotes = 'Use the same coordinate space for all values. The right and bottom edges must be finite and greater than the left and top edges after addition.';

const transform = [
  ['x', 'Optional finite world x in pixels. Defaults to 0.'],
  ['y', 'Optional finite world y in pixels. Defaults to 0.'],
  ['angle', 'Optional finite clockwise rotation in degrees. Defaults to 0.'],
  ['scale', 'Optional finite positive multiplier. Defaults to 1.'],
];

const shapeOffsets = [
  ['options.offset_x', 'Optional finite horizontal center offset from the local origin, in pixels. Defaults to 0.'],
  ['options.offset_y', 'Optional finite vertical center offset from the local origin, in pixels. Defaults to 0.'],
];

const shapeBounds = 'Transformed world centers and bounding half-extents must be within `DBL_MAX / 16` in magnitude.';

const shapeReturn = 'One immutable, reusable collider descriptor. Invalid dimensions, offsets or unknown fields raise a Lua error.';

const worldParameter = 'Required collision world from `world_create`.';

const shapeParameter = 'Required descriptor from `mv.collision.circle` or `mv.collision.rect`.';

const worldHit = 'A one-based integer ID for the first overlapping obstacle in insertion order, or nil if none overlaps.';

const worldDelta = 'Required finite displacement in world pixels, with magnitude at most `DBL_MAX / 16`.';

const constructor = 'Required constructor returned by `mv.process` or `mv.process.define`.';

export default {
  'cameras/camera_follow': {
    description: 'Track a process with a camera. Use this to keep a player or another moving target centered.',
    signatures: ['mv.camera_follow(target)', 'mv.camera_follow(camera, target)', 'camera_follow(target)', 'camera_follow(camera, target)', 'camera:follow(target)'],
    params: [['camera', cameraArgument], ['target', 'Required live process handle.']],
    returns: cameraReturns + ' A dead target also raises an error.',
    ...cameraExample,
    notes: 'Following centers the target origin after each simulation tick, including while the target sleeps or is frozen. A successful set or move cancels following. A terminated target leaves the camera at its last position. Invalid target coordinates leave the camera at its last valid position until tracking can resume.',
    related: ['camera_set', 'camera_move', 'camera_create'],
  },
  'cameras/camera_create': {
    description: 'Create a separate camera view. Assign it to `self.camera` to draw a process through it.',
    signatures: ['mv.camera_create(options)', 'camera_create(options)'],
    params: [['options', 'Required table with the following fields. Pass {} for the defaults.'],
      ['options.x', 'Optional. ' + cameraCoordinate + ' Defaults to 0.'],
      ['options.y', 'Optional. ' + cameraCoordinate + ' Defaults to 0.'],
      ['options.viewport', 'Optional region descriptor in logical screen pixels. Defaults to the full logical window.']],
    returns: 'One managed camera handle. Unknown fields, invalid positions or viewports, and allocation failure raise a Lua error.',
    ...cameraExample,
    notes: 'Process fields and defaults retain their camera handles. Lua closes an explicit camera when it is collected. You can also close it earlier with `camera_destroy`. Use `nil` for the main camera or `false` for screen space in `self.camera`.',
    related: ['region', 'camera_set', 'camera_follow', 'camera_position', 'camera_destroy'],
  },
  'cameras/camera_set': {
    playground: ['cameras-and-scrolling', 'Explore camera position and the viewport'],
    description: 'Set the world position of a camera. Use this when switching to a fixed view or resetting the view at the start of a level.',
    signatures: ['mv.camera_set(x, y)', 'mv.camera_set(camera, x, y)', 'camera_set(x, y)', 'camera_set(camera, x, y)', 'camera:set(x, y)'],
    params: [['camera', cameraArgument], ['x', 'Required. ' + cameraCoordinate], ['y', 'Required. ' + cameraCoordinate]],
    returns: cameraReturns,
    ...cameraExample,
    notes: 'A successful call cancels following. Camera movement leaves process and collider positions unchanged.',
    related: ['camera_move', 'camera_follow', 'camera_position'],
  },
  'cameras/camera_move': {
    description: 'Add a displacement to a camera’s world position. Use this for a pan or a scrolling view.',
    signatures: ['mv.camera_move(dx, dy)', 'mv.camera_move(camera, dx, dy)', 'camera_move(dx, dy)', 'camera_move(camera, dx, dy)', 'camera:move(dx, dy)'],
    params: [['camera', cameraArgument], ['dx', 'Required finite horizontal delta in world pixels.'], ['dy', 'Required finite vertical delta in world pixels.']],
    returns: cameraReturns + ' A resulting position beyond the supported range also raises an error and leaves the camera unchanged.',
    ...cameraExample,
    notes: 'A successful call cancels following. The resulting world coordinates must be finite and at most `FLT_MAX / 64` in magnitude.',
    related: ['camera_set', 'camera_follow', 'scroll'],
  },
  'cameras/camera_position': {
    description: 'Read the world coordinates of a camera. Use this to inspect where a view has moved.',
    signatures: ['mv.camera_position(camera?)', 'camera_position(camera?)', 'camera:position()'],
    params: [['camera', cameraArgument + ' The module and global forms also accept nil for the main camera.']],
    returns: 'Two Lua numbers in world pixels, x then y. Extra arguments and invalid or destroyed camera handles raise a Lua error.',
    ...cameraExample,
    related: ['camera_set', 'camera_move', 'camera_follow'],
  },
  'cameras/camera_destroy': {
    description: 'Close an explicit camera and release its follow target.',
    signatures: ['mv.camera_destroy(camera)', 'camera_destroy(camera)', 'camera:destroy()'],
    params: [['camera', 'Required camera handle from `camera_create`.']],
    returns: 'No values. Repeating the call on the same camera succeeds. An invalid argument raises a Lua error.',
    ...cameraExample,
    notes: 'Processes using the closed view stop drawing until assigned a live camera, nil or false. They continue running and colliding. A closed camera cannot be used for a new assignment. The main camera has no destroy operation.',
    related: ['camera_create', 'camera_position'],
  },

  'collision/collision': {
    description: 'Find the first process that overlaps the running process. Use it to check contact with an enemy or pickup.',
    signatures: ['mv.collision(target)', 'collision(target)'],
    params: [['target', 'Required process constructor or process handle. A constructor selects instances of that type.']],
    returns: 'The first canonical matching process handle in creation order, or nil if no process overlaps. Invalid arguments, invalid source geometry or a call outside the scheduled process coroutine raise a Lua error.',
    ...collisionExample,
    notes: 'The query excludes the source and includes sleeping and frozen processes. A dead target does not match. An invalid explicitly selected live target raises an error. A type query skips instances with invalid colliders or transforms.',
    related: ['collisions'],
  },
  'collision/rect_overlaps': {
    playground: ['collisions', 'Move two rectangles and test for overlap'],
    description: 'Test whether two axis-aligned rectangles share any area. Use it for bounds you track yourself.',
    signatures: ['mv.rect_overlaps(a, b)', 'rect_overlaps(a, b)'],
    params: [['a', 'Required first rectangle table.'],
      ['b', 'Required second rectangle table. Both tables have the following fields.'],
      ...rectangle],
    returns: 'One boolean. Edge contact returns false. Invalid arguments, geometry or unknown fields raise a Lua error.',
    ...collisionExample,
    notes: rectangleNotes,
    related: ['rect_contains', 'overlap'],
  },
  'collision/rect_contains': {
    description: 'Test whether a point is inside an axis-aligned rectangle. Use it for pointer hit testing or bounds checks.',
    signatures: ['mv.rect_contains(rect, x, y)', 'rect_contains(rect, x, y)'],
    params: [['rect', 'Required rectangle table.'],
      ...fields('rect', rectangle),
      ['x', 'Required finite point x.'], ['y', 'Required finite point y.']],
    returns: 'One boolean. The left and top edges are included. The right and bottom edges are excluded. Invalid arguments, geometry or unknown fields raise a Lua error.',
    ...collisionExample,
    notes: rectangleNotes,
    related: ['rect_overlaps'],
  },
  'collision/circle': {
    description: 'Create a reusable circle collider. Assign it to `self.collider` or pass it to a geometry query.',
    signatures: ['mv.collision.circle(options)', 'collision.circle(options)'],
    params: [['options', 'Required table with the following fields.'],
      ['options.radius', 'Required finite positive radius in local pixels.'],
      ...shapeOffsets],
    returns: shapeReturn,
    ...collisionExample,
    notes: shapeBounds + ' When a process uses the collider, its scale and angle also apply to the center offset.',
    related: ['rect', 'overlap'],
  },
  'collision/rect': {
    description: 'Create a reusable rectangle collider. Use it for a box that can rotate with a process.',
    signatures: ['mv.collision.rect(options)', 'collision.rect(options)'],
    params: [['options', 'Required table with the following fields.'],
      ['options.width', 'Required finite positive width in local pixels.'],
      ['options.height', 'Required finite positive height in local pixels.'],
      ...shapeOffsets],
    returns: shapeReturn,
    ...collisionExample,
    notes: shapeBounds + ' The local origin defaults to the center. Use half the width and height as offsets to place the origin at the upper-left corner.',
    related: ['circle', 'overlap'],
  },
  'collision/overlap': {
    description: 'Test two colliders for overlap at arbitrary positions. Use this to check geometry independently of the process scheduler.',
    signatures: ['mv.collision.overlap(shape_a, shape_b, options?)', 'collision.overlap(shape_a, shape_b, options?)'],
    params: [['shape_a', shapeParameter], ['shape_b', shapeParameter],
      ['options', 'Optional table. Omit it to test both shapes at the origin with identity transforms. Explicit nil for this argument raises an error.'],
      ['options.a', 'Optional transform table for the first shape.'],
      ['options.b', 'Optional transform table for the second shape. Both transforms have the following fields.'],
      ...transform],
    returns: 'One boolean. Touching edges return false. Invalid shapes, transforms or unknown fields raise a Lua error.',
    ...collisionExample,
    notes: shapeBounds + ' Floating-point rounding can affect contact at rotated edges.',
    related: ['circle', 'rect', 'world_overlap'],
  },

  'world/world_create': {
    description: 'Create an empty static collision world. Use this for walls or other obstacles that stay fixed.',
    signatures: ['mv.world_create()', 'world_create()'],
    params: [],
    returns: 'One managed collision world handle. Extra arguments or allocation failure raise a Lua error.',
    ...worldExample,
    notes: 'The world owns its rectangles and is released when Lua collects it. You can store it in `self.world` for contextual queries and movement.',
    related: ['world_add_rect', 'world_move'],
  },
  'world/world_add_rect': {
    description: 'Add an axis-aligned obstacle to a static collision world.',
    signatures: ['mv.world_add_rect(world, rect)', 'world_add_rect(world, rect)', 'world:add_rect(rect)'],
    params: [['world', worldParameter], ['rect', 'Required rectangle table in world pixels.'],
      ...fields('rect', rectangle)],
    returns: 'One positive integer obstacle ID, assigned in insertion order starting at 1. Invalid fields or handles, out-of-range geometry, capacity exhaustion or allocation failure raise a Lua error.',
    ...worldExample,
    notes: rectangleNotes + ' ' + shapeBounds + ' The world copies the rectangle.',
    related: ['world_create', 'world_overlap'],
  },
  'world/world_overlap': {
    description: 'Find the first static obstacle that overlaps a placed collider.',
    signatures: ['mv.world_overlap(world, shape, transform?)', 'world_overlap(world, shape, transform?)', 'world:overlap(shape, transform?)'],
    params: [['world', worldParameter], ['shape', shapeParameter],
      ['transform', 'Optional table. Omission or nil uses the identity transform.'],
      ...fields('transform', transform)],
    returns: worldHit + ' Invalid handles, transforms, geometry or unknown fields raise a Lua error.',
    ...worldExample,
    notes: shapeBounds + ' Edge contact does not overlap. This query does not retain the shape.',
    related: ['world_sweep', 'world_collision'],
  },
  'world/world_sweep': {
    description: 'Find first contact along a straight path. Use a sweep to see how far a collider can travel before it touches a wall.',
    signatures: ['mv.world_sweep(world, shape, options)', 'world_sweep(world, shape, options)', 'world:sweep(shape, options)'],
    params: [['world', worldParameter], ['shape', shapeParameter],
      ['options', 'Required table with displacement and start transform fields.'],
      ['options.dx', worldDelta + ' Horizontal.'], ['options.dy', worldDelta + ' Vertical.'],
      ...fields('options', transform)],
    returns: 'One table with boolean `hit`, numeric `fraction` from 0 to 1, numeric `normal_x` and `normal_y`, and an integer obstacle `id` on impact. With no hit, fraction is 1 and the normal is (0, 0). Invalid arguments, initial overlap or an unsupported sweep raise a Lua error.',
    ...worldExample,
    notes: shapeBounds + ' Start and end positions must both be valid. Equal-time hits select the lowest obstacle ID. Moving into a touching obstacle hits at fraction 0. Moving away, parallel or merely grazing an edge is allowed. The [sweep guide](../../../../concepts/collision/#sweeping-a-shape) gives the circle precision limit. The query leaves the shape and process positions unchanged.',
    related: ['world_overlap', 'world_move'],
  },
  'world/world_collision': {
    description: 'Find the first static obstacle that overlaps a process collider. Pass a proposed position to check it before moving.',
    signatures: ['mv.world_collision(options?)', 'mv.world_collision(world, process, options?)', 'world_collision(options?)', 'world_collision(world, process, options?)', 'world:collision(process, options?)'],
    params: [['world', 'A collision world. Omit it and the process together to use the scheduled process’s `self.world`.'],
      ['process', 'A live process handle for the explicit form.'],
      ['options', 'Optional table. Omit it to test the current position. Explicit nil is accepted in the explicit form.'],
      ['options.x', 'Optional finite world x in pixels. Defaults to the process x.'],
      ['options.y', 'Optional finite world y in pixels. Defaults to the process y.']],
    returns: worldHit + ' A disabled or absent collider without automatic geometry also gives nil. Invalid handles, unsupported geometry, invalid transforms or missing contextual process or world raise a Lua error.',
    ...worldExample,
    notes: 'The query uses the process angle, size and collider offset. With no explicit collider, a supported graph supplies one. A false collider disables the query. Position overrides leave the process where it is.',
    related: ['world_overlap', 'world_move'],
  },
  'world/world_move': {
    description: 'Move a process by the part of a displacement that fits before a static obstacle.',
    signatures: ['mv.world_move(dx, dy)', 'mv.world_move(world, process, dx, dy)', 'world_move(dx, dy)', 'world_move(world, process, dx, dy)', 'world:move(process, dx, dy)'],
    params: [['world', 'A collision world. Omit it and the process together to use the scheduled process’s `self.world`.'],
      ['process', 'A live process handle for the explicit form.'],
      ['dx', worldDelta + ' Horizontal.'], ['dy', worldDelta + ' Vertical.']],
    returns: 'One table with boolean `hit`, numeric `moved_x` and `moved_y` in world pixels, numeric `normal_x` and `normal_y`, and integer obstacle `id` on impact. Invalid arguments, missing contextual process or world, unsupported geometry or initial overlap raise a Lua error without moving the process.',
    ...worldExample,
    notes: 'The operation updates x and y and then stops at contact without sliding. A disabled collider or a process with neither a collider nor a graph moves freely. When no explicit collider is set, a supported graph supplies one.',
    related: ['world_create', 'world_sweep', 'world_collision'],
  },
};
