# The device objects made and destroyed (src/game/render/devicecreate: 0x572c40, 0x573a30, 0x573b30, 0x573b90) against the originals, the callees recorders (driver.cpp's kDeviceCreateCalls): there already or not, the allocation failing, the open failing, the raster interface null or not, the log on or off, the blocks set or not.
# usage: devicecreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

OPEN3D, OPENSOFT, RASTERINIT, TRACKFREE, CRTDOWN, ERROR, TEXT, ASK = 20, 21, 22, 23, 24, 25, 26, 0   # (ASK: the surface's slot 0, QueryInterface)
CALLS = {'D3d': (0x572c40, 0x572d50), 'Square': (0x573a30, 0x573b30), 'Destroy': (0x573b30, 0x573b90), 'Open': (0x573b90, 0x573c94)}
B = Bench(list(CALLS.values()), hooks={0x573080: _recorder(OPEN3D, 1), 0x678e70: _recorder(RASTERINIT, 4, cdecl=True), 0x579b80: _recorder(TRACKFREE, 1, cdecl=True), 0x679270: _recorder(CRTDOWN, 0, cdecl=True), 0x56af20: _recorder(ERROR, 2), 0x5645e0: _recorder(TEXT, 1, cdecl=True)})
OPS = list(CALLS) * 2 + ['D3d there', 'Square there', 'Destroy none', 'D3d no memory', 'Square no memory', 'D3d fails']
DIRECT, SURFACE, VT, OBJECT = RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400


def tables():
    """The two classes' tables (0x701670, 0x7016a8; the second dword the virtual base's place, as in the game: 0x228, 0x14)."""
    return {0x701670: struct.pack('<II', 0x12345678, 0x228), 0x7016a8: struct.pack('<II', 0x12345678, 0x14)}


def case(r):
    op = r.deal(OPS)
    name = op.split()[0]
    log = log_region()
    strings(log, TEXT, 1)
    set_return(log, OPEN3D, 0 if op == 'D3d fails' else r.choice([1, 2, -1]))
    set_return(log, RASTERINIT, r.getrandbits(32))
    strings(log, ASK, 0, 1 << 2)
    raster = r.choice([0, RAM + 0x800, r.getrandbits(32)])
    fill(log, ASK, 2, struct.pack('<I', raster))
    object_ = bytearray(r.bytes(0x20))
    if name == 'Destroy' and op != 'Destroy none':
        put(object_, 4, 'I', r.choice([0, r.getrandbits(32)]))
        put(object_, 8, 'I', r.choice([0, r.getrandbits(32)]))
    there = RAM + 0xf00 if 'there' in op else 0
    regions = {LOG: log, OBJECT: object_, DIRECT: put(bytearray(r.bytes(0x20)), 0x14, 'I', SURFACE), SURFACE: struct.pack('<I', VT), VT: vtable({}, 8, com={ASK: 3}), HEAP: heap(0x400, 1 if 'no memory' in op else 0), **tables(),
               0x707550: r.bytes(0x10), 0x879a24: struct.pack('<I', DIRECT), 0x879da8: struct.pack('<I', there), 0x879dac: struct.pack('<I', there if name != 'Destroy' else 0 if op == 'Destroy none' else OBJECT), 0x8bd52c: r.bytes(4), 0x8bd4ac: struct.pack('<I', r.choice([256, 512, r.randrange(1, 1 << 16)])),
               0x8bd508: struct.pack('<I', r.getrandbits(32) & ~0x2000000 | r.choice([0, 0x2000000])), 0x8bcbd0: struct.pack('<II', r.choice([640, 800, r.randrange(1, 4096)]), r.choice([480, 600, r.randrange(1, 4096)])), 0xfbe014: r.bytes(0x24)}
    if name == 'Open':
        return Call(CALLS[name][0], [OBJECT, r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='t')
    if name == 'D3d':
        return Call(CALLS[name][0], [r.choice([1, 1, 0, 2, 4, r.getrandbits(32)])], regions)
    return Call(CALLS[name][0], [], regions)


if __name__ == '__main__':
    B.main(case)
