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
  in symbols.csv, and you say "unconfirmed" in your report. port.py does this by default: a new or renamed row is "low", a row that
  already had the name keeps its confidence. Pass `--known NAME...` only for a name the binary confirms (RTTI, a string, a library's own
  function name). A failed port.py run puts hooks.cpp, symbols.csv, CMakeLists.txt and unverified.csv back as they were; don't revert
  them by hand.
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

## Library functions (zlib, libjpeg)
tp.exe statically links zlib 1.1.3 and IJG libjpeg 6b (and a separate 6a build from 0x64c000). Their copies are not hand-ported: the hook rows
point the exe's addresses at the libraries built from the unmodified sources in third_party/ (tpw_zlib, tpw_jpeg6b, tpw_jpeg6a).
re/libmap.csv maps the exe's functions to their source functions (`checked` = yes once the listing's calls, constants and static tables were
compared with the source).

**Wiring.** One unit per library, src/game/lib/<library>.{h,cpp}, linked to its library privately. The header declares the functions
`extern "C"`, `__cdecl`, with the library's own types by struct tag only (`internal_state *`, `ct_data_s *`), so none of the library's
macros leak into hooks.cpp (deflate.h defines `Len`, `Code`, `Freq`; zutil.h `local`). The .cpp includes the library headers and
static_asserts the layouts the exe's code shares with the library. Spec lines use the library's own names and unit lib/<library>, and
port.py runs as for a port (`--known` the names: they are the library's). The library's `local` (static) functions get external names from
zlib's own switch `-Dlocal=` (zutil.h), set on tpw_zlib; the code is the same.

**What has to agree, and does (zlib):**
- Calling convention: cdecl throughout (ZEXPORT is empty without ZLIB_DLL; the exe's copies all end in a plain RET). convcheck checks each row.
- Layouts: the same types on 32-bit MSVC give the same structs: z_stream 0x38, deflate_state 0x16b8, inflate_huft 8, inflate_blocks_state
  0x40 (lib/zlib.cpp asserts the offsets the exe's code uses). inflate's own internal_state (0x18) and inflate_codes_state (0x1c) are
  private to inflate.c / infcodes.c, so they are checked against the exe's allocation sizes by hand and by the benches.
- Configuration: STDC (tr_static_init is empty, the tables come from trees.h / inffixed.h), no BUILDFIXED, no FASTEST, no DEBUG; the exe's
  copies have the same shape (tr_static_init is the shared bare RET at 0x631270, inflate_trees_fixed hands out the inffixed.h tables).
- Static tables: every one of them (trees.h, inffixed.h, cplens..cpdext, border, inflate_mask, configuration_table's values) is
  byte-for-byte the exe's. The library's code reads its own copies and the exe's code the exe's, so the two can run on one state.
- One thing in the exe is in no source: deflate_fast and deflate_slow run CPUID (leaf 0) in their prologue inside PUSHAD / POPAD and
  store its EAX in bflush's stack slot, which every path writes before reading. It is dead; the benches give the emulator fncmp's fake CPU
  so it can run it.

**Per function: exact, shim, or with its callers.** All of zlib 1.1.3 in the exe is the library's now, but the four functions below.
- *Exact* (a plain hook row): the function only computes, reads tables and writes through the state or its arguments, and calls only
  library functions (and the stream's zalloc / zfree / checkfn pointers). Its bench then compares the library's whole subtree with the
  exe's: adler32; trees.c but _tr_init; deflate.c's putShortMSB, flush_pending, lm_init, read_buf, longest_match, fill_window,
  deflate_stored / fast / slow, deflateEnd; huft_build; inflate_flush, inflate_blocks_reset / new / free, inflate_codes_new / free,
  inflateReset, inflateEnd.
- *Through a shim* (src/game/lib/zlib.cpp, zlib_inflate.cpp): the function leaves a pointer to one of the library's own tables, strings or
  functions in memory the exe's code also sees. third_party/config/zlib_shims.h (force-included into tpw_zlib) renames the library's
  function to zlib_<name>; the shim has the library's name, calls it and puts the exe's address back (the same contents at the exe's
  address), so the memory is exact and both the game and the bench driver can read it (the driver maps the exe's data at its addresses):
  - _tr_init, deflateReset, deflateInit2_: the state's static descriptors (&static_l_desc / d_desc / bl_desc) to the exe's 0x779ba8 /
    0x779bc0 / 0x779bd8.
  - inflate_blocks and inflate (through it): a fixed block's codes state holds &fixed_tl / &fixed_td (inflate_trees_fixed's) and
    pointers into them; mapped to the exe's tables (0x77a0a0 / 0x77b0a0) when inflate_blocks returns with it live, and before it is
    freed.
  - inflateInit2_: the blocks state's checkfn, the library's adler32, to the exe's (0x63ea00, hooked: in game it is the library's again).
  - strm->msg: every error path stores a string literal (inflate, inflate_blocks, inflate_trees_bits / dynamic, inflate_codes,
    inflate_fast, deflate and deflateInit2_ through z_errmsg); zlibshim::exe_message maps a new value to the exe's copy of the text.
- *Only with its callers*: inflate_codes / inflate_blocks hold pointers into the hufts huft_build made, so they were benched (and
  replaced) together, from inflate_blocks down; tests/fncmp/zlib_inflate.py benches the inflate side, zlib.py the deflate side.

**Masks.** huft_build fills its tables by copying a local inflate_huft whose union pad bytes (+2, +3 of each entry) are never set, so the
tables carry whatever was on the stack, different in the exe and the library. fncmp's Mask leaves those bytes of the hufts region out
of the comparison, and the emulator fails a case if the exe's code reads one: zlib_inflate.py's header has the proof from the listing that
inflate_codes / inflate_fast read exop, bits and base only. Every bench that builds tables masks them (huft_build, inflate_trees_bits /
dynamic, inflate_blocks, inflate).

**What stays the exe's, and why:**
- zcalloc (0x63e9e0) and zcfree (folded into 0x642990 with libjpeg's jpeg_free_small / large): replacing them alone would hand the exe's
  free a block from the DLL's heap. With deflateInit2_ and inflateInit2_ the library's they are unreachable from zlib.
- inflate_trees_fixed (0x64a450): Fn_64a450, a hand port already exact (it hands out the exe's fixed tables).
- bi_reverse (0x6404d0): Bits_Reverse, a hand port.
- deflateParams, deflateSetDictionary, inflateSetDictionary, inflateSync, compress, gzio, ...: not in the exe.
- A documented difference, not a shim: deflateInit2_ / inflateInit2_ store the library's zcalloc / zcfree when the caller left zalloc /
  zfree null, and allocate through them from the DLL's CRT heap. The game never takes that path (the zlib codec always passes
  ZlibCodec_ArenaAlloc and a no-op free), and the benches give ZALLOC / ZFREE.

**Benches** (tests/fncmp/zlib.py, zlib_inflate.py): the same regions-and-EAX comparison as a port, so the bench must compare everything the function can
write: the whole state, the pending buffer, the window / prev / head, the stream and its output. The state is laid out as the library
leaves it (descriptors pointing at the exe's static descriptors, which both sides read; d_buf and l_buf overlaid on pending_buf as
deflateInit2_ sets them; between calls nothing pending and no match pending except deflate_slow's lazy one), at small sizes (windowBits
9-10, memLevel 1-3) so the cases stay small and reach the slide, the block flushes and the hash wrap. A state the library never makes
can read past its tables, where the exe's neighbours and the library's differ: with many distant matches at a low memLevel, a block's
codes overrun d_buf in the pending_buf overlay (zlib 1.1.3's bug, CVE-2018-25032; the game's codec uses memLevel 9) and d_code then
reads past _dist_code, so the windowBits 13 case (TOO_FAR) holds one distant match only. Likewise deflate_fast's states keep lm_init's prev_length (a longer one makes
longest_match report a match at a stale match_start, distance 0 when that is strstart). The game's deflateInit2_ calls (0x6192f2,
0x61950c) pass memLevel 9 and windowBits 15, where the overrun cannot happen. The Bench's ranges must cover the exe's whole callee
subtree. fncmp.Bench serves the six zlib entry points (deflateInit2_, deflate, deflateEnd, inflateInit2_, inflate, inflateEnd) with
stand-ins for the codec's bench: a bench of those functions removes them from `B.emu.hooks` first. B.mutants mirrors third_party/, so
mutants edit the library's source: a surviving mutant there means a missing edge case, as for a port (NMAX, _tr_align's second block,
_tr_flush_block's stored-or-static tie, longest_match stopping at exactly nice_match and deflate_slow's TOO_FAR each needed one).

**libjpeg (next):** the same scheme, with more code pointers: every module's init stores its methods in the cinfo structs
(jpeg_std_error's error_exit, src->fill_input_buffer, the memory manager's alloc_small, ...), so most 6b functions only come exact with a
shim that maps the library's method addresses to the exe's, or a bench that compares through such a map. The leaves come first (jidct*,
jfdct*, the colour converters, the Huffman decoders' bit readers, jutils), which take their state by pointer and store no methods. The
exe's 6b memory manager backend is jmemansi.c, not jmemnobs.c: jpeg_mem_init answers 1000000 (0x642b20), jpeg_mem_available is
max_memory_to_use - already_allocated (0x6429a0), jpeg_open_backing_store a tmpfile() (0x6429c0); tpw_jpeg6b is built with jmemnobs.c, so
its build has to follow the exe first. libjpeg has no `-Dlocal=`: jmorecfg.h defines METHODDEF and LOCAL as `static` unconditionally,
and the modules reuse static names (start_pass, ...), so the statics cannot simply be made external. The way in that keeps the source
unmodified is a wrapper unit per module that `#include`s the module's .c in place of compiling it and exports its statics under
module-prefixed names (or as an address table) for the hook rows. 6a is a second copy (tpw_jpeg6a already renames its externals jpeg6a_*
through config/jpeg6a_prefix.h); it gets its own libmap rows, checked like the zlib rows, before any of it is replaced.

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
