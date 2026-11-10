# The loader: tpw.exe (2026-11-10)

`build\tpw.exe` runs the game from the ports without running any of `tp.exe`'s code (plan step 4, survey option (a): the original data layout). It reads the user's decrypted `tp.exe` as a file. The repo never contains the exe or links against it. Source: `src/loader/loader.cpp`, `tools/gen_loader_slots.py`.

## How it starts

1. **The range.** `tpw.exe` is fixed at `0x10000`, with no relocations. Its uninitialised section `.game` (`g_game`, after its code) spans `0x400000-0xfc6000`, so Windows maps the range as part of the image before any code runs. No DLL, heap, stack or file mapping can go there. `LoaderEntry` checks this first.
   - Limit: the linker puts the code first, so tpw.exe's code must stay under `0x3ef000`. It is `0x35a000` now, with `/OPT:REF`. If it grows past that, the loader stops with a message. The fix then: move the game's code into a DLL that the loader loads after the copy.
2. **The copy.** `LoaderEntry` (`/ENTRY`) runs before tpw.exe's C runtime. It does these steps in order:
   - Copies the headers and sections to their addresses, then checks that every hooked function starts with the bytes `hooks.cpp` expects.
   - Zeroes the ranges of the globals that `tpw.exe` has as objects of its own (`.docs/09-globals.md`).
   - Fills the import table (step 4).
   - Rewrites the code slots (step 3).
   - Sets the protection. `.text` and `cseg` become read-only data, which DEP will not execute. Writable sections become read-write. The rest becomes read-only or no-access.
   - Installs the fault handler. Then it starts tpw.exe's C runtime.
3. **Code slots.** At build time, `gen_loader_slots.py` lists every dword in the exe's data sections that holds the address of one of these:
   - a hooked function
   - an adjustor thunk to a hooked function (the loader writes an equivalent of the thunk)
   - the empty `Debug_Printf` `0x5da3c0` (the loader supplies an empty function)

   The loader points each slot at the port. With today's hooks: 4,155 hooked slots, 83 `Debug_Printf` slots, 39 thunks, 4,277 in all. That is the survey's 4,262 plus 15 slots whose targets the survey did not list. `tpw_tests.exe tp.exe` checks the table against its own scan of the exe. Every other code address in the data stays the original's. That includes the C runtime's initialiser tables, which `StaticInit_Run` reads by address.
4. **Imports.** The loader loads each DLL the same way `tp.exe` does: from the exe's folder first, so QMixer, the `wea*.dll` files and a DirectDraw wrapper (dgVoodoo, DDrawCompat) are found, then from the system. A missing DLL or function gets a stub (see "Missing DLLs" below). Each address goes to the copy's slot and to the same slot of `gImports`, the ports' table (`09-globals.md`, "Imports").
5. **WinMain** does the same as the original's `mainCRTStartup` `0x67ee10`, by the game's addresses:
   - FPU control word `0x27f` (PC53, for the initialisers; Direct3D later sets the game's `0x7f`)
   - `GetVersion` into its globals
   - `_heap_init` and `_mtinit`, then `0x687230` and `0x68a160`
   - the command line, the environment block, `_setargv` and `_setenvp`
   - `_cinit`: the FP-init pointer, `CRT_initterm` on the C table `0x72e0ec-0x72e0fc`, then `StaticInit_Run` (the C++ initialisers)
   - the command-line tail, `GetStartupInfoA`, then `Game_WinMain` `0x45a960` with tpw.exe's own module handle (the game only uses it for its window class, its window and DirectInput)
   - `exit` `0x67f920`

   It leaves out the original's SEH frame and `_XcptFilter`.

## Calls into the copy

A call to an address in the copy faults: it is a DEP execute fault. The vectored handler sends the call to the port for that address. Most by-address calls in the ports skip this: `AddrCalls` (`Cd`/`Th`/`St`) asks the loader's resolver (`g_addressTarget`, which is null in the hook DLL and the tests). The resolver handles these cases:

- a hooked function: the port
- `0x5da3c0`: the empty function
- an adjustor thunk: the loader's equivalent of it
- a plain `JMP rel32`: the loader follows it (up to 4 jumps)

Any other address is a **trap**. A message box names the original address and the return address of the call. Then the process exits with code 3. Use `build\tpw.map` for tpw.exe's addresses.

### Known traps (original addresses with no port)

- **Start-up, C library.** Agent B is porting this range. The first trap will be `0x6826b0` (`_heap_init`). After it come `0x6823e0`, `0x687230`, `0x68a160`, `0x689b00`, `0x689850`, the C init table's `0x67a7f0`, `0x67e750` and `0x687c20`, and `exit` `0x67f920` when the game quits (the exit tables add `0x67e810`, `0x68b9f0` and `0x687c40`).
- **By-address calls in `src/game` with no port.** `gen_loader_slots.py --stats` lists them: 47 addresses.
  - Game code: `0x4c9aa0` (`advisorscoring.cpp`) and `0x6792f0`/`0x679350` (`rasterdraw.cpp`).
  - Jumps, which resolve if their targets are hooked: `0x447d90`, `0x44e550` and `0x575bf0`.
  - The other 41 are C library functions, `0x67a520-0x6d76e0`.
- **Not counted by the scan:**
  - the stack-`this` thunk `0x5791b0`
  - the exception-handling funclets and the compiler's exit thunks (only the original's code uses these)
  - the 1,946 + 3,884 + 5 code addresses in `.data`/`.rdata`/`uva_data` whose targets have no port, and the C library's 149 slots. These matter only if a port calls through them.

## Missing DLLs

Each import of a missing DLL or function points at its own no-access address. When something calls the import:

- **A C++ member** (a mangled name): the handler returns 0. A constructor returns `this`. The handler pops the arguments that the undecorated name gives (`dbghelp`'s `UnDecorateSymbolName`). The callers should take 0 as a failure, the same as when the online service is down. This is not checked for each call site.
- **Any other import, or an import by ordinal:** a trap that names `dll!function`.

The eight EA Online DLLs are all `__thiscall` classes:

| DLL | Imports | Class |
|---|---:|---|
| `weavoter.dll` | 6 | `Vote` |
| `weachatr.dll` | 26 | `Chat` |
| `weaauthr.dll` | 5 | `Auth` |
| `weanewsr.dll` | 9 | `News` |
| `weacityr.dll` | 32 | `City` and its lists |
| `weauploadr.dll` | 8 | `Upload` |
| `wearasr.dll` | 8 | `Dialup` (its constructor runs in a static initialiser, its destructor at exit) |
| `weamailr.dll` | 7 | `Mail` |

## Known limits

- **Slow by-address calls.** These go through the fault handler on every call, which costs microseconds each time. It affects by-address calls that do not use `AddrCalls` (environment pointers, casts) and calls through unrewritten pointers. One example is the window procedure `0x46bcb0`, which the ports register by address.
- **The `--tp-exe=` option.** The loader cuts it from `GetCommandLineA`'s buffer before anything reads it. `GetCommandLineW` still has it.
- **DLLs that a wrapper loads later.** If a DirectDraw wrapper loads more DLLs at run time, Windows looks for them in `build\` and the system folders before the current folder.

## How to run it

`tpw.exe [--tp-exe="path\to\tp.exe"] [the game's own options]`. The default is `tp.exe` next to `tpw.exe`. Run it from the game folder, which the game uses as its data folder.

Use the local test copy (`.data\local\TPW`, from `setup-local.cmd`). Save this as `.data\local\run-standalone.cmd`:

```bat
@echo off
rem run-standalone.cmd: tpw.exe (no tp.exe code runs) in the test copy .data\local\TPW, windowed through dgVoodoo.
setlocal
set L=%~dp0
set G=%L%TPW
set OUT=%L%out
if not exist "%G%\tp.exe" (echo Run setup-local.cmd first. & pause & exit /b 1)
md "%OUT%" 2>nul
cd /d "%G%"
rem No hook DLL: tpw.exe loads DINPUT.dll from this folder first.
del /q dinput.dll tpw.ini tpw.log 2>nul
copy /y "%L%dgVoodoo-fps60.conf" "%G%\dgVoodoo.conf" >nul
echo Starting Theme Park World standalone (build\tpw.exe). A "no port for ..." box names the first unported address.
start "" /wait "%L%..\..\build\tpw.exe" --tp-exe="%G%\tp.exe"
set RC=%ERRORLEVEL%
echo %DATE% %TIME% local standalone exit=%RC% >> "%OUT%\results.txt"
if exist enginedebug.txt copy /y enginedebug.txt "%OUT%\enginedebug-standalone.txt" >nul
echo Exit code %RC% (0 = clean; 3 = the loader stopped: a trap or a missing DLL function). Logged to .data\local\out.
pause
```

Until agent B's C library ports are merged, the run stops at the first trap, `0x6826b0`.
