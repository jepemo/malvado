-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local images = { cats = {} }
local names = { "Sprites", "Keyboard", "Texts", "Fade" }
local selected, page = 1, 0
local ticks, text = 0, ""
local previous_mouse = false

local function show_page(number)
  page = number
end

local function draw_menu()
  draw_text {text = "Malvado demo", x = 280, y = 65, size = 36}
  draw_text {text = "Choose with arrows and Enter, or click an option", x = 130, y = 120, size = 20}
  for i, name in ipairs(names) do
    local marker = i == selected and "> " or "  "
    draw_text {text = marker .. name, x = 300, y = 150 + i * 70, size = 32}
  end
end

local function draw_page()
  draw_text {text = names[page] .. " - Escape: menu", x = 20, y = 20, size = 24}
  if page == 1 then
    local cat = images.cats[(math.floor(ticks / 4) % 6) + 1]
    draw_image(cat, 100 + (ticks * 2 % 550), 310)
    local index = 7 + (math.floor(ticks / 4) % 7)
    local sx, sy = (index % 8) * 80, math.floor(index / 8) * 94
    draw_image_region(images.hero, 300, 150, sx, sy, 80, 94, 0, 0, 2, 0)
  elseif page == 2 then
    local character = text_input_next()
    while character do
      text = text .. character
      character = text_input_next()
    end
    draw_text {text = "Type text:", x = 100, y = 200, size = 30}
    draw_text {text = text, x = 100, y = 250, size = 30}
  elseif page == 3 then
    draw_text {text = "This is animated text", x = 800 - (ticks * 3 % 1100), y = 300, size = 40}
  elseif page == 4 then
    draw_text {text = "Press Space to fade the screen", x = 100, y = 220, size = 28}
    draw_text {text = fade_active() and "Fade in progress" or "Ready", x = 100, y = 280, size = 24}
  end
end

local Demo = process(function()
  while true do
    ticks = ticks + 1
    local escape = key_pressed("escape")
    if page == 0 then
      if escape then break end
      if key_pressed("up") then selected = (selected + 2) % 4 + 1 end
      if key_pressed("down") then selected = selected % 4 + 1 end
      if key_pressed("enter") then show_page(selected) end
      local mouse = mouse_button("left")
      if mouse and not previous_mouse then
        local x, y = mouse_position()
        if x >= 280 and x < 550 then
          for i = 1, #names do
            local top = 150 + i * 70
            if y >= top and y < top + 40 then selected = i; show_page(i) end
          end
        end
      end
      previous_mouse = mouse
    elseif escape then
      page = 0
    elseif page == 4 and key_pressed("space") and not fade_active() then
      fade_off()
      fade_on()
    end

    if page == 1 then
      clear_screen {100, 60, 100, 255}
    else
      clear_screen(BLACK)
    end
    if page == 0 then draw_menu() else draw_page() end
    frame()
  end
end)

start(function()
  for i = 1, 6 do images.cats[i] = load_image("examples/demo/assets/cat/c" .. i .. ".png") end
  images.hero = load_image("examples/demo/assets/hero_spritesheet.png")
  clear_screen(BLACK)
  Demo()
end, { width = 800, height = 600, title = "Malvado Demo" })
