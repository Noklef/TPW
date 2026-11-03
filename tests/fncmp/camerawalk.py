# The camera's walk mode toggled (src/game/world/camera: Camera_ToggleWalkMode 0x42ae70) against the original: Camera_SetMode (k 30), Sprites_UseGroundFrames (k 28) and Sprites_UseParkFrames
# (k 29) recorders, the map cell's accessors as they are. The flags 0x790ab0 with a camera mode on or not; the point 0x7a1c70 / 0x7a1c78 on the map, off it or not a number; the map's sizes;
# the cell under the point (+8 a kind walked on or not).
# usage: camerawalk.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

B = Bench([(0x42ae70, 0x42afc2), (0x536310, 0x536340), (0x536390, 0x5363b0), (0x67a830, 0x67a860)],
          hooks={0x42ab20: _recorder(30, 6, cdecl=True), 0x542420: _recorder(28, 0, cdecl=True), 0x542640: _recorder(29, 0, cdecl=True)})
WORLD, MAP = RAM, RAM + 0x1000
CELL = 0x44
TENTH = struct.unpack('<f', struct.pack('<f', 0.1))[0]


def place(r, size):
    v = r.deal([r.uniform(0, size * 10), r.uniform(0, size * 10), r.uniform(0, size * 10), r.uniform(-50, 0), r.uniform(size * 10, size * 30), r.randrange(0, size + 1) * 10.0, float('nan'), 1e30])
    return struct.pack('<f', v)


def column(raw, size):
    v = struct.unpack('<f', raw)[0]
    if v != v or abs(v) >= 1e18:
        whole = 0
    else:
        whole = int(struct.unpack('<f', struct.pack('<f', v * TENTH))[0])
    return min(max(whole, 0), size - 1)


def case(r):
    log = log_region()
    w, h = r.deal([0x80, 0x80, 0x40, 3, 1]), r.deal([0x80, 0x40, 0x10, 3, 1])
    x, z = place(r, w), place(r, h)
    point = bytearray(r.bytes(0xc))
    point[0:4], point[8:0xc] = x, z
    flags = r.getrandbits(32) & ~0x16 | r.deal([0, 0, 0, 2, 4, 0x10, 0x16])
    regions = {LOG: log, 0x790ab0: struct.pack('<I', flags), 0x7a1c70: point, 0x7a0314: struct.pack('<I', w), 0x7a0374: struct.pack('<I', h), 0x7908f0: r.bytes(0xc), 0x790ad0: r.bytes(0xc),
               0x790a38: r.bytes(4), 0x790a9c: r.bytes(4), 0x7cf6ec: struct.pack('<I', WORLD), WORLD: struct.pack('<I', MAP)}
    index = (column(z, h) << 7) + column(x, w) + 1
    around = bytearray(r.bytes(3 * CELL))
    for k in range(3):
        put(around, k * CELL + 8, 'I', r.choice([2, 4, 5, 8, 0x1d, 0x1f, 0x100, r.getrandbits(32)]))
    put(around, CELL + 8, 'I', r.deal([0x1e, 1, 3, 9, 0, 2, 5, 0xa, 0x1f, r.getrandbits(32)]))
    regions[MAP + 0x294 + (index - 1) * CELL] = around
    return Call(0x42ae70, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
