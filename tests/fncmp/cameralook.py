# The view matrix from a place and a target (Camera_SetLookAt, src/game/world/camera: 0x5d8de0) against the original: the place on the target or straight below it now and then (the zero length, the side's fallback), NaNs
# seldom, the second object with and without its fields; Camera_SetFlag80, Matrix_Identity and SceneFlags_Touch real.
# usage: cameralook.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, put  # noqa: E402

B = Bench([(0x5d8de0, 0x5d922a), (0x42afd0, 0x42afed), (0x46f150, 0x46f190), (0x5836b0, 0x5836c2)])
SELF, LIGHT = RAM, RAM + 0x100
NANS = [0x7fc12345, 0x7f812345, 0xffc00000, 0x7f800000]


def fnum(r, lo, hi, nan=0.01):
    return struct.pack('<I', r.choice(NANS)) if r.random() < nan else struct.pack('<f', r.uniform(lo, hi))


def case(r):
    log_region()
    self = bytearray(r.bytes(0x40))
    kind = r.choice(['any', 'any', 'any', 'any', 'same', 'above', 'above', 'level'])
    place = [fnum(r, -200, 200) for _ in range(3)]
    target = [fnum(r, -200, 200) for _ in range(3)]
    if kind == 'same':
        target = list(place)
    elif kind == 'above':
        target = [place[0], fnum(r, -200, 200), place[2]]
    elif kind == 'level':  # (the side's x 0 and its z not: no fallback)
        target = [fnum(r, -200, 200), fnum(r, -200, 200), place[2]]
    self[8:0x14] = b''.join(place)
    self[0x14:0x20] = b''.join(target)
    put(self, 0x20, 'I', LIGHT if r.random() < 0.85 else 0)
    put(self, 0x24, 'I', r.choice([0, 1, 1, 1, 0x100, r.getrandbits(32)]))
    self[0x28:0x30] = fnum(r, -50, 50) + fnum(r, -50, 50)
    put(self, 0x30, 'I', r.choice([0, 0, 1, 0x100, r.getrandbits(32)]))
    regions = {LOG: log_region(), SELF: self, LIGHT: r.bytes(0x40), 0x7909f0: r.bytes(0x40), 0x790ab0: r.bytes(4), 0x8bd508: r.bytes(0x10), 0x8bcc40: r.bytes(0x10)}
    return Call(0x5d8de0, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
