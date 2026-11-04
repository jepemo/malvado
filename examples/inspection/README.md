# Process inspection

Run `./build/malvado run --headless examples/inspection/main.lua`. A controller creates an enemy and reads the runtime identity, tick number, and lineage count. It also finds processes of that type. The snapshot is a copy of the state at the time of the call. It does not keep processes alive.
