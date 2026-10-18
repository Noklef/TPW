# Porting: the agent brief and the merge procedure

Theme Park World (1999) is decompiled OpenRCT2-style: each original function is reimplemented in C++ (src/game) and a hook DLL replaces the
original with its port. The rule is strict bug-for-bug parity, crashes included: quirks are reproduced and marked `// sic`, never fixed. Python
and C# are tooling only.

Porting runs as rounds: several agents, each in its own git worktree with its own address range, and a lead who merges their branches. The
first part of this file is the porting agent's brief; the last part is the lead's merge procedure. Each agent's prompt adds its worktree, branch,
range, open items, known skips and the paths of the two memory files it should read (port-lessons and token-efficiency).

## Your worktree (work ONLY here)
- You own one git worktree. Its path and branch, and your address range, are in your prompt. It already has `.data/nocd` (a junction) and its own
  `_re/` (text.asm and friends).
- Never edit or commit anything in the main tree outside your worktree, and never touch another agent's worktree.
- Commit on your branch only. Never push, rebase, merge or switch branches. The lead merges your branch.
- Commit identity (port.py does this for you): `git -c user.name=Noklef -c user.email=281545466+Noklef@users.noreply.github.com commit -q -F MSGFILE`.
  End messages with the `Co-Authored-By:` trailer your prompt gives. Never commit .data/, _re/ or build/. Stage explicit paths only.

## Hard rules
- NEVER run the game exe or anything that launches it. Reading its bytes from Python is fine (tools do this).
- Do not install anything or change system settings.
- Build ONLY through PowerShell: `cmd /c "<worktree>\build.bat"`. From Git Bash, build.bat hangs or silently does nothing. port.py and
  fncmp's mutants call build.bat from Python, which works.
- Never use `sleep N; cmd` chains: they are blocked. Run long jobs (port.py, mutants) in the background and wait for the completion
  notification. One port.py or mutant run at a time in your tree.
- Never weaken verification. Never exclude a failing case from a bench. If a port is wrong, fix the port, not the bench. If the bench is
  wrong, fix the bench's cases, never by dropping cases.
- Naming: never create bucket files (smallNN, tiny_*, misc*). Put each function in the module for its class/subject: an existing one, or a
  new module named for the subject. Benches are tests/fncmp/<module>.py. Keep names from re/symbols.csv. A name you invent is "low" confidence
  in symbols.csv (pass `--low NAME...` to port.py), and you say "unconfirmed" in your report.
- Leave out Debug_Printf asserts/prints.
- Tooling and test code live only in tools/ or src/testing (or tests/fncmp benches).
- **Never `sed -i` a tracked file from Git Bash**; use Edit/Write or integrate.py. Files are stored with LF and checked out CR LF
  (core.autocrlf, .gitattributes). A script that writes lines must keep exactly one CR LF per line: a CR CR LF once made git treat
  src/hook/hooks.cpp as binary, so every merge of it was a whole-file conflict. If a one-line change shows thousands of lines in
  `git diff --stat`, stop and fix the endings before committing.
- **No backslash escapes through heredoc Python** (`\r\n`, `\x10`): they get mangled. Use Edit/Write, or chr(13)/chr(10).
- **Killing a stuck run:** stop only your own processes, matched by your full worktree path in the command line, never by a short substring.
  All agents share the scratchpad path, so a short match once killed another agent's port.py.

## Read first (short)
- The two memory files your prompt names: port-lessons (the port.py cycle, bench ranges and pitfalls, calling conventions, driver envs; follow
  it) and token-efficiency (listing slices, small outputs, batching).
- The docstrings of tools/port.py, tools/fncmp.py (the header comment explains regions, recorders, fills, heap) and tools/wave.py.
- One or two existing benches as models, e.g. tests/fncmp/drives.py (imports via recorders), tests/fncmp/wrapsources.py (vtable recorders).

## The cycle
1. Find candidates in your range:
   - `python -I tools/wave.py list LO HI`: wave 0, functions whose callees are all ported. `VCALLS=1` also lists functions with virtual or
     indirect calls (benchable with recorder vtables).
   - `python -I tools/leafq.py LO HI` groups candidates by vtable, object and nearest symbol.
   - Port bottom-up. Once a batch lands, its callers may become wave 0.
2. Understand each function from the listing, not Ghidra alone:
   - `python -I tools/wave.py body ADDR` for the instructions; `wave.py dec ADDR` for Ghidra's decompilation (often wrong on calling
     conventions).
   - Count stack pushes; the original's `RET n` must match the declared stack bytes (convcheck verifies this).
   - Slice _re/text.asm numerically. Never use awk /A/,/B/ ranges: they dump 24 MB.
3. Batch 1-3 related functions per spec / bench / commit:
   - Small batches, because a run can be cut off at any point (a usage limit has stopped agents mid-run): whatever is not committed when that
     happens is lost or left for the next agent to re-verify. A stop should cost one or two functions, not a batch.
   - Spec lines: `Name|hexaddr|module/path|description`.
   - Run `python -I tools/port.py SPEC --bench tests/fncmp/X.py 3000 5 --both --msg MSGFILE --mut MUTFILE` in the background. Put `--both`
     before `--msg`. It integrates, builds, runs the hook tests, retcheck, convcheck and the bench, then commits and runs the mutants.
   - Time a new bench at about 60 cases first (`python -I tests/fncmp/X.py 60 5 --both`). Keep regions small.
4. Verification gate:
   - Each function needs at least 10 direct, matching, non-crash bench cases; port.py enforces this.
   - port.py also reruns the bench at the coverage gate's size (10 cases seed 5 + 400 seed 6) and fails the port if a function the bench
     compares gets no case there. The gate runs each bench at only 400 cases, so adding ops to a bench dilutes the older ones: pick with
     `r.deal(list)` (tools/fncmp.py: a shuffled deck, every item once per round) instead of `r.choice`, or weight a shared op up, so every
     function in the bench gets 5+ cases in 400. Check with `FNCMP_COVER=x.jsonl python -I tests/fncmp/B.py 400 6` and read `direct` for
     the bench's addresses (one missing from the dict means 0). Never fix it by dropping ops.
   - Use `--unverified "REASON"` only for a function that truly cannot be compared (hardware, real imports with no recorder possible), never
     to get past a failing bench.
5. Mutants:
   - About 8 targeted ones per module: bounds, flags, signedness, constants, off-by-one.
   - Use 1-2 for one-line leaves; skip trivial ctors/dtors.
   - Grep their output with `-iE "^caught|^survived|^build err"`. A survivor gets a better case, or is noted as equivalent with the reason.
6. Driver/harness:
   - When a port's env calls game code by raw address, or a game vtable must point at the ports, the native driver needs a change in
     tools/fncmp/driver.cpp (envs set with hooked(addr), kResolvedVtables).
   - You may make such changes in your tree. Commit them separately (message starts "fncmp:") and list them in your report: the lead merges
     every branch's driver edits.

## Notes that hold
- **Return values:** if the listing sets EAX on purpose before RET (e.g. `MOV EAX,[ESP+n]`, an out argument), or the function's vtable
  siblings return something, the port returns it and the bench uses ret 'a', even when no caller reads EAX today.
- **Skips:** functions that read uninitialised stack slots, or pass uninitialised stack bytes (struct padding) to recorded callees, cannot be
  compared. Skip them and list them. Don't spend more than a couple of looks.
- **Ghidra mis-splits:** a "function" starting with a conditional jump, or in the middle of another, is not one. Skip it.
- **x87:** the emulator (x87emu) runs x87 code; tools/x87sym.py helps read it. Prefer the portable functions first, then take x87 ones.
- **Shared modules:** adding a function to a module another branch also touches is fine. Never reformat or reorder existing code in it.
- **Cross-range blockers:** if a candidate is blocked only by an unported callee in another agent's range, list it in your report (address
  and blocker); don't port the blocker.
- **Slow benches:** fncmp counts every case left when its 600 s budget runs out as differing. A burst of differences at the end of a run is
  a timeout: time the bench rather than reading them as bugs. mapcell.py's `OPS=ADDR,...` filter helps aim mutants at one accessor.
- **Don't stall.** Commit each batch as soon as its port.py run passes. If one function or bench resists after about two looks (or ~20
  minutes), skip it, note it, and move on. If no job of yours is running, you should be working.
- **Saved work from a previous agent** may be handed to you as files in the scratchpad. Treat it as a draft: read it against the listing,
  rebench it with port.py like new work, and drop anything you can't verify. Main may have changed the same benches since: re-apply patches
  by hand, never over newer code.

## Scope and stop
- Port within your address range, bottom-up, a module at a time.
- Stop after about 30-40 committed functions, or earlier if your context is getting large. Then reply with a short report:
  - your commits (hash and subject);
  - the functions ported, per module, counted from the hooks rows your commits added (not from memory);
  - new modules created;
  - names you invented (unconfirmed);
  - fncmp/driver changes;
  - --unverified rows added and why;
  - mutant survivors and why they are equivalent;
  - anything skipped and why.
- The lead may send you a follow-up message to continue.

## Merging a branch (lead)
In the main tree, one branch at a time:
1. `git merge --no-ff --no-commit port/X`.
2. Resolve conflicts in the tables: `python -I tools/merge/keepboth.py FILES` first (add/add hunks; it refuses a hunk whose sides share a
   key), then `python -I tools/merge/unionmerge.py FILES` for the hunks left. Resolve anything else by hand. `git add` the resolved files.
3. `python -I tools/merge/check.py port/X`: no conflicts left, nothing either side had is lost (benches, hooks rows, symbols, CMake),
   no duplicates or lone CRs, the branch's added hooks count, then build, tpw_tests, retcheck, convcheck and every touched bench at 1000
   cases --both. It stops at the first failure. Check the agent's reported function count against its hooks count.
4. Commit the merge (message: the round, what it ported, anything not merged).
5. `python -I tools/coverage.py --check` (about 400 s; nothing else builds or edits tests/fncmp while it runs).
6. Fast-forward each worktree: `git -C .worktrees/X merge -q --ff-only main`. Start a fresh agent per round.
