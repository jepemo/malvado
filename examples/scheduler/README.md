# Process priority

Run `./build/malvado run --headless examples/scheduler/main.lua`. Setup sets
each worker's initial priority by handle. On the first tick, `High` prints
before `Low`. The `Low` worker raises its own priority with `set_priority(20)`
and prints first on the second tick. Ties use creation order.
