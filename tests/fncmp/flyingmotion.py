# A flying ride slot's motion (src/game/world/flyingrides: 0x55aa80) against the original: records of random content whose slot has a ride (an index in
# 0x7a4610, each the animator bench's animator: tests/fncmp/animator.py world()) or none, and a last motion (bank, entry, loop) the same as the new one or
# not; banks and entries as the animator bench plays them, loops 0, 1 or 2.
# usage: flyingmotion.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import animator  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x46f150, 0x475010), (0x67a400, 0x690000), (0x55aa80, 0x55ab43), (0x5da3c0, 0x5da3c1)], sines=256)
REC = RAM + 0x20000


def case(r):
    regs, nlayers, blocks = animator.world(r)
    index = r.choice([1, 2, 99, r.randrange(1, 100)])
    slot = r.choice([0, 1, 19, r.randrange(20)])
    bank = r.choice([0, 1, 2, 5, r.randrange(12), 0xc, 0xd, 0xe])
    entry, loop = r.randrange(4), r.choice([0, 1, 2])
    rec = bytearray(r.bytes(0x122c))
    s = 0x5c + slot * 0xe4
    put(rec, s + 0xc0, 'I', r.choice([0, 1, 2, 3, 3]))
    if r.random() < 0.3:
        put(rec, s + 0xcc, '3I', bank, entry, loop)
    elif r.random() < 0.5:
        put(rec, s + 0xcc, '3I', r.choice([bank, bank ^ 1]), r.choice([entry, entry ^ 1]), r.choice([loop, 1 - loop if loop < 2 else 0]))
    table = put(bytearray(r.bytes(0x194)), index * 4, 'I', REC)
    rides = put(bytearray(0x10), 4, '3I', animator.ANIM, animator.ANIM, animator.ANIM)
    regs.update({REC: rec, 0x8791f8: table, 0x7a4610: rides})
    return Call(0x55aa80, [index, slot, bank, entry, loop], regs)


if __name__ == '__main__':
    B.main(case)
