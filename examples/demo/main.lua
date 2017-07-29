package.path = package.path .. ";./?/init.lua"
require 'malvado'

local selected_menu = 1

-- Options
local OPTIONS = {
  [1] = { text = "Back" },
  [2] = { text = "Sprites" },
  [3] = { text = "KeyBoard" },
  [4] = { text = "Texts" },
  [5] = { text = "Fade" }
}

SpriteExample1 = process(function(self)
  self.fpg = fpg("assets/cat")
  self.fpgIndex = 0
  self.x = get_screen_width() / 2
  self.y = get_screen_height() / 2
  self.size = 1.5
  self.fps = 15

  local num_frames = 6
  while true do
    if selected_menu == 2 then
      self.fpgIndex = self.fpgIndex+1
      if self.fpgIndex > num_frames then
        self.fpgIndex = 1
      end

      self.x = self.x - 4
      if self.x < 0 then
        self.x = get_screen_width()
      end
    else
      self.fpgIndex = -1
    end

    frame()
  end
end)

SpriteExample2 = process(function(self)
  self.fpg = fpg("assets/hero_spritesheet.png", "image", 5, 8)
  self.size = 2
  self.x = get_screen_width() / 2
  self.y = (get_screen_height() / 2) - (self.height * self.size) - 125
  self.z = 10
  self.fps = 15

  clear_screen(255, 255, 255)

  local ini_run  = 8
  local end_run = 14

  self.fpgIndex = ini_run
  while true do
    if selected_menu == 2 then
      self.fpgIndex = self.fpgIndex+1
      if self.fpgIndex > end_run then
        self.fpgIndex = ini_run
      end

      self.x = self.x + 4
      if self.x > get_screen_width() then
        self.x = 0
      end
    else
      self.fpgIndex = -1
    end

    frame()
  end
end)

KeyBoardExample = process(function(self)

  local hfont = 50

  local font_show = font(hfont, 255, 255, 255, 255)
  local font_hide = font(hfont, 0,   0,   0,   0)

  local x = get_screen_width() / 3
  local y1 = get_screen_height() / 2
  local y2 = get_screen_height() / 2 + hfont
  local text = ""

  local t1 = write(font_show, x, y1, "Press any key")
  local t2 = write(font_show, x, y1, text)

  while true do
    if selected_menu == 3 then
      print(scan_code)
      if scan_code ~= "escape" then
        text = "" .. scan_code
      end

      change_text(t1, "Press any key", x, y1, font_show)
      change_text(t2, text, x, y2, font_show)
    else
      change_text(t1, "", x, y1, font_hide)
      change_text(t2, "", x, y2, font_hide)
    end

    frame()
  end
end)

TextExample = process(function(self)
  self.fps = 15
  local font_show = font(50, 255, 255, 255, 255)
  local font_hide = font(50, 0,   0,   0,   0)

  local x = get_screen_width() / 2
  local y = get_screen_height() / 2
  local text = "This is a animated text"

  local t1 = write(font_show, x, y, text)

  while true do
    if selected_menu == 4 then
      change_text(t1, text, x, y, font_show)
    else
      change_text(t1, text, x, y, font_hide)
    end

    x = x - 10
    if x < 0 then
      x = get_screen_width()
    end

    frame()
  end
end)

FadeExample = process(function(self)
  local r = get_screen_height() / 5
  local x = get_screen_width() / 2
  local y = get_screen_height() / 2
  local entra = 0

  while true do
    if selected_menu == 5 then
      set_color(255, 128, 0)
      draw_fcircle(x, y, r)

      if key("space") and entra == 0 then
        entra = 1
        fade_off()
        fade_on()
      end

      if not key("space") then entra = 0 end
    end

    frame()
  end
end)

MenuOption = process (function(self)
  self.x = get_screen_width() / 5
  self.width = (get_screen_width() / 5) * 3
  self.z = 10

  local fontSize = 60
  local textYPos = fontSize / 3
  local textXPos = fontSize * 1.5

  local font_show = font(fontSize, 255, 255, 255, 255)
  local font_hide = font(fontSize, 0,   0,   0,   0)

  local idtext = write(font_show, self.x+textXPos, self.y+textYPos, self.text)

  while true do
    if (selected_menu == 1 and self.option > 1)
      or (selected_menu > 1 and self.option == 1) then

      change_text(idtext, self.text, self.x+textXPos, self.y+textYPos, font_show)

      set_color(161, 70, 70)
      draw_box(self.x, self.y, self.x + self.width+3, self.y + self.height+3)
      set_color(172, 83, 83)
      draw_box(self.x, self.y, self.x + self.width, self.y + self.height)

      if (self:collision('mouse') and mouse.left) then
        selected_menu = self.option
        print('[' .. self.text .. '] Change to: ' .. self.option)
      end
    else
      change_text(idtext, self.text, self.x+textXPos, self.y+textYPos, font_hide)
    end

    frame()
  end
end)

Menu = process(function(self)
  local yini = 5
  local h = (get_screen_height() / tlen(OPTIONS)) - 10

  -- Menu options
  for ind, obj in ipairs(OPTIONS) do
    MenuOption { text = obj.text, y = yini+(h * (ind-1))+(10 * (ind-1)), height = h, option = ind, parent=self }
  end

  SpriteExample1 { parent = self }
  SpriteExample2 { parent = self }
  KeyBoardExample { parent = self}
  FadeExample { parent = self }
  TextExample { parent = self}

  while not key("escape") do
    if (selected_menu == 2) then
      clear_screen(164, 117, 160)
    else
      clear_screen(0, 0, 0)
    end

    frame()
  end
end)

-- Start
malvado.start(function()
  set_title('Malvad0 Dem0')

  mouse.graph = load_image("cursor.png")
  mouse.angle = -45

  Menu {}
end, true)
