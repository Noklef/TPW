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
- **Exe data that points into a converted range:** in `tpw.exe` the dword would point at the old, zeroed copy. The generator lists each such dword and fails. The slice has none. The next step: the generator emits them as a fixup table (slot, name, offset), and the loader applies it after the copy, as it applies `kCodeSlots`. When the slot's own global is converted, its initialiser holds `&name + offset` instead. The exe holds 1,265 `.data` to `.data`, 521 `.rdata` to `.data` and 1,945 dwords to `.rdata`.
- **A converted object that holds a code address:** the generator fails. The next step: a table that the loader resolves with `Resolve()` (as `kCodeSlots`, with the slot inside the object). When the vtables become C++ tables of ports, the initialiser names the port.
- **A converted object that points into unconverted data:** the dword keeps the original address. That address stays valid while the loader copies that range.

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
- **The literal check** finds only literals inside a converted range. An address computed from a base outside the range (`0x77dcf0 + 0x20`) is not found. Read the uses of the neighbouring addresses when you convert.

## The first slice: the IMA ADPCM decoder (`src/game/core/imaadpcm`)

- **Why this one:** its globals are used by one file only. Nothing points into them or out of them. They are both kinds: constant tables and writable state. They include a byte-indexed table, a dword array, and a struct reached by four literal addresses.
- **Numbers:** 3 objects (`gImaAdpcmSteps` 0x1640 bytes, `gImaAdpcmIndexMoves` 0x20, `gImaAdpcmState` 0x10). They replace 6 literal addresses with 16 uses, in 1 file.
- **Hook DLL unchanged:** `imaadpcm.py` 1000 cases `--both` gives 0 differences, and `tpw_tests` passes. Mutants built through the mirrored tree are caught.
- **tpw.exe:** `tpw.map` puts the three objects in `tpw.exe`'s own `.data` (`0x13cd010` and after). The loader zeroes their old ranges.

## Estimate for the rest

Scan of `src/game` (2026-11-10):

| Section | Addresses | Uses | Used by one file only |
|---|---:|---:|---:|
| `.data` | 3,751 | 11,777 | 2,740 |
| `.rdata` | 1,283 | 2,433 | 980 |
| Small data sections | 140 | 295 | 108 |

These uses are spread over 741 files.

1. **Codemod, 1 round.** 1,109 addresses are scalars that the code reaches only as `global<T>(0xADDR)`, with one type. A codemod declares and rewrites them, with names from `symbols.csv` or from the address.
2. **By hand, 8-10 rounds of three agents.** About 12,000 uses remain. The estimate assumes 500-700 uses per agent round (the slice: 16 uses in one file, about 30 changed lines). A round converts a group of files that share globals, because the check forces all users of a range to convert together.
3. **Fixup tables, 1 round, when first needed.** These are the data-to-data and object-to-code pointers from "Pointers".
4. **Not counted here:** the vtables as C++ tables of ports (3,173 `.rdata` slots), which is a separate step.
