---
id: cb_animations
title: Animations
sidebar_label: Animations
---

## Basic Animation

**Problem:**

I want to move a game entity from 0 to the middle of the screen.

**Solution:**

I need to increment the *self.x* coord in every frame.

```lua
local game_end = false

Hero = process(function(self)
  -- I can adjust the velocity setting the frames per second of the process.
  self.fps = 15
  self.x = 0
  self.y = get_screen_height() / 2

  while not game_end do
    if self.x < (get_screen_width() / 2) then
      self.x = self.x + 1
    end

    frame()
  end
end)
```

## Image animation

**Problem:**

I want to use many images to create an animation.

**Solution:**

There are different ways, but the most simple is to use the *fpg* in directory mode. First of all i can create a directory with the images that compounds the animation. And then create a process:

```lua

-- If i have the animation in the 'assets/cat' directory

Hero = process(function(self)
  -- Directory is the default mode
  self.fpg = fpg('assets/cat')
  -- This is the counter. First initialize with the first animation
  self.fpgIndex = 0
  -- Here I can adjunt the velocity of the animation
  self.fps = 15

  while not game_end do
    self.fpgIndex = self.fpgIndex + 1
    frame()
  end
end)

```
