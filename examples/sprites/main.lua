package.path = package.path .. ";./?/init.lua"
require ('malvado')

Cat = process(function(self)
  self.fpg = fpg("assets/cat")
  self.fpgIndex = 0
  self.x = get_screen_width() / 2
  self.y = get_screen_height() / 2
  self.size = 1.5
  self.fps = 15

  local num_frames = 6

  while not key("escape") do
    self.fpgIndex = self.fpgIndex+1
    if self.fpgIndex > num_frames then
      self.fpgIndex = 1
    end

    frame()
  end
end)

Guy = process(function(self)
  self.fpg = fpg("assets/hero_spritesheet.png", "image", 5, 8)
  self.x = get_screen_width() / 2
  self.y = get_screen_height() / 2
  self.size = 1.5
  self.fps = 15

  clear_screen(255, 255, 255)

  local ini_run  = 8
  local end_run = 14

  self.fpgIndex = ini_run
  while not key("escape") do
    --local x, y = love.mouse.getPosition()
    --self.x = x
    --self.y = y

    self.fpgIndex = self.fpgIndex+1
    if self.fpgIndex > end_run then
      self.fpgIndex = ini_run
    end

    --love.timer.sleep(0.5)

    frame()
  end
end)

-- Start
malvado.start(function()
  set_title('Sprite example')
  --Guy {}
  Cat {}
end)
