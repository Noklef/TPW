# RSSE: the ride/object scripting engine

Every ride, shop, sideshow and park feature in Theme Park World runs a compiled **RSSE** script (`*.RSE`, found inside the `.wad` archives). The C++ engine supplies primitives (animate, bounce a guest, spawn a child object, scream…), and the script choreographs them. Reimplementing this VM faithfully is required for any port, and it's also the clearest window into how each ride behaves.

**Status:** the instruction set and file format are understood well enough to disassemble **all 308 scripts in the game with zero errors** (`tools/rsse_disasm.py --check-all`). The interpreter in the exe has been located and repaired so that it decompiles.

## Where it lives in the exe
| Address | Name | What |
|---|---|---|
| `0x765280` | `RSSE_OpcodeNameTable` | 106 × `{char* mnemonic, char* operand_count}`. The source of `re/rsse_opcodes.csv` |
| `0x551cb0` | `RSSE_ExecuteInstruction` | Fetch-decode-dispatch for one instruction |
| `0x5567d8` | `RSSE_OpcodeHandlerTable` | 106 handler addresses. Ghidra misses this table, so run `tools/FixJumpTable.java` (see `re/README.md`) |
| `0x5587f0` | `RSSE_LoadScriptFile` | Loader ("Not a RSSE script file", "Zero length script") |
| `0x551000`–`0x55a500` | | The whole RSSE subsystem: init, malloc, walk nodes, custom events, variable access |

Script context fields seen so far: `+0x18` code pointer, `+0x1C` variable array, `+0x3C` program counter (in words; set to −10000 on error), `+0x50` code length.

## File format (`.RSE`)
All little-endian 32-bit words.

```
0x00  "RSSE"
0x04  version            0x00010F51 in every shipped script
0x08  variable count     N: the last N strings in the file are variable names
0x0C  ?                  (e.g. 3 for Bouncy Dino)
0x10  ?                  0x32 in all scripts seen so far
0x14  ?
0x18  ?                  (e.g. 0xA for Bouncy Dino)
0x1C  ?
0x20  "Pad Pad Pad Pad "
0x30  code_len           number of code words
0x34  code[code_len]
....  strings until EOF: u32 length (incl. NUL) + bytes.
      First the string table (referenced by 0x1… operands), then the N variable names.
```

### Code words
The top nibble tags each word:

| Tag | Meaning | Example |
|---|---|---|
| `0x8…` | Opcode (index into the table) | `0x80000020` = `BRANCH_Z` |
| `0x4…` | Variable reference | `0x40000006` = var 6 (`VAR_RIDECLOSED`) |
| `0x2…` | Code label (word offset) | `0x20000007` → `L0007` |
| `0x1…` | String-table reference | `0x10000000` = string 0 |
| none | Immediate integer | `500` |

Each opcode is followed by exactly the number of operands given in the table.

### Instruction set (106 opcodes; full list in `re/rsse_opcodes.csv`)
- **Control:** `NOP`, `JSR`, `RETURN`, `BRANCH`, `BRANCH_Z/NZ/PV/NV` (after `TEST`/`CMP`), `END`
- **Scheduling:** `ENDSLICE` (yield until next tick), `WAIT`, `WAITABS`, `WAIT4ANIM`, `CRIT_LOCK`/`CRIT_UNLOCK`
- **Data:** `COPY`, `ADD`, `SUB`, `MULT`, `DIV`, `MOD`, `RAND`, `PUSH`/`POP`, `TEST`, `CMP`, `SETLV`
- **Objects:** `ADDOBJ(_EXT)`, `KILLOBJ`, `FADEOBJ`, `SETOBJPARAM`, `SPAWNCHILD`, `REMOVECHILD`, `GET/SETVARINCHILD`, `GET/SETVARINPARENT`, `GET/SETREMOTEVAR`, `FINDSCRIPTRAND`
- **Animation:** `TRIGANIM`, `WAITANIM`, `LOOPANIM`, `TRIGWAITANIM`, `GETANIM`, `FLUSHANIM`, `TRIGANIMSPEED`, plus `_CH` per-channel variants
- **Guests on rides:** `ADDHEAD`/`DELHEAD`, `LIMBO`/`UNLIMBO`/`INLIMBO`/`LIMBOSPACE`, `BOUNCE*`, `WALKON`/`WALKOFF`/`WALKGET`/`WALKST_FLOAT`…, `TOUR`, `BUMP`, `COAST`, `TURBO`, `HOP`, `HUSH`
- **Effects:** `EVENT(_EXT)`, `SPAWNSOUND`, `STARTSCREAM`/`STOPSCREAM`/`SINGLESCREAM`/`SCREAMLEVEL`, `SETLIGHT`/`COLOURLIGHT`/`ENABLELIGHT`/`DISABLELIGHT`, `SPARK`, `REPAIREFFECT`, `SETREVERB`, `DIPMUSIC`, `GETCUSTPTCLCODE`
- **Time:** `GETTIME`, `SETTIMER`/`GETTIMER`, `YEAR`, `MONTH`, `DAY`, `HOUR`, `MIN`, `SEC`
- **Debug:** `NAME`, `DBGMSG`

84 of the 106 are used by shipped scripts. The unused ones include `PUSH`/`POP`, `MULT`, the light controls, and the calendar opcodes.

### Engine ↔ script contract: variables
Rides export well-known variables that the C++ engine reads and writes, e.g. for the Bouncy Dino:
`VAR_LETMEON`, `VAR_LETMEOFF`, `VAR_CAPACITY`, `VAR_DURATION`, `VAR_BREAKSTAT`, `VAR_ONRIDE`, `VAR_RIDECLOSED`, `VAR_BROKEN`, `VAR_WORN`, `VAR_RUNNING`, `VAR_PAD`, `VAR_PARAM`, `VAR_TEMP`, `VAR_SCREAMING`.
Features use smaller sets (the litter bin only has `VAR_TRIGGER`).

## Example: the litter bin (`bfbin.RSE`)
```
    NAME            str[0] "Litter Bin Feature"
    WAITANIM        0, 0
    ADDOBJ          5, 65535, 93, 10
L0010:
    ENDSLICE                        ; yield
    TEST            VAR_TRIGGER     ; did the engine signal litter?
    BRANCH_Z        L0010
    WAITANIM        5, 0            ; play the bin animation
    WAITANIM        5, 1
    COPY            VAR_TRIGGER, 0
    BRANCH          L0010
```

## Open questions
- Header fields `0x0C`–`0x1C` (stack size? priority? object count?)
- Exact semantics of each opcode. Read them per case in `RSSE_ExecuteInstruction` and document here
- How the engine binds `VAR_*` names to its own state (by name lookup at load?)
- `SPSC` "sprite scripts" in the save file: are they the same VM or a separate one?
