-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local images = { cats = {} }
local names = { "Sprites", "Keyboard", "Texts", "Fade" }
local selected, page = 1, 0
local ticks, text = 0, ""
local previous = {}

local function pressed(key)
  local down = mv.key(key)
  local result = down and not previous[key]
  previous[key] = down
  return result
end

local function show_page(number)
  page = number
end

local function draw_menu()
  mv.draw_text("Malvado demo", 280, 65, 36)
  mv.draw_text("Choose with arrows and Enter, or click an option", 130, 120, 20)
  for i, name in ipairs(names) do
    local marker = i == selected and "> " or "  "
    mv.draw_text(marker .. name, 300, 150 + i * 70, 32)
  end
end

local function draw_page()
  mv.draw_text(names[page] .. " - Escape: menu", 20, 20, 24)
  if page == 1 then
    local cat = images.cats[(math.floor(ticks / 4) % 6) + 1]
    mv.draw_image(cat, 100 + (ticks * 2 % 550), 310)
    local index = 7 + (math.floor(ticks / 4) % 7)
    local sx, sy = (index % 8) * 80, math.floor(index / 8) * 94
    mv.draw_image_region(images.hero, 300, 150, sx, sy, 80, 94, 0, 0, 2, 0)
  elseif page == 2 then
    local character = mv.text_input_next()
    while character do
      text = text .. character
      character = mv.text_input_next()
    end
    mv.draw_text("Type text:", 100, 200, 30)
    mv.draw_text(text, 100, 250, 30)
  elseif page == 3 then
    mv.draw_text("This is animated text", 800 - (ticks * 3 % 1100), 300, 40)
  elseif page == 4 then
    mv.draw_text("Press Space to fade the screen", 100, 220, 28)
    mv.draw_text(mv.fade_active() and "Fade in progress" or "Ready", 100, 280, 24)
  end
end

local Demo = mv.process(function()
  while true do
    ticks = ticks + 1
    local escape = pressed("escape")
    if page == 0 then
      if escape then break end
      if pressed("up") then selected = (selected + 2) % 4 + 1 end
      if pressed("down") then selected = selected % 4 + 1 end
      if pressed("enter") then show_page(selected) end
      local mouse = mv.mouse_button("left")
      if mouse and not previous.mouse then
        local x, y = mv.mouse_position()
        if x >= 280 and x < 550 then
          for i = 1, #names do
            local top = 150 + i * 70
            if y >= top and y < top + 40 then selected = i; show_page(i) end
          end
        end
      end
      previous.mouse = mouse
    elseif escape then
      page = 0
    elseif page == 4 and pressed("space") and not mv.fade_active() then
      mv.fade_off()
      mv.fade_on()
    end

    if page == 1 then
      mv.clear_screen(100, 60, 100)
    else
      mv.clear_screen(0, 0, 0)
    end
    if page == 0 then draw_menu() else draw_page() end
    mv.frame()
  end
end)

mv.start(function()
  for i = 1, 6 do images.cats[i] = mv.load_image("examples/demo/assets/cat/c" .. i .. ".png") end
  images.hero = mv.load_image("examples/demo/assets/hero_spritesheet.png")
  mv.clear_screen(0, 0, 0)
  Demo()
end, { width = 800, height = 600, title = "Malvado Demo" })
