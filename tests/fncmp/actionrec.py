# The action recorder (src/game/core/actionrec) against the originals: its record writers. The recorder's first stream is a record stream (the
# driver's write stub, at most `left` bytes to the cursor) so the bytes each record writes are compared, a short `left` cutting a record off; the
# recording and playback flags are 0, 1 or random.
# usage: actionrec.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Write,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, STUBS, Bench, Call  # noqa: E402

B = Bench([(0x403a40, 0x403ee0), (0x5da3c0, 0x5da3c1)])
THIS, VT, BUF = RAM, RAM + 0x40, RAM + 0x80
# name: (address, operands)
OPS = {'Write': (0x403be0, 1), 'WriteBB': (0x403c20, 3), 'WriteBBW': (0x403c80, 4), 'WriteW': (0x403d00, 2), 'WriteWW': (0x403d60, 3),
       'WriteWWW': (0x403dd0, 4), 'WriteWB': (0x403e60, 3)}


def recorder(r):
    """The recorder at THIS: recording, mode, ?, the first stream (vtable VT, cursor BUF, bytes left), the others, playback."""
    o = bytearray(r.bytes(0x30))
    struct.pack_into('<I', o, 0, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    struct.pack_into('<3I', o, 0xc, VT, BUF, r.choice([0, 1, 2, 3, 4, 5, 6, 7, 0x20, 0x20, 0x20, 0x20]))
    struct.pack_into('<I', o, 0x24, r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))
    vt = bytearray(r.bytes(0x10))
    struct.pack_into('<I', vt, 0xc, STUBS + 0x10)
    return {THIS: o, VT: vt, BUF: r.bytes(0x20)}


def case(r):
    address, n = OPS[r.choice(PICK)]
    return Call(address, [THIS] + [r.getrandbits(32) for _ in range(n)], recorder(r), conv='t', ret='v')


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
