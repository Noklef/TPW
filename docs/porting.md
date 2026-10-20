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
1. Find candidates in your range. Progress is measured in bytes of game code ported, not in functions: most of what is left is a few
   hundred functions over 1 KB, so prefer the biggest candidate you can bench over a run of small leaves.
   - `SORT=size FPU=1 VCALLS=1 python -I tools/wave.py list LO HI`: wave 0, functions whose callees are all ported, the biggest first.
     `VCALLS=1` takes functions with virtual or indirect calls (benchable with recorder vtables), `FPU=1` x87 ones (the emulator runs them).
   - `python -I tools/wave.py unlock LO HI`: the unported callees that alone keep bigger functions out of wave 0, the most bytes first.
     Port those (or name them to the lead when they are another agent's) to open the big ones.
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
   - The coverage gate's "none" must not grow: every hook is compared somewhere. "indirect only" (compared through a caller's bench) is
     enough for a function without logic of its own (a thunk, a forwarder); one with its own decisions gets a direct case.
   - Use `--unverified "REASON"` only for a function that truly cannot be compared (hardware, real imports with no recorder possible), never
     to get past a failing bench.
5. Mutants, where they can find something the cases miss:
   - For code with decisions (branches, clamps, tie-breaks, loop bounds, signedness): about one targeted mutant per decision, at most
     about 8 per module.
   - None for straight-line code (accessors, stores, forwarding thunks, trivial ctors/dtors): the 1000-case comparison pins it.
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
- zcalloc (0x63e9e0): the library's alone would hand the exe's free a block from the DLL's heap. With deflateInit2_ and inflateInit2_
  the library's it is unreachable from zlib. zcfree is folded into 0x642990 with libjpeg's jpeg_free_small / large: that copy is now
  jmemansi.c's, through the exe's free (see libjpeg 6b below), the same code.
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

**libjpeg 6b (the decoder below the API is the library's):** the same scheme, with more code pointers: every module's init stores its
methods in the cinfo structs (src->fill_input_buffer, the memory manager's alloc_small, entropy->decode_mcu, ...), so the inits, and
everything else that stores methods, come exact only through the method-address shim below. Replaced so far (src/game/lib/jpeg6b.h lists
them with their addresses): jutils (jround_up, jcopy_sample_rows, jcopy_block_row, jzero_far), the integer IDCTs (jpeg_idct_islow, _ifast,
_4x4, _2x2, _1x1), the whole of jdcolor.c, jdmerge.c, jdsample.c, jddctmgr.c, jdhuff.c, jdphuff.c, jdcoefct.c, jdmainct.c, jdpostct.c,
jcomapi.c, jdmarker.c, jdinput.c and jquant1.c, jquant2.c but find_biggest_color_pop, and jmemansi.c's jpeg_get_small / large,
jpeg_free_small / large and jpeg_mem_available. tpw_jpeg6b is built with jmemansi.c, as the exe is (jpeg_mem_init answers 1000000 at
0x642b20, jpeg_mem_available is max_memory_to_use - already_allocated at 0x6429a0, jpeg_open_backing_store a tmpfile() at 0x6429c0).
- **Wrapper units.** libjpeg has no `-Dlocal=`: jmorecfg.h defines METHODDEF and LOCAL as `static` unconditionally, and the modules reuse
  static names (build_ycc_rgb_table is in jdcolor.c and jdmerge.c), so a module's statics are reached through
  third_party/wrap/jpeg-6b/<module>.c: it includes jinclude.h and jpeglib.h with JPEG_INTERNALS, redefines METHODDEF and LOCAL without
  `static`, `#define`s each static name to <module>_<name> and `#include`s "../../jpeg-6b/<module>.c" (by its relative path: a plain
  "<module>.c" finds the wrapper itself). A static whose name is also a struct member's (jddctmgr.c's start_pass, jdhuff.c's
  decode_mcu, jdcoefct.c's start_input_pass, consume_data, start_output_pass and decompress_data, jdmarker.c's reset_marker_reader,
  read_markers and read_restart_marker, jquant1.c's color_quantize) is `#define`d before the headers instead, so the member is renamed
  with it, consistently, in that unit alone (the layout is the same). A wrapper also pins the size of the module's private struct
  against the exe's allocation (jquant1's my_cquantizer 0x58, jquant2's 0x2c and its box 0x20) with a negative-array typedef. tpw_jpeg() in third_party/CMakeLists.txt compiles
  wrap/<tree>/<module>.c in the module's place when there is one (it looks at configure time; build.bat configures on every build); the
  source stays unmodified. The cinfo offsets the code shares with the exe are static_asserted in src/game/lib/jpeg6b.cpp (boolean is an
  unsigned char, as jconfig.vc has it and the exe's structs show).
- **The method-address shim** (src/game/lib/jpeg6b.cpp). The library's function that stores methods is built under another name (its
  wrapper `#define`s it to <module>_lib_<name>), and the hook row points the exe's address at a shim with the library's name (jinit_* keep
  theirs, statics are <module>_<name>): it calls the _lib_ function, then exe_method(field) on each method field that function stores.
  kExeMethods pairs each library method with the exe's address of the same method (re/libmap.csv); exe_method swaps a library address
  for the exe's and leaves anything else (NULL, another module's method already stored, such as the upsampler's upsample that
  start_pass_dpost passes through) alone, so the memory is exactly the exe's. Private method fields (jdmerge.c's upmethod, jdsample.c's
  methods[]) go through exe_method_at(slot) by offset. Methods the exe's linker folded map to the shared copy: jdcolor.c's empty
  start_pass_dcolor to the bare RET at 0x631270, jdcoefct.c's dummy_consume_data to the return 0 at 0x66f4d0. In the game every mapped
  address is hooked, so a call through it still runs the library's method (or the exe's own, where it stays the exe's: jpeg_idct_float).
  Mapped so far: jinit_color_deconverter, jinit_merged_upsampler, jinit_upsampler, jinit_inverse_dct and jddctmgr's start_pass,
  jinit_huff_decoder, jinit_phuff_decoder and start_pass_phuff_decoder, jinit_d_coef_controller and start_output_pass,
  jinit_d_main_controller and start_pass_main, jinit_d_post_controller and start_pass_dpost, jinit_marker_reader (with the private
  process_COM / process_APPn[16] by offset), jinit_input_controller, reset_input_controller and finish_input_pass (consume_input),
  jinit_1pass_quantizer and start_pass_1_quant, jinit_2pass_quantizer and start_pass_2_quant (jquant1's finish_pass_1_quant and
  jquant2's finish_pass2 are empty: the bare RET 0x631270).
- **jmemansi.c through the exe's heap.** The exe's jpeg_get_small / large (one copy, 0x642980) call its CRT's malloc (0x67dcd0) and
  jpeg_free_small / large (0x642990, the same copy as zlib's zcfree) its free (0x67ad90). The library's would allocate from the DLL's
  CRT, a different heap, while the exe's own code (zcfree, jmemmgr.c while it stays the exe's) frees the blocks on the exe's. So
  third_party/wrap/jpeg-6b/jmemansi.c `#define`s malloc and free (after the headers have declared the real ones) to jpeg6b.cpp's
  jpeg6b_exe_malloc / jpeg6b_exe_free, which call the exe's two through Jpeg6b_SetHeapEnv's env (the exe's by default; the bench driver
  gives fncmp's heap, the emulator hooks the same two addresses). malloc and free are never replaced (the front of the exe's heap, as
  GameHeap's Mem_Alloc / Mem_Free), so blocks from either side free on either side. jpeg_mem_available is the library's as it is. When
  jmemmgr.c becomes the library's, its allocations go to the exe's heap the same way.
- **Declarations next to jpeglib.h.** jpeg6b.h declares everything with its own struct tags where jpeglib.h's types are untagged
  typedefs (jpeg6b_component_info, jpeg6b_quant_tbl, jpeg6b_huff_tbl, jpeg6b_d_derived_tbl, jpeg6b_bitread_working_state); an enum
  parameter (J_BUF_MODE) is an int there and the shim casts it. jpeg6b.cpp includes both headers, so it renames jpeglib.h's own
  declarations of the two that differ in return type (jpeg_alloc_quant_table, jpeg_alloc_huff_table) away with #define / #undef around
  its include.
- **Build options.** wrap/jpeg-6b/jddctmgr.c is compiled /arch:IA32: start_pass builds the float IDCT's multiplier table with x87 code
  in the exe, and the SSE2 build rounds once from double, a last bit apart (third_party/CMakeLists.txt has the details).
- **Benches** (tests/fncmp/jpeg6b_*.py, one a module: _color, _merge, _sample, _dctmgr, _idct, _utils, _huff, _phuff, _coefct, _mainct,
  _postct, _comapi, _marker, _input, _quant1, _quant2, _memansi): cinfo, the module's private struct and the row arrays as regions; alloc_small (and alloc_large) is fncmp's ZALLOC
  stub (its cdecl (opaque, items, size) is alloc_small's (cinfo, JPOOL_IMAGE, size)); every other module's method the code calls is a
  cdecl recorder, and a recorder that would advance a counter the code passes it (the postprocessor's row group and output row counters,
  the upsampler's) writes it with fill(). The Bench's ranges add the jutils functions the module calls. fncmp refuses a case with an
  empty region (the driver reads a region's bytes as one hex token, so an empty one would desynchronise every case after it). A block or
  row-pointer list on the caller's stack (merged_2v_upsample's work_ptrs, decompress_smooth_data's workspace) is logged with blob(). The
  benches cannot advance the scan counters consume_input would, so decompress_data's loop over it is reached once, with JPEG_SUSPENDED.
  Deal only what the source can meet: the driver runs all of a bench's cases in one process under one 600 s timeout, so one case that
  loops (almost) forever in the library hangs the whole bench and reports nothing. jquant1's select_ncolors with JCS_RGB and other than
  three components reads past RGB_order or an unset Ncolors (0xcdcdcdcd) and loops 2^32 times; a fifth component makes Ncolors[4]
  row_index. The header of each bench lists such edges. Time a new bench's ops natively one at a time before port.py (a small script
  that runs the driver on N cases of one OPS= op with a 20 s timeout) when a case might loop.
  Time them in the emulator too: the --both run's emulator has a 5M-step limit a case, and a case over it counts as a difference.
  jquant2's select_colors and finish_pass1 scan the whole 32x64x32 histogram several times (seconds a case, over the limit with 256
  colours), so its bench deals them one case in 100 with 1..4 colours; the rest are dealt evenly.
  A mutant on a boundary (`<=` to `<`) survives random cases unless the bench builds the edge: jquant2's needed histogram cells at
  0xffff (prescan_quantize's saturating count) and, for find_nearby_colors' `mindist[i] <= minmaxdist`, a colour at the box's lower
  corner with its reflection through the upper corner (whose mindist is exactly the first's maxdist).
  The emulator's one-operand MUL / IMUL / DIV / IDIV now honour a byte or word operand (AL / AX, AH, DX:AX); before, create_colorindex's
  IMUL byte ptr clobbered EDX there and faulted.

**What stays the exe's (libjpeg 6b), and why:**
- jdiv_round_up (0x6428b0) is DivCeil and jpeg_mem_init (0x642b20) is Const_1000000, hand ports already exact; jquant2's
  find_biggest_color_pop (0x6486e0) is Records32_MaxWeight, also a hand port (the library's median_cut calls its own copy).
- The empty methods the exe's linker folded into the bare RET 0x631270 (finish_pass_1_quant, finish_pass2, start_pass_dcolor, ...): a
  hook there would replace every function folded into it.
- jpeg_idct_float (0x64b040): its `(INT32)` casts of out-of-range values differ (the exe's __ftol keeps the low half of a 64-bit FISTP;
  the library's build gives 0x80000000), and /arch:IA32 did not change that. The game selects JDCT_ISLOW / IFAST, never the float IDCT.
- jmemansi.c's backing store: jpeg_open_backing_store (0x6429c0) and its read / write / close methods (0x642a20, 0x642a90, 0x642b00).
  The library's tmpfile() would be a FILE of the DLL's CRT, which the exe's fread / fwrite / fclose cannot take; they stay together
  (as jmemmgr.c's realize_virt_arrays opens one only past max_memory_to_use).
- jerror.c's error_exit (0x6312d0) and output_message (0x631300): they call the exe's CRT exit (0x67f920) and fprintf(stderr) (0x67dad0,
  0x77e840), the DLL's are a different exit and stderr. jpeg_std_error (0x631280) is Codec631280_Init, a hand port already exact: the
  library's would need a shim mapping all five methods (0x6312d0, 0x631340, 0x631300, 0x631380, 0x631440) and the message table
  (the exe's 0x7039d0, last code 0x7b) back, for nothing gained. emit_message (0x631340), format_message (0x631380, sprintf 0x67a650 is
  the exe's CRT's, so it needs the same treatment as malloc) and reset_error_mgr (0x631440) are candidates.
- The game's data source is game code, not libjpeg's: 0x631140 makes the game's JpegStream the source (jpeg_stdio_src's shape), with
  init_source 0x6311b0, fill_input_buffer 0x6311c0, skip_input_data 0x631230, the library's resync_to_restart (jdmarker.c, 0x63b8f0) and
  term_source the shared bare RET 0x631270 (src/game/gfx/jpeg.h).
- Not yet replaced (the next modules, leaves first): jdmaster.c, jdapimin.c / jdapistd.c (0x630ca0 on: jpeg_CreateDecompress checks
  the struct size 0x1b0), jerror.c's emit_message / format_message / reset_error_mgr, jmemmgr.c. Their libmap rows come first, checked
  against the listing.

**libjpeg 6a (0x64c5f0..0x657cb4) is the compressor**: jcapimin / jcapistd, jcparam, jerror, jmemmgr, jcomapi, jcmarker, jcinit, jmemansi,
jutils, jcmainct, jccoefct, jchuff, jcphuff, jcdctmgr, jcprepct, jcsample, jccolor, jcmaster and the three forward DCTs, 160 functions,
all in re/libmap.csv. jpeg_CreateCompress checks version 61 and the struct size 0x168. It is an unoptimised build (frame pointers, no
folding): each function is its own, and a module's static functions follow the first function that refers to them (jinit_* first,
then its methods), so the address order is not the source's. Its only caller is the game's 0x64c258 (from 0x5d4a30: a JPEG writer,
with 0x64c202, the palette test 0x64c552 and 0x64c5c4), which installs its own destination manager (0x64dc00, its methods 0x64dc5a,
0x64dc9c, 0x64dd00 write through 0x5d5350): game code, not jdatadst.c. The exe's 6a was built with jmemansi.c and DEFAULT_MAX_MEM
10000000 (jpeg_mem_init at 0x6503c1 answers 0x989680; the source's default is 1000000), and its jpeg_get_small / large call the same
malloc / free; tpw_jpeg6a is now built so (third_party/CMakeLists.txt). tpw_jpeg6a renames its externals jpeg6a_* through
config/jpeg6a_prefix.h; its statics will need wrap/jpeg-6a/ units like 6b's. Nothing of it is replaced yet: the leaves first (jutils,
jcomapi, the forward DCTs, jcparam's quality functions), then jmemansi.c the 6b way. The two jerror.h
lists differ: 6b added JERR_BAD_DCT_COEF (code 8) and JERR_BAD_HUFF_TABLE (10), dropped 6a's JERR_DHT_COUNTS (6a's 30) and added three
JTRC_THUMB_* traces at the end, so most codes are one apart (two between 10 and 30) and a 6a bench takes its numbers from jpeg-6a's.

## Scope and stop
- Port within your address range, bottom-up, a module at a time.
- Stop after about 40 committed functions or 20 KB of ported code (wave.py's sizes), whichever comes first, or earlier if your context is
  getting large. Then reply with a short report of exceptions only (the lead reads your commits from git):
  - the count and KB ported, counted from the hooks rows your commits added (not from memory);
  - new modules created and names you invented (unconfirmed);
  - fncmp/driver changes;
  - --unverified rows added, mutant survivors and skips, each with its reason;
  - blockers in another agent's range (address and blocker), and anything the lead must act on.
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
5. Fast-forward the worktree (`git -C .worktrees/X merge -q --ff-only main`) and start the agent's next round right away (a fresh agent
   per round).
6. Then `python -I tools/coverage.py --check` (about 400 s; nothing else in the main tree builds or edits tests/fncmp while it runs). If it
   fails, fix it on main and tell the running agents what changed.
