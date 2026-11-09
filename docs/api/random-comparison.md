# Random range comparison for alpha.6

This comparison uses the [BennuGD2 random implementation at the pinned
revision](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/m_rand.c#L35)
and Malvado's delivered [Lua random contract](lua-processes.md). It compares
argument order and generator ownership. It does not certify distribution,
matching sequences, or the behavior of the reference at integer overflow.

BennuGD2 orders its two arguments by minimum and maximum. For a positive range
that fits its intermediate arithmetic and is at most `RAND_MAX`, its code
uses the C library's `rand()` with a remainder operation. `RAND_SEED` calls
`srand()`. Its larger-range branch uses floating-point scaling. Results and
sequences can depend on the C library; this review does not claim statistical
or boundary equivalence for that branch.

Malvado's `mv.rand(min, max)` rejects a reversed range instead of swapping its
bounds. It calls Lua's `math.random`, whose state is set with `math.randomseed`.
A rejected reversed range does not consume a random value. The `lua_rand` test
checks that behavior, inclusive fixed bounds, full Lua integer bounds, and
seeded parity with Lua's generator. The C engine does not expose a corresponding
random API; any future C/Lua random-stream decision remains open in the
[family review](family-review.md).
