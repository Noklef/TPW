# The sprite queue's appends (src/game/render/spritequeue: 0x58a6b0, 0x58a6e0) against the
# originals: lists from empty to full, the second one's counts overrun included (0x101, 0x102).
# usage: spritequeue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Add,Add2)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x58a6b0, 0x58a6de), (0x58a6e0, 0x58a712)])
CALLS = {'Add': 0x58a6b0, 'Add2': 0x58a6e0}


def case(r):
    op = r.choice(OPS)
    queue = bytearray(r.bytes(0x4010))
    counts = [0, 1, 0x7f, 0x80, 0x100, 0x101, 0x102, 0x103, 0x7ff, 0x800, 0x801, r.randrange(0x800)]
    put(queue, 0x2404, 'I', r.choice(counts))
    put(queue, 0x2408, 'I', r.choice(counts))
    return Call(CALLS[op], [RAM, r.getrandbits(32)], {RAM: queue}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
