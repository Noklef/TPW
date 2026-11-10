# Theme Park World: decompilation

A C++ reimplementation of *Theme Park World* (Bullfrog, 1999), built the way [OpenRCT2](https://github.com/OpenRCT2/OpenRCT2) rebuilt *RollerCoaster Tycoon 2*. A hook DLL loads into the original game and replaces its functions one by one with ports. The game stays playable at every step.

**No game files are in this repository.** You need your own copy of the game.

## Status (2026-11-10)

| | |
|---|---|
| Hooked functions | 9,359 (`src/hook/hooks.cpp`) |
| Counted code ported, functions only | 95.7% (2,356,225 / 2,463,079 bytes) |
| Counted code ported, with exception handlers, thunks and split-off pieces assigned to their functions | 100.0% (2,461,656 / 2,462,411 bytes) |
| Not ported (with reasons) | 3: `0x5f4a46` (unrefined 3DNow! `PFRCP`: AMD's table bits are not documented), its thunk `0x5791b0`, `0x675767` (overwrites its own return address and always faults) |
| Out of scope | The Microsoft C library (`0x679000`–`0x6b0000`). The standalone build links a modern CRT instead. |
| Benches | About 1,480 (`tests/fncmp/`), each comparing ports with the original code |
| History | Started 2026-10-13, about 4,600 commits |

"Counted code" is `.text` up to `0x6e7000`, without the C library and the compiler-generated static initialisers (7,034, 224 KB). The hand-written assembly section `cseg` (86 KB: the sprite blitter and the software rasteriser's spans) is outside it and not ported. See [.docs/07-standalone-survey.md](.docs/07-standalone-survey.md).

## Rules

- **Bug-for-bug parity.** A port does what the original does, including its crashes and float rounding. Quirks are kept and marked `// sic`.
- **The ports match behaviour, not bytes.** The compiled ports do not have to match the original machine code byte for byte.
- **C++ only for game code** (`src/game`). Python and Java (Ghidra scripts) are tooling.
- Never commit game files, the exe, `.data/` or `_re/`.

## How it works

```
tp.exe (original, decrypted)          build\dinput.dll (ours)
  FUN_00557a70 ──jmp──────────────►  RSSE_FetchWord()      ported
  FUN_0055f780 ──jmp──────────────►  TerrainBlend_Tile()   ported
  DirectInputCreateA ─────────────►  forwarded to the system dinput.dll
```

1. The game imports only `DirectInputCreateA` from `dinput.dll`. Our proxy `dinput.dll` in the game folder loads first and forwards that call.
2. For each row in `src/hook/hooks.cpp`, it checks the original prologue bytes, then writes a 5-byte `jmp` to the port. If the bytes differ, it skips that hook.
3. `tpw.ini` turns hooks off, all (`[tpw] enabled=0`) or one at a time (`[hooks] Name=0`). Use this to bisect a bad hook.

## Verification

| Step | What it proves | Tool |
|---|---|---|
| fncmp bench | The port and the original give the same registers, return value, x87 `ST0` and memory over thousands of random and "wild" cases. The original runs in an emulator (`tools/x87emu.py`: x86, x87, MMX, 3DNow!, some SSE), with exact NaN and precision rules. | `py -I tests/fncmp/NAME.py CASES SEED --both` |
| Mutants | Each bench must catch small mutations of the port. Survivors are listed and explained. | `tools/port.py` |
| Calling conventions | Each port pops what the original's `RET n` pops (`convcheck`). No port returns void where callers read `EAX` (`retcheck`). No call from a port to an original function leaves a stale `ECX`/`EDX` argument (`cdecx`). | `tools/convcheck.py`, `retcheck.py`, `cdecx.py` |
| Merge check | Build, unit tests, then every bench that a merged branch touches, on both sides. | `tools/merge/check.py` |
| In game | Hooks wired correctly, the game loads and plays. Run by hand. | `.docs/06-in-game-testing.md` |

x87 floats: the game mostly runs the FPU at 24-bit precision. `src/game/math/x87.h` (`X87Reg`) reproduces that, including which NaN survives an operation.

## Workflow

- **Rounds.** Porting agents (A, B, C) each work in a git worktree (`.worktrees/{a,b,c}`, branches `port/{a,b,c}`) on their own address range. A lead merges their branches, one check at a time.
- **One port** (`tools/port.py`): write the C++ port and its bench, add the hook row and the `re/symbols.csv` row, build, bench (normal and wild), commit, then run the mutants.
- **Merge** (`tools/merge/`): `git merge --no-ff --no-commit`, `tables.py` resolves the shared tables (`hooks.cpp`, `symbols.csv`, `CMakeLists.txt`), then `check.py`.
- **Not functions.** Addresses that turn out to be inside another function (switch arms, loop bodies, EH funclets) go into `tools/wave.py`'s missplit set.
- **Libraries.** zlib 1.1.3 and libjpeg 6a and 6b are built from the original sources (`third_party/`). The game's copies are benched against them (`docs/porting-libraries.md`).

Full agent brief and merge procedure: [docs/porting.md](docs/porting.md).

## Layout

| Path | Contents |
|---|---|
| `src/game/` | The ports, by subsystem: `core`, `files`, `rsse` (script VM), `world`, `coaster`, `terrain`, `render`, `gfx`, `ui`, `math`, `lib`, … |
| `src/hook/` | The DLL: proxy, patcher, hook table |
| `src/testing/` | Test-only tooling in the DLL (startup trace, FPU and pool watchers, log). It does not change game behaviour. |
| `tests/fncmp/` | One bench per module. `tests/test_main.cpp` holds the unit tests. |
| `tools/` | fncmp driver and emulator, port and merge tools, checkers, asset format tools (`lip`, `lnd`, `omp`, `hmp`, `md2`, …), Ghidra scripts (`*.java`) |
| `re/` | What we know about `tp.exe`, as text: symbols, types, library map, RSSE opcodes. A Ghidra project can be rebuilt from it. |
| `third_party/` | zlib and libjpeg at the exact versions the exe embeds, unmodified. Our build config is in `config/`. |
| `.docs/` | Background: how the game works, the plan, RSSE scripting, code map, exe audit, in-game testing |
| `_re/`, `.data/` | Not in git: Ghidra database and listings, game files |

## Build

Needs Visual Studio with the C++ x86 tools (it brings CMake and Ninja) and Python 3 (standard library only).

```bat
build.bat
```

The build writes `build\dinput.dll` (the hook DLL), `build\tpw_tests.exe` and `build\tpw_fncmp.exe` (the bench driver). Build from PowerShell or cmd. From Git Bash, `build.bat` hangs.

To run: copy `build\dinput.dll` into the game folder next to `tp.exe`. The hooks target one specific decrypted retail `tp.exe`. Other builds fail the prologue check, and those hooks are skipped. The log goes to `tpw.log`.

## Next

1. **Standalone `tpw.exe`** (plan step 4): stop loading the original exe. Sized in [.docs/07-standalone-survey.md](.docs/07-standalone-survey.md). This needs:
   - the exe's data sections, or real globals in their place
   - calls to the modern C runtime in place of the exe's C library
   - a new entry point and imports
2. **Platform layer:** D3D6 to a modern renderer, DirectSound and QMixer to a new audio library, DInput to Win32 or SDL input.
3. **64-bit, widescreen, fixes and mods.** These come only after parity is complete.

Plan and phases: [.docs/02-decompile-recompile-plan.md](.docs/02-decompile-recompile-plan.md).
