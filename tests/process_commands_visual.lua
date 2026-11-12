-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local Actor = mv.process(function(self)
  while true do
    mv.set_color(0,255,0)
    mv.draw_box(10,10,30,30)
    local image = mv.load_image("tests/fixtures/red.png")
    mv.draw_image(image,10,10)
    mv.draw_fcircle(12,12,1)
    mv.draw_image_region(image,20,10,0,0,2,2)
    mv.draw_text("Native",2,40,10)
    image = nil
    collectgarbage("collect")
    mv.frame()
  end
end)
mv.start(function()
  Actor {x=10,y=10,fps=5,graph=mv.graphics.rect(20,20,{pivot="top-left",color={0,0,255}})}
end, {width=64,height=64,title="Shared native process commands"})
