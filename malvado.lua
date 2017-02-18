--
--  ______        _                _
-- |  ___ \      | |              | |
-- | | _ | | ____| |_   _ ____  _ | | ___
-- | || || |/ _  | | | | / _  |/ || |/ _ \
-- | || || ( ( | | |\ V ( ( | ( (_| | |_| |
-- |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
--
--

Process = {
  graph = nil,
  x = 0,
  y = 0,
  z = 0,
}

--[[
-- Meta class
Rectangle = {area = 0, length = 0, breadth = 0}

-- Derived class method new

function Rectangle:new (o,length,breadth)
   o = o or {}
   setmetatable(o, self)
   self.__index = self
   self.length = length or 0
   self.breadth = breadth or 0
   self.area = length*breadth;
   return o
end

-- Derived class method printArea

function Rectangle:printArea ()
   print("The area of Rectangle is ",self.area)
end
Creating an Object
Creating an object is the process of allocating memory for the class instance. Each of the objects has its own memory and share the common class data.

r = Rectangle:new(nil,10,20)
--]]

-- Library interface
function process(initialization, body)
  return nil
end
function kill(processToKill)
end
