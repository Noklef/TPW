# In-game testing

The fncmp benches prove each reimplemented function matches the original on its own. Only a
run inside the real `tp.exe` shows that the hooks are wired correctly and the game still plays.
This page covers the test-only parts of the hook DLL and the sandbox used to run it.

## What the hook DLL contains

`build\dinput.dll` (sources in `src/hook/`) has two kinds of code. Keep them apart.

| Part | Files | Changes game behaviour? | Purpose |
|---|---|---|---|
| Hook installer | `dllmain.cpp`, `patch.cpp`, `hooks.cpp` | **Yes**: a 5-byte `jmp` to our reimplementation, only after the original prologue bytes are checked | The decompilation itself |
| dinput proxy | `dinput_proxy.cpp`, `dinput.def` | No; forwards `DirectInputCreateA` to the system DLL | How the DLL gets loaded (ddraw.dll is taken by dgVoodoo) |
| **Startup trace** (test tooling) | `trace.cpp` | No; **import-table slots** are redirected through loggers, which then call the real API. One game function is replaced: the retail build's empty debug print `Debug_Printf` (0x5da3c0, a bare `RET`) becomes a logger that preserves every register and flag | Diagnosing why the game fails to start or crashes, from the game's own error messages |
| **FPU watcher** (test tooling) | `dllmain.cpp` (`watch_fpu`) | No; reads the main thread's FPU control word once a second | Checking the 24-bit x87 precision our float ports assume |
| Log writer | `log.cpp` | No | `tpw.log` next to the exe, flushed per line so it survives a crash |

The trace and the FPU watcher were written for the first sandbox runs (October 2026). They are
diagnostics, not part of the reimplementation. Never move game logic into them. Never
"fix" a game bug through an import redirect: game fixes belong in reimplemented functions,
behind the parity rules. When the project moves from hooking `tp.exe` to its own executable,
they go away with the proxy.

### tpw.ini switches

```
[tpw]
enabled=0      ; no hooks: the original game code runs (passthrough)
trace=0        ; no startup trace (on by default)
[hooks]
Name=0         ; one hook off, by its re/symbols.csv name (for bisecting a broken hook)
```

### Reading tpw.log

1. The DLL's link timestamp (hex, from its PE header, so it shows which build the game loaded) and every hook's status (`hooked`, `off (tpw.ini)`, or `SKIPPED` if the exe's bytes differ).
2. `startup trace: N imports redirected`.
3. Timestamped trace lines. Each carries `from XXXXXXXX`, the return address in `tp.exe`. Look that address up in `_re/text.asm` to find the game code that made the call. The trace logs:
   - message boxes, `PostQuitMessage`, `ExitProcess`
   - `CreateWindowExA`, `CreateMutexA`
   - registry opens and reads, with their values
   - failed `CreateFileA` calls (the first 300)
   - `LoadLibraryA`
   - the CD drive queries
   - DirectDraw and DirectSound creation
   - exceptions (the first 30, with registers)
   - `game debug from XXXXXXXX: ...`: the game's own debug messages (the first 2000), e.g. "Error: Unable to open text resource at %s"
4. `FPU control word XXXX: precision ...` each time the precision changes. Our float ports assume 24-bit.
5. `process exiting`.

## The sandbox (`.data/sandbox`, not in git)

A throwaway Windows Sandbox (`.data\sandbox\tpw.wsb`, double-click it). Everything is mapped
read-only except `.data\sandbox\out`. It maps the game install, the CD folder, the no-CD exe,
dgVoodoo (`.data\dgvoodoo-fresh-copy`), `build\` and `.data\sandbox\scripts`.

`scripts\setup.cmd` runs at logon and builds `C:\TPW`:

- the installed game
- the CD's `Data` files the install left out
- the no-CD exe
- dgVoodoo's `DDraw.dll`/`D3DImm.dll` and a windowed `dgVoodoo.conf`
- the registry key, under WOW6432Node, with `Language` = 0x809 (UK English: the only language text on this CD; 0x409 makes the game quit)
- crash dumps (WER LocalDumps to `C:\dumps`)

Windows Sandbox cannot mount the ISO, because it has no virtual disk provider.

Desktop scripts:

- `1-original`: no DLL.
- `2-dll-passthrough`: the DLL with `enabled=0`, so the original game plus the trace.
- `3-dll-hooked`: every hook.

`options\` switches the renderer (dgVoodoo through the GPU, through WARP, or none) and the game settings (default or `safemode.tcf`).

Results come back in `out\`:

- `results.txt`: mode, renderer, settings and exit code per run
- `tpw-<mode>.log`
- `config-<mode>.tcf`
- `setup.log`
- `dumps\`

The comparison that matters is passthrough against hooked, in the same setup. dgVoodoo sits
below everything reimplemented so far, so it cancels out. The one thing it could change is the
FPU precision, and the log records that.

## Status

- 2026-10-15: first run (`1-original`, dgVoodoo, default settings) quit during startup with
  exit code 1, no crash. The trace was added to find out where.
- 2026-10-15: the `2-dll-passthrough` run was really hooked: `run.cmd` wrote `echo enabled=0>> tpw.ini`,
  and cmd read the `0>>` as a redirect of stream 0. It now redirects first (`>>file echo enabled=0`).
  The trace showed the registry and mutex checks passing, `dialog.tcf` and `save\config.tcf` missing,
  then exit 1 at 1.0 s with no window created. WinMain (0x45a960) quits when 0x419710 fails: it loads
  the language text from `Data\Language\` and reports why through the empty debug print. That
  print is now logged. Next: a true passthrough run.
- 2026-10-15: true passthrough, same exit. The game's own message: "Error: Unable to set language
  to American". The registry `Language` was 0x409 (from the no-CD `.reg`), which the game maps to
  "American" (0x5c0e13; unknown ids fall back to 0x809 "English"). This European CD only has
  `Data\Language\English` (and `00000809.*`), so the sandbox now sets `Language` = 0x809.
- 2026-10-15: with `Language` = 0x809, passthrough starts and plays: window at 1.0 s, DirectDraw and
  DirectSound up, the park loads, the advisor speaks. Two sandbox problems, both outside the game code:
  - The mouse sticks to the window's top edge or slides down its side. The sandbox delivers the
    mouse over an RDP-style connection, which breaks DirectInput's relative mouse.
  - At 173 s it crashed: an access violation inside `amdxx32.dll` (the AMD driver, through the
    sandbox vGPU and dgVoodoo). The game had just printed "...this surface should have Restore
    called on it": a lost DirectDraw surface. The run was passthrough, so this is original code.
  So testing moves to the local scripts (below). The sandbox stays for clean-room checks.
- FPU precision, from the passthrough log: 53-bit at start, 24-bit (0x007F) after Direct3D starts
  (3.0 s). The main thread then flips to 53-bit (0x027F) around the advisor's speech (29 s, 40 s,
  71 s, 86 s) and back again. At 125 s it read 0x0C7F: 24-bit with **round toward zero**. Something
  (the game, QMixer or the driver) changes the control word mid-game. Float ports have to match the
  original under each of these modes, not only 24-bit round-to-nearest. To investigate.

## Local runs (`.data/local`, not in git)

The same three modes on the host PC, without the sandbox's mouse and vGPU problems.
`setup-local.cmd` builds a test copy in `.data\local\TPW`:

- the install
- the CD extras
- the no-CD exe
- a windowed dgVoodoo

It never touches the real install and writes no registry values. The host's existing
`Bullfrog Productions Ltd\Theme Park World` key already has `Language` = 0x809.
`1-original`, `2-dll-passthrough` and `3-dll-hooked` run it. They log to `.data\local\out`:

- `results.txt`
- `tpw-<mode>.log`
- `config-<mode>.tcf`

There are no crash dumps, because WER LocalDumps needs admin. The trace's exception log covers the gap.
