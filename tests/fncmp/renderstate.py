# The render state setters (src/game/render/state: 0x5836b0, 0x5d9680-0x5d96c0) against the
# originals: flags words in any state, fog settings of any bits.
# usage: renderstate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Touch,FogOn,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5836b0, 0x5836c2), (0x5d9680, 0x5d96c4), (0x5d96c0, 0x5d974f)])
CALLS = ['Touch', 'FogOn', 'FogOff', 'SetFog', 'FadeFog']


def case(r):
    op = r.choice(OPS)
    regions = {RAM: r.bytes(0x10), 0x8bcbc0: r.bytes(0x10), 0x8bcc40: r.bytes(8), 0x8bd470: r.bytes(0x30)}
    if op == 'FadeFog':
        fade = bytearray(r.bytes(0x60))
        for off in (0x44, 0x48, 0x4c, 0x50, 0x54, 0x58):
            put(fade, off, 'i', r.choice([0, 0x10000, -0x10000, 0xff0000, r.randrange(-2**31, 2**31), r.randrange(-0x2000000, 0x2000000)]))
        if r.random() < 0.3:
            put(fade, 0x50, 'ii', *struct.unpack_from('<ii', fade, 0x44)[:2])
            put(fade, 0x58, 'i', struct.unpack_from('<i', fade, 0x4c)[0])
        regions.update({0x785880: bytes(8) + struct.pack('<i', r.choice([0, 1, 2, 5, -1])) + bytes(4),
                        0x78d8d0: bytes(8) + struct.pack('<I', r.choice([1, 1, 0, 2])) + bytes(4), 0x8c2780: r.bytes(0x410)})
        regions[RAM] = fade
        return Call(0x5d96c0, [RAM], regions, conv='t', ret='a')
    if op == 'Touch':
        return Call(0x5836b0, [RAM], regions, conv='t')
    if op == 'SetFog':
        return Call(0x5d96a0, [RAM, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t')
    return Call({'FogOn': 0x5d9680, 'FogOff': 0x5d9690}[op], [RAM], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
