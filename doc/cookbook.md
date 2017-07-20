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

-- A game is also a process
Level = process(function()
  while not exit_game do
    -- Entities instantiation
    Enemy()
    Hero()
  end
end)

-- The game starts here
malvado.start(function()
  Level()
end)

```
### Level transitions

**Problem:**

A want many game levels.

**Solution:**

Every level is a process. In one special process (level manager) the levels are created.

```lua
require 'malvado'

local playing = false
local level = 1
local exit_game = false

Level1 = process(function()
-- To implement
-- In the end: playing = false
end)

Level2 = process(function()
-- To implement
end)

LevelManager = process(function()

  while not exit_game do
    if not playing then
      playing = true
      fade_off()

      if level = 1 then
        Level1()
      elseif level = 2 then
        Level2()
      end

      fade_on()
    end
  end
end)

-- The game starts here
malvado.start(function()
  LevelManager()
end)
```

## Animations

### Basic Animation

TODO

### Image animation

TODO


## Sounds and Music
