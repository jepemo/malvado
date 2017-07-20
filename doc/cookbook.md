# Cookbook

Basic recipes to create 2D games in malvado game engine.

- [Basic process creation](#basic-process-creation)

## Basic process creation

**Problem**:
I want to create many game entities: game character, live background, etc.

**Solution**
In malvado, a game entity is a process. When a process is alive, it is rendered in every frame.

```lua
require 'malvado'

local exit_game = false

Enemy = process(function()
 -- To implement
end)

Hero = process(function()
  -- To implement
end)

-- The level is also a process
Level = process(function()
  while not exit_game do
    -- Entities instantiation
    Enemy()
    Hero()
  end
end)

-- main
malvado.start(function()
  Level()
end)

```
