# Controller monitor

Connect a controller before or after you start the monitor. Press buttons and move both sticks. The screen shows the active slot, held buttons, and stick values. Disconnect the controller and check that its slot changes to disconnected. Press **Escape** to exit.

Build Malvado and run this command from the repository root:

```sh
./build/malvado-run examples/controllers/main.lua
```

Slots can change after reconnection. Mappings depend on the operating system and device. Values have no dead zone, so a stick at rest can report a small value. The monitor shows analog triggers but does not test rumble. See the [input guide](../../docs/api/lua-processes.md) for supported names and limits.

For an automated window check, set `MALVADO_SMOKE=1`. It exits after five ticks. This check does not simulate a controller.
