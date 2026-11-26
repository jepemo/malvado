-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local step = assert(os.getenv("MALVADO_DODGE_CHECKPOINT"))
local define = mv.process
local constructors = {}
local draw_text = mv.draw_text
local last_text, final_time
local escape, restart_tick
local weak_results = setmetatable({}, {__mode = "v"})
local rounds, shapes = 0, 0

-- Drive keyboard input and observe the tutorial. Scheduling, colliders, graphs,
-- text commands and process cleanup still use the engine.
mv.process = function(fn, options)
    local index = #constructors + 1
    if options and options.defaults and options.defaults.graph then
        shapes = shapes + 1
    end
    local constructor = define(function(self)
        if step == "main" and index == 3 then
            rounds = rounds + 1
            weak_results[rounds] = self.result
        end
        fn(self)
    end, options)
    constructors[index] = constructor
    return constructor
end
mv.key = function(key)
    if key == mv.KEY_ESCAPE then return escape end
    if step ~= "03-movement" then return false end
    local tick = mv.snapshot().tick
    return (key == mv.KEY_LEFT and tick <= 75)
        or (key == mv.KEY_RIGHT and tick > 75 and tick <= 205)
end
mv.key_pressed = function(key)
    return key == mv.KEY_SPACE and mv.snapshot().tick == restart_tick
end
mv.draw_text = function(options)
    last_text = options.text
    final_time = tonumber(options.text:match("^Game over! Time: ([%d.]+) s$")) or final_time
    return draw_text(options)
end

local game = dofile("main.lua")
local function wait_until(condition)
    local deadline = mv.snapshot().tick + 300
    while not condition() do
        assert(mv.snapshot().tick < deadline, "timed out in checkpoint " .. step)
        mv.frame()
    end
end

local function find(index)
    return mv.find(constructors[index])
end

local function wait_for_rounds()
    local previous_time
    for attempt = 1, 3 do
        wait_until(function() return rounds == attempt end)
        local round = assert(find(3)[1])
        local player = assert(find(1)[1])
        assert(player.x == 320 and player.parent == round)
        wait_until(function() return #find(2) >= 2 end)
        local enemies = find(2)
        for _, enemy in ipairs(enemies) do
            assert(enemy.parent == round)
        end
        wait_until(function() return last_text == "Press Space to try again" end)
        assert(final_time > 2 and final_time < 3)
        assert(not previous_time or final_time == previous_time)
        previous_time = final_time
        assert(round.status == mv.STATUS_DEAD and round.reason == "returned")
        assert(player.status == mv.STATUS_DEAD)
        for _, enemy in ipairs(enemies) do
            assert(enemy.status == mv.STATUS_DEAD)
        end
        assert(#mv.snapshot().processes == 2) -- Game and this observer.
        for _ = 1, 3 do mv.frame() end
        assert(rounds == attempt and #mv.snapshot().processes == 2)
        if attempt < 3 then
            last_text = nil
            restart_tick = mv.snapshot().tick + 1
        end
    end
    collectgarbage("collect")
    assert(weak_results[1] == nil and weak_results[2] == nil)
    assert(shapes == 2)
end

local Observe = define(function()
    for _ = 1, 3 do mv.frame() end
    if step == "01-start" then
        assert(last_text == "Dodge!")
    elseif step == "02-player" then
        local player = assert(find(1)[1])
        assert(player.x == 320 and player.y == 320 and player.graph)
    elseif step == "03-movement" then
        local player = assert(find(1)[1])
        while mv.snapshot().tick < 76 do mv.frame() end
        assert(player.x == 32)
        while mv.snapshot().tick < 215 do mv.frame() end
        assert(player.x == 608)
    elseif step == "04-enemy" or step == "05-spawn" then
        local enemy = assert(find(2)[1])
        local y = enemy.y
        if step == "05-spawn" then
            wait_until(function() return #find(2) >= 3 end)
        else
            mv.frame()
        end
        assert(enemy.y > y and enemy.graph)
        wait_until(function() return enemy.status == mv.STATUS_DEAD end)
        assert(enemy.reason == "returned" and #find(1) == 1)
    elseif step == "06-collision" or step == "07-score" then
        local player, round = assert(find(1)[1]), assert(find(3)[1])
        wait_until(function() return round.status == mv.STATUS_DEAD end)
        assert(round.reason == "returned" and player.status == mv.STATUS_DEAD and #find(2) == 0)
        if step == "07-score" then
            assert(tonumber(last_text:match("^Time: ([%d.]+) s$")) > 2)
        end
    else
        assert(step == "main")
        wait_for_rounds()
    end
    escape = true
    wait_until(function() return #mv.snapshot().processes == 1 end)
    print("PASS: Dodge! " .. step)
end)

mv.start(function()
    mv.set_priority(Observe(), 1)
    game.init()
end, {title = game.title, width = game.width, height = game.height})
