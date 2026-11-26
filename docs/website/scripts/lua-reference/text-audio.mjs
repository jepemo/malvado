import { luaColor } from './common.mjs';

const fields = (prefix, definitions) =>
  definitions.map(([name, text]) => [`${prefix}.${name}`, text]);

const textExample = {
  exampleId: 'reference-text',
  exampleSetup: 'With the Malvado SDK and a desktop display, extract the ZIP to an empty directory. Keep the bundled font beside the script and run `malvado run text.lua` there. The ZIP includes the SIL Open Font License for Anonymous Pro Bold.',
  exampleResult: 'The centered score changes from 0 to 10 and switches to the bundled typeface. The terminal prints the label dimensions. The program deletes the label and exits after about 2½ seconds.',
};

const audioExample = {
  exampleId: 'reference-audio',
  exampleSetup: 'With the Malvado SDK, a desktop display and an audio device, extract the ZIP into an empty directory. Keep `tone.wav` beside the script and run `malvado run audio.lua` there.',
  exampleResult: 'A short tone plays as a sound and then loops as music. The terminal reports that each playback stopped. The window closes on its own.',
};

const fontSize = 'Whole Lua number in pixels, from 1 to 1024.';

const textContents = 'Lua string without zero bytes.';

const textPosition = 'Whole Lua number in the C int range, in logical window pixels.';

const textAlign = 'Use `TEXT_ALIGN_LEFT` ("left"), `TEXT_ALIGN_CENTER` ("center") or `TEXT_ALIGN_RIGHT` ("right").';

const fontDescriptor = 'Font descriptor from `font` or `load_font`.';

const textId = 'Required whole Lua number identifying a live entry from `write`.';

const audioPath = 'Required nonempty Lua path string without zero bytes. A relative path starts at the game asset directory.';

const voice = 'Required voice handle from `play_sound`.';

const music = 'Required music handle from `load_music`.';

const volume = 'Required finite Lua number from 0 (silent) to 1 (full volume).';

const playbackReturn = 'No values. An invalid handle, extra arguments or a backend failure raises a Lua error.';

const voiceMutationReturn = playbackReturn + ' A stopped or completed voice raises an error.';

export default {
  'text/font': {
    description: 'Create a descriptor for the built-in font. Use this when a game needs text without a font file.',
    signatures: ['mv.font(options)', 'font(options)'],
    params: [
      ['options', 'Required table.'],
      ['options.size', 'Required. ' + fontSize],
    ],
    returns: 'One descriptor table with a size field. Missing or invalid size, unknown fields or a wrong argument count raise a Lua error.',
    ...textExample,
    related: ['write', 'load_font', 'text_size'],
  },
  'text/load_font': {
    description: 'Load a TrueType or OpenType font. Use the same descriptor for multiple text entries.',
    signatures: ['mv.load_font(options)', 'load_font(options)'],
    params: [
      ['options', 'Required table.'],
      ['options.path', 'Required nonempty TTF or OTF path without zero bytes. Relative paths start at the game asset directory.'],
      ['options.size', 'Required. ' + fontSize],
      ['options.extra_glyphs', 'Optional UTF-8 string of extra characters. Defaults to no extras. Zero bytes and malformed UTF-8 raise an error.'],
    ],
    returns: 'One descriptor table with the default size and a managed font resource. Invalid options, an unavailable window or a font load failure raises a Lua error.',
    ...textExample,
    notes: 'The loader includes printable ASCII and Latin-1 by default. It allows up to 1,024 distinct code points including defaults. The font file must contain the requested glyphs. The loader requires an open window.',
    related: ['font', 'write', 'draw_text'],
  },
  'text/write': {
    description: 'Create a persistent label. Use it for a score or status display that changes less often than the process runs.',
    signatures: ['mv.write(options)', 'write(options)'],
    params: [
      ['options', 'Required table.'],
      ['options.font', 'Required. ' + fontDescriptor],
      ['options.text', 'Required. ' + textContents],
      ['options.x', 'Required horizontal alignment anchor. ' + textPosition],
      ['options.y', 'Required top edge. ' + textPosition],
      ['options.align', 'Optional. Defaults to left. ' + textAlign],
      ['options.color', `Optional ${luaColor}. Defaults to opaque white.`],
      ['options.size', 'Optional. Defaults to the font descriptor size. ' + fontSize],
    ],
    returns: 'One integer text ID. Invalid input or allocation failure raises a Lua error without creating an entry.',
    ...textExample,
    notes: 'Coordinates are in screen space and ignore the process camera. A process-owned label is removed when the owner ends, including on kill. A label created outside a process stays until deletion or shutdown. The entry copies its content and color and retains the font.',
    related: ['change_text', 'delete_text', 'text_size'],
  },
  'text/change_text': {
    description: 'Update a persistent label by ID. Pass only the fields you want to change.',
    signatures: ['mv.change_text(id, options)', 'change_text(id, options)'],
    params: [
      ['id', textId],
      ['options', 'Required table. Missing fields keep their current values.'],
      ['options.text', 'Optional replacement. ' + textContents],
      ['options.x', 'Optional horizontal anchor. ' + textPosition],
      ['options.y', 'Optional top edge. ' + textPosition],
      ['options.align', 'Optional alignment. ' + textAlign],
      ['options.font', 'Optional typeface. ' + fontDescriptor],
      ['options.size', 'Optional replacement size. ' + fontSize],
      ['options.color', `Optional replacement ${luaColor}.`],
    ],
    returns: 'No values. An unknown ID, invalid field or invalid argument count raises a Lua error and preserves the current entry.',
    ...textExample,
    notes: 'Updating the font leaves the current size in effect unless you also supply size. The entry keeps its original owner.',
    related: ['write', 'delete_text', 'text_size'],
  },
  'text/delete_text': {
    description: 'Remove a persistent label immediately. Use it when a message or HUD element should disappear.',
    signatures: ['mv.delete_text(id)', 'delete_text(id)'],
    params: [
      ['id', textId],
    ],
    returns: 'No values. An invalid or already removed ID or extra argument raises a Lua error.',
    ...textExample,
    notes: 'Deletion releases the text and its retained font reference. The process itself continues running.',
    related: ['write', 'change_text'],
  },
  'text/text_size': {
    description: 'Measure text in pixels. Use the result when laying out a panel or checking whether a label fits.',
    signatures: ['mv.text_size(id)', 'text_size(id)', 'mv.text_size(font, text)', 'text_size(font, text)'],
    params: [
      ['id', textId],
      ['font', 'Required in the two-argument form. ' + fontDescriptor],
      ['text', 'Required in the two-argument form. ' + textContents],
    ],
    returns: 'Two Lua integers, width then height, in pixels. Invalid arguments, a deleted ID or a bad style raise a Lua error.',
    ...textExample,
    notes: 'The ID overload uses the label’s effective font and size. Empty text or headless measurement gives (0, 0). For multiple lines, width is the widest line and height includes each line.',
    related: ['write', 'change_text', 'font'],
  },

  'audio/load_sound': {
    description: 'Load a reusable WAV sample. Load it once when you need to play the same effect many times.',
    signatures: ['mv.load_sound(path)', 'load_sound(path)'],
    params: [
      ['path', audioPath],
    ],
    returns: 'One managed sound handle. An invalid path, a missing or invalid WAV, an unavailable device or headless mode raises a Lua error.',
    ...audioExample,
    notes: 'The function opens the audio device if needed. Each voice shares the sample while retaining its own playback state.',
    related: ['play_sound'],
  },
  'audio/play_sound': {
    description: 'Start one independent voice for a sound. Multiple voices of the same sound can overlap.',
    signatures: ['mv.play_sound(sound, options?)', 'play_sound(sound, options?)', 'sound:play(options?)'],
    params: [
      ['sound', 'Required sample handle from load_sound.'],
      ['options', 'Optional table. Omission or nil uses the default volume.'],
      ['options.volume', 'Optional finite Lua number from 0 to 1. Defaults to 1.'],
    ],
    returns: 'One managed voice handle, or nil if all 32 voice slots are occupied by playing or paused voices. Invalid arguments, unknown options or failed playback raise a Lua error.',
    ...audioExample,
    notes: 'The runner retains a voice and its sample until playback stops or finishes, including when the creating process ends.',
    related: ['stop_sound', 'pause_sound', 'sound_playing'],
  },
  'audio/stop_sound': {
    description: 'Stop one voice and release its playback resources.',
    signatures: ['mv.stop_sound(voice)', 'stop_sound(voice)', 'voice:stop()'],
    params: [
      ['voice', voice],
    ],
    returns: playbackReturn,
    ...audioExample,
    notes: 'A stopped or completed voice can be stopped again safely. Stopping frees its slot.',
    related: ['play_sound', 'pause_sound'],
  },
  'audio/pause_sound': {
    description: 'Pause one voice at the current playback position.',
    signatures: ['mv.pause_sound(voice)', 'pause_sound(voice)', 'voice:pause()'],
    params: [
      ['voice', voice],
    ],
    returns: voiceMutationReturn,
    ...audioExample,
    notes: 'A paused voice keeps its slot.',
    related: ['resume_sound', 'sound_playing'],
  },
  'audio/resume_sound': {
    description: 'Resume one paused voice from the same position.',
    signatures: ['mv.resume_sound(voice)', 'resume_sound(voice)', 'voice:resume()'],
    params: [
      ['voice', voice],
    ],
    returns: voiceMutationReturn,
    ...audioExample,
    related: ['pause_sound', 'stop_sound'],
  },
  'audio/sound_playing': {
    description: 'Check whether a voice is currently playing.',
    signatures: ['mv.sound_playing(voice)', 'sound_playing(voice)', 'voice:is_playing()'],
    params: [
      ['voice', voice],
    ],
    returns: 'One boolean. Paused, stopped and completed voices return false. Invalid arguments raise a Lua error.',
    ...audioExample,
    notes: 'Completion releases the native playback state. The retained handle remains safe to query.',
    related: ['play_sound', 'stop_sound'],
  },
  'audio/sound_volume': {
    description: 'Set the volume of one live voice. Use it to change an effect that is already playing.',
    signatures: ['mv.sound_volume(voice, volume)', 'sound_volume(voice, volume)', 'voice:set_volume(volume)'],
    params: [
      ['voice', voice],
      ['volume', volume],
    ],
    returns: voiceMutationReturn,
    ...audioExample,
    related: ['play_sound', 'pause_sound'],
  },
  'audio/load_music': {
    description: 'Open a WAV or OGG stream for longer audio or repeated music.',
    signatures: ['mv.load_music(path, options?)', 'load_music(path, options?)'],
    params: [
      ['path', audioPath],
      ['options', 'Optional table. Omission or nil uses one play.'],
      ['options.loop', 'Optional boolean. Defaults to false. True loops the stream.'],
    ],
    returns: 'One managed music handle. Invalid arguments, unknown options, a load failure, unavailable audio or headless mode raise a Lua error.',
    ...audioExample,
    notes: 'This function opens the audio device when needed. Playback starts when you call play_music.',
    related: ['play_music', 'update_music'],
  },
  'audio/play_music': {
    description: 'Start playback of the given music stream.',
    signatures: ['mv.play_music(music)', 'play_music(music)', 'music:play()'],
    params: [
      ['music', music],
    ],
    returns: playbackReturn,
    ...audioExample,
    notes: 'The runner keeps active and paused streams alive until stop, completion or shutdown.',
    related: ['stop_music', 'music_playing'],
  },
  'audio/update_music': {
    description: 'Refill a music stream once. Call it explicitly if you drive audio outside the game loop.',
    signatures: ['mv.update_music(music)', 'update_music(music)', 'music:update()'],
    params: [
      ['music', music],
    ],
    returns: playbackReturn,
    ...audioExample,
    notes: 'The running game already updates active streams, including when process execution is paused.',
    related: ['play_music'],
  },
  'audio/stop_music': {
    description: 'Stop a music stream and release its playback reference.',
    signatures: ['mv.stop_music(music)', 'stop_music(music)', 'music:stop()'],
    params: [
      ['music', music],
    ],
    returns: playbackReturn,
    ...audioExample,
    notes: 'The Lua handle can be used to play the stream again.',
    related: ['play_music', 'pause_music'],
  },
  'audio/pause_music': {
    description: 'Pause music while keeping the current playback position.',
    signatures: ['mv.pause_music(music)', 'pause_music(music)', 'music:pause()'],
    params: [
      ['music', music],
    ],
    returns: playbackReturn,
    ...audioExample,
    related: ['resume_music', 'stop_music'],
  },
  'audio/resume_music': {
    description: 'Resume paused music from the current playback position.',
    signatures: ['mv.resume_music(music)', 'resume_music(music)', 'music:resume()'],
    params: [
      ['music', music],
    ],
    returns: playbackReturn,
    ...audioExample,
    related: ['pause_music', 'stop_music'],
  },
  'audio/music_playing': {
    description: 'Check whether a music stream is currently playing.',
    signatures: ['mv.music_playing(music)', 'music_playing(music)', 'music:is_playing()'],
    params: [
      ['music', music],
    ],
    returns: 'One boolean. A paused, stopped or completed stream returns false. Invalid arguments raise a Lua error.',
    ...audioExample,
    related: ['play_music'],
  },
  'audio/music_volume': {
    description: 'Set the volume of one music stream. Use it to set a background track’s level.',
    signatures: ['mv.music_volume(music, volume)', 'music_volume(music, volume)', 'music:set_volume(volume)'],
    params: [
      ['music', music],
      ['volume', volume],
    ],
    returns: playbackReturn,
    ...audioExample,
    related: ['play_music'],
  },
};
