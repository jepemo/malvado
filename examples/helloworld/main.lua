package.path = package.path .. ";./?/init.lua"
require 'malvado'

-- Define global font (size, color (r, g, b), alfa)
local font = font(60, 0, 0, 0, 255)

-- Define (not run) one process
HelloWorld = process(function(self)
  -- Runs until escape key is pressed
  while not key("escape") do
    -- Set background to grey
    love.graphics.setBackgroundColor(231, 231, 231)

    -- Write the "Hello world" text in the process position
    write(font, self.x, self.y, self.text)

    -- Render the process
    frame()
  end
end)

-- Main init
function love.load()
  -- Set the title text of the window
  set_title("Hello world")

  -- Launch the background process
  -- The application runs until there is no running process.
  HelloWorld { x=240, y=280, text="Hello World" }
end

-- boilerplate
function love.draw() malvado.draw() end
