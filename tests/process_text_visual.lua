-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
mv.import_globals()

local fonts, retained_fonts

local function labels(emit, descriptors)
  local anchors = {20, 180, 340}
  local alignments = {TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER, TEXT_ALIGN_RIGHT}
  for row, descriptor in ipairs(descriptors) do
    for column, alignment in ipairs(alignments) do
      local color = {255, 255, 255, 255}
      emit {
        text = "Wide\ni", x = anchors[column], y = (row - 1) * 70 + 5,
        align = alignment, font = descriptor, size = column == 2 and 22 or nil,
        color = color,
      }
      color[1] = 0
    end
  end
end

local actor = process(function()
  labels(draw_text, fonts)
  -- A nonpositive size still renders the built-in font at its minimum.
  draw_text {text = "small", x = 340, y = 54, size = 0, align = TEXT_ALIGN_RIGHT}
  -- The commands keep their effective style after descriptor changes and GC.
  for _, descriptor in ipairs(fonts) do
    descriptor.size = 1
    descriptor.resource = nil
  end
  fonts = nil
  collectgarbage("collect")
  frame()

  labels(write, retained_fonts)
  write {text = "small", x = 340, y = 54, font = font {size = 1}, align = TEXT_ALIGN_RIGHT}
  for _ = 1, 3 do frame() end
end)

start(function()
  local path = "fixtures/fonts/AnonymousPro-Bold.ttf"
  fonts = {font {size = 20}, load_font {path = path, size = 24}}
  retained_fonts = {font {size = 20}, load_font {path = path, size = 24}}
  actor {fps = 5}
end, {width = 360, height = 140, title = "Process text alignment"})
