# Modules and Functions Specification

This document describes modules and their functions in a format ready to implement.
Each module contains a list of its functions, with name, inputs/outputs (if known), and description.

---

## Module: Screen

### Functions
- **set_mode(width: int, height: int, depth?: int, flags?: int)** → bool
  Sets video/window mode.

- **set_title(title: string)** → void
  Sets window/application title.

- **set_icon(icon_path: string)** → void
  Sets window icon.

- **get_window_size()** → (width: int, height: int)
  Gets window size.

- **get_window_pos()** → (x: int, y: int)
  Gets window position.

- **set_window_pos(x: int, y: int)** → void
  Moves window to position.

- **get_desktop_size()** → (width: int, height: int)
  Returns desktop resolution.

- **mode_is_ok(width: int, height: int, depth: int, flags: int)** → bool
  Checks if video mode supported.

- **set_fps(fps: int)** → void
  Sets max frames per second.

- **screen_clear(color?: color)** → void
  Clears screen.

- **screen_get(dest_graph, x:int, y:int, w:int, h:int)** → void
  Copies a rectangle from screen.

- **screen_put(src_graph, x:int, y:int)** → void
  Draws src_graph onto screen.

---

## Module: Sound

### Functions
- **sound_init()** → void
  Initializes audio subsystem.

- **sound_close()** → void
  Shuts down audio subsystem.

- **reserve_channels(n: int)** → void
  Reserves n audio channels.

- **load_wav(path: string)** → handle
  Loads WAV sound effect.

- **play_wav(handle)** → void
  Plays sound effect.

- **pause_wav(handle)** → void
  Pauses WAV sound.

- **resume_wav(handle)** → void
  Resumes WAV sound.

- **stop_wav(handle)** → void
  Stops WAV sound.

- **set_wav_volume(sound_id: int, volume: int)** → void
  Sets volume of WAV.

- **load_song(path: string)** → handle
  Loads music track.

- **play_song(handle)** → void
  Plays music track.

- **pause_song(handle)** → void
  Pauses music track.

- **resume_song(handle)** → void
  Resumes music track.

- **stop_song(handle)** → void
  Stops music track.

- **set_song_volume(volume: int)** → void
  Sets volume for music track.

- **set_music_position(seconds: float)** → void
  Sets playback position.

- **set_panning(channel:int, pan:int)** → void
  Sets stereo panning.

- **set_distance(channel:int, dist:float)** → void
  Sets distance attenuation.

- **set_position(channel:int, x:int, y:int)** → void
  Sets 3D/positioned audio.

- **reverse_stereo()** → void
  Reverses stereo channels.

- **is_playing_wav(handle)** → bool
  Returns if WAV playing.

- **is_playing_song(handle)** → bool
  Returns if song playing.

---

## Module: Key (Keyboard)

### Functions
- **key(scancode:int)** → bool
  Checks if key pressed.

---

## Module: Mouse

### Functions
- (State variables and functions) – position, buttons etc.

---

## Module: Joy (Joystick)

### Functions
- **joy_number()** → int
  Total number of joysticks.

- **joy_select(joy:int)** → void
  Selects joystick.

- **joy_axes(joy:int)** → int
  Number of axes.

- **joy_buttons(joy:int)** → int
  Number of buttons.

- **joy_getaxis(joy:int, axis:int)** → float
  Reads axis value.

- **joy_getbutton(joy:int, button:int)** → bool
  Reads button state.

- **joy_gethat(joy:int, hat:int)** → int
  Reads hat control.

- **joy_name(joy:int)** → string
  Joystick name.

---

## Module: Map (Graphics)

### Functions
- **map_new(width:int,height:int,flags?)** → graph
  Creates new map.

- **map_load(path:string)** → graph
  Loads map/bitmap.

- **map_save(path:string,graph)** → void
  Saves map.

- **map_del(graph)** → void
  Deletes map.

- **map_clone(graph)** → graph
  Clones map.

- **map_clear(graph,color?)** → void
  Clears map to color.

- **map_put(dest_graph,x:int,y:int,src_graph)** → void
  Blit source onto dest.

- **map_xput(...)** → void
  Extended blit.

- **map_xputnp(...)** → void
  Extended blit no palette.

- **map_get_pixel(graph,x:int,y:int)** → color
  Reads pixel.

- **map_put_pixel(graph,x:int,y:int,color)** → void
  Sets pixel.

- **graphic_info(graph,info_type:int)** → int
  Queries info (width,height,...).

- **graphic_set(graph,info_type:int,value:int)** → void
  Sets info.

---

## Module: Draw (Primitives)

### Functions
- **draw_line(x1:int,y1:int,x2:int,y2:int)** → void
  Draws line.

- **draw_rect(x:int,y:int,w:int,h:int)** → void
  Draws rectangle outline.

- **draw_box(x:int,y:int,w:int,h:int)** → void
  Draws filled box.

- **draw_circle(x:int,y:int,r:int)** → void
  Draw circle outline.

- **draw_fcircle(...)** → void
  Draw filled circle.

- **drawing_color(r:int,g:int,b:int)** → void
  Sets drawing color.

- **drawing_alpha(a:int)** → void
  Sets alpha.

- **drawing_map(graph)** → void
  Target map for drawing.

- **drawing_z(z:int)** → void
  Sets drawing depth.

- **drawing_stipple(mask:int)** → void
  Sets stipple pattern.

---

## Module: Text / String

### Functions
- **text_width(text:string,font?)** → int
  Width of text.

- **text_height(text:string,font?)** → int
  Height of text.

- **delete_text(id:int)** → void
  Deletes text object.

- **move_text(id:int,x:int,y:int)** → void
  Moves text object.

- **get_text_color()** → (r,g,b)
  Current text color.

- **set_text_color(r:int,g:int,b:int)** → void
  Set text color.

- **substr(s:string,start:int,len?)** → string
  Substring.

- **split(s:string,sep:string)** → array[string]
  Splits string.

- **join(arr:array[string],sep:string)** → string
  Joins array of strings.

- **trim(s:string)** → string
  Trim spaces.

- **lcase(s:string)** / **ucase(s:string)** → string
  Lower/Uppercase.

- **strrev(s:string)** → string
  Reverse string.

- **len(s:string|array)** → int
  Length.

- **format(fmt:string,...)** → string
  Format string.

---

## Module: Time / Timers

### Functions
- **time()** → int
  Current system time.

- **ftime()** → int
  Time in ms.

- **get_timer()** → int
  Returns timer value.

- **set_fps(fps:int)** → void
  Sets frames per second.

---

## Module: Math

### Functions
- **abs(x)**, **sin(x)**, **cos(x)**, **tan(x)**, **sqrt(x)**, **ln(x)**, **log(x)**, **log2(x)**, **atan2(y,x)**, **pow(a,b)**
  Basic math functions.

---

## Module: Rand

### Functions
- **rand()** → int
  Random integer.

- **rand_seed(seed:int)** → void
  Seed random generator.

---

## Module: Regex

### Functions
- **regex(pattern:string,text:string)** → bool/match
  Match pattern.

- **regex_replace(text,pattern,replacement)** → string
  Replace matches.

---

## Module: Sys / File / Dir

### Functions
- **exec(cmd:string)** → int
  Execute system command.

- **getenv(name:string)** → string
  Get environment variable.

- **exists(path:string)** → bool
  File exists.

- **rm(path:string)** → int
  Remove file.

- **mkdir(path:string)** → int
  Make directory.

- **rmdir(path:string)** → int
  Remove directory.

- **chdir(path:string)** → int
  Change directory.

- **diropen/dirread/dirclose**
  Directory enumeration.

- **fopen/fclose/fread/fwrite/fflush/fseek/ftell/feof**
  File operations.

- **flength(path:string)** → int
  File size.

---

## Module: Proc (Processes)

### Functions
- **signal(pid:int)** → void
  Sends signal.

- **signal_action(pid:int,action:int)** → void
  Sets action for signal.

- **get_id()** → int
  Current process id.

- **let_me_alone()** → void
  Stop other processes.

---

## Module: Blendop (Blending)

### Functions
- **blendop_new()**, **blendop_assign()**, **blendop_apply()**, **blendop_grayscale()**, **blendop_identity()**, **blendop_intensity()**, **blendop_swap()**, **blendop_tint()**, **blendop_translucency()**

---

## Module: Scroll

### Functions
- **start_scroll()** → void
- **stop_scroll()** → void
- **move_scroll(x:int,y:int)** → void

---

## Module: Pathfinding

### Functions
- **path_find(...)** → *path*
- **path_getxy(...)** → coordinates
- **path_wall(...)** → bool

---

## Module: Timers

### Functions
- (see Time)

---

## Module: Miscellaneous Functions

### Functions
- **say(text:string)** → void
  Log text.

- **say_fast(text:string)** → void
  Fast log text.

- **rgb(r,g,b)** → color
- **rgba(r,g,b,a)** → color
- **rgb_get(color)** → (r,g,b)
- **rgba_get(color)** → (r,g,b,a)

---

Notes:
- This file is a starting point for implementing your own runtime.
- Functions without documented signatures are placeholders.
- Grouping by module helps break down implementation tasks.
