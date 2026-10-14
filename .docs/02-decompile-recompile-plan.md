# Decompile → recompile plan

## Language: C++, not C#

The game is native 32-bit x86 machine code compiled from **C++** (MSVC 5/6). It isn't .NET, so tools like ILSpy/dnSpy, which turn .NET back into C#, don't apply. A native decompiler (Ghidra) produces **C-like pseudocode**, which is far closer to C++ than to C#.

Why the rebuild should be C++:

1. **Incremental replacement needs binary compatibility.** The practical path (below) swaps the original functions one at a time while the game keeps running. Each replacement has to match the original calling convention (`__thiscall`/`__cdecl`/`__stdcall`), struct layouts and pointer arithmetic exactly. C++ compiled for 32-bit x86 can do this; C# can't.
2. **Save files and data are raw memory layouts.** The chunked save format and many asset formats are structs written straight to disk. C/C++ reproduces those layouts naturally.
3. **No second translation step.** Decompiler output → C++ is a cleanup job. Decompiler output → C# would be a full rewrite of 12,124 functions.

C# *is* a good fit for **tooling**: asset viewers, a park editor, a save-file inspector. A **clean-room C# reimplementation** (like OpenKeeper did for Dungeon Keeper 2 in Java) is also legitimate, but it's a from-scratch rewrite that uses the decompilation only as reference material, not a recompile.

## Strategy: incremental hook-and-replace (the OpenRCT2 approach)

This is how **OpenRCT2** rebuilt *RollerCoaster Tycoon 2*, a close cousin of this game. It's also how the Diablo (devilution), LEGO Island and many other recompilation projects work:

```
┌──────────────── original tp.exe (no-CD, decrypted) ────────────────┐
│  12,124 functions, all original at first                           │
│     FUN_00412340  ──jmp──►  tpw.dll: Ride_Open()      (reimplemented) │
│     FUN_00455a10  ──jmp──►  tpw.dll: Peep_Update()    (reimplemented) │
│     FUN_00501000  (still original)                                  │
└────────────────────────────────────────────────────────────────────┘
```

1. A DLL is loaded into the original game process as a **proxy `dinput.dll`** in the game folder. The game imports exactly one function from `DINPUT.dll` (`DirectInputCreateA`), so ours loads first and forwards that call to the system DLL. We deliberately don't use `ddraw.dll`: the DirectX wrappers needed on modern Windows (dgVoodoo2, DDrawCompat) are `ddraw.dll` proxies themselves. Neither name is a Windows KnownDLL, so the game folder takes precedence.
2. On load, it patches the entry of each function we've rewritten with a `jmp` to our C++ version.
3. **The game stays playable at every step.** Each rewritten function can be tested against the original by toggling its hook.
4. When every game function is replaced, drop the original exe and link `tpw.exe` standalone. It still loads the user's own `data/` folder.
5. Only then switch to 64-bit and modern backends wholesale (or earlier per-subsystem, behind the hook layer).

The alternative, **decompile everything and make it compile in one go**, never produces a runnable game until the very end, so bugs can't be bisected. That's why almost every successful project used incremental replacement.

## Repository layout

```
TPW/
├─ .data/                    (git-ignored) original install + no-CD exe; each user supplies their own
│  ├─ game-install-files/
│  └─ nocd/tp.exe            ← decompilation input (sha256 5472f97b…1b805)
├─ _re/                      (git-ignored) Ghidra project + raw full decompile dumps (regenerable)
├─ .docs/                    findings, file-format specs, this plan
├─ re/                       (tracked) the *knowledge* exported from Ghidra:
│  ├─ symbols.csv            address → name, signature, status (original/reimplemented/verified)
│  ├─ types.h                recovered structs/enums (Ride, Peep, Park, RSSE VM state…)
│  └─ ghidra_scripts/        import/export scripts, so the Ghidra DB can be rebuilt from git
├─ tools/                    asset tooling (Python today, C# later if wanted)
│  ├─ wadtool.py             DWFB + RefPack read/extract/repack, byte-exact ✔
│  ├─ samtool.py             .sam parser
│  ├─ rsse_disasm.py         .RSE disassembler
│  ├─ spritescript.py        sprite script table disassembler (scripts compiled into tp.exe)
│  └─ …                      wct/md2/str/bf4/sdt converters
├─ src/
│  ├─ hook/                  proxy dinput.dll, function patcher, hook registry
│  ├─ game/                  reimplemented game code, one folder per subsystem:
│  │   ├─ core/              game loop, clock, save/load chunks, .sam config
│  │   ├─ park/              terrain, paths, objects, finance, research, challenges
│  │   ├─ rides/             ride system, track rides, coasters, flying rides
│  │   ├─ peeps/             guests, staff, pathfinding
│  │   ├─ rsse/              script VM
│  │   ├─ advisor/           advisor, scoring, lip-sync speech
│  │   ├─ ui/                menus, HUD, fonts, strings
│  │   └─ render/            scene, sky, particles, weather
│  ├─ platform/              DirectDraw/D3D6 → modern renderer, DInput, DSound/QMixer → miniaudio, net stub
│  └─ third_party/           zlib 1.1.3, libjpeg 6b (original sources; not decompiled)
└─ CMakeLists.txt            Win32 (x86) target for hook phase; x64 later
```

## Phases

### Phase 0: Run it as-is (a reference build)
Goal: see the original running on modern Windows, so there's a ground truth to compare against.
- The no-CD exe removes the SafeDisc/`secdrv.sys` problem.
- A DirectX wrapper (**dgVoodoo2** or **DDrawCompat**) fixes DirectDraw/D3D6 on modern GPUs.
- Movies are missing from the install (they're on the CD). The game may need `Data\Movies` stubs or the CD contents.
- The no-CD exe is third-party. A static audit (`05-nocd-exe-audit.md`) found no sign of tampering, so it's low risk. A VM or Windows Sandbox is still the cautious choice for a first run.

### Phase 1: Reverse-engineering foundation *(started)*
- [x] Ghidra project from the decrypted exe (`_re/ghidra`, 12,124 functions)
- [x] Full raw decompile export (`_re/decomp-raw/decomp_*.c`, `functions.tsv`): 11,760 functions, 1 failure, ~2.1 MB of code. Regenerate with `tools/ExportDecomp.java`:
  ```
  "<GHIDRA>\support\analyzeHeadless.bat" C:\git\Noklef\TPW\_re\ghidra TPW -import C:\git\Noklef\TPW\.data\nocd\tp.exe -overwrite -scriptPath C:\git\Noklef\TPW\tools -postScript ExportDecomp.java C:\git\Noklef\TPW\_re\decomp-raw
  ```
  Ghidra refuses project paths with a component starting with `.`, which is why the project lives in `_re/` and not `.data/`.
- [x] Mapped the save/load module order from the decompiled loader (see `01-how-the-game-works.md`)
- [ ] Apply Function ID signatures for MSVC CRT/STL. Identify the zlib/libjpeg/TGQ functions and mark them **library: don't decompile**.
- [ ] Name functions from debug strings (`"Coasters failed to load"` → `Coasters_Load`). `tools/string_hints.py` lists the strings each function references. Only **932 of 7,938** unnamed functions reference a string directly, so the rest need:
  - **call-graph propagation**: helpers called only from one named subsystem belong to it
  - **vtable grouping**: functions sharing a vtable are methods of one C++ class
- [x] Repaired Ghidra's missed jump table in the RSSE interpreter (`tools/FixJumpTable.java`; ~19 KB of code was invisible). Other big switches may need the same treatment.
- [ ] Recover the core structs: Ride, Peep, Park, Object, RSSE script context, the save chunk header.
- [x] `re/symbols.csv` + `tools/ApplySymbols.java`: names live in git and are applied to Ghidra (see `re/README.md`). First batch: the save/load system (all 17 module load/save pairs), stream I/O, the CRT allocator.
- [x] Recovered types (`re/types.h`) + `tools/ApplyTypes.java`. First batch: the RSSE context and file headers. Turns `*(int *)(param_1 + 0x3c)` into `ctx->pc`.

### Phase 2: Asset formats and tools
These also become the engine's loaders later.
- [x] `.wad` + RefPack
- [x] `.str` string tables (`tools/bfst.py`, custom code page via `MBToUni.dat`; all strings decode)
- [x] `.sam` balance files: key schemas read from the exe, all 400 files checked (`tools/samtool.py`); `ObjectDef` struct generated from the object schema
- [x] `.bf4` fonts → PNG glyph sheet + metrics (`tools/bf4.py`)
- [x] `.wct` v0x13 textures → PNG (`tools/wct2png.py`). Exact quantiser from the caller `0x564e20`: Y step `1 + q/2`, Cb/Cr `1 + q/4`, alpha `q + 1`.
- [x] `.wct` v0x12 (993 mostly-UI textures): same one-level D4, rows stored `[low…, high…]`; streams are `BILZ` = 28-byte header + zlib 1.1.3 (the save-file compressor). All textures now decode.
- [x] `.md2` geometry → OBJ (`tools/md2.py`; all 639 geometry meshes pass consistency checks)
- [x] `.md2` node hierarchy: OBJ export assembles multi-part models (the advisor renders correctly)
- [x] `.md2` material groups → textured export (`tools/md2.py --wad <wad> <dir>`: OBJ + MTL + PNG)
- [x] `.md2` motion blocks (tools/md2motion.py; all 1,278 check out): position curves, rotation/scale keys, visibility, vertex and UV animation tracks. Ease tables, path tracks and the material-animation table not fully traced
- [x] `.RSE` disassembler (`tools/rsse_disasm.py`): all 308 scripts decode with 0 errors. See `03-rsse-scripting.md`.
- [x] `.sdt` sound banks → WAV (`tools/sdt.py`, pure-Python MPEG Layer I/II decoder)
- [x] Save games / `.TPWI` (`tools/tpsave.py`): container, BILZ payload, module split, and the **whole world module** (header, 128×128 map cells, all 16 thing models) decode byte-exact, with the game's own field names. Other 16 modules: split only.
- [x] `.omp` emitters and `.tpt` weather test data (tools/omp.py)
- [x] `.lnd` terrain texture source and `.tct` tile tables (tools/lnd.py)
- [x] Procedural terrain blend 0x55f780 (MMX) reimplemented in tools/lnd.py `--blend`
- [x] `.hmp` object height maps (tools/hmp.py)
- [x] `.sgn` sign styles, including their wavelet-coded backgrounds (tools/sgn.py)
- [x] `.lip` advisor lip-sync and the response table (tools/lip.py)
- [x] `.esp`/`.tpc`/`.fpc` people and effect sprites (tools/esp.py; loader Sprite_LoadEsp 0x540d90)
- [x] `.plb` particle library and `.emt` emitters (tools/plb.py)
- [x] `cat_*.map` sound categories and `SndReverb.map` (tools/sndmap.py)
- [x] TP2M `base.map` terrain attribute maps (tools/tp2m.py; cell defaults via MapCell_Reset 0x536490)

### Phase 3: Hook layer
- [x] CMake 32-bit build (`build.bat`, VS 2022 Build Tools + Ninja) of a proxy `dinput.dll` that forwards `DirectInputCreateA` to the system DLL
- [x] Function patcher (`src/hook/patch.cpp`): 5-byte `jmp`, **only after checking the original prologue bytes**, so an unknown exe build is left untouched
- [x] Hook registry (`src/hook/hooks.cpp`), per-function toggles in `tpw.ini` (`[hooks] Name=0`, `[tpw] enabled=0`), results logged to `tpw.log`
- [x] `tpw_tests`: unit tests of reimplementations, a patcher self-test, and a check of every hook's expected bytes against `tp.exe` read as a file
- [x] First replacements: `RSSE_FetchWord`, `RSSE_OperandValue`
- [x] `TerrainBlend_Tile` (0x55f780, `src/game/terrain/`): the MMX terrain blender in portable C++, matching tools/lnd.py byte for byte on every tile of all 4 themes x 3 detail levels (both written from the same reading of the asm, so the in-game run is still the real test)
- [x] `RefPack_Decode` / `RefPack_DecodedSize` (0x641f10 / 0x642110, `src/game/files/`): .wad decompression; identical to tools/wadtool.py on all 6804 compressed entries. They are callee-pops virtual methods (`ret 0xc` / `ret 4`), so the C++ uses `__stdcall`. Check the original `ret` before choosing a convention: prologue bytes do not show it
- [x] `RefPack_Encode` (0x641a50, not hooked: unreachable in the game) ported for tools: re-encoding the decoded data reproduces all 6804 compressed .wad entries byte for byte, so the shipped wads were packed by this very routine and repacked wads can match them exactly
- [x] Path curves (`Curve_Bezier`/`BezierTangent`/`Linear`/`LinearTangent`/`CatmullRom`, 0x474840-0x474cc0, `src/game/motion/`): x87 code ported to float. The game runs the FPU at single precision (Direct3D without DDSCL_FPUPRESERVE), so each component reproduces the original expression tree, recovered from the asm with `tools/x87sym.py`; the compiler associated x, y and z differently, and a single formula would differ in the last bit about 13% of the time. Bit-exact against those trees on 20,000 random cases
- [x] `Quat_ToMatrix` (0x474120) and `Path_PercentAt` (0x474ea0) the same way (the latter through exact `modf`/`fmod`); 12 hooks in all
- [x] `Quat_Slerp` (0x4742a0) and `Motion_RotationAt` (0x474490): the slerp reads the game's own sine table (0x7b0968), which is built before Direct3D lowers the FPU precision, and models the CRT `acos`, which keeps the caller's precision (float `sqrt((1+x)(1-x))`) and returns `fpatan` through a double. `SinTable_Build` (0x4708d0) reproduces the startup table. Bit-exact against a float32 model of the asm on 30,000 cases over all three branches; 14 hooks in all
- [x] Matrix library, first half (0x46f150-0x46fd40, `src/game/math/`): identity, angle extractors, the six rotations, Euler builders (gSinTable and fsin/fcos), axis scaling, point/vector transforms. Again Ghidra's C has the wrong association for the transforms and the axis lengths; bit-exact against the x87 trees on 40,000 cases. 36 hooks in all
- [x] Matrix library, second half: multiplies (0x4700f0, 0x4702d0, 0x4704c0; store order kept, since callers may pass the same matrix twice), NR LU decomposition/back substitution (0x46fe20, 0x46fd40) and the inverse built on them (0x470040), look-at basis (0x470780), merge sort by float key (0x470630), `Vec3_Distance` (0x470940). Checked with `tools/x87emu.py`, an interpreter that runs the original listing at 24-bit precision rather than a model of it: 18,000 cases (singular matrices, pivot ties, NaN inputs, aliased arguments) bit-exact, and the earlier slerp and first-half work re-verified the same way (16,000 cases). 45 hooks in all
- [x] Vector helpers 0x431480-0x431660 (`src/game/math/vector`): `Float_Modf`, `Vec3_RotateYCS`, table-driven `Vec3_RotateX`/`Vec3_RotateY`/`Matrix_RotateYAngle`; 15,000 cases bit-exact under `x87emu`. 50 hooks in all
- [x] `Motion_ApplyUVs` (0x4745c0, `src/game/motion/uvanim`): UV animation tracks, set or added into the node's UV blocks; 4,000 cases bit-exact under `x87emu` (its `__ftol` hooked). 51 hooks in all
- [x] `Motion_FindKeys` (0x470b60) and `Motion_ApplyMaterial` (0x470c90) (`src/game/motion/keys`): 6,000 cases bit-exact under `x87emu`. 53 hooks in all
- [x] `Matrix_PreMul33` (0x470d10), `Motion_LerpPackedVertices` (0x470e90) and `Motion_LerpPackedBox` (0x4711d0) (`src/game/motion/vertexanim`): packed 10:10:10 vertex keys lerped into the mesh and its bounding box; 3,000 cases bit-exact under `x87emu`. 56 hooks in all
- [x] `Motion_UnpackVertices` (0x4714a0, `src/game/motion/vertexanim`): 2,000 cases bit-exact under `x87emu` (scratch harness: memory images with pointer fixups, run by both). 57 hooks in all
- [x] `Track_Apply` (0x471860, `src/game/motion/track`): every channel of a motion track; its globals go through a `MotionEnv` so it runs outside the game. 4,500 random tracks bit-exact under `x87emu` running it and all its callees, every instruction covered. 58 hooks in all
- [x] `Motion_Apply` (0x4721f0) and `Motion_CopyVertices` (0x472270) (`src/game/motion/track`); the copy 1,000 cases equal under `x87emu`. 60 hooks in all
- [x] `Motion_ResetChannels` (0x472310, `src/game/motion/track`): 2,000 cases equal under `x87emu`, every instruction run. 61 hooks in all
- [x] `Animator_ResetLayers` (0x4726d0) and the layer helpers 0x472bc0-0x472f40 (`src/game/motion/animator`; `src/game/math/x87.h` rounds fild-integer products exactly): 9,000 cases equal under `x87emu`, every instruction run, plus 600 double-rounding traps (`x87emu` now rounds the exact result once, with the register's wide exponent). 70 hooks in all
- [x] `Layer_Play` (0x472f60), `Layer_ClearQueue` (0x473270), `Animator_Play` (0x4732a0) and `Motion_UpdateClocks` (0x473440) (`src/game/motion/animator`): 6,000 cases equal under `x87emu`, every instruction run but one unreachable. 74 hooks in all
- [x] `Animator_IsIdle` (0x473490), `Animator_RefreshNodes` (0x473550) and `Layer_Update` (0x4735d0), the per-tick layer update (`src/game/motion/animator`; Track_Apply, the material and the holder calls go through `AnimatorEnv`): 3,000 cases equal under `x87emu` (calls out recorded), every instruction run. 77 hooks in all
- [x] `Node_UpdateNormals` (0x473a40) and `Model_UpdateNormals` (0x473c30) (`src/game/motion/normals`): face normals of moved vertices, kept in x87 precision with its wide exponent where the original never stores to a float; 3,000 cases equal under `x87emu`, coordinates up to 1e18 and inf/NaN included, every instruction run. 79 hooks in all
- [x] `Animator_Update` (0x473c70), the per-tick animator driver (`src/game/motion/animator`): 1,500 cases equal under `x87emu` (the motion code it calls run in the emulator too; Track_Apply, the material, holder and attach calls recorded), every instruction run. 80 hooks in all
- [x] `Animator_Reset` (0x473e30), `Block_BindNodes` (0x470a00) and the layer accessors 0x473f50-0x4740d0 (`Layer_GetPlayback`, `Layer_GetFlags`, `Layer_GetQueued`, `Animator_SetQueuedSpeed`, `Layer_SetSpeed`, `Animator_MotionEndMs`, `Motion_SetFlags`): 3,906 cases equal under `x87emu`, every instruction run but the store in `Motion_SetFlags` and a dead branch. 89 hooks in all
- [x] The sprite script VM (`src/game/sprite/script`): `SpriteScript_Run` (0x475010) and its stack and block skipping (0x4750c0-0x475260), the 18 opcodes 0x476020-0x476a10 (GOTO, IF/ELSE/ENDIF, GOSUB, REPEAT/UNTIL, SET/ADD/SUB/MOVE, the play opcodes; END is the empty `Debug_Printf`), `Sprite_Show` (0x540b90). Scripts are threaded code in one table (0x74dab8-0x74f7c0). Checked under `x87emu` with the globals, the code table and the script at their real addresses (the driver is based at 0x400000 so its .bss covers them): 17,000 cases equal over three seeds, real and generated scripts, every instruction run. `fild n; fadd f` needs `X87_Add` (one rounding of the exact sum). 113 hooks in all. `tools/spritescript.py` lists the scripts: 863 instructions, 42 entry points from code and 4 special actions; `--check` finds every jump on an instruction and every IF/REPEAT closed
- [x] The sprite script list (`src/game/sprite/script`): `SpriteScripts_Init`/`Shutdown`/`Update`/`Draw`/`Save`/`Load`, `SpriteScript_Start`/`Init`/`Jump`/`IsAt`/`Clone`/`CopyFrom`/`Kill`, the register and position accessors and `SpriteScripts_WrapSprites` (0x475290-0x475f40), and the sprite lookups they use (`Sprite_GetAction` 0x540c70, `Sprite_GetAnimFrameCount` 0x540c60, `SpriteFolder_Count` 0x541f60). The heap, clock, run, draw and stream calls go through a `SpriteScriptEnv`. Checked under `x87emu` with stubs on both sides: 2,100 random call sequences (about 45,000 calls) equal, every reachable instruction run (Shutdown's free loop never runs: it clears the ready flag first, so the scripts leak). 136 hooks in all
- [x] The sprite draw lists (`src/game/sprite/draw`): `SpriteDraw_Begin` (0x540ab0), `Sprite_DrawScript` (0x542010) and `SpriteDraw_End` (0x540b30). A shown script fills its draw record (+0xd0: position and ground height, the frame for its animation and direction, mirrored past the sprite's directions, colour) and is appended to one of 16 lists by map cell (4x4) or to the flag-0x40000 list; the direction is r15 plus the camera heading in eighths. The ground height, the pick and its setter go through a `SpriteDrawEnv`. Checked under `x87emu` with stubs on both sides: 5,000 random cases equal over five seeds, every instruction run, a planted bug caught. `X87_Add` now returns the x87's NaN when both operands are NaNs (the larger significand). 139 hooks in all
- [x] Sprite definition helpers (`src/game/sprite/sprite`): `Sprite_Ctor` (0x540bd0), `SpriteFolder_RandomSprite`/`Sprite_ActionCount`/`Sprite_RandomAnim` (0x541f70-0x541fd0), `Sprite_FrameEntry` (0x5423a0, unused), the draw switch (0x5408e0/0x5408f0), and the park RNG `World_Rand`/`World_SetSeed` (0x516330/0x516370, `src/game/world/random`; `x87emu` learnt ROR/ROL). Checked under `x87emu`: 3,000 random call sequences equal, every instruction run, a planted bug caught. 148 hooks in all
- [x] The park's thing table (`src/game/world/things`): `Things_InitTable` (0x515f30), first/next/previous/add (0x516130-0x516270), `ThingSlot_Release` (0x516200), `Thing_GetHandle` (0x50b350), the delete queue `World_QueueDelete` (0x516310) with `HandleQueue_Push`/`Pop` (0x51ad40/0x51ad90), and `World_DeleteAllThings` (0x515fb0). Checked under `x87emu` against random call sequences that keep the table consistent (700 equal); planted bugs in the queue and DeleteAll were all caught, except ones that cannot change behaviour. 159 hooks in all
- [x] The string classes (`src/game/core/string`): `String` and `WString` (0x5f5fc0-0x5f6c50, 0x5f7120/0x5f7150, 0x5f78e0-0x5f7e60), MFC-style heap strings with capacity in steps of 8: construction, assignment, appending, `Format` (MFC's FormatV length estimate, then the CRT vsprintf), the buffer, comparisons, searches and views. The narrow copies keep MSVC6's dword-at-a-time `strncpy`/`strncat` and the inline `strcpy`/`strcat`, which decide what a string appended to itself becomes (the original re-reads the doubled length and writes past the capacity). Checked under `x87emu` running the original CRT too (`x87emu` learnt the shift flags, STOSW/CMPSB/LEAVE/JECXZ): 2,950 random call sequences equal over nine seeds, every reachable instruction run, 9 of 11 planted bugs caught (the other two cannot change behaviour). 208 hooks in all
- [x] The file and system layer (`src/game/files/file`, 0x5f5660-0x5f5fb0): the base directory (from the command line), path resolution against it, whole-file loading, the `FileStream` open/read/write/seek/close wrappers, the Win32 file calls, and the QueryPerformanceCounter clock. Win32 goes through the game's import table, so the port calls what the game imported. The original's quirks are kept: `File_Load` resolves twice (harmless with the absolute exe folder as the base), an empty base path reads the char before it, a failed allocation or read is not noticed. Checked under `x87emu` against stubs on both sides: 9,000 random call sequences equal over nine seeds, every reachable instruction run, 16 of 18 planted bugs caught (the other two cannot change behaviour). 237 hooks in all
- [x] The file library's reference-counted objects, stream base and handle class (`src/game/files/handles`): two `RefObject` flavours (count at +8 / +0xc, Interlocked add/release that tears down, releases the linked object and deletes itself at 0), the `StreamBase` (vtable 0x703100) that forwards read/write/seek/position to the object it holds, `FileStream`'s destructors, and the `Handle` (vtable 0x703180) whose result-pointer methods forward to a `RefObject2` and whose two `Open` functions build and open the 0x28- and 0xe0-byte objects. Checked under `x87emu` with stub vtables on both sides: 10,000 random call sequences equal over five seeds, all 607 instructions run, 17 of 18 planted bugs caught (the other cannot change behaviour). 277 hooks in all
- [x] `StreamBase_OpenMemory` (the memory stream SaveGame_Save writes into), the `__stdcall` C library wrappers 0x5f76c0-0x5f78c0 (`src/game/core/libc`: strlen/strcpy/strcat written out with the originals' dword-then-byte copies, the rest calling the game's own MSVC6 CRT so printing and conversions stay as they were; `0x67f330` turned out to be `iswctype`, not `_isctype`) and the palette helpers (`src/game/gfx/palette`: RGB to RGBX, and the nearest-colour search, which ranks by squared distance, then summed distance, then a darkness weight, then index). Checked under `x87emu` with stub CRT functions on both sides: 8,000 random call sequences equal (overlapping copies and crafted colour ties included), every reachable instruction run, every planted bug that can change behaviour caught (11 of 16; the other five cannot). 297 hooks in all
- [x] The in-memory stream source and the semaphore helpers (`src/game/files/memsource`, `src/game/core/semaphore`), with the plain `RefObject`'s constructor, destructors and forwarders: the `MemorySource` (vtable 0x703868) keeps its bytes in a list of fixed-size chunks and reads, writes and seeks across them; `Semaphore`/`SemaphoreLock` wrap CreateSemaphoreA/WaitForSingleObject/ReleaseSemaphore through the game's import table. The original's quirks are kept, among them a chunk-list gap: a read that reaches the size leaves no chunk current, the next write starts a chunk past the old one's end, and a later read over it computes a negative span and copies ~4 GB (the harness skips those reads on both sides). Checked under `x87emu` with stub allocator, Win32 and link vtable on both sides: 4,500 random call sequences equal over four seeds, all 464 instructions run, 20 of 20 planted bugs caught. 322 hooks in all
- [x] The zlib codec (`src/game/files/zlibcodec`, vtable 0x703128, the same codec interface as RefPack's): one-shot and streaming compress/decompress behind the 28-byte "BILZ" header, over the statically linked zlib 1.1.3 with a bump-arena allocator in a work buffer (zfree is the empty Debug_Printf). The original's quirks are kept: both one-shot calls size the work buffer for deflate, the arena advances even on a failed request, the streaming compressor sends the header out with the packed size still 0. Checked under `x87emu` against a fake zlib (consume/produce/return code from a value queue) on both sides: 2,500 random call sequences equal, all 391 instructions run, 25 of 25 planted bugs caught. 330 hooks in all
- [x] The directory source and the `RefObject2` base (`src/game/files/dirsource`, `src/game/files/handles`): the base's constructor, destructors, path accessor and its two writer methods, and the folder-backed source (vtable 0x7038d0) that `Handle_Open28` mounts. It resolves and checks the folder, then opens files in it as CreateFileA or mmio stream sources, lists it with FindFirst/NextFileA into a directory entry, and sets attributes, makes, removes and renames there by folder + name. The original's quirks are kept: the entry's name points into its own copy of the pattern after the last separator, FindFirst without a pattern still closes the earlier search, and a failed source allocation reaches the release as a null (a crash the harness steers around). Checked under `x87emu` with the real strings, semaphores and handle code on both sides, and the file layer, Win32 search calls, stream sources and text writer as stubs: 6,200 random call sequences equal over four seeds, all 633 new instructions run, 31 of 31 planted bugs caught. 349 hooks in all
- [x] The file and mmio stream sources and the text writer (`src/game/files/sources`, `src/game/files/textwriter`): the CreateFileA source (vtable 0x703e30) that reads, writes, seeks, sizes, flushes and hands out read-only mapped views, the buffered mmioOpenA source (vtable 0x703e60) for read-only opens, and the fixed-buffer text writer (vtable 0x703e08) that `RefObject2` writes paths with, sizing every assign/append through its SetLength and copying with the game's strncpy/strncat wrappers. The originals' quirks are kept: the file source's destructor leaves the handle open, dropping the mapping closes it before the whole-file view's unmap (which then does nothing), a view at an offset up to the allocation granularity maps the whole file, an mmio read or write error moves the position back one, `Format` stores _vsnprintf's -1 as the length, and clearing a zero-capacity writer writes at buffer[-1]. Checked under `x87emu` with RefObject's constructor/destructor and the semaphores on both sides, and the import table, Path_Resolve and the Lib_* wrappers as stubs: 6,500 random call sequences equal over four seeds, all 786 new instructions run, 42 of 42 planted bugs caught. 386 hooks in all
- [x] The WAD directory tree and its finder (`src/game/files/waddir`, `src/game/files/wadfind`, `src/game/core/ptrlist`): loading a WAD's header, records and name block into a tree of pooled folders and files, the pattern finder (folder walk, name-hash lookup for exact names, wildcard tokens for the rest) and FindFirst/FindNext over it. The originals' quirks are kept (see the headers): leaked buffers and finders on reload, a double free after a failed name-block read, `*` matching only the first occurrence, wildcards after a `*` dropped, a `.*` pattern also tried without it, the exact-name hash covering the folder part. Checked under `x87emu` against the original: 2,200 + 900 random call sequences equal, 1,099 of 1,105 and 1,027 of 1,033 instructions run (the misses are null-allocation and unreachable arms), planted bugs caught but for equivalent ones and one harness blind spot (the finder's first cursor). 423 hooks in all
- [ ] First in-game run with the DLL. This needs Phase 0 (the game running at all), in a VM if the no-CD exe is untrusted. Install: copy `build\dinput.dll` next to `tp.exe`, run, read `tpw.log`

### Phase 4: Reimplement subsystems (the long haul)
Roughly in dependency order: core/config/files → save/load chunks → RSSE VM → park/terrain → rides → peeps/staff → finance/research/challenges → advisor → UI.
Track progress as "% of non-library functions reimplemented".

### Phase 5: Platform layer
- [ ] Renderer: replace D3D6 immediate mode and the MMX software path with D3D11 or OpenGL. Removes the DirectX-wrapper dependency.
- [ ] Audio: DirectSound + QMixer → miniaudio (3D positioning, reverb presets from `SndReverb.map`)
- [ ] Input: DInput → Win32/SDL
- [ ] Movies: TGQ via FFmpeg's `eatgq` decoder (or skip)
- [ ] Online: stub `wea*.dll` features (the servers are gone)

### Phase 6: Standalone
- [ ] Remove the dependency on the original exe; `tpw.exe` + the user's own `data/`
- [ ] 64-bit, widescreen and high-DPI, bug fixes, mod support

## Scale and honesty check
**Update (code map, `04-code-map.md`):** only **~3,550 functions (1.16 MB) are game logic** that needs real decompilation. About 1,750 are engine code to *rewrite* against modern APIs, ~1,650 are known libraries we don't decompile, ~320 are online code to stub, and ~770 still need identifying. The original estimate below was made before that split.

- 11,760 functions in the exe: **7,938 real unnamed functions** + 3,724 compiler-generated thunks and exception-handling funclets (`thunk_FUN_*`, `Catch@*`, `Unwind@*`) + 98 named imports. Ghidra's Function ID labelled almost none of the MSVC 5/6 CRT, so part of the 7,938 is CRT/STL/zlib/libjpeg/TGQ that still needs identifying (e.g. by matching against those libraries compiled with the same compiler). That still leaves **several thousand game functions**.
- OpenRCT2 (similar game, similar size) took about 3–4 years of community work to replace all the original code. A one-person project should treat Phases 0–3 as the near-term goal. Phase 4 delivers value continuously, because the game is playable throughout.
- **Legal:** the repo should contain only original code, tools and docs, never the game's exe or data. Each player supplies their own copy. `.data/` and `_re/` are git-ignored for this reason.
