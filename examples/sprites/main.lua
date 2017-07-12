package.path = package.path .. ";./?/init.lua"
require ('malvado')

Cat = process(function(self)
  self.fpg = load_fpg("cat.zip")
  self.graph = 0
  self.x = get_screen_width() / 2
  self.y = get_screen_height() / 2

  while not key("escape") do
    self.graph = self.graph+1
    print (self.graph)
    if self.graph > 6 then
      self.graph = 1
    end

    frame()
  end
end)


function love.load()
  set_title('Sprite example')
  Cat {}
end

-- boilerplate
function love.draw() malvado.draw() end
