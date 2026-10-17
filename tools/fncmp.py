"""Differential tests of reimplementations: the hooked C++ (build/tpw_fncmp.exe) against the
original code run by x87emu, on random cases, comparing every byte the cases give.

A test script defines a case generator and calls run():

    import sys; sys.path.insert(0, r'C:/git/Noklef/TPW/tools')
    from fncmp import *
    B = Bench([(0x43e1b0, 0x43e480)])           # listing ranges the original needs
    def case(r):                               # r: Rand (seeded; r.wild for extreme values)
        node = r.bytes(0x60)
        r.floats(node, range(0x34, 0x58, 4))
        return Call(0x43e1b0, [A, B_, r.f(), OUT], {A: node, ...})
    B.main(case)                                # cmd line: CASES SEED [--wild] [--show N]

Call(address, args, regions, conv='c'|'s'|'t'|'f', ret='v'|'a'|'b'|'w'|'i'|'d'): ret 'a' compares
EAX, 'b' AL (a flag returned over whatever EAX held), 'w' AX (likewise a u16), 'c' the x87 control word left (Call(cw=) going in, 0x7f; 'wc' AX too), 'i' EAX and EDX, 'd' the double left in
st0. Region addresses must be in the game's data (0x6e7000-0xfc5200) or at 0x10000000 and up (RAM, ALLOC.. are free slots); anything a case
does not set reads as the exe image there, else zero, in both. The emulator is strict, like
the driver: an access outside the regions' 64 KB chunks, the stack and the game's data (the
exe's code is readable too) faults, so a case with a wild pointer shows up as such ("crash
... (emu: write of ADDR at PC)") instead of passing in the emulator and crashing natively. A
case that faults in both counts as the same and is tallied ("crash in both"). STRICT=0 turns
it off.

Mutation testing: B.mutants(case, [(path, old, new), ...]) applies each edit to the source
(old must occur once), rebuilds, runs, restores, and reports which edits were caught.

Expected result: 0 differ with and without --wild (see the precision policy in
src/game/math/x87.h). Suites live in tests/fncmp/.

A case can hand the code a record stream (record_stream(at, cursor, left) is its region):
reads and writes through its vtable copy from or to cursor, at most left bytes, in both (the
driver's stubs at STUBS, the emulator's hooks).

A case can hand the code objects whose vtables point at recorders (vtable({slot: stack arguments}) is the vtable's bytes; give the case a log_region()): a
call through slot k with n stack arguments (thiscall, the callee pops them) appends (k, n, this, the arguments) to the log region at LOG and returns a hash of
them, in both the driver and the emulator, so a function that calls virtual methods can be compared without the game's classes. A case can make slot k return a value
of its own instead of the hash: set_return(log, k, value) on the log region.

COM-style slots (stdcall, `this` the first stack argument, ECX undefined) are vtable(com={slot: stack arguments including this}): they log this as 0 and k | 0x100.

A case can hand the code a heap (heap() is its region at HEAP): the same bump allocator in
both for Mem_Alloc / Mem_Free (and memmove 0x67bb90 in the emulator); _Xlen / _Xran fail the
case (the driver reports a crash, the emulator raises).

A case can hand the code a fake CPU (cpu_region() is its region at CPU): CPUID's answers and whether EFLAGS.ID toggles, in both (the
emulator's CPUID / PUSHFD / POPFD, the driver's CpuInfo env).

The emulator gets __ftol, modf and _CIfmod stubs; Bench(..., sines=N) also gives both sides
an N-entry sine table at 0x7b095c (see sintable.h).
"""
import math
import os
import random
import re
import struct
import subprocess
import sys
import tempfile
import json

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import x87emu  # noqa: E402

EXE = os.path.join(ROOT, '.data', 'nocd', 'tp.exe')
DRIVER = os.path.join(ROOT, 'build', 'tpw_fncmp.exe')
# FNCMP_COVER=file (tools/coverage.py): each run appends what it verified to the file, one JSON line: the addresses its cases called whose
# cases matched without a crash ("direct", with their counts) and the hooked originals entered inside those cases ("indirect").
COVER = os.environ.get('FNCMP_COVER')


def hooked_addresses():
    """The addresses src/hook/hooks.cpp hooks."""
    return set(int(x, 16) for x in re.findall(r'0x00([0-9a-fA-F]{6}), \(void', open(os.path.join(ROOT, 'src', 'hook', 'hooks.cpp')).read()))
MUT = os.path.join(ROOT, 'build', 'mut')  # mutants: a copy of the sources (tree) and its build (out)
MIRRORED = ('CMakeLists.txt', 'src', 're', 'tests', os.path.join('tools', 'fncmp'))
LISTING = os.path.join(ROOT, '_re', 'text.asm')
RAM = 0x30000000  # free addresses for case regions: RAM, RAM + 0x1000, ...
STUBS = 0x2f000000  # the record stream's read and write (driver.cpp's kStubs)
GAME_LOW, GAME_HIGH = 0x6e7000, 0xfc5200  # the game's data, which the driver maps (driver.cpp's kGameLow/High)
CRASH = 'crash c0000005'  # how the driver reports an access violation
HEAP = 0x2e000000  # heap(): Mem_Alloc / Mem_Free's blocks (driver.cpp's kHeap)


def record_stream(at, cursor, left):
    """A record stream's region at `at`: vtable (at + 0x10), cursor, bytes left; then the vtable."""
    return bytearray(struct.pack('<4I4I', at + 0x10, cursor, left, 0, 0, 0, STUBS, STUBS + 0x10))


SEEK = STUBS + 0x70  # a memory stream's seek (driver.cpp stub_seek)


def memory_stream(at, start, size):
    """A seekable stream over size bytes at start (a region the case maps), at `at`: vtable (at + 0x20), cursor, bytes left, start, size; then the vtable:
    +8 read and +0xc write (the record stream's), +0x10 seek (offset, origin 0 / 1 / 2): the new position, or -1 past the end (nothing moved)."""
    return bytearray(struct.pack('<8I8I', at + 0x20, start, size, start, size, 0, 0, 0, 0, 0, STUBS, STUBS + 0x10, SEEK, 0, 0, 0))


def _seek(e):  # thiscall (offset, origin): pops its two arguments (driver.cpp stub_seek)
    sp, s = e.r['ESP'], e.r['ECX']
    offset, origin = e.u32(sp), e.u32(sp + 4)
    cursor, left, start, size = struct.unpack('<4I', e.read(s + 4, 16))
    to = ({0: 0, 1: cursor - start}.get(origin, size) + offset) & 0xffffffff
    if to > size:
        e.r['EAX'] = 0xffffffff
    else:
        e.write(s + 4, struct.pack('<2I', (start + to) & 0xffffffff, size - to))
        e.r['EAX'] = to
    e.r['ESP'] = sp + 8


def zlib_results(log, init=None, deflate=None, inflate=None):
    """The game's zlib is not compared: both sides run stand-ins (driver.cpp zlib_copy) that copy what fits from the input to the output, answering 1
    (Z_STREAM_END) when the input is used up, 0 (Z_OK) when they moved some, -5 (Z_BUF_ERROR) when they could not, and 0 from the inits; each call is
    logged as the entry k 0x140 + its index in deflateInit2, deflate, deflateEnd, inflateInit2, inflate, inflateEnd with its scalar arguments (deflate /
    inflate: the flush, avail_in, avail_out). This overrides the inits', deflate's or inflate's answer (None: their own) in the log region the case hands
    over."""
    for i, v in enumerate((init, deflate, inflate)):
        if v is not None:
            struct.pack_into('<II', log, LOG_ZLIB + 8 * i, 1, v & 0xffffffff)


def _zlib_result(e, which, natural):
    flag, value = struct.unpack('<2I', e.read(LOG + LOG_ZLIB + 8 * which, 8))
    return value if flag else natural & 0xffffffff


def _zlib_log(e, k, args):  # (driver.cpp zlib_log)
    count = e.u32(LOG)
    if count < LOG_ENTRIES:
        e.write(LOG + 4 + 4 * LOG_ENTRY * count, struct.pack('<%dI' % LOG_ENTRY, k, len(args), 0, *(list(args) + [0] * (LOG_ENTRY - 3 - len(args)))))
        e.write(LOG, struct.pack('<I', count + 1))


def _zlib_copy(which):
    def hook(e):  # cdecl (z, flush)
        z, flush = e.u32(e.r['ESP']), e.u32(e.r['ESP'] + 4)
        nin, ain, tin, nout, aout, tout = struct.unpack('<6I', e.read(z, 24))
        _zlib_log(e, 0x140 + 3 * which - 2, (flush, ain, aout))
        n = min(ain, aout)
        if n:
            e.write(nout, e.read(nin, n))
        e.write(z, struct.pack('<6I', (nin + n) & 0xffffffff, ain - n, (tin + n) & 0xffffffff, (nout + n) & 0xffffffff, aout - n, (tout + n) & 0xffffffff))
        e.r['EAX'] = _zlib_result(e, which, 1 if ain == n else 0 if n else -5)
    return hook


def _zlib_init(k, n):
    def hook(e):  # cdecl (z, the scalar arguments)
        _zlib_log(e, k, struct.unpack('<%dI' % n, e.read(e.r['ESP'] + 4, 4 * n)))
        e.r['EAX'] = _zlib_result(e, 0, 0)
    return hook


def _zlib_end(k):
    def hook(e):  # cdecl (z)
        _zlib_log(e, k, ())
        e.r['EAX'] = 0
    return hook


def _record(write):
    def hook(e):  # thiscall (buffer, count): pops its two arguments
        sp, s = e.r['ESP'], e.r['ECX']
        buf, count = e.u32(sp), e.u32(sp + 4)
        cursor, left = e.u32(s + 4), e.u32(s + 8)
        n = min(count, left)
        if n:
            src, dst = (buf, cursor) if write else (cursor, buf)
            e.write(dst, e.read(src, n))
        e.write(s + 4, struct.pack('<2I', (cursor + n) & 0xffffffff, left - n))
        e.r['EAX'] = n
        e.r['ESP'] = sp + 8
    return hook


def fb(x):
    """float -> its bits"""
    return struct.unpack('<I', struct.pack('<f', x))[0]


def bf(b):
    """bits -> float"""
    return struct.unpack('<f', struct.pack('<I', b & 0xffffffff))[0]


def f32(x):
    return bf(fb(x))


SPECIAL = [0, 0x80000000, fb(1.0), fb(-1.0), fb(0.5), 0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000,
           fb(3e38), fb(-3e38), fb(1e-40), fb(-1e-40), fb(1e-30), 0x7f800001, 0xff812345]


class Rand(random.Random):
    """random with float-bits helpers; wild adds extreme exponents and arbitrary bits."""

    def __init__(self, seed, wild=False):
        super().__init__(seed)
        self.wild = wild
        self.decks = {}

    def deal(self, items):
        """the next of items from a shuffled deck, dealt out before it is reshuffled: n cases give each item n // len(items) or more of them,
        where choice() leaves a bench of many functions some with none in a short run (the coverage gate's 400 cases)."""
        deck = self.decks.setdefault(tuple(items), [])
        if not deck:
            deck.extend(items)
            self.shuffle(deck)
        return deck.pop()

    def f(self, scale=None, special=0.06):
        """the bits of a float in -scale..scale (scale picked if None), sometimes special."""
        if self.wild and self.random() < 0.3:
            if self.random() < 0.5:
                return fb(self.uniform(-1, 1) * 10.0 ** self.randrange(-45, 39))
            return self.getrandbits(32)
        if self.random() < special:
            return self.choice(SPECIAL)
        return fb(self.uniform(-1, 1) * (scale or self.choice([1, 10, 300, 5000])))

    def pos(self, scale=None):
        return fb(self.uniform(0, scale or self.choice([1, 5, 50])))

    def bytes(self, n):
        return bytearray(self.getrandbits(8) for _ in range(n))

    def floats(self, buf, offsets, scale=None):
        for off in offsets:
            struct.pack_into('<I', buf, off, self.f(scale))
        return buf


def put(buf, off, fmt, *values):
    struct.pack_into('<' + fmt, buf, off, *values)
    return buf


class Call:
    def __init__(self, address, args, regions, conv='c', ret='v', cw=0x7f):
        self.address, self.args, self.regions, self.conv, self.ret = address, list(args), dict(regions), conv, ret
        self.cw = cw


def sine_table(n):
    """sintable.h's layout at 0x7b095c: scale, mask, (pad), values."""
    return bytearray(struct.pack('<fIi', f32(n / (2 * math.pi)), n - 1, 0) +
                     struct.pack('<%df' % n, *[math.sin(2 * math.pi * i / n) for i in range(n)]))


def slice_listing(ranges):
    import bisect
    lines = open(LISTING).read().splitlines()
    addrs = [int(l[:8], 16) for l in lines]
    out = []
    for lo, hi in ranges:
        out += lines[bisect.bisect_left(addrs, lo):bisect.bisect_left(addrs, hi)]
    path = os.path.join(tempfile.gettempdir(), 'fncmp_%d.asm' % os.getpid())
    open(path, 'w').write('\n'.join(out) + '\n')
    return path


def heap(size=0x4000, fail=0):
    """The heap region: +0 next block, +4 hash of the sizes asked for, +8 frees, +0xc hash of the
    freed pointers; blocks from +0x10 (16-byte aligned, never reused, filled with 0xcd so code
    must set what it reads). Code that allocates (Mem_Alloc 0x67acc0, Mem_Free 0x67acb0) gets
    these in both; give the case this region at HEAP. fail (1-15): that allocation answers 0 (its
    countdown in the next block's low bits; a next block of 0 fails the first)."""
    return bytearray(struct.pack('<4I', HEAP + 0x10 + fail, 0, 0, 0)) + bytearray([0xcd]) * (size - 0x10)


def _alloc(e):  # cdecl (size) (driver.cpp heap_alloc)
    nxt, h, size = e.u32(HEAP), e.u32(HEAP + 4), e.u32(e.r['ESP'])
    countdown, p = nxt & 15, nxt & ~15
    if countdown == 1:
        e.write(HEAP, struct.pack('<2I', p, (h * 33 ^ size) & 0xffffffff))
        e.r['EAX'] = 0
        return
    e.write(HEAP, struct.pack('<2I', (p + ((size + 15) & ~15) + (countdown - 1 if countdown else 0)) & 0xffffffff, (h * 33 ^ size) & 0xffffffff))
    e.r['EAX'] = p


def _free(e):  # cdecl (block)
    frees, h = e.u32(HEAP + 8), e.u32(HEAP + 12)
    e.write(HEAP + 8, struct.pack('<2I', frees + 1, (h * 33 ^ e.u32(e.r['ESP'])) & 0xffffffff))


def global_heap():
    """Regions pointing the game's GlobalAlloc / GlobalFree import slots (0x6fd1cc, 0x6fd26c) at
    the stubs that serve them from heap() (which the case must also have): GlobalAlloc hashes
    the size, then the flags, and zeroes the block got for GMEM_ZEROINIT; GlobalFree returns 0."""
    return {0x6fd1cc: struct.pack('<I', STUBS + 0x20), 0x6fd26c: struct.pack('<I', STUBS + 0x30)}


def _global_alloc(e):  # stdcall (flags, size)
    sp = e.r['ESP']
    flags, size = e.u32(sp), e.u32(sp + 4)
    e.write(sp, struct.pack('<I', size))
    _alloc(e)
    e.write(HEAP + 4, struct.pack('<I', (e.u32(HEAP + 4) * 33 ^ flags) & 0xffffffff))
    if flags & 0x40 and size and e.r['EAX']:   # (not for a failed allocation)
        e.write(e.r['EAX'], bytes(size))
    e.r['ESP'] = sp + 8


VSTUBS = STUBS + 0x1000  # the recorders: slot k (0-31) with n (0-15) stack arguments at VSTUBS + 0x400 * n + 0x20 * k
LOG = STUBS + 0x100000  # their log (the case maps it, see log_region()): a count, then 12 dwords an entry (k, n, this, up to 7 arguments), room for 80
LOG_ENTRY = 12
LOG_ENTRIES = 40  # (the room: 4 + 48 * 40 bytes)
LOG_RETURNS = 0x800  # from here an override for each slot: a flag and a value (flag 2: the value is an argument's index, see echo())


def recorder(k, n, com=False):
    if com:  # (slots 0-63, 0x800 for each n)
        return VSTUBS + 0x4000 + 0x800 * n + 0x20 * k
    return VSTUBS + 0x400 * n + 0x20 * k


def vtable(slots, size=32, com=None):
    """A vtable's bytes: slot k calls a recorder taking slots[k] stack arguments (k not in slots: no arguments); com[k] for a COM-style slot."""
    com = com or {}
    return bytearray(struct.pack('<%dI' % size, *[recorder(k, com[k], True) if k in com else recorder(k, slots.get(k, 0)) for k in range(size)]))


def log_region():
    return bytearray(0x1000)


def set_return(log, k, value):
    """Makes slot k's recorder return `value` (in the log region the case hands over)."""
    struct.pack_into('<II', log, LOG_RETURNS + 8 * k, 1, value & 0xffffffff)


def echo(log, k, arg):
    """Makes slot k's recorder return its argument `arg` as passed (a read that reads all it is asked for, a seek that gets where it is sent)."""
    struct.pack_into('<II', log, LOG_RETURNS + 8 * k, 2, arg)


LOG_STRINGS = 0xa00  # from here a mask for each slot: the arguments that are C strings, and out arguments (see strings())
LOG_FILL = 0xb00  # two fills (+ 0x80 the second): (k | 0x80000000, argument, length), the bytes from + 0x10 (see fill())
LOG_BYTES = 0xc00  # from here a mask for each slot: BYTE arguments, logged as their low byte (the rest of the pushed register is not the callee's)
LOG_ZLIB = 0xf00  # the zlib stand-ins' overrides: the inits', deflate's, inflate's (a flag and a value each, see zlib_results())


def strings(log, k, mask, out=0):
    """Makes slot k's recorder log and hash each argument in `mask` (bit i: argument i) as a hash of the C string it points to, not its address, and each
    in `out` as 0 (a stack address that differs between the sides; a fill writes through it)."""
    struct.pack_into('<I', log, LOG_STRINGS + 4 * (k & 63), mask | out << 16)


def fill(log, k, arg, data, which=0):
    """Makes slot k's recorder write `data` (at most 0x70 bytes) through its argument `arg` (unless null), as an import fills an out structure; which: 0 or 1."""
    at = LOG_FILL + 0x80 * which
    struct.pack_into('<3I', log, at, (k & 63) | 0x80000000, arg, len(data))
    log[at + 0x10:at + 0x10 + len(data)] = data


def fill_arg(log, k, arg, source, which=0):
    """Makes slot k's recorder write its argument `source` (a dword, as passed) through its argument `arg`: ReadFile's bytes read, all it was asked for."""
    struct.pack_into('<3I', log, LOG_FILL + 0x80 * which, (k & 63) | 0x80000000, arg, 0x80000000 | source)


IMPORT_TABLE = 0x6fd000  # the game's import address table (0x400 bytes covers the slots the benches use)


def import_table(log, imports):
    """The import table's bytes (map them at IMPORT_TABLE) with each import a COM-style recorder (stdcall): imports {name: (slot k, slot address, stack
    arguments, string arguments mask, out arguments mask)}; sets the masks in log (strings())."""
    table = bytearray(0x400)
    for k, at, n, mask, out, *low in imports.values():   # (a sixth: the BYTE arguments, see LOG_BYTES)
        struct.pack_into('<I', table, at - IMPORT_TABLE, recorder(k, n, True))
        strings(log, k, mask, out)
        if low:
            struct.pack_into('<I', log, LOG_BYTES + 4 * (k & 63), low[0])
    return table


def string_hash(read, p):
    if not p:
        return 0
    h = 0x811c9dc5
    for i in range(0x400):
        c = read(p + i, 1)[0]
        if not c:
            break
        h = ((h ^ c) * 0x01000193) & 0xffffffff
    return h


def vhash(k, n, this, args):
    h = (this * 0x9e3779b1 + (k + 1) * 0x10001 + n) & 0xffffffff
    for a in args:
        h = (h * 33 ^ a) & 0xffffffff
    return h


def _recorder(k, n, com=False):
    def hook(e):
        sp = e.r['ESP'] + (4 if e.tail else 0)
        args = [e.u32(sp + 4 * i) for i in range(n)]
        raw = list(args)
        this = 0 if com else e.r['ECX']
        kk = k | 0x100 if com else k
        mask = e.u32(LOG + LOG_STRINGS + 4 * (k & 63))
        low = e.u32(LOG + LOG_BYTES + 4 * (k & 63))
        fills = [(LOG + LOG_FILL + 0x80 * f,) + struct.unpack('<3I', e.read(LOG + LOG_FILL + 0x80 * f, 12)) for f in range(2)]
        for i in range(n):
            for at, fk, fa, fn in fills:
                if fk == (k & 63) | 0x80000000 and fa == i and args[i]:
                    if fn & 0x80000000:   # (fill_arg())
                        e.write(args[i], struct.pack('<I', raw[fn & 0xff] if (fn & 0xff) < n else 0))
                    else:
                        e.write(args[i], e.read(at + 0x10, min(fn, 0x70)))
            if mask >> i & 1:
                args[i] = string_hash(e.read, args[i])
            if mask >> (16 + i) & 1:
                args[i] = 0
            if low >> i & 1:
                args[i] &= 0xff
        count = e.u32(LOG)
        if count < LOG_ENTRIES:
            entry = LOG + 4 + 4 * LOG_ENTRY * count
            e.write(entry, struct.pack('<%dI' % LOG_ENTRY, kk, n, this, *(args + [0] * 7)[:7], 0, 0))
            e.write(LOG, struct.pack('<I', count + 1))
        flag, value = struct.unpack('<2I', e.read(LOG + LOG_RETURNS + 8 * k, 8))
        e.r['EAX'] = raw[value] if flag == 2 and value < n else value if flag else vhash(kk, n, this, args)
        if e.tail:  # the return address goes up over the arguments the callee would have popped
            ret = e.u32(e.r['ESP'])
            e.r['ESP'] += 4 * n
            e.write(e.r['ESP'], struct.pack('<I', ret))
        else:
            e.r['ESP'] += 4 * n
    return hook


CDECL_LOG = STUBS + 0x50  # a callback of one cdecl argument that logs it (the entry k 0x13f, n 1, this 0)


def _cdecl_log(e):  # (driver.cpp stub_cdecl_log)
    sp = e.r['ESP'] + (4 if e.tail else 0)
    arg = e.u32(sp)
    mask = e.u32(LOG + LOG_STRINGS + 4 * 0x3f)   # (strings(log, 0x3f, ...), as driver.cpp's record_call applies it)
    if mask & 1:
        arg = string_hash(e.read, arg)
    if mask >> 16 & 1:
        arg = 0
    if e.u32(LOG + LOG_BYTES + 4 * 0x3f) & 1:
        arg &= 0xff
    count = e.u32(LOG)
    if count < LOG_ENTRIES:
        entry = LOG + 4 + 4 * LOG_ENTRY * count
        e.write(entry, struct.pack('<%dI' % LOG_ENTRY, 0x13f, 1, 0, arg, *([0] * 8)))
        e.write(LOG, struct.pack('<I', count + 1))
    e.r['EAX'] = e.u32(LOG + LOG_RETURNS + 8 * 0x3f + 4) if e.u32(LOG + LOG_RETURNS + 8 * 0x3f) else vhash(0x13f, 1, 0, [arg])
    if e.tail:
        ret = e.u32(e.r['ESP'])
        e.write(e.r['ESP'], struct.pack('<I', ret))


CDECL_LOG2 = STUBS + 0x80  # a callback of two cdecl arguments that logs them (the entry k 0x13e, n 2, this 0; set_return(log, 0x3e, v) overrides)


def _cdecl_log2(e):  # (driver.cpp stub_cdecl_log2)
    sp = e.r['ESP'] + (4 if e.tail else 0)
    args = [e.u32(sp), e.u32(sp + 4)]
    count = e.u32(LOG)
    if count < LOG_ENTRIES:
        e.write(LOG + 4 + 4 * LOG_ENTRY * count, struct.pack('<%dI' % LOG_ENTRY, 0x13e, 2, 0, *args, *([0] * 7)))
        e.write(LOG, struct.pack('<I', count + 1))
    e.r['EAX'] = e.u32(LOG + LOG_RETURNS + 8 * 0x3e + 4) if e.u32(LOG + LOG_RETURNS + 8 * 0x3e) else vhash(0x13e, 2, 0, args)



CPU = 0x2d000000  # the fake CPU a case maps (cpu_region()): CPUID and EFLAGS.ID in the emulator, driver.cpp's stub_cpuid / stub_has_cpuid natively


def cpu_region(id_writable, leaves):
    """The fake CPU's region (at CPU): +0 1 when EFLAGS.ID can be toggled, then from +0x10 the (EAX, EBX, ECX, EDX) CPUID answers for leaf 0, 1,
    0x80000001 and any other leaf, 16 bytes each (leaves: up to four tuples, the rest zero)."""
    b = bytearray(0x50)
    struct.pack_into('<I', b, 0, 1 if id_writable else 0)
    for i, regs in enumerate(leaves[:4]):
        struct.pack_into('<4I', b, 0x10 + 0x10 * i, *(x & 0xffffffff for x in regs))
    return b


def _cpuid(e, leaf):
    i = {0: 0, 1: 1, 0x80000001: 2}.get(leaf, 3)
    return struct.unpack('<4I', e.read(CPU + 0x10 + 0x10 * i, 16))


def _id_writable(e):
    return e.u32(CPU) != 0


COUNTER = STUBS + 0x60  # QueryPerformanceFrequency / QueryPerformanceCounter for an import slot (driver.cpp stub_counter); counter(log, ...) sets it up


def counter(log, value, step, results):
    """Sets COUNTER up in the log region: each call writes the 64-bit `value` through its argument, then steps it by `step`, and returns bit 0 of `results`, which then shifts right."""
    struct.pack_into('<IIQQ', log, 0x7e8, results & 0xffffffff, 0, value & (2 ** 64 - 1), step & (2 ** 64 - 1))


def _counter(e):  # stdcall (the 64-bit out)
    sp = e.r['ESP'] + (4 if e.tail else 0)
    now, step = struct.unpack('<QQ', e.read(LOG + 0x7f0, 16))
    e.write(e.u32(sp), struct.pack('<Q', now))
    e.write(LOG + 0x7f0, struct.pack('<Q', (now + step) & (2 ** 64 - 1)))
    results = e.u32(LOG + 0x7e8)
    e.write(LOG + 0x7e8, struct.pack('<I', results >> 1))
    e.r['EAX'] = results & 1
    if e.tail:
        ret = e.u32(e.r['ESP'])
        e.r['ESP'] += 4
        e.write(e.r['ESP'], struct.pack('<I', ret))
    else:
        e.r['ESP'] += 4


def _mark(e):  # thiscall, no other arguments (driver.cpp stub_mark)
    e.write(e.r['ECX'], struct.pack('<I', 0x6fd800))


def _global_free(e):  # stdcall (block)
    _free(e)
    e.r['EAX'] = 0
    e.r['ESP'] += 4


def _memmove(e):  # cdecl (to, from, n) -> to
    sp = e.r['ESP']
    to, frm, n = e.u32(sp), e.u32(sp + 4), e.u32(sp + 8)
    if n:
        e.write(to, e.read(frm, n))
    e.r['EAX'] = to


def _throw(name):
    def hook(e):
        raise RuntimeError('the original called %s' % name)
    return hook


def _ftol(e):
    x = e.st.pop(0)
    v = int(x) if math.isfinite(x) and -2 ** 63 <= int(x) < 2 ** 63 else -2 ** 63
    v &= (1 << 64) - 1
    e.r['EAX'], e.r['EDX'] = v & 0xffffffff, v >> 32


def _modf(e):  # modf(double x, double *whole): args at [ESP] (hooks see no return address)
    sp = e.r['ESP']
    x = struct.unpack('<d', e.read(sp, 8))[0]
    frac, whole = math.modf(x)
    e.write(e.u32(sp + 8), struct.pack('<d', whole))
    e.st.insert(0, frac)


def _cifmod(e):  # _CIfmod: st1 fmod st0, popping both; NaNs come through as from fprem
    y, x = e.st.pop(0), e.st.pop(0)  # (assumed: the CRT's dispatcher, 0x6843e0, is not emulated)
    if math.isnan(x) or math.isnan(y):
        e.st.insert(0, x87emu.farith(None, x, y))
    else:
        e.st.insert(0, math.fmod(x, y) if math.isfinite(x) and y != 0 else x87emu.INDEFINITE)


class Bench:
    def __init__(self, ranges, sines=None, hooks=None, strict=True):
        self.emu = x87emu.Emu([slice_listing(ranges)])
        self.emu.hooks.update({0x67a830: _ftol, 0x67b280: _modf, 0x67b24a: _cifmod,
                               STUBS: _record(False), STUBS + 0x10: _record(True),
                               STUBS + 0x20: _global_alloc, STUBS + 0x30: _global_free, STUBS + 0x40: _mark, CDECL_LOG: _cdecl_log, COUNTER: _counter, SEEK: _seek, CDECL_LOG2: _cdecl_log2,
                               0x6351a0: _zlib_init(0x140, 7), 0x635450: _zlib_copy(1), 0x6357c0: _zlib_end(0x142), 0x636600: _zlib_init(0x143, 3),
                               0x636710: _zlib_copy(2), 0x6365b0: _zlib_end(0x145),
                               0x67acc0: _alloc, 0x67acb0: _free, 0x67bb90: _memmove,
                               0x6918f0: _throw('_Xlen'), 0x691ba0: _throw('_Xran')})
        self.emu.hooks.update({recorder(k, n): _recorder(k, n) for k in range(32) for n in range(16)})  # (n up to 15 fits below the COM recorders at +0x4000)
        self.emu.hooks.update({recorder(k, n, True): _recorder(k, n, True) for k in range(64) for n in range(12)})
        self.emu.hooks.update(hooks or {})
        self.emu.cpuid, self.emu.id_writable = _cpuid, _id_writable
        self.extra = {0x7b095c: sine_table(sines)} if sines else {}
        self.driver = DRIVER
        # strict: the original faults, as the driver would, outside the case's regions, the
        # stack and the game's data (and only reads the exe's other sections). STRICT=0 in the
        # environment turns it off.
        self.strict = strict and os.environ.get('STRICT', '1') != '0'
        self.ranges = [(GAME_LOW, GAME_HIGH, True)] + [(va, va + len(d), False) for va, d in x87emu.exe_sections()]
        self.hooked = hooked_addresses() if COVER else None
        self.entered = set()  # (with COVER) the hooked originals the last case entered

    def original(self, c):
        e = self.emu
        e.ranges = None
        e.mem.clear()
        for k, v in c.regions.items():
            e.write(k, bytes(v))
        if self.strict:  # what the driver maps: the case's regions (in 64 KB chunks), the stack
            e.chunks = {0x0fff, 0x1000} | {p for k, v in c.regions.items() for p in range(k >> 16, ((k + max(len(v), 1) - 1) >> 16) + 1)}
            e.ranges = self.ranges
        e.reset()
        if 'c' in c.ret:
            e.cw = c.cw
        args = c.args
        if c.conv in 'tf' and args:
            e.r['ECX'] = args[0]
            args = args[1:]
            if c.conv == 'f' and args:
                e.r['EDX'] = args[0]
                args = args[1:]
        for a in reversed(args):
            e.push(a & 0xffffffff)
        e.push(x87emu.SENTINEL)
        pc = c.address
        entered, hooked, stand_ins = self.entered, self.hooked, e.hooks
        entered.clear()
        try:
            for _ in range(5_000_000):
                if pc == x87emu.SENTINEL:
                    break
                if hooked is not None and pc in hooked and pc not in stand_ins:
                    entered.add(pc)
                at = pc
                pc = e.step(pc)
            else:
                return 'step limit'
        except x87emu.MemFault as f:
            return '%s (emu: %s at %08x)' % (CRASH, f, at)
        except x87emu.DivFault as f:
            return 'crash %08x (emu: %s at %08x)' % (f.code, f, at)
        st0 = struct.unpack('<Q', struct.pack('<d', e.st[0]))[0] if c.ret == 'd' and e.st else 0
        head = ['%08x' % e.r['EAX'] if c.ret in 'ai' else '%02x' % (e.r['EAX'] & 0xff) if c.ret == 'b' else '%04x' % (e.r['EAX'] & 0xffff) if 'w' in c.ret else '-', '%08x' % e.r['EDX'] if c.ret == 'i' else '%04x' % e.cw if 'c' in c.ret else '-',
                '%016x' % st0 if c.ret == 'd' else '-']
        return head + [e.read(k, len(v)).hex() for k, v in c.regions.items()]

    def cases(self, gen, n, seed, wild):
        r = Rand(seed, wild)
        out = []
        for _ in range(n):
            c = gen(r)
            for k, v in self.extra.items():
                c.regions.setdefault(k, v)
            out.append(c)
        return out

    def native(self, cases):
        inp = ''.join('%x %s %s %x %s %x %s\n' % (
            c.address, c.conv, c.ret + (':%x' % c.cw if 'c' in c.ret else ''), len(c.args), ' '.join('%x' % (a & 0xffffffff) for a in c.args), len(c.regions),
            ' '.join('%x %s' % (k, bytes(v).hex()) for k, v in c.regions.items())) for c in cases)
        try:  # a case that loops forever in the C++ would hang the bench
            p = subprocess.run([self.driver, EXE], input=inp, capture_output=True, text=True, timeout=600)
            if os.environ.get('FNCMP_CRASHAT'):   # (driver.cpp crash_filter)
                sys.stderr.write(p.stderr)
        except subprocess.TimeoutExpired as t:
            p = subprocess.CompletedProcess(t.cmd, 0x102, t.stdout or '', 'timed out')
            p.stdout = p.stdout.decode() if isinstance(p.stdout, bytes) else p.stdout
        out = []
        for c, line in zip(cases, p.stdout.splitlines()):
            w = line.split()
            if len(w) < 3 or w[0] in ('crash', 'nohook', 'fncmp:'):
                out.append(line)
                continue
            out.append([w[0] if c.ret in 'ai' else w[0][-2:] if c.ret == 'b' else w[0][-4:] if 'w' in c.ret else '-', w[1] if c.ret == 'i' else w[1][-4:] if 'c' in c.ret else '-', w[2] if c.ret == 'd' else '-'] + w[3:])
        if len(out) < len(cases):
            out += ['driver stopped (exit %x): %s' % (p.returncode & 0xffffffff, p.stderr.strip()[:200])] * (len(cases) - len(out))
        return out

    def run(self, gen, n, seed, wild=False, show=3, quiet=False):
        """Returns the number of differing cases."""
        cases = self.cases(gen, n, seed, wild)
        nat = self.native(cases)
        bad = crashed = 0
        direct, indirect = {}, set()
        for i, (c, cpp) in enumerate(zip(cases, nat)):
            emu = self.original(c)
            if emu == cpp:
                if COVER and not isinstance(emu, str):
                    direct[c.address] = direct.get(c.address, 0) + 1
                    indirect |= self.entered
                continue
            if isinstance(emu, str) and isinstance(cpp, str) and cpp.startswith('crash ') and emu.split(' (emu:')[0] == cpp:
                crashed += 1  # the original faults too, with the same exception: the same
                continue
            bad += 1
            if bad > show:
                continue
            print('DIFF case %d: %x(%s)' % (i, c.address, ', '.join('%x' % (a & 0xffffffff) for a in c.args)))
            if isinstance(cpp, str) or isinstance(emu, str):
                print('  cpp %s\n  emu %s' % (cpp if isinstance(cpp, str) else 'ok', emu if isinstance(emu, str) else 'ok'))
                continue
            names = ['EAX', 'EDX', 'ST0'] + ['%x' % k for k in c.regions]
            for name, x, y in zip(names, cpp, emu):
                if x != y:
                    d = next((j for j in range(0, min(len(x), len(y)), 8) if x[j:j + 8] != y[j:j + 8]), 0)
                    print('  %s +%x: cpp %s emu %s' % (name, d // 2, x[d:d + 16], y[d:d + 16]))
        if COVER:
            with open(COVER, 'a') as f:
                f.write(json.dumps({'bench': os.path.basename(sys.argv[0]), 'cases': n, 'differ': bad, 'direct': {'%x' % k: v for k, v in sorted(direct.items())},
                                    'indirect': ['%x' % k for k in sorted(indirect)]}) + '\n')
        if not quiet:
            print('%d cases%s, seed %d: %d differ%s' % (n, ' (wild)' if wild else '', seed, bad,
                                                       ' (%d crash in both)' % crashed if crashed else ''))
        return bad

    def main(self, gen):
        """CASES SEED [--wild] [--show N] [--both]: run (--both: normal then wild)."""
        a = sys.argv[1:]
        n, seed = int(a[0]) if a else 2000, int(a[1]) if len(a) > 1 else 1
        show = int(a[a.index('--show') + 1]) if '--show' in a else 3
        bad = 0
        if '--wild' not in a or '--both' in a:
            bad += self.run(gen, n, seed, False, show)
        if '--wild' in a or '--both' in a:
            bad += self.run(gen, n, seed + 1, True, show)
        sys.exit(1 if bad else 0)

    def mutants(self, gen, edits, n=1500, seed=7, wild=False):
        """Applies each (path, old, new) edit alone to a copy of the sources (build/mut), builds
        its driver there and runs; True where caught. The working tree and build/ are untouched."""
        tree = mirror()
        out = os.path.join(MUT, 'out')
        results = []
        for path, old, new in edits:
            rel = os.path.relpath(path if os.path.isabs(path) else os.path.join(ROOT, path), ROOT)
            full = os.path.join(tree, rel)
            src = open(full, newline='').read()
            if src.count(old) != 1:
                raise ValueError('%s: %r occurs %d times' % (path, old, src.count(old)))
            open(full, 'w', newline='').write(src.replace(old, new))
            try:
                ok = build(tree, out, 'tpw_fncmp')
                self.driver = os.path.join(out, 'tpw_fncmp.exe')
                bad = self.run(gen, n, seed, wild, show=0, quiet=True) if ok else None
            finally:
                open(full, 'w', newline='').write(src)
                self.driver = DRIVER
            print('%-9s %s: %r -> %r' % ('BUILD ERR' if bad is None else 'caught' if bad else 'SURVIVED',
                                        os.path.basename(path), old[:50], new[:50]))
            results.append(bool(bad))
        return results


def mirror():
    """Brings build/mut/tree up to the working tree's sources, rewriting only the files that
    differ (so its build stays incremental); returns its path."""
    tree = os.path.join(MUT, 'tree')
    for top in MIRRORED:
        here = os.path.join(ROOT, top)
        files = [here] if os.path.isfile(here) else [
            os.path.join(d, f) for d, _, fs in os.walk(here) if '__pycache__' not in d for f in fs]
        for f in files:
            to = os.path.join(tree, os.path.relpath(f, ROOT))
            data = open(f, 'rb').read()
            if not os.path.exists(to) or open(to, 'rb').read() != data:
                os.makedirs(os.path.dirname(to), exist_ok=True)
                open(to, 'wb').write(data)
    return tree


def build(src=None, out=None, target=None):
    """Runs build.bat (on another source tree, into another directory, for one target); True if
    it succeeded."""
    env = {k: v for k, v in os.environ.items() if k not in ('TPW_SRC', 'TPW_OUT', 'TPW_TARGET')}
    env.update({k: v for k, v in (('TPW_SRC', src), ('TPW_OUT', out), ('TPW_TARGET', target)) if v})
    p = subprocess.run(['cmd', '/c', os.path.join(ROOT, 'build.bat')], capture_output=True, text=True,
                       stdin=subprocess.DEVNULL, env=env)
    if p.returncode:
        print('\n'.join(l for l in p.stdout.splitlines() if 'error' in l.lower())[:2000])
    return p.returncode == 0
