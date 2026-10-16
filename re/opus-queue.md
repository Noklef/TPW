# Functions held back for Opus

Unported leaf functions that are too heavy for a quick Sonnet pass. Regenerate the x87 part with
`python -I tools/leafq.py 570000 5a0000 --groups 1500 --peek 44` (`fpu=` is the x87 instruction count).
Remove a line here when the function is ported.

## x87-heavy (0x570000-0x5a0000)

| Area | Functions (x87 count) |
|---|---|
| Mat4 maths | 0x578820 (13), 0x578900 (45), 0x578be0 (35), 0x579250 (50), 0x579330 (217), 0x579690 (63), 0x579800 (211) |
| Camera and mesh | 0x5741b0 (99), 0x5747d0 (42), 0x574110 (30), 0x574960 (16), 0x577460 (26), 0x57b410 (18) |
| Engine and weather | 0x57d920 (155), 0x57e4b0 (154), 0x581320 (31), 0x581410 (111), 0x5812f0 (12) |
| Light, camera and sprite | 0x582eb0 (34), 0x583080 (21), 0x583220 (29), 0x583280 (24), 0x584ef0 (16), 0x591510 (90) |
| Wavelet codec | 0x592150 (37), 0x5924f0 (34), 0x592b80 (15), 0x592c50 (110), 0x5933a0 (86), 0x5935a0 (117) |

## Deferred for other reasons

- 0x58faf0: texture cache eviction with overlapping REP MOVS; needs careful memory-semantics testing.
- 0x591210: uses RDTSC, so it is not deterministic to test.

## Not x87, still open (few x87 instructions)

0x57ac90 (3), 0x57cb10 (3), 0x5832e0 (8), 0x5925e0 (6), 0x592670 (6).

## Other ideas

- 0x630000-0x650000 is statically linked zlib 1.1.3; it could be ported exactly.
