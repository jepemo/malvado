# UTF-8 conversion in C

This headless example decodes a UTF-8 string one scalar at a time and encodes each scalar again. It uses explicit byte lengths. The encoded bytes are not NUL-terminated. Invalid or incomplete input makes decoding return zero.

```sh
cmake -S . -B build-c -DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF
cmake --build build-c
./build-c/example-c-utf8
```

The output is `U+0041` and `U+00F1`. The example does not need a window or Lua. See the [input guide](../../../docs/api/lua-processes.md) for text input and error behavior.
