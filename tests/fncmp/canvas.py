# The blending canvas's set-up (src/game/gfx/canvas: 0x617370-0x6175b3) against the originals: the four drivers' static initialisers (their
# globals 0xfa7e30-0xfa7e3f random), the constructor, SetSurface with a surface of the old format or another (depths 8 to 32 or odd, paletted or
# not; now and then none) and the clip's address.
# usage: canvas.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Init8,Init16,Init24,Init32,Ctor,SetSurface,Clip)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x617370, 0x6175b4), (0x5fa680, 0x5fa6a0)])
CANVAS, INFO = RAM, RAM + 0x1000
DRIVERS = 0xfa7e30   # (32, 24, 16, 8 bits)
CALLS = {'Init8': 0x617370, 'Init16': 0x617390, 'Init24': 0x6173b0, 'Init32': 0x6173d0, 'Ctor': 0x6173e0, 'SetSurface': 0x617420, 'Clip': 0x6175b0}


def surface_format(r):
    bits = r.choice([8, 16, 24, 32, 15, 0, r.getrandbits(32)])
    return [r.choice([0, 0xff0000, 0xf800, 0x7c00, r.getrandbits(32)]), r.choice([0, 0xff00, 0x7e0, 0x3e0, r.getrandbits(32)]),
            r.choice([0, 0xff, 0x1f, r.getrandbits(32)]), r.choice([0, 0, 0xff000000, r.getrandbits(32)]), bits,
            r.choice([0, 0, 0, 1, r.getrandbits(32)])]


def case(r):
    op = r.deal(OPS)
    canvas = bytearray(r.bytes(0x870))
    regions = {CANVAS: canvas, DRIVERS: r.bytes(0x10)}
    address = CALLS[op]
    if op.startswith('Init'):
        return Call(address, [], regions)
    if op == 'SetSurface':
        old = surface_format(r)
        put(canvas, 8, '6I', *old)
        info = bytearray(r.bytes(0x28))
        put(info, 0, '6I', *(old if r.random() < 0.3 else surface_format(r)))
        if r.random() < 0.3:   # (one field differing)
            k = r.randrange(6)
            put(info, 4 * k, 'I', r.getrandbits(32) if r.random() < 0.5 else (struct.unpack_from('<I', info, 4 * k)[0] ^ 1))
        regions[INFO] = info
        return Call(address, [CANVAS, INFO if r.random() < 0.95 else 0], regions, conv='t')
    return Call(address, [CANVAS], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
