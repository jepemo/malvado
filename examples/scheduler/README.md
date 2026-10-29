# Process priority

Run `malvado-run --headless examples/scheduler/main.lua`. The high-priority process prints first in each tick, although it was created later. Ties use creation order. Priority changes during a tick take effect on the next tick.
