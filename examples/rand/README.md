# Roll a die

Use the Malvado Lua SDK to try `mv.rand` without opening a window.
Copy [roll.lua](roll.lua) to an empty directory and run this command there.

```sh
malvado run --headless roll.lua
```

The program prints `Dice roll:` followed by an integer from 1 to 6, then
exits. Change the call to `mv.rand(6, 6)` to make the result always 6.
