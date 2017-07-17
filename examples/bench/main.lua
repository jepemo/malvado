package.path = package.path .. ";./?/init.lua"
require 'malvado'

-- boilerplate
function love.update(dt) malvado.update(dt) end
