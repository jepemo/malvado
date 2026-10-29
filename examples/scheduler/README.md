# Prioridad de procesos

Ejecuta `malvado-run --headless examples/scheduler/main.lua`. Aunque se cree después, el proceso de prioridad alta imprime primero en cada tick. Los empates siguen el orden de creación. Los cambios de prioridad realizados durante un tick se aplican en el siguiente.
