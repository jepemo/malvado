package.path = package.path .. ";./?/init.lua"
require 'malvado'

-- Define (not run) one process
HelloWorld = process(function(self)
  -- Set background to grey
  clear_screen(250, 250, 250)

  -- Define global font (size, color (r, g, b))
  local font = font(60, 0, 0, 0)

  -- Write the "Hello world" text in the process position
  write(font, self.x, self.y, self.text)

  -- Runs until escape key is pressed
  while not key("escape") do
    -- Render the process
    frame()
  end
end)

-- Start (main)
malvado.start(function()
  -- Set the title text of the window
  set_title("Hello world")

  -- Launch the background process
  -- The application runs until there is no running process.
  HelloWorld { x=240, y=280, text="Hello World" }
end, true)
