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
| **Startup trace** (test tooling) | `trace.cpp` | No; only **import-table slots** are redirected through loggers, which then call the real API | Diagnosing why the game fails to start or crashes |
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

1. The build time and every hook's status (`hooked`, `off (tpw.ini)`, or `SKIPPED` if the exe's bytes differ).
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
- the registry key, under WOW6432Node
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
  exit code 1, no crash. The trace was added to find out where. Next: the same with `2-dll-passthrough`.
