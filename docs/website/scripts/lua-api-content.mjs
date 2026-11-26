import processesInput from './lua-reference/processes-input.mjs';
import graphicsImages from './lua-reference/graphics-images.mjs';
import textAudio from './lua-reference/text-audio.mjs';
import spatial from './lua-reference/spatial.mjs';
import fpg from './lua-reference/fpg.mjs';
import platformModule from './lua-reference/platform-module.mjs';

// Entry keys use the stable <family>/<function> routes. See the
// function reference in docs/development/documentation-templates.md.
export default {
  ...processesInput,
  ...graphicsImages,
  ...textAudio,
  ...spatial,
  ...fpg,
  ...platformModule,
};
