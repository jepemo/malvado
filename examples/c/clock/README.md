# Primer ejemplo C: reloj lógico

Ejemplo headless de la API C inicial. No requiere Lua ni raylib y muestra dos ticks al avanzar 1/30 de segundo con una frecuencia de 60 Hz.

```sh
cmake -S . -B build-c -DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF
cmake --build build-c
ctest --test-dir build-c --output-on-failure
./build-c/example-c-clock
```

Es una prueba del contexto y reloj, no un juego completo: el scheduler y el binding común siguen pendientes.
