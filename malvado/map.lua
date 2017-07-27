--[[
  ______        _                _
 |  ___ \      | |              | |
 | | _ | | ____| |_   _ ____  _ | | ___
 | || || |/ _  | | | | / _  |/ || |/ _ \
 | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 |_||_||_|\_||_|_| \_/ \_||_|\____|\___/

 malvado - A game programming library with  "DIV Game Studio"-style
            processes for Lua/Love2D.

 Copyright (C) 2017-present Jeremies Pérez Morata

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
--]]

--- Map module implements load graphic utilities
-- @module malvado.map

--- Sets the color of the next primitive to be painted. It's an alias of: love.graphics.setColor.
-- @param size Size
-- @param r Red (default 255)
-- @param g Green (default 255)
-- @param b Blue (default 255)
-- @param a Alpha (default 255)
-- @return Object font
function font(size, r, g, b, a)
  -- Default values
  r = r or 255
  g = g or 255
  b = b or 255
  a = a or 255

  return {
    font = love.graphics.newFont(size),
    r = r,
    g = g,
    b = b,
    a = a
  }
end

--- Load a image to be used in a process
-- @param image_path Path of the image
-- @return image object
function load_image(image_path)
  return {
    type = 'image',
    data = love.graphics.newImage(image_path)
  }
end

local function load_fpg_zip(path)
  return {
    type = 'zip',
    data = nil
  }
end

local function load_fpg_directory(path)
  local files = love.filesystem.getDirectoryItems(path)

  images = {}
  for k, file in ipairs(files) do
    local imgPath = path .. "/" .. file
    debug("Loading: " .. imgPath)
  	table.insert(images, love.graphics.newImage(imgPath))
    --images[file] = love.graphics.newImage(imgPath)
  end

  --print_v(images)

  return {
    type = 'directory',
    data = images
  }
end

local function load_fpg_image(path, num_rows, num_cols)
  local image = love.graphics.newImage(path)
  local image_w, image_h = image:getDimensions()
  local quad_w, quad_h = image_w / num_cols, image_h / num_rows

  local animation_table = {}
  for y = 0, num_rows do
		for x = 0, num_cols do
			animation_table[#animation_table + 1] = love.graphics.newQuad(x * quad_w, y * quad_h, quad_w, quad_h, image_w, image_h)
		end
	end

  animation_table[#animation_table] = nil

  return {
    type = 'image',
    anim_table = animation_table,
    data = image
  }
end

--- Create a image library to use in the processes
-- @param path images library path
-- @param type Library type: image, zip, directory(default)
-- @param num_rows Image num rows (needed for the type image)
-- @param num_cols Image num colums (needed for the type image)
-- @return fpg object
function fpg(path, type, num_rows, num_cols)
  type = type or "directory"

  local fpg_obj = nil

  if type == 'image' then
    fpg_obj = load_fpg_image(path, num_rows, num_cols)
  elseif type == "zip" then
    fpg_obj = load_fpg_zip(path)
  elseif type == "directory" then
    fpg_obj = load_fpg_directory(path)
  end

  return fpg_obj
end

function load_fpg(fpg_path)
  local dir = love.filesystem.getSourceBaseDirectory()
  --local path = dir .. '/' .. fpg_path
  local path = fpg_path
  --local virtual_dir = dir .. '/' .. fpg_path .. ".virtual"
  --local virtual_dir = fpg_path .. ".virtual"
  local virtual_dir = 'content'
  print ('path:' .. path)
  print ('virtual_dir:' .. virtual_dir)
  --print(love.filesystem.getSourceBaseDirectory())
  --print(love.filesystem.getWorkingDirectory( ))
  --print(love.filesystem.getAppdataDirectory( ))
  --print(love.filesystem.getUserDirectory( ))
  print(love.filesystem.getSaveDirectory( ))
  local success = love.filesystem.mount(path, virtual_dir)
  print('success=' .. tostring(success))

  files = {}
  local files_input = love.filesystem.getDirectoryItems(virtual_dir)

  local exists = love.filesystem.exists(virtual_dir .. '/' .. 'c1.png')
  print('exists:' .. tostring(exists))

  local pathfile = '/home/jere/workspaces/src/github.com/jepemo/malvado/examples/sprites/cat.zip'
  local isFile = love.filesystem.exists( 'cat.zip' )
  print('isFile:' .. tostring(isFile))

  print_v(files_input)

  print 'files_start'
  for k, file in ipairs(files_input) do
    print(k .. ". " .. file)
    table.insert(files, love.graphics.newImage(file))
  end
  print 'files_end'
  os.execute("sleep 10")

  love.filesystem.unmount(path)

  print_v(files)

  return {
    type = 'fpg_zip',
    data = files
  }
end

--------------------------------------------------------------------------------
-- FADE
--------------------------------------------------------------------------------

local function inc_value(current_value, maxmin_value, direction, increment)
  if direction == 1 and current_value >= maxmin_value then
    return 0
  end
  if direction == -1 and current_value <= maxmin_value then
    return 0
  end

  if direction == 1 and current_value < maxmin_value then
    return increment
  end

  if direction == -1 and current_value > maxmin_value then
    return -increment
  end
end

FadeProcess = process(function(self)
  local in_progress = false
  local width = get_screen_width()
  local height = get_screen_height()

  local r = 0
  local r_end = 0

  local g = 0
  local g_end = 0

  local b = 0
  local b_end = 0

  local a = 0
  local a_end = 255

  local speed = 1
  self.z = 1000000

  local dir_r = 0
  local dir_g = 0
  local dir_b = 0
  local dir_a = 0

  while true do
    if in_progress == true then
      local inc_r = inc_value(r, r_end, dir_r, speed)
      local inc_g = inc_value(g, g_end, dir_g, speed)
      local inc_b = inc_value(b, b_end, dir_b, speed)
      local inc_a = inc_value(a, a_end, dir_a, speed)

      r = r + inc_r
      g = g + inc_g
      b = b + inc_b
      a = a + inc_a

      if inc_r == 0 and
         inc_g == 0 and
         inc_b == 0 and
         inc_a == 0 then
        in_progress = false
      end
    else
      data = self:recv()

      if data ~= nil then
        r_end = data.r
        g_end = data.g
        b_end = data.b
        a_end = data.a
        speed = data.speed

        if r_end > r then dir_r = 1 else dir_r = -1 end
        if g_end > r then dir_g = 1 else dir_g = -1 end
        if b_end > r then dir_b = 1 else dir_b = -1 end
        if a_end > r then dir_a = 1 else dir_a = -1 end

        in_progress = true
      end
    end

    love.graphics.setColor(r, g, b, a)
    love.graphics.rectangle("fill", 0, 0, width, height)
    frame()
  end
end)

fade_process = FadeProcess { z = 1000000, internal = true }

--- Creates a fade transition to the specified colour
-- @param rc Red
-- @param gc Green
-- @param bc Blue
-- @param ac Alpha
-- @param speedc Speed
function fade(rc, gc, bc, ac, speedc)
  send(fade_process, { r = rc, g = gc, b = bc, a = ac, speed = speedc})
end

--- Exists from the fade off (change alpha to 0)
-- @param speed Speed
function fade_on(speed)
  speed = speed or 4
  fade(0, 0, 0, 0, 16)
end

--- Go to a dark fade
-- @param speed Speed
function fade_off(speed)
  speed = speed or 4
  fade(0,0,0, 255, 16)
end
