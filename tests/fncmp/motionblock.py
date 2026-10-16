# Block_MarkModel (src/game/motion/track: 0x470b30) against the original: blocks with tracks,
# without, or none.
# usage: motionblock.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x470b30, 0x470b54)])
HOLDER, MODEL, BLOCK = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    ram = put(r.bytes(0x400), 4, 'I', MODEL)
    put(ram, 0x212, 'H', r.choice([0, 0, 1, r.getrandbits(16)]))
    return Call(0x470b30, [HOLDER, r.choice([BLOCK, BLOCK, 0])], {RAM: ram}, ret='a')


if __name__ == '__main__':
    B.main(case)
