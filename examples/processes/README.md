# Procesos y mensajes

Ejecuta `malvado-run --headless examples/processes/main.lua`. El padre crea un hijo, le envía una tabla y lo detiene; ambos terminan y el programa sale. El mensaje conserva la referencia a la tabla Lua compartida.
