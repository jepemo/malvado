package.path = package.path .. ";./?/init.lua"
require ('malvado')

Bird = process(function(self)
  --self.fpg = load_fpg("assets/cat.zip")
  self.fpg = load_fpg_image("assets/bird.png", 3, 5)
  self.fpgIndex = 0
  self.x = get_screen_width() / 2
  self.y = get_screen_height() / 2
  self.size = 2
  self.z = 10

  clear_screen(0, 0, 0)

  while not key("escape") do
    self.fpgIndex = self.fpgIndex+1
    --print (self.fpgIndex)
    if self.fpgIndex > 6 then
      self.fpgIndex = 1
    end

    frame()
  end
end)


function love.load()
  set_title('Sprite example')
  Bird {}
end

-- boilerplate
function love.draw() malvado.draw() end
