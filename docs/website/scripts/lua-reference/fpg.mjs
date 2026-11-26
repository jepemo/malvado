const fields = (prefix, definitions) =>
  definitions.map(([name, text]) => [`${prefix}.${name}`, text]);

const fpgExample = {
  exampleId: 'reference-fpg-metadata',
  exampleSetup: 'With an asset-enabled Malvado SDK, extract the ZIP to an empty directory. Keep `graphics.fpg` beside the script and run `malvado run --headless fpg-metadata.lua` there.',
  exampleResult: 'The program checks the palette, gamma, control points and generated data. It prints sprite 7 (2 × 2, 7.png) and sprite 42 (2 × 1, 42.png), then exits.',
};

const fpgImageExample = {
  exampleId: 'reference-fpg-image',
  exampleSetup: 'With a graphical, asset-enabled Malvado SDK and a desktop display, extract the ZIP to an empty directory. Keep `graphics.fpg` beside the script and run `malvado run fpg-image.lua` there.',
  exampleResult: 'The program prints a 2 × 2 image size, draws the enlarged sprite and closes the window after two seconds.',
};

const fpgHandle = 'Required FPG handle from `mv.fpg.parse` or `load_asset`. Requires asset support.';

const manifestHandle = 'Required manifest handle from `mv.fpg.parse_manifest` or `load_asset`. Requires asset support.';

const fpgId = 'Required Lua number representable as an integer. Valid FPG IDs are 1 through 999. An integer outside that range is a miss.';

const fpgIndex = 'Required Lua number representable as an integer. Enumeration starts at 1, in file order.';

const pointIndex = 'Required Lua number representable as an integer. Point indices start at 0.';

const fpgLookupError = ' Invalid types or argument count raise a Lua error.';

const conversionOptions = [
  ['options', 'Optional table. Omission, nil or {} uses the conversion default.'],
  ['options.transparent_zero', 'Optional boolean. Defaults to false. True makes palette index 0 transparent.'],
];

export default {
  'fpg/parse': {
    description: 'Read an uncompressed 8-bit BennuGD graphics library. Use this to inspect or convert historical sprites.',
    signatures: ['mv.fpg.parse(bytes)'],
    params: [
      ['bytes', 'Required binary Lua string containing the complete supported FPG data, at most 64 MiB.'],
    ],
    returns: 'One managed FPG handle. Invalid or unsupported files, truncated records and allocation failures raise a Lua error.',
    ...fpgExample,
    notes: 'Requires asset support and works headless. The parser keeps a copy and Lua collects the handle. See the FPG format guide for record limits, accepted headers and palette encoding.',
    related: ['parse_manifest', 'fpg_count', 'load_asset'],
  },
  'fpg/parse_manifest': {
    description: 'Read metadata from a converted 8-bit FPG manifest without loading its PNGs.',
    signatures: ['mv.fpg.parse_manifest(json)'],
    params: [
      ['json', 'Required Lua string containing version 1 FPG manifest JSON, at most 16 MiB.'],
    ],
    returns: 'One managed manifest handle. Invalid JSON, invalid or missing fields, unsupported schemas, bad sprite metadata and allocation failure raise a Lua error.',
    ...fpgExample,
    notes: 'Requires asset support and works headless. Parsing does not open or check the PNG files. Use load_asset with the manifest path to attach a base directory for rendering.',
    related: ['parse', 'manifest_count', 'load_asset'],
  },
  'fpg/fpg_count': {
    description: 'Count the sprites in the library. Use it to enumerate entries.',
    signatures: ['mv.fpg_count(fpg)', 'fpg_count(fpg)', 'fpg:count()'],
    params: [
      ['fpg', fpgHandle],
    ],
    returns: 'One integer sprite count.' + fpgLookupError,
    ...fpgExample,
    related: ['parse', 'fpg_id_at'],
  },
  'fpg/fpg_palette': {
    description: 'Read the original palette. Use it when inspecting indexed colors.',
    signatures: ['mv.fpg_palette(fpg)', 'fpg_palette(fpg)', 'fpg:palette()'],
    params: [
      ['fpg', fpgHandle],
    ],
    returns: 'One binary Lua string of 768 RGB bytes in palette order. Each channel ranges from 0 to 63.' + fpgLookupError,
    ...fpgExample,
    related: ['parse', 'fpg_id_at'],
  },
  'fpg/fpg_gamma': {
    description: 'Read the original gamma table. Use it to preserve the metadata during conversion.',
    signatures: ['mv.fpg_gamma(fpg)', 'fpg_gamma(fpg)', 'fpg:gamma()'],
    params: [
      ['fpg', fpgHandle],
    ],
    returns: 'One binary Lua string of 576 original gamma bytes.' + fpgLookupError,
    ...fpgExample,
    related: ['parse', 'fpg_id_at'],
  },
  'fpg/fpg_id_at': {
    description: 'Read the sprite ID at a position in file order.',
    signatures: ['mv.fpg_id_at(fpg, index)', 'fpg_id_at(fpg, index)', 'fpg:id_at(index)'],
    params: [
      ['fpg', fpgHandle],
      ['index', fpgIndex],
    ],
    returns: 'One integer sprite ID, or nil if the index is absent or less than 1.' + fpgLookupError,
    ...fpgExample,
    notes: 'An enumeration position is distinct from the stored sprite ID.',
    related: ['fpg_count', 'fpg_sprite'],
  },
  'fpg/fpg_sprite': {
    description: 'Read one sprite’s metadata and indexed pixels.',
    signatures: ['mv.fpg_sprite(fpg, id)', 'fpg_sprite(fpg, id)', 'fpg:sprite(id)'],
    params: [
      ['fpg', fpgHandle],
      ['id', fpgId],
    ],
    returns: 'One independent table with integer id, width and height in source pixels, and point_count, plus binary name_raw (32 bytes) and filename_raw (12 bytes). The pixels field contains one palette index byte per pixel in row order. A missing ID returns nil.' + fpgLookupError,
    ...fpgExample,
    notes: 'The legacy name fields contain raw bytes.',
    related: ['fpg_point', 'fpg_id_at'],
  },
  'fpg/fpg_point': {
    description: 'Read a sprite control point. Point 0 is its historical pivot.',
    signatures: ['mv.fpg_point(fpg, id, index)', 'fpg_point(fpg, id, index)', 'fpg:point(id, index)'],
    params: [
      ['fpg', fpgHandle],
      ['id', fpgId],
      ['index', pointIndex],
    ],
    returns: 'One table with integer x and y in sprite pixels and boolean defined. Undefined points retain coordinates (−1, −1). An absent ID or point returns nil.' + fpgLookupError,
    ...fpgExample,
    related: ['fpg_sprite'],
  },
  'fpg/manifest_count': {
    description: 'Count the sprites in the library. Use it to enumerate entries.',
    signatures: ['mv.manifest_count(manifest)', 'manifest_count(manifest)', 'manifest:count()'],
    params: [
      ['manifest', manifestHandle],
    ],
    returns: 'One integer sprite count.' + fpgLookupError,
    ...fpgExample,
    related: ['parse_manifest', 'manifest_id_at'],
  },
  'fpg/manifest_palette': {
    description: 'Read the original palette. Use it when inspecting indexed colors.',
    signatures: ['mv.manifest_palette(manifest)', 'manifest_palette(manifest)', 'manifest:palette()'],
    params: [
      ['manifest', manifestHandle],
    ],
    returns: 'One binary Lua string of 768 RGB bytes in palette order. Each channel ranges from 0 to 63.' + fpgLookupError,
    ...fpgExample,
    related: ['parse_manifest', 'manifest_id_at'],
  },
  'fpg/manifest_gamma': {
    description: 'Read the original gamma table. Use it to preserve the metadata during conversion.',
    signatures: ['mv.manifest_gamma(manifest)', 'manifest_gamma(manifest)', 'manifest:gamma()'],
    params: [
      ['manifest', manifestHandle],
    ],
    returns: 'One binary Lua string of 576 original gamma bytes.' + fpgLookupError,
    ...fpgExample,
    related: ['parse_manifest', 'manifest_id_at'],
  },
  'fpg/manifest_id_at': {
    description: 'Read the sprite ID at a position in file order.',
    signatures: ['mv.manifest_id_at(manifest, index)', 'manifest_id_at(manifest, index)', 'manifest:id_at(index)'],
    params: [
      ['manifest', manifestHandle],
      ['index', fpgIndex],
    ],
    returns: 'One integer sprite ID, or nil if the index is absent or less than 1.' + fpgLookupError,
    ...fpgExample,
    notes: 'An enumeration position is distinct from the stored sprite ID.',
    related: ['manifest_count', 'manifest_sprite'],
  },
  'fpg/manifest_sprite': {
    description: 'Read one converted sprite’s metadata and PNG filename.',
    signatures: ['mv.manifest_sprite(manifest, id)', 'manifest_sprite(manifest, id)', 'manifest:sprite(id)'],
    params: [
      ['manifest', manifestHandle],
      ['id', fpgId],
    ],
    returns: 'One independent table with integer id, width and height in source pixels, and point_count, plus binary name_raw (32 bytes) and filename_raw (12 bytes). The image field is the relative PNG filename, such as 7.png. A missing ID returns nil.' + fpgLookupError,
    ...fpgExample,
    notes: 'The legacy name fields contain raw bytes.',
    related: ['manifest_point', 'manifest_id_at'],
  },
  'fpg/manifest_point': {
    description: 'Read a sprite control point. Point 0 is its historical pivot.',
    signatures: ['mv.manifest_point(manifest, id, index)', 'manifest_point(manifest, id, index)', 'manifest:point(id, index)'],
    params: [
      ['manifest', manifestHandle],
      ['id', fpgId],
      ['index', pointIndex],
    ],
    returns: 'One table with integer x and y in sprite pixels and boolean defined. Undefined points retain coordinates (−1, −1). An absent ID or point returns nil.' + fpgLookupError,
    ...fpgExample,
    related: ['manifest_sprite'],
  },
  'fpg/manifest_transparent_zero': {
    description: 'Read whether the manifest marks palette index 0 as transparent.',
    signatures: ['mv.manifest_transparent_zero(manifest)', 'manifest_transparent_zero(manifest)', 'manifest:transparent_zero()'],
    params: [
      ['manifest', manifestHandle],
    ],
    returns: 'One boolean. True means the export uses transparent index 0.' + fpgLookupError,
    ...fpgExample,
    related: ['fpg_json', 'parse_manifest'],
  },
  'fpg/fpg_rgba': {
    description: 'Convert an indexed sprite to RGBA bytes. Use the result when another tool needs decoded pixels.',
    signatures: ['mv.fpg_rgba(fpg, id, options?)', 'fpg_rgba(fpg, id, options?)', 'fpg:rgba(id, options?)'],
    params: [
      ['fpg', fpgHandle],
      ['id', fpgId],
      ...conversionOptions,
    ],
    returns: 'One binary Lua string with four byte channels per pixel in row order, or nil for an absent ID. Invalid arguments or a conversion failure raise a Lua error.',
    ...fpgExample,
    notes: 'Six-bit channels are shifted left two bits. Alpha is 255 except for index 0 when transparent_zero is true. Gamma is preserved separately.',
    related: ['fpg_image', 'parse_manifest'],
  },
  'fpg/fpg_png': {
    description: 'Encode a sprite as an indexed PNG. Use it to export a sprite without opening a window.',
    signatures: ['mv.fpg_png(fpg, id, options?)', 'fpg_png(fpg, id, options?)', 'fpg:png(id, options?)'],
    params: [
      ['fpg', fpgHandle],
      ['id', fpgId],
      ...conversionOptions,
    ],
    returns: 'One complete indexed PNG as a binary Lua string, or nil for an absent ID. Invalid arguments or a conversion failure raise a Lua error.',
    ...fpgExample,
    notes: 'Pixel indexes are preserved. Palette channels are shifted left two bits. Transparency is optional. This produces bytes without creating files.',
    related: ['fpg_image', 'parse_manifest'],
  },
  'fpg/fpg_json': {
    description: 'Encode a version 1 manifest for the library. Pair it with the indexed PNGs when exporting sprites.',
    signatures: ['mv.fpg_json(fpg, options?)', 'fpg_json(fpg, options?)', 'fpg:json(options?)'],
    params: [
      ['fpg', fpgHandle],
      ...conversionOptions,
    ],
    returns: 'One JSON Lua string without a trailing zero byte. Invalid arguments or a conversion failure raise a Lua error.',
    ...fpgExample,
    notes: 'Use the same transparency option for each PNG and the manifest. Save each PNG as the sprite ID followed by .png. This call produces bytes without creating files.',
    related: ['fpg_image', 'parse_manifest'],
  },
  'fpg/fpg_image': {
    description: 'Upload one indexed sprite as a drawable image.',
    signatures: ['mv.fpg_image(fpg, id, options?)', 'fpg_image(fpg, id, options?)', 'fpg:image(id, options?)'],
    params: [
      ['fpg', fpgHandle],
      ['id', fpgId],
      ...conversionOptions,
    ],
    returns: 'One independent managed image, or nil for an absent ID. A valid ID requires an open window. Invalid arguments or a conversion failure raise a Lua error. Window or upload failures also raise an error.',
    ...fpgImageExample,
    notes: 'The image keeps its pixels after the FPG is collected. Use fpg_point and a draw_image pivot if you need the historical pivot.',
    related: ['fpg_sprite', 'fpg_point'],
  },
};
