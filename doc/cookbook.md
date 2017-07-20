# Cookbook

Basic recipes to create 2D games in malvado game engine.

- [Game organization](#game-organization)
  - [Basic entity creation](#basic-entity-creation)
  - [Level transitions](#level-transitions)
- [Animations](#animations)
  - [Basic Animation](#basic-animation)
  - [Image animation](#image-animation)
- [Sounds and Music](#sounds-and-music)


## Game organization
### Basic entity creation

**Problem:**

I want to create many game entities: a game character, live background, etc.

**Solution:**

In malvado, a game entity is a *process*. When a process is alive, it is rendered every frame. But it also can contains only logic.

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
### Level transitions

TODO

## Animations

### Basic Animation

TODO

### Image animation]

TODO


## Sounds and Music
