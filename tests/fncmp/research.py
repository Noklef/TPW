# The research lab's group and project queries (src/game/world/research: 0x503850, 0x5039f0,
# 0x503b30, 0x504040, 0x504750) against the originals: current projects none or any, categories
# matching or 5, mResearchDone about mCost (equal, NaNs, infinities), project counts 0 to 0x100 and
# negative.
# usage: research.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Effort,ObjId,Count,NotDone,Ctor)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x503850, 0x503861), (0x5039f0, 0x503a20), (0x503b30, 0x503b79), (0x504040, 0x504081),
           (0x504750, 0x50475e)])
LAB = RAM
CALLS = {'Effort': 0x503850, 'ObjId': 0x5039f0, 'Count': 0x503b30, 'NotDone': 0x504040, 'Ctor': 0x504750}
FLOATS = [0.0, -0.0, 1.0, 100.0, 1e30, float('inf'), float('-inf'), float('nan')]


def fbits(r):
    if r.random() < 0.1:
        return r.choice([0x7fa00000, 0xffc00001, 0x7f800001])  # signalling and other NaNs
    return struct.unpack('<I', struct.pack('<f', r.choice(FLOATS + [r.uniform(-50, 200)])))[0]


def case(r):
    op = r.choice(OPS)
    lab = bytearray(r.bytes(0x1480))
    for i in range(0x100):
        p = 0xc + i * 20
        put(lab, p + 4, 'I', r.choice([0, 1, 2, 3, 4, 5, r.getrandbits(32)]))
        cost = fbits(r)
        put(lab, p + 0xc, 'II', cost, r.choice([cost, cost, fbits(r)]) if r.random() < 0.5 else fbits(r))
    for g in range(5):
        put(lab, 0x140c + g * 16 + 8, 'i', r.choice([-1, -2, 0, r.randrange(0x100), r.randrange(0x100)]))
    put(lab, 0x145c, 'i', r.choice([0, 1, 2, r.randrange(0x101), 0x100, -1, -5]))
    group = r.choice([0, 1, 2, 3, 4, r.randrange(5)])
    if op == 'Ctor':
        return Call(0x504750, [LAB + 0x140c + group * 16], {LAB: lab}, conv='t', ret='a')
    if op == 'Count':
        return Call(0x503b30, [LAB, r.choice([0, 1, 2, 3, 4, 5, 5, r.getrandbits(32)])], {LAB: lab}, conv='t', ret='a')
    if op == 'ObjId':
        return Call(0x5039f0, [LAB, RAM + 0x1500 + r.randrange(4), group], {LAB: lab, RAM + 0x1500: r.bytes(8)},
                    conv='t', ret='a')
    return Call(CALLS[op], [LAB, group], {LAB: lab}, conv='t', ret='b' if op == 'NotDone' else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
