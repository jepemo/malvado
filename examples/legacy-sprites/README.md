# Legacy sprites

This example shows the partial legacy compatibility layer. It imports global functions, loads a PNG directory with `fpg`, and animates a process with `fpgIndex` and `fps`. Press **Escape** to exit.

On macOS or Linux, build Malvado and run this command from the repository root:

```sh
./build/malvado-run examples/legacy-sprites/main.lua
```

The example uses the cat images from `examples/demo/assets/cat`. For an automated window check, set `MALVADO_SMOKE=1`. It exits after four process activations. Directory `fpg` mode is not yet supported on Windows. See the [legacy contract](../../docs/api/lua-contract.md#legacy-shim) for the supported subset and limits.
