import { referenceExample, luaColor, colorParameter } from './common.mjs';

const shapes = referenceExample('shapes',
  'Look for a filled and outlined blue rectangle, a filled and outlined orange circle, a blue line, and a centered white label on a dark background. The program closes after two seconds. You can also close the window.', true);

const startup = referenceExample('startup',
  'The program prints a virtual size of 320 × 240, resizes it to 640 × 360, then prints the two process cadences (30 and 20) and the elapsed wall time. It exits when both processes finish.');

const primitiveColor = `Optional ${luaColor}. The default comes from the current drawing color.`;

const pixelInteger = 'Required Lua number with a whole value in the C int range, in world pixels.';

const primitiveReturn = 'No values. Invalid options, missing process context or failure to queue the command raise a Lua error.';

const drawingNotes = 'Call from the scheduled process coroutine. Commands use the process camera, region and scroll. They draw after its graph, in call order. Reissue the command after the next activation to keep it visible.';

const rectangle = [
  ['x', 'Required finite number. Left edge.'],
  ['y', 'Required finite number. Top edge.'],
  ['width', 'Required finite positive number.'],
  ['height', 'Required finite positive number.'],
];

const fields = (prefix, definitions) =>
  definitions.map(([name, text]) => [`${prefix}.${name}`, text]);

const transform = [
  ['x', 'Optional finite world x in pixels. Defaults to 0.'],
  ['y', 'Optional finite world y in pixels. Defaults to 0.'],
  ['angle', 'Optional finite clockwise rotation in degrees. Defaults to 0.'],
  ['scale', 'Optional finite positive multiplier. Defaults to 1.'],
];

const graphicsExample = referenceExample('graphics',
  'A blue rectangle and an orange circle scroll through a fixed clip. The graphs are redrawn automatically while the processes are alive. The window closes after two seconds.', true);

const imagesExample = {
  exampleId: 'reference-images',
  exampleSetup: 'With the graphical Malvado SDK and a desktop display, extract the ZIP to an empty directory. Keep `assets/tile.png` in its subdirectory and run `malvado run images.lua` beside the script.',
  exampleResult: 'The terminal prints a 32 × 32 tile size and the selected asset directory. The window shows a red tile and a rotated crop, then closes after two seconds.',
};

const atlasExample = {
  exampleId: 'reference-atlas',
  exampleSetup: 'With a graphical, asset-enabled Malvado SDK and a desktop display, extract the ZIP to an empty directory. Keep `sheet.json` and `sheet.png` beside the script and run `malvado run atlas.lua` there.',
  exampleResult: 'The program prints the first sprite name and 4 × 4 region size. It shows an automatic animation, a manually sampled copy and a static sprite. It prints “Animation finished” and exits.',
};

const graphicsSignatures = (...calls) => calls.flatMap((call) => [`mv.${call}`, `mv.graphics.${call}`, call]);

const graphicDimension = 'Required finite Lua number in local pixels, from `FLT_MIN` to `FLT_MAX / 16`.';

const graphicColor = `Optional ${luaColor}. Defaults to opaque white.`;

const graphicReturn = 'One immutable, reusable graph descriptor. Invalid input or unknown fields raise a Lua error.';

const scrollCoordinate = 'Required finite Lua number in world pixels with magnitude at most `FLT_MAX / 16`.';

const screenInteger = 'Required whole Lua number in the C int range, in logical screen pixels.';

const graphLibrary = 'Atlas, FPG or FPG manifest handle. Omit it to use the scheduled process’s `self.file`.';

const selector = 'Required positive whole Lua number in C int range, or an exact case-sensitive atlas name without zero bytes. Digit strings are names.';

const graphLimits = 'Requires asset support. The implicit form requires a scheduled process with a library in `self.file`. Explicit selection works from setup or another process.';

const playbackHandle = 'Optional live process handle from this runtime. Omit it to inspect the scheduled process. Explicit nil is invalid.';

const playbackError = 'Invalid arguments, dead handles or missing scheduled context for an implicit call raise a Lua error. Requires asset support.';

const animationTime = 'Required finite Lua number from 0 to 1e12, in milliseconds. Fractional values are accepted.';

const atlasHandle = 'Required atlas handle from `load_asset`, `parse_atlas` or `parse_aseprite`. Requires asset support.';

const imageHandle = 'Required live managed image, such as from `load_image` or PNG `load_asset`.';

const assetPath = 'Required nonempty Lua string without zero bytes. An absolute path is used directly. A relative path starts at the current game asset directory.';

const imageDrawPosition = 'Required finite Lua number in world pixels for the pivot location. The process camera and scroll apply.';

const imageDrawTransform = [
  ['options.x', imageDrawPosition + ' Horizontal.'],
  ['options.y', imageDrawPosition + ' Vertical.'],
  ['options.scale', 'Optional finite positive uniform scale. Defaults to 1.'],
  ['options.angle', 'Optional finite clockwise angle in degrees. Defaults to 0.'],
];

const imageTransformBounds = 'Transform values, scaled dimensions and the scaled pivot must fit in the native float range.';

const animationName = 'Required exact, case-sensitive animation name string without zero bytes.';

const loopField = 'Optional boolean. Defaults to true. False holds the last frame after the sequence ends.';

export default {
  'graphics/rect': {
    description: 'Create a filled rectangle graph. Assign it to a process to keep the rectangle visible between activations.',
    signatures: graphicsSignatures('rect(options)'),
    params: [['options', 'Required table.'],
      ['options.width', graphicDimension], ['options.height', graphicDimension],
      ['options.color', graphicColor],
      ['options.pivot', 'Optional string. Defaults to "center". Use "top-left" to put the origin at the upper-left corner.']],
    returns: graphicReturn,
    ...graphicsExample,
    notes: 'The process position, angle and size transform the rectangle. The graph also provides the default collider unless the process supplies a collider or disables it.',
    related: ['circle', 'draw_rect', 'region'],
  },
  'graphics/circle': {
    description: 'Create a filled circle graph. Assign it to a process to draw a circle centered at its position.',
    signatures: graphicsSignatures('circle(options)'),
    params: [['options', 'Required table.'], ['options.radius', graphicDimension], ['options.color', graphicColor]],
    returns: graphicReturn,
    ...graphicsExample,
    notes: 'The process size scales the circle. The graph can also supply its default collider.',
    related: ['rect', 'draw_circle', 'region'],
  },
  'graphics/region': {
    description: 'Make a reusable screen-space clip. Assign it to `self.region` to keep process drawing inside a fixed rectangle.',
    signatures: graphicsSignatures('region(options)'),
    params: [['options', 'Required table.'],
      ['options.x', screenInteger + ' Left edge.'], ['options.y', screenInteger + ' Top edge.'],
      ['options.width', screenInteger + ' Must be positive.'], ['options.height', screenInteger + ' Must be positive.']],
    returns: 'One immutable region descriptor. Invalid arguments or unknown fields raise a Lua error.',
    ...graphicsExample,
    notes: 'The clip intersects the window and the selected camera’s viewport. Set `self.region = nil` to clear it. It does not change process positions or collision. You can also use it as a camera viewport.',
    related: ['scroll', 'camera_create'],
  },
  'graphics/scroll': {
    description: 'Create an absolute drawing offset. Assign it to `self.scroll` to scroll one process’s graph and immediate drawing.',
    signatures: graphicsSignatures('scroll(x, y)'),
    params: [['x', scrollCoordinate + ' Horizontal offset.'], ['y', scrollCoordinate + ' Vertical offset.']],
    returns: 'One immutable scroll descriptor. Invalid arguments or offsets outside the supported range raise a Lua error.',
    ...graphicsExample,
    notes: 'Rendering subtracts the offset. The clip stays fixed in screen space and gameplay coordinates stay unchanged. Assign nil to clear the offset.',
    related: ['region', 'camera_move'],
  },
  'graphics/animation': {
    playground: ['animation', 'Try the animation timeline'],
    description: 'Create an automatic animation for `self.graph`. Use a named atlas sequence or assemble your own sequence from sprite IDs.',
    signatures: graphicsSignatures('animation(sequence, options?)', 'animation(library, sequence, options?)'),
    params: [['library', graphLibrary + ' Named sequences require an atlas.'],
      ['sequence', 'Required exact, nonempty atlas animation name without zero bytes, or a dense array of 1–4,096 existing positive integer sprite IDs. Named sequences can contain up to 8,192 expanded frames.'],
      ['options', 'Optional table. Omission or nil uses defaults.'],
      ['options.loop', loopField],
      ['options.fps', 'Optional finite number for a custom list, from 1/3600 to 1,000 frames per second. Defaults to 8. Uniform durations round to the nearest microsecond. The named form uses imported durations and accepts only loop.']],
    returns: 'One immutable animation descriptor. Invalid libraries, names, sprite IDs, list shapes, options, missing renderable data or allocation failures raise a Lua error.',
    ...atlasExample,
    notes: graphLimits + ' A process plays the descriptor on its simulation clock. A named sequence reuses its cached identity while in use. Assigning it again preserves playback. Clear the graph or switch definitions to reset it. The descriptor retains frame resources and supplies a fixed default collider over the frames.',
    related: ['graph', 'animation_elapsed', 'animation_finished', 'animation_sample'],
  },
  'graphics/animation_elapsed': {
    description: 'Read a running process’s animation time. Use it to synchronize a secondary visual with the process graph.',
    signatures: [...graphicsSignatures('animation_elapsed()', 'animation_elapsed(handle)'), 'handle:animation_elapsed()'],
    params: [['handle', playbackHandle]],
    returns: 'One Lua number in milliseconds, or nil if there is no active animation clock. ' + playbackError,
    ...atlasExample,
    notes: 'Looping playback wraps within the sequence. The query leaves the clock unchanged. The clock advances on simulation ticks and pauses while the process sleeps or is frozen.',
    related: ['animation', 'animation_finished'],
  },
  'graphics/animation_finished': {
    description: 'Check whether a non-looping animation has finished. Use it to end an effect after its final frame plays.',
    signatures: [...graphicsSignatures('animation_finished()', 'animation_finished(handle)'), 'handle:animation_finished()'],
    params: [['handle', playbackHandle]],
    returns: 'One boolean. True means the non-looping animation reached its full duration, including its final frame. A looping, static, cleared or still-playing graph yields false. ' + playbackError,
    ...atlasExample,
    notes: 'Completed definitions hold their final frame. Reassigning the same cached definition preserves completion. Clear the graph or select a different definition to reset it.',
    related: ['animation', 'animation_elapsed'],
  },
  'graphics/graph': {
    description: 'Select one static sprite for `self.graph`. Use it when a process should keep displaying the same library graphic.',
    signatures: [...graphicsSignatures('graph(selector)', 'graph(library, selector)'), 'atlas:graph(selector)', 'fpg:graph(id)', 'manifest:graph(id)'],
    params: [['library', graphLibrary], ['selector', selector + ' FPG and manifest selectors accept only numeric IDs.']],
    returns: 'One immutable sprite descriptor. An invalid library or selector, missing sprite or unavailable visual image for a windowed selection raises a Lua error.',
    ...atlasExample,
    notes: graphLimits + ' Descriptors retain their library and image. A cached ID and its name share a descriptor. Atlas graphics use metadata pivots. FPG graphics use control point 0 if defined, or the upper-left corner. Headless selection can supply geometry without a texture.',
    related: ['animation', 'draw_sprite', 'load_asset'],
  },

  'drawing/draw_rect': {
    description: 'Queue a filled rectangle or an outline. Use an outline to show a boundary without covering the area inside it.',
    signatures: ['mv.draw_rect(options)', 'draw_rect(options)'],
    params: [
      ['options', 'Required table with the following fields.'],
      ['options.x', pixelInteger + ' Left edge.'],
      ['options.y', pixelInteger + ' Top edge.'],
      ['options.width', pixelInteger + ' Must be positive.'],
      ['options.height', pixelInteger + ' Must be positive.'],
      ['options.filled', 'Optional boolean. Defaults to true. Use false for a one-pixel outline.'],
      ['options.color', primitiveColor],
    ],
    returns: primitiveReturn,
    ...shapes,
    notes: 'The far edges at x + width and y + height are exclusive. Both must fit a C int. At unit scale the last column and row are one pixel before those edges. ' + drawingNotes,
    related: ['draw_circle', 'draw_line', 'set_color'],
  },
  'drawing/draw_circle': {
    description: 'Queue a filled circle or an outline at a world position. Use a circle to mark a point or show an area around it.',
    signatures: ['mv.draw_circle(options)', 'draw_circle(options)'],
    params: [
      ['options', 'Required table with the following fields.'],
      ['options.x', pixelInteger + ' Center x.'],
      ['options.y', pixelInteger + ' Center y.'],
      ['options.radius', pixelInteger + ' Must be positive.'],
      ['options.filled', 'Optional boolean. Defaults to true. Use false for the native one-pixel outline.'],
      ['options.color', primitiveColor],
    ],
    returns: primitiveReturn,
    ...shapes,
    notes: 'The center plus and minus the radius must fit a C int on both axes. ' + drawingNotes,
    related: ['draw_rect', 'draw_line', 'set_color'],
  },
  'drawing/draw_line': {
    description: 'Queue a line between two points. Use it to connect objects or show a direction.',
    signatures: ['mv.draw_line(options)', 'draw_line(options)'],
    params: [
      ['options', 'Required table with the following fields.'],
      ['options.x1', pixelInteger + ' Start x.'],
      ['options.y1', pixelInteger + ' Start y.'],
      ['options.x2', pixelInteger + ' End x.'],
      ['options.y2', pixelInteger + ' End y.'],
      ['options.thickness', 'Optional finite number in world pixels. Defaults to 1. Fractional values from `FLT_MIN` through `FLT_MAX / 16` are valid.'],
      ['options.color', primitiveColor],
    ],
    returns: primitiveReturn,
    ...shapes,
    notes: 'The stroke is centered on the segment and has flat ends. Equal endpoints draw a square centered on that pixel. ' + drawingNotes,
    related: ['draw_rect', 'draw_circle', 'set_color'],
  },
  'drawing/draw_text': {
    description: 'Queue a text label for the current process activation. Use this for a label that moves in the same view as your drawing.',
    signatures: ['mv.draw_text(options)', 'draw_text(options)'],
    params: [
      ['options', 'Required table with the following fields.'],
      ['options.text', 'Required Lua string without zero bytes. Use `\\n` to start another line.'],
      ['options.x', pixelInteger + ' Horizontal alignment anchor.'],
      ['options.y', pixelInteger + ' Top of the text.'],
      ['options.align', 'Optional string. Use `TEXT_ALIGN_LEFT` (`"left"`, default), `TEXT_ALIGN_CENTER` (`"center"`) or `TEXT_ALIGN_RIGHT` (`"right"`). Add the `mv.` prefix when using module constants.'],
      ['options.font', 'Optional descriptor from `font` or `load_font`. Defaults to the built-in font at size 20.'],
      ['options.size', 'Optional whole Lua number in the C int range, in pixels. Defaults to the selected font size. With a font descriptor, the range is 1 through 1024.'],
      ['options.color', `Optional ${luaColor}. Defaults to opaque white.`],
    ],
    returns: primitiveReturn,
    ...shapes,
    notes: 'Alignment uses the measured width of the whole string. The command copies the text and style and retains a custom font until the command is released. ' + drawingNotes,
    related: ['font', 'load_font', 'write'],
  },
  'drawing/set_color': {
    description: 'Set the shared default color for later rectangle, circle and line commands.',
    signatures: ['mv.set_color(color)', 'set_color(color)'],
    params: [['color', colorParameter]],
    returns: 'No values. Invalid arguments raise a Lua error and leave the default unchanged.',
    ...shapes,
    notes: 'Each game starts with opaque white. Commands copy the color when queued. Pass a color directly in a command to override the shared default for that command.',
    related: ['clear_screen', 'draw_rect', 'draw_line'],
  },
  'drawing/clear_screen': {
    description: 'Set the background color for future renders. Call it when a level or screen needs a new backdrop.',
    signatures: ['mv.clear_screen(color)', 'clear_screen(color)'],
    params: [['color', colorParameter]],
    returns: 'No values. Invalid arguments raise a Lua error and leave the background unchanged.',
    ...shapes,
    notes: 'The background starts at opaque black. The function validates all channels and uses RGB for the opaque background.',
    related: ['set_color'],
  },

  'images/set_assets_dir': {
    description: 'Set the base directory for subsequent relative asset loads. Use it to keep level or game assets in a separate folder.',
    signatures: ['mv.set_assets_dir(path?)', 'set_assets_dir(path?)'],
    params: [['path', 'Optional nonempty Lua path string without zero bytes. Relative overrides start at the game directory. Omission, nil or "." restores that directory.']],
    returns: 'No values. Invalid paths, extra arguments or allocation failures raise a Lua error and preserve the previous directory.',
    ...imagesExample,
    notes: 'The directory itself can be absent when you set it. The setting belongs to the game. Existing libraries keep their own source directories. A returned game table’s assets_dir takes effect before its initializer.',
    related: ['get_assets_dir', 'load_asset'],
  },
  'images/get_assets_dir': {
    description: 'Read the current asset directory. Use it when opening a game file directly with the Lua standard library.',
    signatures: ['mv.get_assets_dir()', 'get_assets_dir()'],
    params: [],
    returns: 'One absolute path string. Extra arguments raise a Lua error.',
    ...imagesExample,
    related: ['set_assets_dir'],
  },
  'images/load_asset': {
    description: 'Load a PNG or sprite library from a file. Use this as the general gameplay asset loader.',
    signatures: ['mv.load_asset(path, options?)', 'load_asset(path, options?)'],
    params: [['path', assetPath],
      ['options', 'Optional table. Omission or nil uses the defaults.'],
      ['options.format', 'Optional string. Omit it to detect the format from file contents. Use "png", "fpg", "atlas", "aseprite" or "fpg-manifest" to select an importer explicitly.'],
      ['options.pivot', 'Optional PNG-only string. Defaults to "top-left". Use "center" for the exact center when drawing as a process graph or deriving a collider.'],
      ['options.transparent_zero', 'Optional binary-FPG-only boolean. Defaults to true, which makes palette entry 0 transparent when used as a library graphic.']],
    returns: 'One managed image, atlas, FPG or manifest handle, according to the format. Invalid arguments or options, missing or invalid files, unsupported formats and failed loads raise a Lua error.',
    ...atlasExample,
    notes: 'Files must be seekable and at most 64 MiB. Format-specific metadata limits can be smaller. PNG needs an open window. Other formats require asset support and can load metadata without a window. Load through an open window to get atlas or manifest textures. Child images resolve relative to the library file.',
    related: ['load_image', 'parse_atlas', 'parse_aseprite', 'graph', 'set_assets_dir'],
  },
  'images/load_image': {
    description: 'Load a standalone PNG. Use the image with `draw_image` or as a process graph.',
    signatures: ['mv.load_image(path)', 'load_image(path)'],
    params: [['path', assetPath]],
    returns: 'One managed image handle. An invalid path, an absent window, unsupported image data or a load failure raises a Lua error.',
    ...imagesExample,
    notes: 'The image uses the upper-left pivot for process drawing and its default collider. Call during startup after the window opens or from a process. Lua releases the image when its references are gone.',
    related: ['image_size', 'draw_image', 'load_asset'],
  },
  'images/image_size': {
    description: 'Read the dimensions of a loaded image. Use them to choose a source region or place the image.',
    signatures: ['mv.image_size(image)', 'image_size(image)', 'image:size()'],
    params: [['image', imageHandle]],
    returns: 'Two Lua integers in source pixels, width then height. An invalid image or argument count raises a Lua error.',
    ...imagesExample,
    related: ['load_image', 'draw_image'],
  },
  'images/draw_image': {
    description: 'Queue an entire image or a cropped region. Use this to draw a single tile or a portion of a sprite sheet.',
    signatures: ['mv.draw_image(options)', 'draw_image(options)'],
    params: [['options', 'Required table.'],
      ['options.image', imageHandle],
      ...imageDrawTransform,
      ['options.source', 'Optional source rectangle table in image pixels. The default is the entire image.'],
      ['options.source.x', 'Required nonnegative whole number when a source is supplied. Left edge.'],
      ['options.source.y', 'Required nonnegative whole number when a source is supplied. Top edge.'],
      ['options.source.width', 'Required positive whole number when a source is supplied.'],
      ['options.source.height', 'Required positive whole number when a source is supplied.'],
      ['options.pivot', 'Optional "top-left" (default), "center" or a point table. The center uses exact half dimensions.'],
      ['options.pivot.x', 'Required finite horizontal point offset from the source corner when a point is supplied, in unscaled source pixels.'],
      ['options.pivot.y', 'Required finite vertical point offset from the source corner when a point is supplied, in unscaled source pixels.']],
    returns: 'No values. Invalid arguments, invalid source bounds or transform, a non-process call or failure to queue drawing raises a Lua error.',
    ...imagesExample,
    notes: imageTransformBounds + ' The source must fit inside the image. The pivot can lie outside the source. The queued command retains the image until the process runs again or ends. Reissue the draw on each activation to keep it visible.',
    related: ['load_image', 'draw_sprite', 'graph'],
  },
  'images/parse_atlas': {
    description: 'Read Malvado atlas metadata for an image you already loaded. Use `load_asset` when you have a metadata file path.',
    signatures: ['mv.parse_atlas(image, json)', 'parse_atlas(image, json)'],
    params: [['image', imageHandle], ['json', 'Required Lua string containing schema 1 or 2 Malvado atlas JSON, at most 1 MiB.']],
    returns: 'One managed atlas handle. Invalid arguments, malformed or unsupported metadata and sprite regions outside the image raise a Lua error.',
    ...atlasExample,
    notes: 'Requires asset support. The atlas retains the supplied image. It validates region bounds but does not compare the metadata filename with the image path.',
    related: ['parse_aseprite', 'draw_sprite', 'load_asset'],
  },
  'images/parse_aseprite': {
    description: 'Import Aseprite JSON array metadata for an image you already loaded.',
    signatures: ['mv.parse_aseprite(image, json)', 'parse_aseprite(image, json)'],
    params: [['image', imageHandle], ['json', 'Required Lua string containing a supported Aseprite JSON array export, at most 1 MiB.']],
    returns: 'One managed atlas handle. Invalid arguments, unsupported metadata or dimensions that disagree with the image raise a Lua error.',
    ...atlasExample,
    notes: 'Requires asset support. The atlas retains the image and uses its declared dimensions. It accepts unrotated frames, scale 1 and RGBA8888 when a pixel format is specified. It imports supported tags and a pivot slice. See the [Aseprite format limits](../../../../concepts/images/#import-an-aseprite-json-array).',
    related: ['parse_atlas', 'atlas_animation_frame', 'draw_sprite'],
  },
  'images/draw_sprite': {
    description: 'Queue an atlas sprite using its stored pivot and crop. Use this when you choose the frame yourself for each activation.',
    signatures: ['mv.draw_sprite(options)', 'draw_sprite(options)'],
    params: [['options', 'Required table.'],
      ['options.atlas', atlasHandle],
      ['options.id', 'Required positive whole Lua number in C int range that identifies an existing sprite.'],
      ...imageDrawTransform],
    returns: 'No values. Invalid options, an unknown ID, missing visual data or a call outside the scheduled process coroutine raise a Lua error.',
    ...atlasExample,
    notes: imageTransformBounds + ' The command retains the atlas and its texture until the process next runs or ends. The atlas supplies the source rectangle and pivot, including trim offsets.',
    related: ['draw_image', 'atlas_sprite_info', 'atlas_sprite_id', 'graph'],
  },
  'images/atlas_sprite_info': {
    description: 'Read sprite metadata. Use it to display information such as a crop size or origin.',
    signatures: ['mv.atlas_sprite_info(atlas, selector)', 'atlas_sprite_info(atlas, selector)', 'atlas:sprite_info(selector)'],
    params: [['atlas', atlasHandle], ['selector', selector]],
    returns: 'One independent table, or nil if the sprite is absent. Fields are integer id, string name, source-region x, y, width and height, pivot_x and pivot_y relative to that region, original source_width and source_height, and trim_x and trim_y. Geometry fields are integers in pixels. Invalid arguments or selectors raise a Lua error.',
    ...atlasExample,
    notes: 'Changing the returned table leaves the atlas unchanged.',
    related: ['atlas_sprite_id', 'draw_sprite'],
  },
  'images/atlas_sprite_id': {
    description: 'Find the numeric ID of a named sprite. Use the ID with `draw_sprite`.',
    signatures: ['mv.atlas_sprite_id(atlas, name)', 'atlas_sprite_id(atlas, name)', 'atlas:sprite_id(name)'],
    params: [['atlas', atlasHandle], ['name', 'Required exact case-sensitive sprite name string without zero bytes.']],
    returns: 'One integer ID or nil if no sprite has that name. Invalid arguments raise a Lua error.',
    ...atlasExample,
    related: ['atlas_sprite_info', 'draw_sprite'],
  },
  'images/atlas_animation_frame': {
    description: 'Find the sprite ID at a given time in a named atlas animation. Use it when your game owns the playback clock.',
    signatures: ['mv.atlas_animation_frame(atlas, name, elapsed_ms, options?)', 'atlas_animation_frame(atlas, name, elapsed_ms, options?)', 'atlas:animation_frame(name, elapsed_ms, options?)'],
    params: [['atlas', atlasHandle], ['name', animationName], ['elapsed_ms', animationTime],
      ['options', 'Optional table. Omission or nil uses the default loop behavior.'],
      ['options.loop', loopField]],
    returns: 'One integer sprite ID. Invalid arguments, unknown options or an unknown animation name raise a Lua error.',
    ...atlasExample,
    notes: 'The query uses imported durations and direction. Frame boundaries are whole milliseconds. The query does not change process playback.',
    related: ['atlas_animation_color', 'atlas_animation_repeats', 'draw_sprite', 'animation_sample'],
  },
  'images/atlas_animation_color': {
    description: 'Read an imported Aseprite animation tag color. Use it when inspecting or presenting imported metadata.',
    signatures: ['mv.atlas_animation_color(atlas, name)', 'atlas_animation_color(atlas, name)', 'atlas:animation_color(name)'],
    params: [['atlas', atlasHandle], ['name', animationName]],
    returns: 'One fresh positional RGBA table with integer channels from 0 to 255, or nil if the animation or color is absent. Invalid arguments raise a Lua error.',
    ...atlasExample,
    related: ['atlas_animation_frame', 'atlas_pivot_color'],
  },
  'images/atlas_pivot_color': {
    description: 'Read the imported Aseprite pivot slice color. Use it when inspecting metadata from a sprite sheet.',
    signatures: ['mv.atlas_pivot_color(atlas)', 'atlas_pivot_color(atlas)', 'atlas:pivot_color()'],
    params: [['atlas', atlasHandle]],
    returns: 'One fresh positional RGBA table with integer channels from 0 to 255, or nil if no pivot slice color was imported. Invalid arguments raise a Lua error.',
    ...atlasExample,
    related: ['atlas_animation_color', 'parse_aseprite'],
  },
  'images/atlas_animation_repeats': {
    description: 'Read the repeat count recorded in an Aseprite animation tag.',
    signatures: ['mv.atlas_animation_repeats(atlas, name)', 'atlas_animation_repeats(atlas, name)', 'atlas:animation_repeats(name)'],
    params: [['atlas', atlasHandle], ['name', animationName]],
    returns: 'One integer from 0 to 65,535, or nil if the tag or count is absent. Invalid arguments raise a Lua error.',
    ...atlasExample,
    notes: 'The repeat count is metadata. Set the loop option to control playback.',
    related: ['atlas_animation_frame', 'animation'],
  },

  'animations/animation_sample': {
    description: 'Select a static sprite at a given time in an animation. Use it for a preview or an explicitly sampled pose.',
    signatures: ['mv.animation_sample(animation, elapsed_ms)', 'animation_sample(animation, elapsed_ms)', 'animation:sample(elapsed_ms)'],
    params: [['animation', 'Required animation descriptor from `animation`. Requires asset support.'], ['elapsed_ms', animationTime]],
    returns: 'One reusable static graph descriptor. Invalid descriptors, times or argument counts raise a Lua error.',
    ...atlasExample,
    notes: 'Sampling honors the definition’s loop rule and uses microsecond precision. It leaves process playback unchanged.',
    related: ['animation', 'atlas_animation_frame', 'animation_elapsed'],
  },
};
