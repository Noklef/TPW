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
│  ├─ wadtool.py             DWFB + RefPack ✔
│  ├─ samtool.py             .sam parser
│  ├─ rsse_disasm.py         .RSE disassembler
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
- [ ] `.sam` (trivial text), `.bf4` fonts
- [x] `.wct` v0x13 textures → PNG (`tools/wct2png.py`, 88% of textures). Quantiser scale is still empirical (0.56 luma, 0.298 chroma, measured against each file's stored average colour); find the exact formula in the decoder's caller.
- [ ] `.wct` v0x12 (`BILZ`, 993 mostly-UI textures): port `WaveletCodec_DecodeV12` (`0x593ab0`)
- [ ] `.md2` meshes → glTF/OBJ
- [x] `.RSE` disassembler (`tools/rsse_disasm.py`): all 308 scripts decode with 0 errors. See `03-rsse-scripting.md`.
- [ ] `.sdt`/`.map` sound banks → WAV
- [ ] `.omp`, `.tpt`, `.TPWI`, `.hmp`, `.lnd`, `.tct` level data

### Phase 3: Hook layer
- [x] CMake 32-bit build (`build.bat`, VS 2022 Build Tools + Ninja) of a proxy `dinput.dll` that forwards `DirectInputCreateA` to the system DLL
- [x] Function patcher (`src/hook/patch.cpp`): 5-byte `jmp`, **only after checking the original prologue bytes**, so an unknown exe build is left untouched
- [x] Hook registry (`src/hook/hooks.cpp`), per-function toggles in `tpw.ini` (`[hooks] Name=0`, `[tpw] enabled=0`), results logged to `tpw.log`
- [x] `tpw_tests`: unit tests of reimplementations, a patcher self-test, and a check of every hook's expected bytes against `tp.exe` read as a file
- [x] First replacements: `RSSE_FetchWord`, `RSSE_OperandValue`
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
