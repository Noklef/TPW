# The draw buffer's methods (src/game/render/drawbuffer: 0x582390, 0x582640-0x5827f0) against the
# originals: streams 0 to 3, batch slots in and around the table, modes with and without 4,
# scratch areas full and not.
# usage: drawbuffer.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Reset,Alloc,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x582390, 0x5823ed), (0x582640, 0x582804)])
CALLS = {'Reset': 0x582390, 'Alloc': 0x582640, 'Begin': 0x582670, 'Vertices': 0x5826f0, 'Resume': 0x582710,
         'End': 0x582780, 'VertexLimit': 0x5827d0, 'IndexLimit': 0x5827f0}
BUF = RAM + 0x20          # stream 0's record would be at BUF - 0x10
BATCHES = RAM + 0x100     # 16 batches of 0x38


def case(r):
    op = r.choice(OPS)
    mem = bytearray(r.bytes(0x100 + 16 * 0x38))
    b = 0x20
    put(mem, b + 0, 'I', r.choice([0, 1, 2, 3, 4, 5, 6, 7, r.getrandbits(32)]))
    put(mem, b + 4, 'I', r.randrange(4))
    put(mem, b + 0x44, 'I', BATCHES)
    put(mem, b + 0x48, 'I', r.choice([0, 1, 8, 16, r.randrange(17)]))
    for off in (0x4c, 0x50, 0x54):
        put(mem, b + off, 'I', r.randrange(16))
    if op == 'Reset':
        put(mem, b + 0x54, 'I', r.choice([-2, -1, 0, 7, 15, 16, r.randrange(16)]) & 0xffffffff)
    start = r.getrandbits(32)
    put(mem, b + 0x5c, 'I', start)
    put(mem, b + 0x3c, 'I', r.choice([start, start + 0x100, start + 0x10000, r.getrandbits(32)]) & 0xffffffff)
    regions = {RAM: mem}
    if op == 'Reset':
        args = [BUF, r.choice([0, 1, 2, 3, 0x100, 0x101, r.getrandbits(32)])]
    elif op == 'Alloc':
        args = [BUF, r.choice([0, 1, 0x10, 0x1000, 0xffff, r.getrandbits(32)])]
    elif op in ('Begin', 'Resume'):
        args = [BUF, r.choice([0, 1, 2, 3, 4, 5, 6, 7, r.getrandbits(32)])]
    elif op == 'End':
        args = [BUF, r.getrandbits(32), r.getrandbits(32)]
    else:
        args = [BUF]
    void = op in ('Reset', 'End')
    return Call(CALLS[op], args, regions, conv='t', ret='v' if void else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
