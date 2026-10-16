# The procedural terrain texture's view rectangle (src/game/terrain/procedural: 0x56df70) against
# the original: with and without the object.
# usage: terraintexrect.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x56df70, 0x56dfa2)])


def case(r):
    tex = r.choice([RAM, RAM, RAM, 0])
    args = [r.getrandbits(32) for _ in range(4)]
    return Call(0x56df70, args, {0x879d60: struct.pack('<I', tex), RAM: r.bytes(0x248)})


if __name__ == '__main__':
    B.main(case)
