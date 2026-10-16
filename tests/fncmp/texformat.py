# The texture format helpers (src/game/render/texformat: 0x581790, 0x5839a0, 0x5839e0) against the originals:
# any settings bytes, any current format; the driver flags over any bits, with and without a request or texture.
# usage: texformat.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetArgb32,Table)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x581790, 0x581880), (0x590c40, 0x590ca8), (0x590fb0, 0x590fcb), (0x5839a0, 0x5839ee)])
CALLS = {'DriverFlags': 0x581790, 'SetArgb32': 0x5839a0, 'Table': 0x5839e0}


def case(r):
    op = r.choice(OPS)
    if op == 'DriverFlags':
        obj = bytearray(r.bytes(0x10))
        put(obj, 4, 'I', RAM + 0x100)
        put(obj, 8, 'I', r.getrandbits(32))
        flags = struct.pack('<I', r.getrandbits(32))
        request = bytearray(r.bytes(0x20))
        put(request, 0, 'I', RAM + 0x200 if r.random() < 0.8 else 0)
        regions = {RAM: obj, RAM + 0x100: flags, RAM + 0x200: r.bytes(0x10), RAM + 0x300: request,
                   RAM + 0x400: r.bytes(0x60), 0x8bcbc8: r.bytes(4),
                   0x8bd520: r.bytes(4) + struct.pack('<I', r.choice([1, 1, 0, 4]))}
        tex = RAM + 0x400 if r.random() < 0.85 else 0
        if r.random() < 0.5:
            t = bytearray(r.bytes(0x60))
            put(t, 8, 'I', r.getrandbits(32) | r.choice([0, 0x400, 0x30000, 0x100000]))
            put(t, 0x3c, 'I', r.choice([0, 0, 1]))
            regions[RAM + 0x400] = t
        return Call(0x581790, [RAM, RAM + 0x300, tex], regions, conv='t', ret='a')
    settings = bytearray(r.bytes(0x90))
    put(settings, 4, 'I', r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    return Call(CALLS[op], [RAM], {RAM: settings}, conv='t', ret='a' if op == 'Table' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
