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
      draw_box(self.x, self.y, self.x + self.width+5, self.y + self.height+5)
      set_color(172, 83, 83)
      draw_box(self.x, self.y, self.x + self.width, self.y + self.height)

      if (self:collision('mouse') and mouse.left) then
        selected_menu = self.option

        print(self.text .. ' -> ' .. tostring(selected_menu))
      end
    else
      --print ('entra: ' .. self.text)
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

  FadeExample { z = 0 , parent=self }
  while not key("escape") do
    --print (selected_menu)
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
