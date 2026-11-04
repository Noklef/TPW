# The Direct3D setup (src/game/render/d3dsetup: 0x573080, 0x5736a0, 0x573720, 0x573740, 0x572d50, 0x572f70, 0x573060, 0x573660) against the originals: COM recorders for all the objects' calls, each result 0 mostly so that the failures come at every step
# (their messages through the engine debug log's and error list's real code, the log's C library calls recorders as in enginedebug.py), the objects given by fills, the descriptions logged by their contents, stack pointers as 0; the debug flags any; the attached calls
# (core/vobj3), D3dSetup_CreateViewport real. The format picks: masks of the bit counts the table may hold, or others.
# usage: d3dsetup.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, blob, fill, heap, log_region, put, recorder, set_return, strings  # noqa: E402
import enginedebug  # noqa: E402

CALLS = {'Open': (0x573080, 0x5735f6 + 0x6b), 'Close': (0x5736a0, 0x573711), 'Reopen': (0x573720, 0x573733), 'Clear': (0x573740, 0x573820), 'Teardown': (0x572d50, 0x572dd0), 'Pick': (0x572f70, 0x57305d), 'OnFormat': (0x573060, 0x573074), 'OnZ': (0x573660, 0x57369c)}
B = Bench(list(CALLS.values()) + [(0x572dd0, 0x572f64), (0x5644e0, 0x5645a0), (0x5911d0, 0x5911f0), (0x56a770, 0x56ae60), (0x56af20, 0x56af9c), enginedebug.RANGE, enginedebug.FALLBACK, (0x67a400, 0x690000)], hooks=enginedebug.HOOKS)
SELF, VB, BASEVT, ADAPTER = RAM, RAM + 0x300, RAM + 0x310, RAM + 0x400
DRAW, DRAWVT, D3D, D3DVT, DEV, DEVVT, VIEW, VIEWVT, MAT, MATVT, MEMBER, MEMBERVT, FORMAT, SECOND = (RAM + 0x1000 + 0x100 * k for k in range(14))
QI, SURFACE, MATERIAL, DEVICE, ENUMZ, CREATEVIEW, CAPS, ADDVIEW, ENUMTEX, CURRENT, STATE, BACKGROUND, SETVIEW, SETMAT, HANDLE, RELEASE, VBASE, ATTACH, DETACH, CLEAR = (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 16, 17, 18, 19, 20, 21, 22, 23, 25)   # (not the debug log's 11-14)
OPS = ['Open', 'Open', 'Open', 'Open', 'Open', 'Close', 'Reopen', 'Clear', 'Teardown', 'Pick', 'OnFormat', 'OnZ', 'OnZ']


def table(r, size, slots):
    t = bytearray(r.bytes(size * 4))
    for slot, (k, n) in slots.items():
        put(t, slot * 4, 'I', recorder(k, n, True))
    return t


def hresult(r):
    return r.choice([0] * 28 + [1, 0x88760000 + r.randrange(0x300), 0x80004005, r.getrandbits(32)])


def mask(r, bits):
    return ((1 << bits) - 1) << r.randrange(33 - bits) & 0xffffffff if bits else 0


def world(r, log, regions):
    for slot in (QI, SURFACE, MATERIAL, DEVICE, ENUMZ, CREATEVIEW, CAPS, ADDVIEW, ENUMTEX, CURRENT, BACKGROUND, SETVIEW, SETMAT, HANDLE, ATTACH, DETACH, CLEAR):
        set_return(log, slot, hresult(r))
    flags = r.choice([0, 0, 0x1000000, 0x2000000, 0x3000000, r.getrandbits(32)])
    if flags & 0x1000000:   # (an error pushed with this flag logs through EngineErrors' environment, the driver's stub: engineerrors.py's matter)
        for slot in (CREATEVIEW, ADDVIEW, SETVIEW, CURRENT, ATTACH, DETACH, CLEAR):
            set_return(log, slot, 0)
    set_return(log, VBASE, r.choice([1, 1, 1, 1, 0, 2]))
    fill(log, QI, 2, struct.pack('<I', D3D), which=0)
    strings(log, SURFACE, 0, out=1 << 2)
    blob(log, SURFACE, 1, 0x7fffffff)
    fill(log, SURFACE, 2, struct.pack('<I', r.getrandbits(32)), which=1)
    strings(log, ENUMZ, 0, out=1 << 3)
    blob(log, ENUMZ, 3, 0xff)
    fill(log, ENUMZ, 3, struct.pack('<8I', r.choice([0x20, 0x20, 0x20, 0x20, 0x1f, 0]), *[r.getrandbits(32) for _ in range(7)]), which=2)
    fill(log, DEVICE, 3, struct.pack('<I', DEV), which=3)
    fill(log, CREATEVIEW, 1, struct.pack('<I', VIEW), which=4)
    fill(log, MATERIAL, 1, struct.pack('<I', MAT), which=5)
    fill(log, HANDLE, 2, struct.pack('<I', r.getrandbits(32)), which=6)
    strings(log, SETMAT, 0, out=1 << 1)
    blob(log, SETMAT, 1, 0xfffff)
    strings(log, SETVIEW, 0, out=1 << 1)
    blob(log, SETVIEW, 1, 0x7ff)
    enginedebug.texts(log)
    if r.random() < 0.3:
        set_return(log, enginedebug.FOPEN, 0)
    errors = bytearray(r.bytes(0x204))
    put(errors, 0x200, 'I', r.choice([0, 1, 0x3f, 0x40, r.randrange(0x40)]))
    me = bytearray(r.bytes(0x240))
    put(me, 0, 'I', VB)
    put(me, 4, 'I', r.choice([0, D3D, D3D]))
    put(me, 8, 'I', r.choice([0, DEV, DEV]))
    put(me, 0xc, 'I', r.choice([0, VIEW, VIEW]))
    put(me, 0x10, 'I', r.choice([0, MAT, MAT]))
    put(me, 0x228, 'I', BASEVT)
    adapter = bytearray(r.bytes(0x430))
    put(adapter, 8, 'I', DRAW)
    put(adapter, 0x14, 'I', MEMBER)
    vendor, chip = r.choice([(0x121a, 1), (0x121a, 2), (0x10de, 1), (0x10de, 5), (r.getrandbits(32), r.getrandbits(32))]) if r.random() < 0.3 else (0x10de, 5)
    put(adapter, 0x428, '2I', vendor, chip)
    base = bytearray(r.bytes(4))
    put(base, 0, 'I', recorder(VBASE, 0))
    regions.update({SELF: me, VB: struct.pack('<2I', r.getrandbits(32), 0x228), BASEVT: base, ADAPTER: adapter, 0x879a24: struct.pack('<I', ADAPTER),
                    DRAW: struct.pack('<I', DRAWVT), DRAWVT: table(r, 16, {0: (QI, 3), 2: (RELEASE, 1), 6: (SURFACE, 4)}),
                    D3D: struct.pack('<I', D3DVT), D3DVT: table(r, 16, {2: (RELEASE, 1), 5: (MATERIAL, 3), 6: (CREATEVIEW, 3), 8: (DEVICE, 5), 10: (ENUMZ, 4)}),
                    DEV: struct.pack('<I', DEVVT), DEVVT: table(r, 32, {2: (RELEASE, 1), 3: (CAPS, 3), 5: (ADDVIEW, 2), 8: (ENUMTEX, 3), 12: (CURRENT, 2), 22: (STATE, 3)}),
                    VIEW: struct.pack('<I', VIEWVT), VIEWVT: table(r, 32, {2: (RELEASE, 1), 8: (BACKGROUND, 2), 17: (SETVIEW, 2), 20: (CLEAR, 7)}),
                    MAT: struct.pack('<I', MATVT), MATVT: table(r, 16, {2: (RELEASE, 1), 3: (SETMAT, 2), 5: (HANDLE, 3)}),
                    MEMBER: struct.pack('<I', MEMBERVT), MEMBERVT: table(r, 16, {3: (ATTACH, 2), 8: (DETACH, 3)}),
                    0x879ad0: errors, 0x879cd4: r.bytes(4), 0x879a2c: bytearray(struct.pack('<I', r.choice([0, 1]))), 0x879a28: bytearray(struct.pack('<I', r.choice([0, 1]))),
                    0x8bd508: struct.pack('<I', flags), 0x8bd598: struct.pack('<I', r.choice([0, 1, 1, 1, 1, 1, 0x100, r.getrandbits(32)])),
                    0x8bcbd0: struct.pack('<2I', r.getrandbits(12), r.getrandbits(12))})


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap()}
    op = r.deal(OPS)
    address = CALLS[op][0]
    if op in ('Pick', 'OnFormat'):
        regions[FORMAT] = struct.pack('<4I', *[r.getrandbits(32) for _ in range(4)]) + struct.pack('<4I', *[mask(r, b) for b in r.choice([(5, 6, 5, 0), (5, 5, 5, 1), (4, 4, 4, 4), (8, 8, 8, 8), (8, 8, 8, 0), (5, 5, 5, 0), (r.randrange(9), r.randrange(9), r.randrange(9), r.randrange(9))])])
        regions[0x8bd558] = r.bytes(0x80)
        regions[0x8bd508] = struct.pack('<I', r.choice([0, 0x1000000, r.getrandbits(32)]))
        regions[0x879a2c] = bytearray(struct.pack('<I', r.choice([0, 1])))
        enginedebug.texts(log)
        if op == 'Pick':
            return Call(address, [r.getrandbits(32), FORMAT, r.getrandbits(32)], regions, conv='s', ret='a')
        return Call(address, [FORMAT, r.getrandbits(32)], regions, conv='s', ret='a')
    if op == 'OnZ':
        regions[FORMAT] = struct.pack('<4I', r.getrandbits(32), r.choice([0x400, 0x400, 1]), r.getrandbits(32), r.choice([0x10, 0x10, 0x18, 0x20])) + r.bytes(0x10)
        regions[SECOND] = struct.pack('<2I', r.getrandbits(32), r.choice([0x400, 0x400, 2])) + r.bytes(0x18)
        return Call(address, [r.choice([FORMAT, FORMAT, FORMAT, FORMAT, 0]), r.choice([SECOND, SECOND, SECOND, SECOND, 0])], regions, conv='s', ret='a')
    world(r, log, regions)
    if op == 'Open':
        return Call(address, [SELF, r.choice([0, 0x10, 0x10, 0xef, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 'Close':
        return Call(address, [SELF + 0x228], regions, conv='t', ret='a')
    if op == 'Reopen':
        return Call(address, [SELF + 0x228], regions, conv='t', ret='a')
    if op == 'Clear':
        strings(log, CLEAR, 0, out=1 << 2)
        blob(log, CLEAR, 2, 0xf)
        put(regions[SELF], 0xc, 'I', VIEW)
        floats = [r.choice([0.0, 1.5, -1.5, 639.5, 100000.0, -2.5, 3e9, r.uniform(-5000, 5000)]) for _ in range(4)]
        regions.update({0x8bcbf8: struct.pack('<2f', *floats[:2]), 0x8bcc00: struct.pack('<2f', *floats[2:]), 0x8bcbe8: r.bytes(8),
                        0x8bcbc8: struct.pack('<I', r.choice([0, 0x80000000, 0x4000000, 0x2000, 0x84002000, r.getrandbits(32)])), 0x8bd47c: r.bytes(4)})
        return Call(address, [SELF + 0x228], regions, conv='t', ret='a')
    regions[0x879da8] = struct.pack('<I', r.choice([0, SELF, SELF, SELF]))
    regions[0x8bd52c] = r.bytes(4)
    return Call(address, [], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
