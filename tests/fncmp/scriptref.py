# An object's reference to a sprite script (src/game/sprite/scriptref: 0x4d4100, 0x4d4140, 0x4d4190)
# against the originals, with the script list not ready (0x7b49f0 zero: the script calls then do
# little, as in the KillSlot cases): references with and without an animation or period, entry point
# numbers about the table.
# usage: scriptref.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=IsAt,Start,Apply)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4d4100, 0x4d4134), (0x4d4140, 0x4d416d), (0x4d4190, 0x4d41d0), (0x475010, 0x476a30), (0x540b90, 0x540d78),
           (0x541f60, 0x541f70), (0x5da3c0, 0x5da3c1)])
CALLS = ['IsAt', 'Start', 'Apply']


def case(r):
    op = r.choice(OPS)
    ref = bytearray(r.bytes(0x20))
    put(ref, 0, 'I', r.choice([0, 1, 2, 5, 0xffffffff, r.randrange(8)]))
    put(ref, 4, 'I', r.choice([0, 0, 1, 3, 7, r.randrange(16)]))
    put(ref, 8, 'I', r.choice([0, 0, 1, 100, r.getrandbits(32)]))
    regions = {RAM: ref, 0x7b49f0: bytearray(16)}
    anim = r.choice([0, 1, 2, 3, 7, r.randrange(16)])
    if op == 'IsAt':
        return Call(0x4d4100, [RAM, anim], regions, conv='t', ret='a')
    if op == 'Start':
        return Call(0x4d4140, [RAM, anim, r.randrange(4), r.randrange(4)], regions, conv='t')
    return Call(0x4d4190, [RAM], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
