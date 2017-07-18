package.path = package.path .. ";./?/init.lua"
require ('malvado')

Cat = process(function(self)
  self.fpg = load_fpg("assets/cat.zip")
  self.fpgIndex = 0
  self.x = get_screen_width() / 2
  self.y = get_screen_height() / 2
  self.size = 1.5

  local num_frames = 6

  while not key("escape") do
    self.fpgIndex = self.fpgIndex+1
    if self.fpgIndex > num_frames then
      self.fpgIndex = 1
    end

    frame()
  end
end)

Bird = process(function(self)
  self.fpg = load_fpg_image("assets/bird.png", 3, 5)
  self.fpgIndex = 0
  self.x = get_screen_width() / 2
  self.y = get_screen_height() / 2
  self.size = 1.5

  clear_screen(255, 255, 255)

  local num_frames = 14

  while not key("escape") do
    self.fpgIndex = self.fpgIndex+1
    if self.fpgIndex > num_frames then
      self.fpgIndex = 1
    end

    frame()
  end
end)

-- Start
malvado.start(function()
  set_title('Sprite example')
  Bird {}
  --Cat {}
end)
