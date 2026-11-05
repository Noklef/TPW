# The ground the world view sees (src/game/render/viewfootprint: Polygon_ScanRows 0x56d600, WorldView_UpdateFootprint 0x56bb00) against the originals: the ray, hull and clip real, Mat4_SetFar and the debug log recorders; the C library real for the log's text.
# usage: viewfootprint.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fb, log_region, put, strings  # noqa: E402

SETFAR, TEXT = 1, 2


def _sprintf(e):
    """The original's sprintf answered by Python's, which rounds a %f tie as the driver's host sprintf does (25.4453125 to 25.445312); the exe's C library rounds it up. What the code passes is what is compared."""
    sp = e.r['ESP'] + (4 if e.tail else 0)
    text = e.read(e.u32(sp + 4), 0x40).split(b'\0')[0] % struct.unpack('<2d', e.read(sp + 8, 16))
    e.write(e.u32(sp), text + b'\0')
    e.r['EAX'] = len(text)


B = Bench([(0x56bb00, 0x56bd53), (0x56d600, 0x56dd4c), (0x56dd50, 0x56dd58), (0x56b940, 0x56baf3), (0x56bd60, 0x56c5a0), (0x56c5a0, 0x56d600), (0x578d20, 0x579020), (0x67a400, 0x692000)],
          hooks={0x578ca0: _recorder(SETFAR, 1), 0x564650: _recorder(TEXT, 1, cdecl=True), 0x67a650: _sprintf})
VIEW, POLYGON, SCAN, OUT, CAM, GRID, ORIGIN = RAM, RAM + 0x400, RAM + 0x600, RAM + 0x800, RAM + 0xa00, RAM + 0xb00, RAM + 0xc00
# The C library's float conversions, set at its start (0x67a610): %f traps without them.
CRT_FLOATS = struct.pack('<5I', 0x680a30, 0x6805d0, 0x680660, 0x680570, 0x680640)


def scene(r, log):
    strings(log, TEXT, 1)
    return struct.pack('<I', r.getrandbits(32) & ~0x1000000 | (r.random() < 0.08) << 24)


def table(r):
    """A scan table, its cells any: what the original leaves in them shows."""
    data = bytearray(r.bytes(0x110))
    put(data, 0, '2i', r.randrange(-1, 0x80), r.randrange(-1, 0x80))
    return data


def polygon(r, logged):
    """Up to 8 vertices in the table's rows, a flat edge now and then; whole numbers now and then. NaNs only if not logged (the bench's sprintf is the host's, which spells them otherwise)."""
    data = bytearray(r.bytes(0xd4))
    n = r.choice([0, 1, 2, 3, 3, 4, 5, 8])
    put(data, 0xd0, 'i', n)
    last = 10.0
    for i in range(n):
        z = r.choice([r.uniform(0, 126), r.uniform(0, 126), float(r.randrange(127)), last, int(last) + r.random()])   # (the last: an edge within one cell)
        x = r.choice([r.uniform(0, 126), float(r.randrange(127)), r.uniform(-200, 300)])
        last = z
        put(data, i * 12, '3I', fb(x), r.f() if r.random() < 0.1 else fb(r.uniform(-9, 9)), fb(z))
    if n and not logged and r.random() < 0.03:
        put(data, r.randrange(n) * 12 + r.choice([0, 8]), 'I', r.choice([0x7fc12345, 0xffc00000]))
    return data


def footprint(r, regions):
    size = r.choice([64.0, 128.0, 256.0])
    step = size / r.choice([64, 128, 128])
    stepz = r.choice([step, size / 128])
    view = bytearray(r.bytes(0x2d8))
    a = r.uniform(-5, 5)
    put(view, 0x30, '2I', fb(a), fb(r.choice([a, r.uniform(-5, 5)])))
    put(view, 0x3c, '4I', 0, fb(size), 0, fb(size))
    put(view, 0x4c, '2I', fb(step), fb(stepz))
    put(view, 0x58, 'i', r.choice([0, 20, 64, 127, 200]))
    t = r.uniform(-4, 4)
    c, s = math.cos(t), math.sin(t)
    m = [c, 0, s, 0, 0, c, -s, 0, -s, s, c, 0, r.uniform(0, size), r.uniform(5, 60), r.uniform(0, size), 1]
    regions.update({VIEW: view, CAM: struct.pack('<16I', *[fb(x) for x in m]), GRID: put(bytearray(r.bytes(0x20)), 0, 'I', ORIGIN), ORIGIN: r.bytes(0x20),
                    0x87afe8: struct.pack('<I', CAM), 0x879d60: struct.pack('<I', GRID), 0x87af58: struct.pack('<I', fb(r.uniform(20, 100))), 0x8bcbcc: struct.pack('<I', fb(r.choice([1.333, 1.0, 1.6]))),
                    0x8bd4c0: struct.pack('<I', fb(r.uniform(50, 2000)))})
    return Call(0x56bb00, [VIEW], regions, conv='t', ret='a')


def case(r):
    log = log_region()
    regions = {LOG: log, 0x8bd508: scene(r, log), 0x77eca0: CRT_FLOATS}
    if r.random() < 0.3:
        return footprint(r, regions)
    regions.update({POLYGON: polygon(r, regions[0x8bd508][3] & 1), SCAN: table(r), OUT: table(r)})
    return Call(0x56d600, [POLYGON, SCAN, OUT, r.getrandbits(32), r.choice([0, 20, 64, 127, 200, -1])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
