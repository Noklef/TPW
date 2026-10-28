# The closure of 0x463060 / 0x463c80 / 0x436fa0

Every unported function the three roots reach through direct calls (agent B, round 31, from the branch at ee059a51; sizes from
tools/wave.py extent). Harness answers (__ftol, modf, _CIfmod, Mem_Alloc / Mem_Free, 0x5da3c0) and the CRT past 0x67a400 are left out;
known skips are listed (SKIP) but not followed. Owners: A 0x4e0000-0x530000 and 0x6b0000-0x6e7000; C 0x480000-0x4e0000 and 0x658000-0x679000; B the rest
(0x500000-0x530000 moved from C to A after this was written; the owners below are relabelled to match).
A snapshot: hooks.cpp is the record of what is ported. Since it was written the hubs 524960 (GameMode_MapClick, ported by C), 50b780
(Thing_Delete), 527ee0 (Path_CheckDeadEnds), 40fb10, 463c80, 53afa0 and 5367a0 have all been ported (527ee0 and 5367a0 first on the agents' branches).

Totals (by the owners when written): 217 functions, 0x2348a bytes: A 12 (0x1c72), B 91 (0x10f5d), C 114 (0x108bb).
(A snapshot: the totals count 524960 (GameMode_MapClick, src/game/world/mapclick) and 4df390 (Object_Close), both ported since.)

Flags: x87 (FPU code), vcalls (virtual or indirect calls: recorder benches), SKIP (a known skip). "Unported callees" are the
callees inside this closure.

## 1. Outside the cycle, leaves first

Layer 0: every callee ported or a harness answer. Layer n: callees of layers below n only. Biggest first within a layer.

| layer | address | size | owner | name | flags | unported callees |
|---|---|---|---|---|---|---|
| 0 | 509e10 | 4141 | A |  |  | - |
| 0 | 469a80 | 2766 | B |  | x87 | - |
| 0 | 434740 | 1992 | B |  | x87 | - |
| 0 | 587db0 | 1137 | B | SpriteFrames_Load | SKIP | - |
| 0 | 535dd0 | 816 | B |  |  | - |
| 0 | 492750 | 664 | C |  |  | - |
| 0 | 50be80 | 503 | A |  |  | - |
| 0 | 4c7db0 | 490 | C |  | x87 | - |
| 0 | 4aa120 | 446 | C |  |  | - |
| 0 | 4fa2a0 | 436 | A |  | x87 | - |
| 0 | 503b80 | 428 | A |  | x87 | - |
| 0 | 41e740 | 416 | B |  | vcalls SKIP | - |
| 0 | 4a0e30 | 380 | C |  |  | - |
| 0 | 59ae20 | 373 | B |  | vcalls | - |
| 0 | 4f1400 | 273 | A |  |  | - |
| 0 | 497d20 | 232 | C |  |  | - |
| 0 | 4fa470 | 187 | A |  | x87 | - |
| 0 | 4c8ae0 | 184 | C |  |  | - |
| 0 | 493800 | 180 | C |  | x87 | - |
| 0 | 4d4ad0 | 157 | C |  |  | - |
| 0 | 4c8ba0 | 156 | C |  |  | - |
| 0 | 439780 | 146 | B |  |  | - |
| 0 | 4c7600 | 117 | C |  |  | - |
| 0 | 51c640 | 104 | A |  |  | - |
| 0 | 481af0 | 85 | C |  |  | - |
| 0 | 51c0e0 | 70 | A |  |  | - |
| 0 | 51c300 | 70 | A |  |  | - |
| 0 | 51c0a0 | 62 | A |  |  | - |
| 0 | 51c2c0 | 62 | A |  |  | - |
| 0 | 51c6b0 | 62 | A |  |  | - |
| 0 | 5041f0 | 26 | A |  |  | - |
| 0 | 4b3620 | 24 | C |  |  | - |
| 0 | 498780 | 6 | C |  |  | - |
| 0 | 4c9040 | 6 | C |  |  | - |
| 1 | 5587f0 | 2153 | B | RSSE_LoadScriptFile | x87 | 51c640 (A) |
| 1 | 501db0 | 1563 | A |  | x87 | 4c7600 (C) 4fa470 (A) |
| 1 | 59b060 | 1162 | B | Advisor_HandleMessage | vcalls | 59ae20 (B) |
| 1 | 4c7720 | 872 | C |  | x87 | 4c7db0 (C) |
| 1 | 4e5260 | 608 | A |  |  | 4f1400 (A) |
| 1 | 509bf0 | 542 | A | TagSystem_HandleMessage | vcalls | 509e10 (A) |
| 1 | 542640 | 536 | B | Sprites_UseParkFrames | vcalls | 41e740 (B) 587db0 (B) |
| 1 | 542420 | 532 | B | Sprites_UseGroundFrames | vcalls | 41e740 (B) 587db0 (B) |
| 1 | 5994e0 | 489 | B |  |  | 51c0a0 (A) 51c0e0 (A) 51c2c0 (A) |
| 1 | 47fcd0 | 437 | B |  | x87 | 492750 (C) |
| 1 | 469940 | 310 | B |  |  | 469a80 (B) |
| 1 | 5131f0 | 160 | A |  |  | 51c300 (A) |
| 1 | 4d06b0 | 154 | C |  | x87 | 4c7db0 (C) |
| 1 | 4818c0 | 86 | C |  |  | 4b3620 (C) |
| 1 | 4aaec0 | 69 | C |  |  | 4aa120 (C) |
| 1 | 493c50 | 50 | C |  |  | 493800 (C) |
| 1 | 4c8150 | 46 | C |  |  | 4c8ae0 (C) |
| 1 | 4c8180 | 46 | C |  |  | 4c8ba0 (C) |
| 1 | 481560 | 27 | C |  |  | 4a0e30 (C) |
| 1 | 551300 | 18 | B |  |  | 51c300 (A) |
| 2 | 509360 | 1551 | A |  | x87 | 4c8150 (C) 4c8180 (C) |
| 2 | 432df0 | 1061 | B |  | x87 | 469940 (B) |
| 2 | 512f00 | 746 | A |  | x87 | 5131f0 (A) |
| 2 | 4bd090 | 460 | C |  | x87 | 4e5260 (A) |
| 2 | 49fa00 | 304 | C |  |  | 4d06b0 (C) |
| 2 | 4c7370 | 281 | C | ParkAnalyser_HandleMessage | vcalls | 4c7720 (C) |
| 2 | 4dcf90 | 257 | C |  |  | 5587f0 (B) |
| 2 | 480810 | 220 | C |  | vcalls | 47fcd0 (B) |
| 2 | 59aa70 | 220 | B |  |  | 5994e0 (B) |
| 2 | 5012f0 | 157 | A |  |  | 501db0 (A) |
| 2 | 500870 | 136 | A |  |  | 501db0 (A) |
| 2 | 46cf10 | 109 | B |  |  | 4e5260 (A) |
| 2 | 439670 | 107 | B |  |  | 51c2c0 (A) 551300 (B) |
| 2 | 481920 | 58 | C |  |  | 493c50 (C) |
| 2 | 481520 | 16 | C |  |  | 4aaec0 (C) |
| 3 | 433bd0 | 916 | B |  | x87 | 432df0 (B) |
| 3 | 4bd260 | 405 | C |  | x87 | 46cf10 (B) |
| 3 | 4fb360 | 387 | A | Visitor_HandleMessage | x87 vcalls | 5012f0 (A) 501db0 (A) |
| 3 | 501390 | 322 | A |  |  | 5012f0 (A) 501db0 (A) 50be80 (A) |
| 3 | 4e0450 | 271 | A |  |  | 500870 (A) 5012f0 (A) |
| 3 | 4a5e10 | 268 | C |  |  | 480810 (C) |
| 3 | 50ca50 | 268 | A | UIMsgReceiver_HandleMessage | vcalls | 481520 (C) 4818c0 (C) 481920 (C) 481af0 (C) |
| 3 | 443f40 | 245 | B |  | x87 | 432df0 (B) |
| 3 | 4392a0 | 230 | B |  | x87 | 439670 (B) 439780 (B) |
| 3 | 4424e0 | 219 | B |  | x87 | 432df0 (B) |
| 3 | 5127a0 | 218 | A | Weather_HandleMessage | vcalls | 512f00 (A) |
| 3 | 43bf20 | 133 | B |  |  | 439670 (B) |
| 3 | 49b520 | 73 | C |  |  | 480810 (C) |
| 3 | 4a0770 | 39 | C |  |  | 49fa00 (C) |
| 4 | 43be10 | 269 | B |  |  | 4392a0 (B) |
| 4 | 4423f0 | 232 | B |  |  | 4424e0 (B) |
| 4 | 4425c0 | 219 | B |  |  | 4424e0 (B) |
| 4 | 4a2ac0 | 179 | C |  |  | 4bd090 (C) 4bd260 (C) |
| 4 | 444150 | 62 | B |  |  | 443f40 (B) |
| 5 | 435730 | 699 | B |  |  | 43be10 (B) 43bf20 (B) |
| 5 | 444360 | 521 | B |  |  | 4423f0 (B) 4425c0 (B) |
| 5 | 435340 | 446 | B |  | x87 | 444150 (B) |
| 5 | 442220 | 396 | B |  |  | 4423f0 (B) 4425c0 (B) |
| 5 | 481470 | 23 | C |  |  | 4a2ac0 (C) |
| 6 | 42ab20 | 835 | B | Camera_SetMode |  | 481470 (C) |
| 6 | 444770 | 834 | B |  |  | 442220 (B) 4423f0 (B) 4425c0 (B) 444360 (B) |
| 6 | 442880 | 675 | B |  |  | 442220 (B) 4423f0 (B) 4425c0 (B) 444360 (B) |
| 6 | 445220 | 651 | B |  |  | 442220 (B) 444360 (B) |
| 6 | 442c70 | 92 | B |  |  | 435730 (B) |
| 7 | 42a560 | 434 | B | Camera_FollowThing |  | 42ab20 (B) 542420 (B) 542640 (B) |
| 7 | 42ae70 | 337 | B | Camera_ToggleWalkMode | x87 | 42ab20 (B) 542420 (B) 542640 (B) |
| 8 | 4e15b0 | 184 | A |  |  | 42a560 (B) |
| 8 | 42a190 | 53 | B |  |  | 42a560 (B) |
| 9 | 48ae70 | 371 | C |  | vcalls | 42a190 (B) 42ae70 (B) 4e15b0 (A) |
| 9 | 48ac40 | 368 | C |  |  | 42a190 (B) 42ae70 (B) 4e15b0 (A) |
| 10 | 481b50 | 16 | C |  |  | 48ac40 (C) |
| 10 | 481b60 | 16 | C |  |  | 48ae70 (C) |

## 2. The cycle

111 functions (A 5, 0x1348 bytes, B 51, 0xb46f bytes, C 55, 0xbe6d bytes) call each other through one strongly connected component: every one of them reaches itself again.
No order leaves them all leaves-first: whichever lands first still calls an unported member. The eight hubs below cut every cycle
(a greedy feedback vertex set): with their incoming edges cut, the rest layers cleanly, as listed. A member's port can only be
built and benched once its callees exist, so either a hub lands in the same batch as the members that call it back, or the
members call their hub through an env (as world/things.cpp's ThingTableEnv calls 0x50b780: the game's address in game, a
recorder in the bench) until the hub is ported, and the env is then pointed at the port.

The same holds for SKIP callees in part 1: 0x542420 and 0x542640 call 0x41e740 and 0x587db0 (known skips), so they, and
through them 0x42a560 / 0x42ae70 / 0x42a190 and the root 0x463c80, wait for those two or need them behind an env.

Hubs (the cycle closers): 524960 (A, 11530 bytes), 50b780 (A, 407 bytes, Thing_Delete), 53afa0 (B, 510 bytes), 5367a0 (B, 1133 bytes), 527ee0 (A, 1974 bytes, Path_CheckDeadEnds), 40fb10 (B, 367 bytes), 463060 (B, 2300 bytes), 463c80 (B, 299 bytes).

Layers with the hubs' incoming edges cut (callees marked * are hubs):

| layer | address | size | owner | name | flags | unported callees |
|---|---|---|---|---|---|---|
| 0 | 52ccb0 | 1333 | A |  |  | 53afa0 (B)* |
| 0 | 52c6f0 | 1290 | A |  |  | 53afa0 (B)* |
| 0 | 52bdd0 | 888 | A |  |  | 527ee0 (A)* 5367a0 (B)* |
| 0 | 53bc60 | 716 | B |  |  | 53afa0 (B)* |
| 0 | 4d0370 | 602 | C |  |  | 40fb10 (B)* |
| 0 | 436d10 | 587 | B |  |  | 463060 (B)* |
| 0 | 538fc0 | 580 | B |  |  | 5367a0 (B)* |
| 0 | 52fc80 | 450 | A |  |  | 53afa0 (B)* |
| 0 | 532dd0 | 398 | B |  |  | 53afa0 (B)* |
| 0 | 4dd500 | 359 | C |  |  | 463060 (B)* |
| 0 | 506d10 | 305 | A |  |  | 40fb10 (B)* |
| 0 | 522d50 | 238 | A |  |  | 463060 (B)* |
| 0 | 434570 | 222 | B |  |  | 463c80 (B)* |
| 0 | 522b90 | 201 | A |  |  | 463060 (B)* |
| 0 | 522f20 | 201 | A |  |  | 463060 (B)* |
| 0 | 5229e0 | 170 | A |  |  | 463060 (B)* |
| 0 | 50b980 | 144 | A |  |  | 463c80 (B)* |
| 0 | 52c150 | 103 | A |  |  | 5367a0 (B)* |
| 0 | 522a90 | 14 | A |  |  | 463c80 (B)* |
| 1 | 53b280 | 2082 | B |  |  | 527ee0 (A)* 538fc0 (B) 53afa0 (B)* |
| 1 | 54ae50 | 681 | B |  | x87 | 463c80 (B)* 51c300 (A) |
| 1 | 446d50 | 590 | B |  | x87 | 52c150 (A) |
| 1 | 504c70 | 356 | A |  | vcalls | 506d10 (A) |
| 1 | 53bb20 | 309 | B |  |  | 538fc0 (B) |
| 1 | 5365d0 | 269 | B |  |  | 5229e0 (A) 522a90 (A) 535dd0 (B) |
| 1 | 55cda0 | 188 | B |  |  | 463c80 (B)* 51c2c0 (A) |
| 1 | 4f8ae0 | 40 | A |  |  | 50b980 (A) |
| 1 | 536780 | 28 | B |  |  | 522a90 (A) |
| 1 | 512000 | 16 | A |  |  | 50b980 (A) |
| 2 | 5348d0 | 3278 | B |  | x87 | 5365d0 (B) |
| 2 | 5311c0 | 2019 | B |  |  | 5365d0 (B) 53b280 (B) 53bb20 (B) |
| 2 | 530120 | 972 | B |  |  | 5365d0 (B) |
| 2 | 545610 | 459 | B |  |  | 54ae50 (B) |
| 2 | 545450 | 445 | B |  |  | 54ae50 (B) |
| 2 | 55d3d0 | 249 | B |  |  | 55cda0 (B) |
| 2 | 4feb50 | 192 | A |  |  | 501db0 (A) 50b780 (A)* |
| 2 | 452ca0 | 132 | B |  |  | 5367a0 (B)* 53b280 (B) |
| 2 | 4d6cf0 | 99 | C | Handyman_HandleMessage | vcalls | 504c70 (A) |
| 2 | 4da0c0 | 78 | C | Mechanic_HandleMessage | vcalls | 504c70 (A) |
| 2 | 4d5fb0 | 69 | C | Guard_HandleMessage | vcalls | 504c70 (A) |
| 2 | 4fb330 | 41 | A |  |  | 4f8ae0 (A) |
| 2 | 4d4450 | 13 | C | Staff_HandleMessageDefault |  | 504c70 (A) |
| 3 | 52a970 | 2472 | A |  | x87 | 53afa0 (B)* 545450 (B) |
| 3 | 52b400 | 1434 | A |  |  | 53b280 (B) 53bc60 (B) 545450 (B) |
| 3 | 508f70 | 966 | A |  |  | 40fb10 (B)* 509360 (A) |
| 3 | 558500 | 718 | B |  |  | 51c2c0 (A) 51c300 (A) 55d3d0 (B) |
| 3 | 4d65d0 | 393 | C |  |  | 4feb50 (A) |
| 3 | 442ea0 | 149 | B |  |  | 452ca0 (B) |
| 3 | 43eaf0 | 123 | B |  |  | 434570 (B) 452ca0 (B) |
| 4 | 4f7c40 | 3784 | A |  | x87 vcalls | 40fb10 (B)* 46cf10 (B) 481560 (C) 49b520 (C) 4a0770 (C) 4a5e10 (C) 4fa2a0 (A) 4fa470 (A) 50b980 (A) |
| 4 | 4e0e60 | 879 | A |  |  | 40fb10 (B)* 4e0450 (A) |
| 4 | 559060 | 740 | B | RSSE_KillScript | x87 | 51c6b0 (A) 558500 (B) |
| 4 | 436fa0 | 420 | B |  |  | 43bf20 (B) 43eaf0 (B) 463c80 (B)* |
| 4 | 508e70 | 223 | A |  |  | 508f70 (A) |
| 4 | 4d6260 | 198 | C |  |  | 4d65d0 (C) |
| 5 | 4db090 | 1839 | C |  |  | 40fb10 (B)* 4d4ad0 (C) 4dcf90 (C) 4dd500 (C) 4e0e60 (A) |
| 5 | 4de1f0 | 678 | C |  |  | 4e0e60 (A) 501390 (A) |
| 5 | 4dd0a0 | 611 | C |  | x87 | 40fb10 (B)* 50b980 (A) 559060 (B) |
| 5 | 508a30 | 378 | A | StaffHQ_HandleMessage | vcalls | 4d6260 (C) 508e70 (A) |
| 5 | 4d5eb0 | 246 | C |  |  | 4f7c40 (A) 4feb50 (A) |
| 5 | 4d9fe0 | 212 | C |  |  | 4f7c40 (A) |
| 5 | 4df390 | 177 | C |  |  | 4e0e60 (A) |
| 5 | 4d6c90 | 94 | C |  |  | 4f7c40 (A) |
| 5 | 4d4620 | 59 | C |  |  | 4f7c40 (A) |
| 5 | 502920 | 57 | A |  |  | 4f7c40 (A) 5041f0 (A) |
| 6 | 519ef0 | 898 | A |  |  | 40fb10 (B)* 4df390 (C) |
| 6 | 529e10 | 568 | A |  | x87 | 4db090 (C) |
| 6 | 447520 | 561 | B |  |  | 435730 (B) 4423f0 (B) 4425c0 (B) 4df390 (C) |
| 6 | 5346d0 | 503 | B |  |  | 4de1f0 (C) 5367a0 (B)* |
| 7 | 4474d0 | 67 | B |  |  | 447520 (B) |
| 8 | 52f580 | 699 | A |  |  | 4474d0 (B) 59aa70 (B) |
| 9 | 5370e0 | 7692 | B |  | x87 | 527ee0 (A)* 52c6f0 (A) 52ccb0 (A) 52f580 (A) 5365d0 (B) 5367a0 (B)* 538fc0 (B) 53afa0 (B)* 53b280 (B) |
| 9 | 532fc0 | 5684 | B |  | x87 | 527ee0 (A)* 52c6f0 (A) 52ccb0 (A) 52f580 (A) 532dd0 (B) 5346d0 (B) 5348d0 (B) 5365d0 (B) 5367a0 (B)* |
| 9 | 527960 | 908 | A |  | x87 | 52b400 (A) 52f580 (A) |
| 10 | 5286a0 | 975 | A |  |  | 532fc0 (B) |
| 10 | 52ba50 | 800 | A |  |  | 5370e0 (B) 538fc0 (B) 53afa0 (B)* |
| 10 | 536100 | 504 | B |  |  | 532fc0 (B) |
| 10 | 452d30 | 211 | B |  |  | 532fc0 (B) 5367a0 (B)* |
| 11 | 528a70 | 4756 | A |  | x87 | 4de1f0 (C) 5286a0 (A) 529e10 (A) 532fc0 (B) 5348d0 (B) 536780 (B) 5367a0 (B)* |
| 11 | 434060 | 1109 | B |  | x87 | 433bd0 (B) 452d30 (B) 463060 (B)* |
| 11 | 442f40 | 1017 | B |  | x87 | 432df0 (B) 433bd0 (B) 442ea0 (B) 452d30 (B) 469940 (B) |
| 11 | 444cb0 | 626 | B |  | x87 | 432df0 (B) 434570 (B) 442ea0 (B) 452d30 (B) |
| 11 | 5168f0 | 502 | A |  |  | 481b50 (C) 519ef0 (A) |
| 11 | 516b00 | 248 | A |  |  | 481b60 (C) 4db090 (C) |
| 11 | 52c1c0 | 214 | A |  |  | 536100 (B) |
| 12 | 5234d0 | 4714 | A |  |  | 434740 (B) 524960 (A)* 527960 (A) 528a70 (A) 52b400 (A) 52f580 (A) 536100 (B) |
| 12 | 445a70 | 1086 | B |  | x87 | 4423f0 (B) 4425c0 (B) 442f40 (B) 452d30 (B) 52c1c0 (A) |
| 12 | 435020 | 787 | B |  | x87 | 434060 (B) 435340 (B) 442880 (B) 442c70 (B) 444770 (B) 498780 (C) |
| 12 | 444f30 | 738 | B |  |  | 442220 (B) 444cb0 (B) 445220 (B) |
| 12 | 503d40 | 664 | A | ResearchLab_HandleMessage | x87 vcalls | 40fb10 (B)* 503b80 (A) 516b00 (A) |
| 12 | 446050 | 626 | B |  |  | 4423f0 (B) 4425c0 (B) 442f40 (B) 52c1c0 (A) |
| 12 | 436ac0 | 586 | B |  | x87 | 434060 (B) |
| 12 | 435570 | 403 | B |  |  | 434060 (B) 435730 (B) 442880 (B) 498780 (C) |
| 12 | 4d02d0 | 153 | C | BankAccount_HandleMessage | vcalls | 4d0370 (C) 5168f0 (A) |
| 12 | 435530 | 61 | B |  |  | 444cb0 (B) |
| 13 | 446740 | 1271 | B |  | x87 | 435530 (B) 435730 (B) 442220 (B) 444360 (B) 444cb0 (B) 445220 (B) 497d20 (C) |
| 13 | 52f200 | 892 | A |  |  | 446050 (B) 4474d0 (B) 59aa70 (B) |
| 13 | 4368f0 | 461 | B |  | x87 | 435730 (B) 436ac0 (B) 436d10 (B) |
| 13 | 50b550 | 256 | A | Thing_HandleMessage |  | 4c7370 (C) 4d02d0 (C) 4d4450 (C) 4d5fb0 (C) 4d6cf0 (C) 4da0c0 (C) 4fb360 (A) 503d40 (A) 508a30 (A) 509bf0 (A) 50ca50 (A) 5127a0 (A) 59b060 (B) |
| 13 | 4419b0 | 73 | B |  |  | 435020 (B) |
| 14 | 446300 | 918 | B |  |  | 4423f0 (B) 4425c0 (B) 446740 (B) 446d50 (B) |
| 14 | 52fe50 | 625 | A |  |  | 444f30 (B) 4de1f0 (C) 524960 (A)* 52c6f0 (A) 52ccb0 (A) 52f200 (A) 52f580 (A) 52fc80 (A) |
| 14 | 5327e0 | 244 | B |  |  | 524960 (A)* 52ba50 (A) 52f200 (A) 5311c0 (B) |
| hub | 524960 | 11530 | A | GameMode_MapClick (ported since, by C) | x87 | 435570 (B) 4419b0 (B) 445a70 (B) 446050 (B) 446300 (B) 4de1f0 (C) 4df390 (C) 5234d0 (A) 527ee0 (A)* 528a70 (A) 52a970 (A) 52b400 (A) 52bdd0 (A) 52c6f0 (A) 52ccb0 (A) 52f200 (A) 52f580 (A) 52fc80 (A) 530120 (B) 5327e0 (B) 536100 (B) 545450 (B) |
| hub | 463060 | 2300 | B |  | x87 | 4368f0 (B) 469940 (B) |
| hub | 527ee0 | 1974 | A | Path_CheckDeadEnds | x87 | 50b780 (A)* 52f200 (A) 52f580 (A) 52fe50 (A) 530120 (B) 5367a0 (B)* 53b280 (B) 545610 (B) |
| hub | 5367a0 | 1133 | B |  |  | 4de1f0 (C) 5346d0 (B) 5365d0 (B) |
| hub | 53afa0 | 510 | B |  |  | 522a90 (A) 522b90 (A) 522d50 (A) 522f20 (A) |
| hub | 50b780 | 407 | A | Thing_Delete |  | 40fb10 (B)* 4c9040 (C) 4d4620 (C) 4d5eb0 (C) 4d6c90 (C) 4d9fe0 (C) 4dd0a0 (C) 4f7c40 (A) 4fb330 (A) 502920 (A) 512000 (A) |
| hub | 40fb10 | 367 | B |  | vcalls | 50b550 (A) |
| hub | 463c80 | 299 | B |  |  | 42a190 (B) 436fa0 (B) |
