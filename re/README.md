# re/: reverse-engineering knowledge (tracked)

The Ghidra database (`_re/ghidra`) is binary, large and git-ignored. **This folder is the source of truth** for everything we've learned about `tp.exe`, in diff-friendly text. Anyone with the exe can rebuild a fully annotated Ghidra project from it.

| File | Contents |
|---|---|
| `symbols.csv` | `address,name,kind,confidence,notes`, one row per named function or data label |
| `vtable_functions.txt` | Addresses of virtual methods / thunk targets that need `CreateFunctions.java` |
| `rsse_opcodes.csv` | The RSSE script VM instruction set (index, mnemonic, operand count), extracted from the exe's table at `0x765280` |

## Repairs (Ghidra analysis fixes)
Ghidra's auto-analysis misses some things. Apply these once after import, before `ApplySymbols`:

```
REM Virtual methods reachable only through vtables (801 functions Ghidra never created).
REM Regenerate the list with tools/vtables.py + tools/find_vtable_functions.py after big changes.
"<GHIDRA>\support\analyzeHeadless.bat" C:\git\Noklef\TPW\_re\ghidra TPW -process tp.exe -noanalysis ^
    -scriptPath C:\git\Noklef\TPW\tools -postScript CreateFunctions.java C:\git\Noklef\TPW\re\vtable_functions.txt
```

```
REM RSSE interpreter jump table: 106 opcode handlers (adds ~19 KB of code to RSSE_ExecuteInstruction)
"<GHIDRA>\support\analyzeHeadless.bat" C:\git\Noklef\TPW\_re\ghidra TPW -process tp.exe -noanalysis ^
    -scriptPath C:\git\Noklef\TPW\tools -postScript FixJumpTable.java 0x551d3d 0x5567d8 106 ^
    C:\git\Noklef\TPW\re\rsse_opcodes.csv RSSE_op_
```

## Naming conventions
- `Subsystem_Verb` in PascalCase: `Coasters_Load`, `Stream_Read`, `SaveGame_SaveModules`.
- `confidence`: `high` = proven by strings or call structure; `medium` = strongly suggested; `low` = guess, verify before relying on it.
- `notes` become Ghidra plate comments. Record *why* a name is believed, quirks, and original-code bugs.

## Applying to Ghidra
```
"<GHIDRA>\support\analyzeHeadless.bat" C:\git\Noklef\TPW\_re\ghidra TPW -process tp.exe -noanalysis ^
    -scriptPath C:\git\Noklef\TPW\tools -postScript ApplySymbols.java C:\git\Noklef\TPW\re\symbols.csv
```
The script is idempotent: re-run it after every edit to `symbols.csv`.

## Rebuilding from scratch
1. Import and analyse the exe and dump the decompile (see `tools/ExportDecomp.java`, `.docs/02-decompile-recompile-plan.md`).
2. Apply the **Repairs** above.
3. Run `ApplySymbols.java` as above.
4. Re-run `ExportDecomp.java` to get a decompile dump with the names applied. Pass addresses after the output dir to decompile only those functions (into `selected.c`).
