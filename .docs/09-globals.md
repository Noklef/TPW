# Real globals (2026-11-10)

Step 6 of the standalone plan: every global of the game a named C++ object, so that `tpw.exe` no longer needs `tp.exe`'s data. This file describes the mechanism and the first slice. Source: `src/game/core/gamedata.h`, `tools/gamedata.py`.

## Declaring a global

A module header declares each global once, by its original address and size:

```cpp
GAME_DATA(s32[8], gImaAdpcmIndexMoves, 0x0077dd00, 0x20);
```

- The macro declares `extern s32 (&gImaAdpcmIndexMoves)[8]` and checks `sizeof` against the size.
- The code uses the name. A literal inside a converted range (`[address, address + size)`) anywhere in `src`, outside comments and strings, fails the build.
- One declaration a line. The type has no commas (use a typedef).
- Names: the `symbols.csv` data row's name when there is one; otherwise a new row, `low`.

## How each target defines it

`tpw_game` is one static library that all four targets link, so the game code compiles once. The name is a reference, and each target defines it in a source that CMake generates into `build/` (`tools/gamedata.py`):

| Target | Generated source | The name refers to |
|---|---|---|
| hook DLL, `tpw_tests`, `tpw_fncmp` | `gamedata_address.cpp` | the original address: `T &name = *(T *)address;` |
| `tpw.exe` | `gamedata_object.cpp` | an object of its own, with the original's initial bytes |

- Both are constant-initialised (no dynamic initialiser), so a name is valid before any code runs.
- The hook DLL and the driver read and write the original addresses as before. Their behaviour does not change, and the benches' regions stay at the original addresses. The cost is one extra load for each access.
- Every build now needs Python (`find_program(TPW_PYTHON ... REQUIRED)`). `tools/fncmp.py` mirrors `tools/gamedata.py` for the mutant builds.

## Structs and arrays

One declaration covers the whole extent that the code reaches from one base:

- An array: `base + 4 * i` becomes `name[i]`.
- A struct defined in the header: the separate field addresses become members (`gImaAdpcmState.index[1]` for `0x77dd2c`).
- `u8[n]` when the code reads at byte offsets (the step table, indexed by a byte offset).

The size is explicit, so the generator knows the extent without a compiler. The `static_assert` ties it to the type.

Reads past an object's end reach the original's neighbours in the hook DLL, as before, and `tpw.exe`'s neighbours in `tpw.exe`. When a port relies on the neighbour (a table read past its end, two globals read as one), declare both as one object.

## Pointers

- **In code:** `&name` replaces a literal address that is passed as a pointer. Every literal must go, so a caller and a callee cannot disagree about where the global is.
- **The fixup table (2026-11-11):** `gamedata.py object` scans every 4-aligned dword of the exe's data sections (`pointers()`) and emits `kGameDataFixups` (`GameDataFixup` in `gamedata.h`). The loader applies it at start-up, in `MapPorts` after `kCodeSlots`, and then zeroes the converted ranges:
  - **Exe data that points into a converted range** (in the copy or in a converted object): the dword gets the object's own address plus the offset.
  - **A converted object that holds a code address:** a code slot of the copy's table (`gen_loader_slots.py`: a hooked function, an adjustor thunk to one, `Debug_Printf`) inside the object is resolved as the copy's are. Any other code address keeps the original, and a call through it traps as in the copy.
  - **A converted object that points into unconverted data:** the dword keeps the original address. That address stays valid while the loader copies that range.
  - **Rejected:** such a dword across an object's edge, an object over the import address table (the loader fills it in the copy), and a pointer whose bytes read as the end of a text (three printable bytes and a zero, after a printable one). Settle that one in `gamedata.py`'s `TEXT` (left as it is) or `POINTERS`.
  - **Not seen:** a pointer to just past an object (`end`) stays the copy's. It cannot be told from a pointer to the next global. Declare both as one object when the code walks to it. Any other dword with a pointer's value is taken for one, as the code-slot table does.
  - **Check:** `gamedata.py`'s `self_check()` (made-up exe) at each `object` run, and `tpw_tests`' `test_gamedata_fixups` (a rescan of the exe against `build/gamedata_fixups.h`, the table in original addresses).

## Initial values

- `gamedata.py object EXE OUT` reads them from the user's `tp.exe` at build time, the same way `codeslots.h` is made. They go only into `build/gamedata_object.cpp`, so nothing of the exe is committed.
- An all-zero object becomes `{}` (`.bss`). A `.rdata` constant is declared `const T` and works the same way.
- No reason was found why this cannot work. `tpw.exe` already needs the exe at build time, and the hook DLL needs no initial values, because it uses the game's own.

## The loader

- It still copies the whole image, so unconverted globals keep working by address.
- After the copy, it zeroes each converted range (`kGameDataRanges`, from the generated source), so those bytes are not the original's any more.
- Later: when every range in a section is converted, the loader stops copying that section. The code-slot rewrite for that section ends at the same time.

## Limits

- **Inline assembly** cannot use a reference as a memory operand (`fmul dword ptr ds:[0x711bec]`). Load the address first (`mov eax, gName` loads the reference's address), into a register that is free at that point.
- **The literal check** finds only literals inside a converted range. Since 2026-11-11 `reach_errors()` also finds a literal base outside the range whose constant offset reaches a converted object: `0x77dcf0 + 0x20`, `at<T>(cast(0xBASE), n)`, and a local set to `cast(0xBASE)` then used with a constant offset in its function. An offset from an index or a variable is not found. Read the uses of the neighbouring addresses when you convert.
- **Static objects (2026-11-11):** `static_errors()` rejects a converted object inside a static object that `staticinit.cpp` / `staticinitmanual.cpp` reach by the literal base (a `kMain` object, an exit thunk's, a `Th` call's `this`, a register or pushed argument). The object's own code writes the original address, while the member's name reads its copy in `tpw.exe`, and no bench sees it, as the hook DLL binds both to the original. The extent is `STATIC`'s size, else the largest `STATIC_FUNCS` size of the functions called on it, else up to the next static base. Convert the object as a whole or leave the member a literal. A static object that only code outside staticinit reaches by its literal base is not seen.

## The first slice: the IMA ADPCM decoder (`src/game/core/imaadpcm`)

- **Why this one:** its globals are used by one file only. Nothing points into them or out of them. They are both kinds: constant tables and writable state. They include a byte-indexed table, a dword array, and a struct reached by four literal addresses.
- **Numbers:** 3 objects (`gImaAdpcmSteps` 0x1640 bytes, `gImaAdpcmIndexMoves` 0x20, `gImaAdpcmState` 0x10). They replace 6 literal addresses with 16 uses, in 1 file.
- **Hook DLL unchanged:** `imaadpcm.py` 1000 cases `--both` gives 0 differences, and `tpw_tests` passes. Mutants built through the mirrored tree are caught.
- **tpw.exe:** `tpw.map` puts the three objects in `tpw.exe`'s own `.data` (`0x13cd010` and after). The loader zeroes their old ranges.

## The codemod (`tools/gamedata_codemod.py`, 2026-11-10)

It converts a `.data` scalar that the code reaches only as `global<T>(0xADDRESS)` (any accessor a file defines as `T &name(u32)`, with its default type), with one type. Rerunnable: `list` shows what is left and why, `apply N [DIR...]` converts.

- **Owner:** the one file that uses it (its header); for a shared one, the one file that writes it, and the other users include that header. The declarations go at the end of the header, after a marker comment.
- **Names:** the `symbols.csv` data row. Otherwise named for the use, unconfirmed: `g`, the ported function that uses it (the common `Prefix_` of several, else the file's name), `_`, the address (`gVertexFog_Colour_8bd484`). A new name gets a `symbols.csv` row, `low`.
- **Result:** 75 addresses, 166 uses, 38 headers. The scan found 1,407 such scalars (the round 68 estimate of 1,109 used fewer accessor forms). The rest stay for the hand rounds:

| Left alone | Addresses | Why |
|---|---:|---|
| Agent B's files (`crt*`, `staticinit*`) | 626 | B changes them this round |
| A base of unknown extent less than 0x100 below | 678 | a literal used as a pointer, with an offset or in a table: a struct or array that may reach the scalar. In `tpw.exe` the two would split. Each needs a look at the base's uses. |
| Written by 2-4 files | 19 | no single owner |
| A type that the owner's header does not know | 7 | |
| Overlaps a neighbour | 2 | |

- **Pointers:** no candidate holds a code address or is pointed at by exe data, so nothing waits for the fixup round from this codemod.

## Estimate for the rest

Scan of `src/game` (2026-11-10):

| Section | Addresses | Uses | Used by one file only |
|---|---:|---:|---:|
| `.data` | 3,751 | 11,777 | 2,740 |
| `.rdata` | 1,283 | 2,433 | 980 |
| Small data sections | 140 | 295 | 108 |

These uses are spread over 741 files.

1. **Codemod, 1 round** (done: 75, see "The codemod" for why not more). 1,109 addresses are scalars that the code reaches only as `global<T>(0xADDR)`, with one type. A codemod declares and rewrites them, with names from `symbols.csv` or from the address.
2. **By hand, 8-10 rounds of three agents.** About 12,000 uses remain. The estimate assumes 500-700 uses per agent round (the slice: 16 uses in one file, about 30 changed lines). A round converts a group of files that share globals, because the check forces all users of a range to convert together.
3. **Fixup tables, 1 round, when first needed.** These are the data-to-data and object-to-code pointers from "Pointers".
4. **Not counted here:** the vtables as C++ tables of ports (3,173 `.rdata` slots), which is a separate step.
