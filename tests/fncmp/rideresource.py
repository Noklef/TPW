# A ride's resource loaded (src/game/model/rideresource: 0x4629d0) against the original. Its calls are recorders (driver.cpp's kRideResourceEnv: 0x46e650 1, Quickload_LoadRide 2, RideModel_Load 3,
# RideBase_Prepare 4, Face_MarkTextures 5, HoardingMesh_Build 6, Model_MarkFlag80Faces 7, HeightMap_LoadOrBuild 8, GamePaths_FontsFolder 9, ObjectDefs_FindAt410 10, Sign_Apply 11), answering 1 or not;
# names with or without an extension or a '.' before a '\'; a model of any part flags and chains; both lists of parts; 0xd0's model or none.
# usage: rideresource.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, fill, log_region, put, set_return, strings  # noqa: E402

HOOKS = {0x46e650: _recorder(1, 1, cdecl=True), 0x461a60: _recorder(2, 3, cdecl=True), 0x461f10: _recorder(3, 5, cdecl=True), 0x46a5a0: _recorder(4, 5, cdecl=True),
         0x46a550: _recorder(5, 3, cdecl=True), 0x452e70: _recorder(6, 3, cdecl=True), 0x44ba60: _recorder(7, 1, cdecl=True), 0x451640: _recorder(8, 6, cdecl=True),
         0x468250: _recorder(9, 1, cdecl=True), 0x412f50: _recorder(10, 1), 0x467a80: _recorder(11, 6, cdecl=True)}
B = Bench([(0x4629d0, 0x46302b), (0x5c5520, 0x5c5530), (0x5da3c0, 0x5da3c1), (0x67a400, 0x690000)], hooks=HOOKS)
MODEL, MODEL2, HOLDER, INNER, FIRST, EXTRAS, ENTRIES, EXTRA = (RAM + 0x100 * k for k in range(8))
PARTS, SMALL, CHAIN, TEXTURES, DATA, A8, B10, NAMES = (RAM + 0x1000 * k for k in range(1, 9))
CELLS, DEF = RAM + 0x800, RAM + 0x840
LONG, SHORT = 6, 7
LOADS = [b'ride', b'ride.x', b'a.b\\ride', b'c.\\ride', b'.\\r', b'', b'p.q.r']
OUTS = [b'out', b'out.md2', b'o.b\\out', b'o.\\out', b'.\\o', b'']
PATHS = [b'path', b'C:\\where']


def parts(r):
    """The two lists of parts (0xa0 and 0x58 bytes: the shorter's fields overlap, sic) and what they point to."""
    long_, short = bytearray(r.bytes(LONG * 0xa0 + 0x80)), bytearray(r.bytes(SHORT * 0x58 + 0x80))
    links = bytearray(r.bytes(4 * 0x10))
    for k in range(4):
        put(links, k * 0x10 + 1, 'B', r.deal([2, 2, 2, 0]) | r.getrandbits(8) & ~2)
        put(links, k * 0x10 + 8, 'I', CHAIN + (k + 1) * 0x10 if k < 3 and r.random() < 0.7 else 0)
        put(links, k * 0x10 + 0xc, 'I', r.deal([0, 0, 0, 1]))
    textures = bytearray(r.bytes(8 * 0x10))
    for k in range(8):
        put(textures, k * 0x10, 'I', r.choice([0, DATA + r.randrange(8)]))
    for blob, stride, n in ((long_, 0xa0, LONG), (short, 0x58, SHORT)):
        for k in range(n):
            at = k * stride
            kind = r.getrandbits(16) & ~0x240 | r.deal([0, 0x200]) | r.deal([0, 0x40, 0x20000]) | r.randrange(4) << 16 & ~(0x20000 if stride == 0x58 else 0)
            put(blob, at, 'I', kind)
            put(blob, at + 0xc, 'I', r.choice([0, CHAIN, CHAIN + 0x10, CHAIN + 0x20]))
    for blob, stride, n in ((long_, 0xa0, LONG), (short, 0x58, SHORT)):
        for k in range(n):
            put(blob, k * stride + 0x6c, 'I', TEXTURES + r.randrange(4) * 0x10)
    for k in range(LONG):
        put(long_, k * 0xa0 + 0x5a, 'H', r.randrange(4))
    return {PARTS: long_, SMALL: short, CHAIN: links, TEXTURES: textures, DATA: r.bytes(8)}


def case(r):
    log = log_region()
    strings(log, 1, 0, 1)
    strings(log, 2, 7)
    strings(log, 3, 1, 4 | 8)
    strings(log, 8, 8)
    strings(log, 9, 0, 1)
    strings(log, 11, 2 | 4 | 8)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 10, 1 << 16)   # (the id: the rest of the pushed register is not the callee's)
    other = r.choice([MODEL2, MODEL2, 0])
    fill(log, 1, 0, r.choice(PATHS) + b'\0')
    fill(log, 3, 2, struct.pack('<II', MODEL, other), which=1, records=2)
    fill(log, 3, 3, r.choice(OUTS) + b'\0', which=2)
    fill(log, 9, 0, b'fonts' + b'\0', which=3)
    set_return(log, 2, r.choice([1, 1, 1, 1, 0, 2]))
    set_return(log, 3, r.choice([1, 1, 1, 1, 1, 0, 2]))
    set_return(log, 9, r.choice([1, 1, 1, 0, 2]))
    set_return(log, 10, r.getrandbits(32))

    model, model2, holder, inner = bytearray(r.bytes(0x100)), bytearray(r.bytes(0x100)), bytearray(r.bytes(0x40)), bytearray(r.bytes(0x90))
    put(model, 8, 'I', HOLDER)
    put(model2, 8, 'I', HOLDER)
    put(model, 0xd0, 'I', other)
    put(holder, 4, 'I', INNER)
    put(holder, 0x28, 'I', r.choice([0, EXTRAS, EXTRAS]))
    put(holder, 0x2c, 'I', r.choice([0, FIRST]))
    count2 = r.randrange(0, 4)
    put(inner, 0x36, 'H', r.randrange(5))
    put(inner, 0x42, 'H', r.choice([0, 1, 3, 5]))
    put(inner, 0x44, 'H', r.choice([0, 1, 3, 5]))
    put(inner, 0x46, 'H', r.randrange(3))
    put(inner, 0x48, 'H', count2)
    put(inner, 0x50, 'II', A8, B10)
    put(inner, 0x70, 'III', PARTS, SMALL, r.getrandbits(32))
    put(inner, 0x7c, 'I', ENTRIES)
    extras = put(bytearray(r.bytes(0x10)), 4, 'I', r.choice([0, EXTRA, EXTRA, EXTRA]))
    entries = bytearray(r.bytes(4 * 0x14))
    for k in range(4):
        put(entries, k * 0x14, 'I', r.getrandbits(32) & ~0x580f02 | r.deal([0, 2]) | r.deal([0, 0, 1 << r.randrange(8, 12), 0x80000, 0x100000, 0x400000]))
    names = bytearray()
    at = {}
    for w in LOADS:
        at[w] = NAMES + len(names)
        names += w + b'\0'
    regions = {MODEL: model, MODEL2: model2, HOLDER: holder, INNER: inner, FIRST: r.bytes(0x60), EXTRAS: extras, ENTRIES: entries, EXTRA: r.bytes(4 * 0x14),
               A8: r.bytes(4 * 8), B10: r.bytes(4 * 0x10), NAMES: names, CELLS: r.bytes(8), DEF: r.bytes(4), LOG: log,
               0x79f7d0: struct.pack('<I', r.randrange(50)), 0x7a439c: struct.pack('<I', r.randrange(6)), 0x7ae6a8: r.bytes(0x20), 0x7a0854: r.bytes(4), 0x786b68: r.bytes(0x10)}
    regions.update(parts(r))
    flags = r.getrandbits(32) & ~0x2c89 | r.deal([0, 1]) | r.deal([0, 0x2000]) | r.deal([0, 8]) | r.deal([0, 0x80]) | r.deal([0, 0x400]) | r.deal([0, 0x800])
    flags = flags & ~0x20000 | r.deal([0, 0x20000])
    return Call(0x4629d0, [NAMES + len(LOADS[0]) + 1 if r.random() < 0.5 else NAMES, at[r.choice(LOADS)], r.choice([CELLS, CELLS, 0]), r.choice([0, RAM + 0x840]), flags, r.getrandbits(32),
                           r.choice([0, 1, 2]), r.choice([0, DEF, DEF]), r.getrandbits(32), r.choice([0, 3, 9]), r.choice([0, 4, 16])], regions)


if __name__ == '__main__':
    B.main(case)
