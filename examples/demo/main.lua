package.path = package.path .. ";./?/init.lua"
require 'malvado'

local selected_menu = 0

-- Options
local OPTIONS = {
  MENU_MAIN = 0,
  MENU_SPRITES = 1,
  MENU_KEYBOARD = 2,
  MENU_TEXTS = 3,
  MENU_FADE = 9
}

FadeExample = process(function(self)
  local r = get_screen_height() / 5
  local x = get_screen_width() / 2
  local y = get_screen_height() / 2
  local entra = 0

  while selected_menu == OPTIONS.MENU_FADE do
    set_color(255, 128, 0)
    draw_fcircle(x, y, r)

    if key("space") and entra == 0 then
      entra = 1
      fade_off()
      fade_on()
    end

    if not key("space") then entra = 0 end

    frame()
  end
end)

MenuOption = process (function(self)
  self.x = get_screen_width() / 5
  self.width = (get_screen_width() / 5) * 3
  self.z = 10

  write(font(60, 255, 255, 255), self.x, self.y, self.text)

  while selected_menu == 0 do
    --[[
    set_color(172, 83, 83)
    draw_box(self.x, self.y, self.x + self.width, self.y + self.height)
    set_color(161, 70, 70)
    draw_box(self.x, self.y, self.x + self.width+5, self.y + self.height+5)
    ]]--
    love.graphics.setColor(172, 83, 83)
    love.graphics.rectangle("fill", self.x, self.y, self.width, self.height)
    love.graphics.setColor(161, 70, 70)
    love.graphics.rectangle("fill", self.x, self.y, self.width+10, self.height+10)
    frame()
  end
end)

Menu = process(function(self)
  local yini = 5
  local h = (get_screen_height() / tlen(OPTIONS)) - 10

  -- Menu options
  MenuOption { text = "Texts", y = yini, height = h, option = OPTIONS.MENU_TEXTS, parent=self }

  FadeExample { z = 0 , parent=self }
  while not key("escape") do
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
