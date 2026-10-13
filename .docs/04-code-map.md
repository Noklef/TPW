# Code map of `tp.exe`

The MSVC linker lays out object files roughly in source order, so **address ranges correspond to source modules**. This map was built by bucketing all functions by address and looking at the debug strings, imports and already-named symbols in each bucket (`tools/string_hints.py`, `tools/vtables.py`).

## Overview

| Range | Functions¹ | Size | Category | Contents |
|---|---:|---:|---|---|
| `0x400000`–`0x560000` | 3,250 | 1,048 KB | **game** | Park simulation: save system, clock, camera, coasters, rides, people/staff, park history, world, particles, meshes, track rides, RSSE VM |
| `0x560000`–`0x590000` | 505 | 172 KB | engine | Renderer: DirectDraw/Direct3D 6 init and devices, software rasteriser, display buffers, weather |
| `0x590000`–`0x5a0000` | 113 | 47 KB | **game** | Advisor + advisor scoring |
| `0x5a0000`–`0x5b0000` | 39 | 10 KB | **game** | Language / text |
| `0x5b0000`–`0x5d0000` | 318 | 71 KB | online | EA Online: login, news, FTP download, city lists, park upload, chat, mail/postcards |
| `0x5d0000`–`0x5e0000` | 184 | 35 KB | engine | Memory / disk / system utilities (`Debug_Printf` lives here) |
| `0x5e0000`–`0x5f0000` | 147 | 52 KB | **game** | Front-end globe / level select (`ISLANDFOV`, `SPINSPEED`, `GLOBERADIUS…` tunables) |
| `0x5f0000`–`0x630000` | 421 | 60 KB | engine | Bullfrog core library: streams (`FileStream_*`), memory streams, threads, async I/O, messaging |
| `0x630000`–`0x650000` | 492 | 95 KB | library | IJG libjpeg + **EA GIMEX** image import/export |
| `0x650000`–`0x670000` | 770 | 127 KB | library? | Mixed, no debug strings: RIFF/WAV reading (`mmioRead`/`mmioSeek`), sockets, interlocked/thread helpers, heavy 16-bit (`short`) arithmetic. Most likely more EA libraries |
| `0x670000`–`0x6b0000` | 999 | 215 KB | library | MSVC C runtime + STL (locale facets, iostreams, heap, startup) |
| `0x6b0000`–`0x6d0000` | 640 | 100 KB | engine | Sound: banks (`BANK.map`), SFX, reverb (`SndReverb.map`) |
| `0x6d0000`–`0x700000` | 158 | 41 KB | library | SoundFont manager (`SFMAN`, for `.sf2`) + compiler-generated exception funclets |

¹ Excluding the 3,724 compiler-generated `thunk_`/`Catch@`/`Unwind@` stubs.

### Totals by category
| Category | Functions | What to do with it |
|---|---:|---|
| **game** | **~3,550** | Decompile → clean C++ → hook-replace. **This is the real decompilation work.** |
| engine | ~1,750 | Understand the interfaces the game calls, then **rewrite** on modern APIs (renderer, audio, file I/O) instead of translating line by line |
| library | ~1,650 | **Don't decompile.** Link the original open-source library (libjpeg, zlib) or a modern replacement (GIMEX → stb_image, SFMAN → a SoundFont synth) |
| online | ~320 | Stub out; the servers are gone |
| unknown | ~770 | Identify. Likely library code, which would shrink the work further |

## Inside the game area (`0x400000`–`0x560000`)
Finer 64 KB buckets, from string words and named symbols:

| Range | Dominant topics |
|---|---|
| `0x400000` | Action recording, balance files, clock, vanilla time, quick save |
| `0x410000` | Save/load masters, park file handling |
| `0x420000` | Camera, terrain, textures, movies |
| `0x430000` | **Coasters** (track pieces, cars, loops, directions) |
| `0x440000`–`0x450000` | Theme/world setup, terrain base, advisor hooks |
| `0x460000` | **Ride system**, animation, fonts |
| `0x470000`–`0x480000` | Sprite scripts, cursors, IME text input, UI (mouse modes such as `CMMPlaceStaff`) |
| `0x490000`–`0x4c0000` | **Park history / statistics** (`CHistory`) |
| `0x4d0000`–`0x500000` | **People**: guests, staff (handymen, mechanics, guards, entertainers), destinations, objects |
| `0x510000` | **World** (Thing array, map, macro AI, control manager), vehicles, sound save |
| `0x520000` | Particles |
| `0x530000` | Dynamic meshes |
| `0x540000` | Track rides, movies |
| `0x550000` | Game system + **RSSE VM** |

The word "Thing" appears in almost every game bucket ("Thing has been allocated in Thing…"). The simulation is built around a central **Thing array** (saved as part of `WRLD`), which is Bullfrog's long-standing C-style entity system, rather than one deep C++ class hierarchy.

## C++ classes
`tools/vtables.py` finds **399 vtables** (1,319 distinct virtual methods). 266 have an identifiable base class. Few are named: there's no RTTI, and only about 16 class names appear in strings (`CStaff`, `CObject`, `CThing`, `CHistory`, `CAdvisor`, `CTagSystem`, `CMapCell`, `CTrackCell`, `CControlManager`, `CGoldenTicketControl`, `CParkAnalyser`, `CMMPlaceStaff`…).

The largest hierarchies live in the engine and library areas (streams, STL locale facets, the online layer). Game-side C++ is mostly UI "mouse mode" classes (`CMM*`) and manager singletons.

Without RTTI, vtables are packed back-to-back, so a vtable's extent is bounded by the next known vtable start. `tools/vtables.py` handles this.
