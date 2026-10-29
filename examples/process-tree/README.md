# Process tree

Run `malvado-run --headless examples/process-tree/main.lua`. A controller creates a group with two children, waits for them to start, and stops the group and its descendants with `kill_tree`.
