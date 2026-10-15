# In-game testing

The fncmp benches prove each reimplemented function matches the original on its own. Only a
run inside the real `tp.exe` shows that the hooks are wired correctly and the game still plays.
This page covers the test-only parts of the hook DLL and the sandbox used to run it.

## What the hook DLL contains

`build\dinput.dll` has two kinds of code, kept in separate folders and libraries.

| Part | Files | Changes game behaviour? | Purpose |
|---|---|---|---|
| Hook installer | `src/hook/`: `dllmain.cpp`, `patch.cpp`, `hooks.cpp` | **Yes**: a 5-byte `jmp` to our reimplementation, only after the original prologue bytes are checked | The decompilation itself (with `src/game/`) |
| dinput proxy | `src/hook/`: `dinput_proxy.cpp`, `dinput.def` | No; forwards `DirectInputCreateA` to the system DLL | How the DLL gets loaded (ddraw.dll is taken by dgVoodoo) |
| **Startup trace** (test tooling) | `src/testing/trace.cpp` | No; **import-table slots** are redirected through loggers, which then call the real API. One game function is replaced: the retail build's empty debug print `Debug_Printf` (0x5da3c0, a bare `RET`) becomes a logger that preserves every register and flag | Diagnosing why the game fails to start or crashes, from the game's own error messages |
| **FPU watcher** (test tooling) | `src/testing/fpu_watch.cpp` | No; reads the main thread's FPU control word once a second | Checking the 24-bit x87 precision our float ports assume |
| **Log writer** (test tooling) | `src/testing/log.cpp` | No | `tpw.log` next to the exe, flushed per line so it survives a crash |

**How the separation is enforced** (since 2026-10-15):

- `src/testing/` builds as its own library, `tpw_testing`. It links into the hook DLL only. The game
  library (`tpw_game`), the tests and fncmp never see it.
- The DLL reaches it through one header, `src/testing/testing.h`. Every call in `dllmain.cpp` sits
  under `#ifdef TPW_TEST_TOOLING`.
- `cmake -DTPW_TEST_TOOLING=OFF` builds the DLL as the reimplementation alone: no log, no trace,
  no watcher, KERNEL32 as its only import. The default is ON, for in-game testing.
- CMake refuses to configure if any file under `src/game/` mentions `testing/`, `tpw::testing` or
  `TPW_TEST_TOOLING`.
- With tooling on, the first line of `tpw.log` says "test tooling on".

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

### Local run log

- 2026-10-15, `2-dll-passthrough`: first try exited 1 at once. A windowless `tp.exe` left over from an
  earlier launch still held the `TPWMutex` single-instance mutex (`CreateMutexA` last error 183).
- 2026-10-15, `2-dll-passthrough` again: the park loads and the music plays, but the picture freezes.
  Pressing Print Screen advances it a frame. At 122 s it crashed with the **same crash as the sandbox**,
  so it isn't the sandbox's vGPU, and it isn't the AMD driver either. That earlier blame was wrong:
  `0x7463xxxx` just happened to sit inside `amdxx32.dll`. The cause is a **stack buffer overflow in the
  original game** (sic):
  - The DirectDraw error recorder 0x56af20 runs `sprintf(buf[100], "%s", DDErrorString(hr))`
    (0x56a770 maps the HRESULT to text). It does this when the flags word at 0x8bd508 has bit
    0x1000000 set and the severity argument is 3.
  - `DDERR_SURFACELOST`'s description is 165 bytes ("Access to this surface is being refused ... should
    have Restore called on it."), so it overwrites the return address with the text "ject". EIP
    0x7463656A is "ject"; the stack at the crash starts "ntin".
  - The root is lost DirectDraw surfaces that the game never restores: that is also the freeze. The
    overflow then turns the lost surface into a crash. Neither function is hooked yet. When ported,
    the overflow has to be kept (or documented as a deliberate divergence).
- 2026-10-15, `2-dll-passthrough` uncapped, take 2: the UI responds (menus, "Create new"), but game
  time does not advance. The camera and the park's people stay frozen, and each Print Screen press
  nudges time forward. Print Screen stalls one frame, so that frame gets a real time step.
- 2026-10-15, passthrough with dgVoodoo `FPSLimit = 60`: **plays normally**. Built rides and quit from
  the menu after 89 s with exit 0. So on a modern PC, uncapped frames finish so fast that the game's
  per-frame time step rounds to nothing. It was written for 1999 frame times; the slower sandbox
  hid it. The exact code is not yet found; the 0x5f5f10 ms timer itself is exact integer
  arithmetic. When found, it is another bug-for-bug item. `run-local.cmd` now always uses the
  capped config (`.data\local\dgVoodoo-fps60.conf`).
- 2026-10-15, `3-dll-hooked` (954/954 hooks, 60 fps cap): **first fully hooked run plays**. You can build
  and play. Two problems:
  - Alt-tab crashed it at 61 s with the same original-game overflow (0x56af20 on
    `DDERR_SURFACELOST`, EIP "ject"). Not a hook. Avoid alt-tab until surface loss is handled; Print
    Screen is safe.
  - **Graphics differ: a hooking bug.** A newly built entrance path drew as solid black tiles, and
    the ground texture behind a ride came out stretched up and to the left. Suspect:
    `TerrainBlend_Tile` (0x55f780), which builds the ground textures including the path overlay. It
    was verified only against our own Python model (`tools/lnd.py`), never against the original
    MMX code, so a mistake shared by both would pass. Next run: all hooks except that one
    (`4-hooked-no-terrainblend`; `.data\local\ini\<label>.ini` becomes `tpw.ini`).
- 2026-10-15, all hooks except `TerrainBlend_Tile`: **same glitches**, so it is cleared. The game's
  `enginedebug.txt` says "Using procedural textures: 0": this setup never calls the blender, and
  the ground uses the pre-made wavelet textures. The glitches are black ground under ride queues
  and entrance paths, plus green and brown streaks fanning out from rides. Next: passthrough on the
  same saved park (baseline), then group bisection: `5-hooked-no-math` (106 maths, model and
  animation hooks off) and `6-hooked-no-gfx` (332 surface, pixel-format, codec, drawing and sprite
  hooks off). `run-local.cmd` now also keeps `enginedebug-<label>.txt`.
- 2026-10-15, bisection round 0: `1-original` clean, hooked with the 106 maths hooks off **clean**, hooked
  with the 332 graphics hooks off **glitched**. So the culprit is in the maths group. Round 1 splits it
  into four groups, using two runs that each switch off two groups:
  - G1 `Matrix_` (28)
  - G2 `Vec3_`/`Quat_`/`Angle_`/`Float_`/`Curve_`/`Path_PercentAt`/`FpuControl_` (26)
  - G3 `Motion_`/`Animator_` (24)
  - G4 `Model_`/`World_`/`Object_`/`Thing*`/`Ride*` (28)

  Run `round1-a` switches off G1 and G2; `round1-b` switches off G1 and G3. The results map to a group:

  | round1-a | round1-b | Culprit |
  |---|---|---|
  | clean | clean | G1 |
  | clean | glitch | G2 |
  | glitch | clean | G3 |
  | glitch | glitch | G4 |
- 2026-10-15, round 1: `round1-a` clean, `round1-b` clean, so the culprit is in **G1, the 28 `Matrix_` hooks**.
  Their aliasing behaviour matches the original: `Matrix_Mul` reads `b` live, and `Matrix_PreMul`
  copies `b` and reads `a` live, as 0x4702d0 does. Round 2 splits the 28 in registry order into
  groups of 7: G1 Identity..RotateY4, G2 RotateZ4..ScaleAxesV, G3 ScaleAxes..LUDecompose,
  G4 Invert..PreMul33. Same pattern: `round2-a` switches off G1+G2, `round2-b` switches off G1+G3.
