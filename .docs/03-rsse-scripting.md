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
0x0C  stack size         words (Bouncy Dino: 3)
0x10  slice budget       instructions per time slice (50 in every script)
0x14  limbo slots
0x18  rider slots        (Bouncy Dino: 10)
0x1C  walker slots       (see "Header, fully mapped" below)
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

## Header, fully mapped (from `RSSE_LoadScriptFile`)
| Offset | Context field | Meaning |
|---|---|---|
| `0x04` | | version, must equal `0x00010F51` (checked against `DAT_00879190`) |
| `0x08` | `+0x8C` → allocates vars at `+0x1C` | variable count |
| `0x0C` | `+0x54` → allocates `+0x20` | stack size in words (shared by `JSR`/`PUSH` from the top, `HUSH` from the bottom) |
| `0x10` | `+0x94` | instruction budget per time slice (50 in every shipped script) |
| `0x14` | `+0x58` → allocates `+0x24` | **limbo** slots, 8 bytes each `{guest, release_time}` |
| `0x18` | `+0x64` → allocates `+0x28` | **rider** slots, 16 bytes each `{guest, node, release_time, start_time}` (Bouncy Dino: 10) |
| `0x1C` | `+0x7C` → allocates `+0x2C` | third table, 32-byte entries (walkers, used by `WALK*`) |
| `0x20` | | 16 pad bytes, skipped |
| `0x30` | `+0x50` | code length in words |

## Runtime model
**Scheduler** (`RSSE_Tick`, `0x5516b0`), once per game tick:
- `tick++`. A script runs if `(script_id ^ tick) & 7 == 0`, i.e. **every 8th tick, staggered by id**, or **every tick** if its `TURBO` byte (`+0xB8`) is set.
- Each run gets `budget = header[0x10]` instructions (counter at `+0x98`). An instruction executed while the global critical-section flag is set (`CRIT_LOCK`) **doesn't consume budget**. `ENDSLICE` and any wait zero the counter, which yields.
- A script stops for good when `pc < 0` (`END`, or any error sets `pc = −10000`).
- Afterwards the script's speed/wear value (`+0xC0`) is pushed into its animation, track and child objects.

All running scripts are on a doubly linked list (head `DAT_008791b0`, count `DAT_008791ac`), found by id with `RSSE_FindScriptById`.

**Script context** (0xF4 bytes) fields known so far:
`+0x00/+0x04` list next/prev · `+0x08` script id · `+0x0C` child script id · `+0x18` code · `+0x1C` vars · `+0x20` stack · `+0x24` limbo table · `+0x28` rider table · `+0x2C` walker table · `+0x3C` pc · `+0x40` stack pointer (down) · `+0x44` heap pointer (up) · `+0x48` **result register** · `+0x50` code length · `+0x54` stack size · `+0x58/+0x60` limbo capacity/count · `+0x64/+0x6C` rider capacity/count · `+0x70` first rider node · `+0x74` `NAME` string · `+0x8C` var count · `+0x94/+0x98` budget/remaining · `+0x9C` sound · `+0xA0` wait deadline · `+0xA4` animation end time · `+0xA8` current looping animation · `+0xB0` list of added objects · `+0xB8` turbo · `+0xC0` speed/wear (short) · `+0xC8` owning object (index into `DAT_007a4610`)

## Instruction semantics (from `RSSE_ExecuteInstruction`)
**Operands.** A *source* is a variable (`0x4…`) or an immediate, which is **sign-extended from its low 16 bits**. A *destination* must be a variable, otherwise the instruction is ignored. Labels must carry the `0x2…` tag, and a branch to a non-label ends the script. Most instructions also write their result to the **result register** (`+0x48`), which the conditional branches test.

| Op | Semantics |
|---|---|
| `NOP` | nothing |
| `CRIT_LOCK` / `CRIT_UNLOCK` | set / clear the global critical-section flag (no budget consumed while set). `CRIT_UNLOCK` also yields |
| `COPY d, s` | d = s; R = d |
| `SETLV x` | R = raw word x |
| `ADD d, s` | d += s; R = d |
| `SUB d, a, b` / `MULT` / `DIV` / `MOD` | d = a op b; R = d. Division or modulo by 0 gives 0 |
| `RAND d, n` | d = R = \|rng() >> 1\| mod (n+1), i.e. 0..n using the world RNG (`World_Rand`) |
| `TEST v` | R = v |
| `CMP a, b` | R = a − b |
| `BRANCH L` | goto L |
| `BRANCH_Z/NZ/NV/PV L` | goto L if R == 0 / ≠ 0 / < 0 / > 0 |
| `JSR L` / `RETURN` | push `pc` (tagged as label) / pop and jump |
| `PUSH s` / `POP d` | stack from the top; R = value |
| `HUSH s` / `HOP d` | second LIFO from the bottom of the same block ("heap") |
| `END` | pc = −10000 (script terminates) |
| `ENDSLICE` | yield until the script's next turn |
| `WAIT n` | sleep *n* time units scaled by game speed. Implemented by rewinding `pc` onto itself and yielding until the deadline (`+0xA0`) passes |
| `WAITABS n` | same, unscaled |
| `WAIT4ANIM` | sleep until the current animation's end time (`+0xA4`) |
| `GETTIME d` | d = R = game clock |
| `TURBO n` | run every tick instead of every 8th (`+0xB8`) |
| `NAME s` | record name string (debug) |
| `DBGMSG s` | skip; compiled out in retail |
| `TRIGANIM ch, anim, d` | start animation *anim* on channel *ch* of the owner's mesh; d = R = duration − 300 ms (min 300); records end time for `WAIT4ANIM` |
| `WAITANIM ch, anim` | play an animation and wait for it to finish |
| `LOOPANIM ch, anim` | loop an animation (no-op if already looping it) |
| `TRIGWAITANIM`, `GETANIM`, `TRIGANIMSPEED`, `FLUSHANIM`, `*_CH` | variants: trigger+wait, query, speed, stop; per-channel forms |
| `BOUNCE guest, secs` | put *guest* into a free **rider** slot until now + secs×1000. R = 1 if seated, 0 if full |
| `UNBOUNCE`, `FORCEUNBOUNCE`, `BOUNCING d` | release riders whose time is up / release unconditionally / count riders |
| `BOUNCESETNODE`, `BOUNCESETBASE` | configure where riders are positioned |
| `LIMBO guest, secs` | hold *guest* invisibly in a **limbo** slot until now + secs×1000. R = 1/0 |
| `UNLIMBO`, `FORCEUNLIMBO`, `INLIMBO`, `LIMBOSPACE` | release due / release all / count / free slots |
| `ADDOBJ…` / `KILLOBJ` / `FADEOBJ` / `SETOBJPARAM` | create, remove, fade or configure a sub-object (prop, particle, sound), kept on the list at `+0xB0` |
| `SPAWNCHILD s` / `SPAWNSOUND s` / `REMOVECHILD` | load and start another `.RSE` as a child script (`+0x0C`) / kill it |
| `GET/SETVARINCHILD`, `GET/SETVARINPARENT`, `GET/SETREMOTEVAR` | read/write variables of the child, the parent, or any script by id |
| `FINDSCRIPTRAND` | pick a random live script matching criteria |
| `EVENT a, b, c` / `EVENT_EXT` | `RSSE_TriggerCustomEvent(ctx, a, b, c, 1000)` (guest reactions, messages) |
| `ADDHEAD` / `DELHEAD` | attach/detach a guest "head" (visible rider) to the owner's mesh |
| `WALKON`, `WALKOFF`, `WALKGET`, `WALKST_FLOAT`, `WALKFLOATSTAT`, `WALKFLOATSTOP` | walk-through attractions: guests follow walk nodes (`RSSE_SetupWalkNodes`) |
| `TOUR`, `BUMP`, `COAST` | ride-type drivers: tour vehicles, bumper cars, coaster cars (call into the track/coaster code) |
| `ENABLELIGHT`/`DISABLELIGHT`/`SETLIGHT`/`COLOURLIGHT` | control the owner's mesh lights |
| `STARTSCREAM`/`STOPSCREAM`/`SINGLESCREAM`/`SCREAMLEVEL` | rider scream sounds |
| `SPARK`, `REPAIREFFECT`, `GETCUSTPTCLCODE` | particle effects |
| `SETREVERB`, `DIPMUSIC` | audio environment |
| `SETTIMER`/`GETTIMER`, `YEAR`…`SEC` | per-script timer; real-time clock fields (via the C runtime's `localtime`) |

## Open questions
- How the engine binds `VAR_*` names to its own state (name lookup at load, or fixed indices per ride type?)
- Exact arguments of the ride drivers (`TOUR`, `BUMP`, `COAST`) and the `WALK*` family
- `SPSC` "sprite scripts" in the save file: are they the same VM or a separate one?
