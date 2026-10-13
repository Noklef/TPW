# Theme Park World — how the game works

What we've established by inspecting the installed copy and the decrypted exe. **Static analysis only:** nothing here has been run, so runtime behaviour is inferred from code, strings and data.

## 1. The executables

| File | What it is |
|---|---|
| `tp.exe` (254 KB) | **SafeDisc v1 loader stub**, not the game. Checks the CD, decrypts `TP.icd` in memory, then jumps into it. Has the `BoG_` SafeDisc signature and imports `drvmgt.dll`/`wnaspi32.dll`. |
| `TP.icd` (3.7 MB) | **The real game.** Its `.text`, `CSEG`, `.data` and the smaller data sections are encrypted (entropy ≈ 8.0). The key comes from the CD at runtime, so this file can't be decompiled directly. |
| `clcd16.dll`, `clcd32.dll`, `clokspl.exe`, `drvmgt.dll`, `secdrv.sys` | SafeDisc runtime. `secdrv.sys` is a kernel driver that **Windows has blocked since the 2015 MS15-097 update**, so it's the most likely reason the game won't start on Windows 10/11. |
| `.data/nocd/tp.exe` (3.7 MB) | **`TP.icd` with SafeDisc removed** ("v2.0 all languages" no-CD). Verified against our `TP.icd`: identical entry point (`0x67EE10`), identical 11-section layout, identical sizes. Only the encrypted contents differ, and the `.rdata` differences are the re-built import names. **This is our decompilation input.** |
| `QMixer.dll` | QSound QMixer: 3D positional audio on top of DirectSound. |
| `usp10.dll` | Uniscribe (complex-script text rendering). It ships with Windows nowadays. |
| `dplayerx.dll` | SafeDisc helper. Despite the name, it isn't DirectPlay. |
| `wea*.dll` (8 files) | EA Online client libraries: `auth`, `chat`, `city`, `mail`, `news`, `ras` (dial-up), `upload` (park uploads), `vote`. Their servers have been offline for ~20 years. |
| `Ip.exe` + `ip.cfg` | EA "Install Program": a hardware-requirements checker (P200 MMX, 32 MB RAM, 8× CD). It isn't needed. |

### Build facts (decrypted exe)
- 32-bit x86 PE, image base `0x400000`, linker 5.10 → **Microsoft Visual C++ 5/6 era**. Compiler/linker stamp: `Mar 24 2000 15:14:05`.
- Written in **C++ with RTTI disabled** for game code. The only RTTI type names are MSVC STL (`std::basic_ifstream`, locale facets…). Game class names show up through C++-mangled export/import names instead (`?UploadParkFile@Upload@@…`, `ccServerList`…).
- No PDB, so there are no symbols. Function names will come from the ~10,800 strings: the game has many verbose debug messages ("Loading the global.sam went horribly wrong! Shout at Bjarne!").
- Internal codename: **"Theme Park 2"** (from ride description headers).

### Imports → platform dependencies (all must be replaced in a port)
| DLL | Used for | Modern replacement |
|---|---|---|
| `DDRAW.dll` | DirectDraw + **Direct3D (DX6 immediate mode)**. Prints `ERROR: DIRECTX6 NOT INSTALLED`; explicitly rejects PowerVR1/Voodoo1 HAL | D3D11 / OpenGL / Vulkan via SDL3 or bgfx |
| — (in exe) | **MMX software rasteriser** (the "Software Render" option) | Drop it, or keep it as a reference |
| `DINPUT.dll` | Mouse/keyboard | Win32 raw input / SDL |
| `DSOUND.dll` + `QMIXER.dll` | Audio mixing, 3D sound, reverb (`SndReverb.map`) | miniaudio / OpenAL Soft |
| `WINMM.dll` | Timers, maybe CD audio | `QueryPerformanceCounter` |
| `WSOCK32.dll` + `wea*.dll` | Online play, chat, postcards, park upload/voting | Stub out (or a hobby server later) |
| `IMM32.dll`, `USP10.dll`, `GDI32.dll` | Text input/IME, font rendering for `.ttf` in `fonts.wad` | Keep via Win32 or switch to FreeType |

### Statically linked third-party code (can be swapped for the open-source originals rather than decompiled)
- **zlib 1.1.3** (`deflate 1.1.3 Copyright 1995-1998 Jean-loup Gailly`)
- **IJG libjpeg 6/6b** (`Copyright (C) 1998, Thomas G. Lane`): postcard JPEGs, maybe textures
- **EA TGQ video decoder**, in sections `TQIA_DAT`, `IDCT_DAT`, `UVA_DATA`, `LBMPEG_D`. Plays intro movies `Data\Movies\*.tgq` (bf, roll, roc, plan, mir, jug, grav, buc, bub). FFmpeg has an `eatgq` decoder. **Note:** the movies aren't in this install (they were probably left on the CD).
- MSVC C runtime + STL (Ghidra's Function ID will auto-label most of it).

## 2. Game architecture (from strings and data layout)

### Main subsystems
From the save-game error strings (each subsystem serialises itself in a chunked save file):

`Game System` · `Clock` · `Vanilla time` · `Camera` · `Ride System` · `Track rides` · `Coasters` · `Flying rides` · `RSSE scripts` · `Advisor scoring system` · `Cheat system`

The master load-game function (`FUN_00415270`) reads a fixed sequence of modules. After the action recording and the world, each module is followed by a 4-byte tag that is verified (the game calls this "SAD"):

| # | Tag | Module | # | Tag | Module |
|---|---|---|---|---|---|
| 1 | `WRLD` | World / AI | 10 | `FLYR` | Flying rides |
| 2 | `SPSC` | Sprite scripts | 11 | `RSSE` | RSSE script VM state |
| 3 | `PART` | Particles | 12 | `KAME` | Camera |
| 4 | `MESS` | Message centre | 13 | `COAS` | Coasters |
| 5 | `CLOK` | Clock | 14 | `ADVS` | Advisor |
| 6 | `VANT` | "Vanilla time" | 15 | `SOUN` | Sound |
| 7 | `GSYS` | Game system | 16 | `CHTS` | Cheats |
| 8 | `RSYS` | Ride system | 17 | `ADSC` | Advisor scoring |
| 9 | `TRAK` | Track rides | | | |

Saving (`SaveGame_Save`) serialises all modules into an in-memory stream, compresses the buffer, and writes the result after a header. All file I/O goes through a small polymorphic C++ stream class, so file and memory streams share one vtable interface (read `+0x08`, tell `+0x18`, open `+0x24`).

#### Save file layout (`tools/tpsave.py`)
| Offset | Contents |
|---|---|
| `0x000` | u32 version (400; the loader refuses anything above 500) |
| `0x004` | u8 language (0 English … 6 Japanese) |
| `0x005` | `0x500` bytes: UTF-16 legal notice. The loader rejects the file if it was edited |
| `0x505` | `0x100` bytes (first 32 copied to/from `DAT_00802080`) |
| `0x605` | u32 **big-endian** magic `0x01221985` |
| `0x609` | u32: 1 if a preview header follows |
| `0x60d` | `BILZ` blob (zlib) holding the module stream above |

Every serializer announces each field through a debug stub, `Save_FieldName(size, "mName", isLoad)`. That stub compiles to `return 0`, but its string arguments survive, so the save format comes with the developers' own field names. The world module is:

1. The world header (game tick, random seed, the handles of singleton things such as the bank and weather).
2. Object controls, the staff pool, the game clock (`mFunnyTimeStart` is a FILETIME for 2000-01-01) and the park gates.
3. **128×128 map cells.** Each cell has a status byte selecting a map cell (52 bytes), a track cell (31) and an "RE" cell (10).
4. A linked list of **things**, each a `next` handle, a model number and that model's fields:

| Model | Thing | Model | Thing |
|---|---|---|---|
| 1 | Visitor | 10 | MechanicHQ |
| 3 | Object (ride/shop/facility) | 11 | Advisor (message queue) |
| 4–8 | Mechanic, Handyman, Entertainer, Guard, Researcher | 12 / 17 | TagSystem / UIMsgReceiver (base only) |
| 9 | StaffHQ (strikes, training budget) | 13 | ParkAnalyser (32×32 grid + stats) |
| 14 | ResearchLab | 15 | Weather |
| 16 | BankAccount (8 loans) | 19 | Challenges |

Thing positions are 8.8 fixed-point cell coordinates; cell ids are `y*128 + x + 1`. Some fields named `pv` are raw pointers that were saved by mistake. `Easymode.TPWI` was written by a debug build: unused name characters hold MSVC's `0xCDCD` heap fill.

**Retail build quirk:** the developers' debug/assert function (`Debug_Printf`, `0x5da3c0`) is an **empty function**, so all logging was compiled out. The thousands of log strings survive only as call arguments, which makes them ideal for naming functions.

This list is effectively the game's **top-level module map**. Each tag's load/save pair is a natural boundary for reimplementation.

Other subsystems seen in strings: peeps/guests (`Peep %d added to ride %d`), staff (`Cannot place staff member here`), litter (`mLitterScript`), research/upgrades, loans/finance (`Cannot pay off a loan you haven't bought`), challenges, weather/particles (`Tp2.plb`), lip-synced advisor speech, online chat/lobby, park hoardings (billboards), and a debug screenshot dump (`Scr%05ld.tga`).

### RSSE: the ride/object scripting VM
Every ride, shop, sideshow and park feature has a compiled script (`*.RSE`, magic `RSSE`). There are **308** of them in the data. The exe contains the opcode name table (81 named ops). Some examples:

- **flow:** `BRANCH`, `BRANCH_Z`, `BRANCH_NZ`, `BRANCH_PV`, `BRANCH_NV`, `RETURN`, `ENDSLICE`, `WAITABS`, `CRIT_LOCK`/`CRIT_UNLOCK`
- **animation:** `TRIGANIM`, `WAITANIM`, `LOOPANIM`, `FLUSHANIM`, `TRIGANIMSPEED`, `…_CH` channel variants
- **objects:** `ADDOBJ`, `KILLOBJ`, `FADEOBJ`, `SPAWNCHILD`, `REMOVECHILD`, `GET/SETVARINCHILD`, `…INPARENT`, `LIMBO`/`UNLIMBO`
- **physics/rides:** `BOUNCE`, `BOUNCESETNODE`, `COAST`, `TURBO`, `WALKON`/`WALKOFF`/`WALKFLOAT…`
- **effects:** `SPAWNSOUND`, `STARTSCREAM`/`SCREAMLEVEL`, `SETLIGHT`/`COLOURLIGHT`, `SPARK`, `DIPMUSIC`, `SETREVERB`, `REPAIREFFECT`
- **misc:** `EVENT`, `GETTIME`, `MONTH`, `GET/SETTIMER`, `GET/SETREMOTEVAR`, `FINDSCRIPTRAND`, `DBGMSG`

Rides are therefore *data-driven*. The C++ engine provides primitives, and the `.RSE` bytecode choreographs each ride. A port must reimplement this VM faithfully, and it's also the best way to understand ride behaviour.

## 3. Data files (`data/`, 153 MB, 814 files)

### Archives: `*.wad` (magic `DWFB`)
Bullfrog's archive format, shared with Dungeon Keeper 2. There are 312 archives holding **13,394 entries**.

```
0x00  char[4]  "DWFB"
0x04  u32      version (2)
0x48  u32      entry count
0x4C  u32      name-table offset
0x50  u32      name-table size
0x58  entry[count], 40 bytes each:
      u32 ?, name_off, name_len, data_off, stored_size, comp_type, unpacked_size, 0, 0, 0
```
`comp_type` 4 = **EA RefPack/QFS** (stream starts `10 FB` + 24-bit big-endian size), 0 = stored.
`tools/wadtool.py` implements both. All 13,394 entries decompress with sizes verified.

### Formats inside the archives and loose files
| Ext | Count | Content | Status |
|---|---|---|---|
| `.wct` | 8,375 | **Wavelet-compressed textures**: one-level Daubechies-4, YUV 4:2:0, optional alpha. 7,382 × v`0x13` (custom LZ77 bitstream, interleaved rows) and 993 × v`0x12` (`BILZ` = zlib streams, rows `[low, high]`, mostly UI). Quantiser: Y `1+q/2`, chroma `1+q/4`, alpha `q+1` | **Both decoded** (`tools/wct2png.py`) |
| `.md2` | 2,118 | 3D meshes, **not** Quake MD2: magic `0x1CD15D46`, version 221. A serialised C struct whose pointers are file offsets, relocated by `MD2_Load` (`0x46d6d0`). Two kinds: ~640 **geometry** meshes (flag bit 0) and ~1,100 **motion-only** files (`…M1`–`…M5`, data in the block at header word `0x26`). Geometry lives in 160-byte **nodes** (named parts, e.g. the advisor's "Mouth - Aah/Eee/Ooh/Sss" lip-sync shapes). Positions and UVs are **structure-of-arrays in blocks of 4** (`X[4] Y[4] Z[4]`), triangles index render vertices, which map to shared positions. Full layout in `tools/md2.py` | geometry decoded (`tools/md2.py`, 639/639 consistent); textured export works (`--wad`); motion data still open. Wads carry two texture sets, `stexture\` and `textures\` (likely software vs Direct3D renderer); only the first entry of each set carries the folder prefix |
| `.txt` | 935 | `qickload.txt` preload lists (plain text) | readable |
| `.lip` | 639 | Lip-sync timing for advisor speech | needs RE |
| `.hmp` | 435 | Object height maps: 5x5 cells per footprint tile, tile max heights and footprint flags; a cache the game rebuilds from the .md2 (tools/hmp.py) | Decoded |
| `.sam` | 344 + 56 loose | **"Balance files"**: plain-text `Group.Key value` lines (`#` comments; `Table[i].Key`; `Group.A.B.C v1 v2 v3`; shape grids between `---` lines). Each kind has a key schema built into the exe (0x3c-byte records: type, name, range, table rows), which also fixes where every value lives in memory: one 4-byte slot each, in schema order. Kinds: object (ride/shop/sideshow/feature descriptions), level (`Standard.sam`), track (coaster types), advisor, challenges, theme, graphics (`low/med/high.sam`), resolution, sound, hoardings, online | **Decoded**: `tools/samtool.py` dumps the 11 schemas and checks all 400 files (every key known); `ObjectDef` in `re/types.h` is generated from the object schema |
| `.rse` | 308 | RSSE script bytecode | **decoded** (`tools/rsse_disasm.py`, all 308 scripts; see `03-rsse-scripting.md`) |
| `.sgn` | 84 | **Sign styles** for painting a park/ride name onto `sign1/sign2.tga`: two text lines (TTF font, position, `LOGFONTA`), outline/shadow effects, gradient fills and a background image (v101: raw wavelet-encoder streams with float quantisers) | **Decoded** (`tools/sgn.py` → PNG) |
| `.esp`/`.tpc`/`.fpc` | 46/46/29 | Entertainer/costume sprites (`esprites.wad`) | needs RE |
| `.ttf` | 17 | TrueType fonts | standard |
| `.bf4` | 37 loose | Bullfrog bitmap font (magic `F4FB`): 249 Unicode glyphs, each raw 4bpp, nibble-RLE 4bpp or 1bpp, with x/y offsets and advance | **decoded** (`tools/bf4.py` → PNG sheet + metrics) |
| `.str` | 21 loose | String tables (magic `BFST`); text in a custom 8-bit code page mapped by `MBToUni.dat` | **decoded** (`tools/bfst.py`) |
| `.sdt` | 43 banks | Sound banks: named entries of plain MPEG-2 Layer I/II audio (see below) | **decoded** (`tools/sdt.py` → WAV) |
| `cat_*BANK.map`, `cat_*SFX.map`, `SndReverb.map` | — | Sound category / reverb maps (start with a GUID) | needs RE |
| `.omp` | 4 | `Scape.omp`: the theme's fixed ambient sound and particle emitters (`OBJ_` records; tools/omp.py) | Decoded |
| `.tpt` | 2 | `test.tpt`: weather keyframes (temperature, wind, cloud by day); not read by tp.exe | Decoded, unused |
| `.lnd` | 4 | `terrain.wad/base.lnd`: procedural terrain texture source: 5 blend ramps, a 384x344x4 control map (on/off + 3 blend weights), and 13 source textures at 3 detail levels (64/32/16-pixel tiles) | **Decoded** (`tools/lnd.py`); the MMX blend itself still open |
| `.tct` | 4 | `<Theme>.tct`: text table mapping path/queue tile indices to texture names (`PathTex`, `QueueTex` sections) | readable |
| `.TPWI` | 1 | A real **saved game** used as the level's starting state (same format as player saves) | **decoded** (`tools/tpsave.py`, world module) |
| `.tga`, `.ani`, `.cur` | — | Standard image/cursor formats | standard |
| `00000409.016` etc. | — | Plain BMPs (installer/autorun art) | standard |

#### Sound banks: `.sdt` (`tools/sdt.py`)
The game loads `<name>HD.sdt` (the exe also knows an `HW.sdt` variant, which isn't shipped) next to `cat_<name>BANK.map` / `cat_<name>SFX.map`.

| Offset | Contents |
|---|---|
| 0 | u32 entry count, then count × u32 absolute entry offsets |
| entry + 0x00 | u32 header size (always 40), u32 data size |
| entry + 0x08 | char[16] file name, e.g. `blnl_error1.mp2` (cut at 16 bytes: `rj_laypath_2.mp`) |
| entry + 0x18 | u32 format: low 16 bits = sample rate, next byte = 0x10 (16-bit), top byte 0x24 mono / 0x25 stereo |
| entry + 0x1c | u32 0 |
| entry + 0x20 | u32 length of the original sound in bytes of 16-bit PCM |
| entry + 0x24 | u32 0 |
| entry + 0x28 | the MPEG audio frames |

The audio is MPEG-2 (22,050 Hz) Layer I or Layer II, plus a few MPEG-1 Layer I files at 44,100 Hz. Windows has no Layer I decoder out of the box, so `tools/sdt.py` carries a small pure-Python Layer I/II decoder. Decoded audio lags the original by the filterbank's 481-sample delay, and the last frame is padded, so the sound is `pcm[481:][:length]`. Every entry decodes with at least that many samples. The trim offset is inferred from the codec, not read from the game's mixer. Each entry's data ends with one stray byte after the last frame.

Survey of all 43 banks: 3,310 entries, about 2 hours of audio. 2,650 are Layer I (5 of them MPEG-1 at 44.1 kHz) and 660 are Layer II; 20 are stereo. All decode without errors. Every Layer II frame is consumed to within 2 bytes, and the rest is zero padding, which checks the allocation tables. Loader: `SoundBank_OpenSdt` (0x6c58f0) tries `HD.sdt`, then `HW.sdt`, or the other order when its third argument is 0.

### Content layout
Four themed worlds, each with the same structure:
`data/levels/{jungle,hallow,fantasy,space}/` → `rides/`, `shops/`, `sideshow/`, `features/`, `upgrades/`, `sky/`, `ssky/`, `Sound/`, `speech/`, plus `terrain.wad`, `scape.omp`, `Standard.sam`, `Easy_Standard.sam`, `Online_Standard.sam`.

The `s`-prefixed twins (`ssky`, `stexture`, `sweather`, `ssharete.wad`) are low-detail variants for the software renderer.

### Settings
- `data/{low,med,high}.sam`: graphics presets. They expose hidden options (32-bit rendering, triple buffering, bump mapping, sky shadow).
- `data/_Resolution.sam`: rename to `Resolution.sam` to force resolutions up to 2048×1536.
- `safemode.tcf` → `save\config.tcf`: forces the lowest settings.
- Registry: `HKLM\Software\Bullfrog Productions Ltd\Theme Park World` (`Version`, `Language`, `BuildTypeCode`).
