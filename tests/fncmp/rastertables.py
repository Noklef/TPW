# The software rasteriser's small routines (src/game/render/rasterformat: 0x679150-0x6792c0) against the originals: _nh_malloc a recorder answering from a table, free the heap's hash.
# Bench limit: no empty mask for the lowest bit (it never ends, in both).
# usage: rastertables.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _free, _recorder, answers, heap, log_region  # noqa: E402

NHMALLOC = 28
B = Bench([(0x679150, 0x67916d), (0x679170, 0x679184), (0x679190, 0x6791ea), (0x6791f0, 0x679227), (0x679230, 0x679267), (0x679270, 0x679281), (0x679290, 0x6792b7), (0x6792c0, 0x6792e7), (0x67dcd0, 0x67dcf0)],
          hooks={0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x67ad90: _free})
A_MALLOC = RAM
OPS = [n for n in ('bits', 'low', 'tables', 'frame', 'frame', 'z', 'z', 'freeframe', 'freez', 'freeall', 'freeall') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def mask(r):
    return r.choice([0xf800, 0x7e0, 0x1f, 0xff0000, 0x80000000, 0x40000000, 0xffffffff, 0x7fffffff, 1, r.getrandbits(32), r.getrandbits(32) << r.randrange(32) & 0xffffffff])


def case(r):
    name = r.deal(OPS)
    if name == 'bits':
        return Call(0x679150, [r.choice([0, mask(r)])], {}, conv='c', ret='a')
    if name == 'low':
        return Call(0x679170, [mask(r) or 1], {}, conv='c', ret='a')
    if name == 'tables':
        return Call(0x679190, [], {0xfbe2e0: r.bytes(0x400)}, conv='c')
    log = log_region()
    answers(log, NHMALLOC, A_MALLOC)
    blocks = [r.choice([HEAP + 0x100 * (i + 1), HEAP + 0x100 * (i + 1) + r.randrange(0x20)]) for i in range(2)]
    regions = {LOG: log, HEAP: heap(0x1000), A_MALLOC: struct.pack('<4I', r.choice([blocks[0], blocks[0], 0]), 0, 0, 0), 0xfb1548: struct.pack('<I', r.randrange(2)),
               0xfac514: r.bytes(0x18) + struct.pack('<4I', r.choice([0, 0, HEAP + 0x300]), r.getrandbits(32), r.choice([0, 0, HEAP + 0x400]), r.getrandbits(32))}
    if name in ('frame', 'z'):
        return Call(0x6791f0 if name == 'frame' else 0x679230, [r.choice([0, 1, 640, 0x10000, r.getrandbits(32)]), r.choice([0, 1, 480, 0x10000, r.getrandbits(32)])], regions, conv='c', ret='a')
    return Call({'freeframe': 0x679290, 'freez': 0x6792c0, 'freeall': 0x679270}[name], [], regions, conv='c')


if __name__ == '__main__':
    B.main(case)
