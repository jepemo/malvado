# Processes and messages

Run `./build/malvado run --headless examples/processes/main.lua`. The parent creates a child, sends it a table, and stops it. Both processes end and the program exits. The message retains a reference to the shared Lua table.
