# A stream of vtable 0x6fd930 constructed (src/game/files/handles: 0x532670 Stream6fd930_Ctor) against the original, with StreamBase_Ctor.
# usage: stream6fd930.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x532670, 0x532682), (0x5f7180, 0x5f7190)])


def case(r):
    return Call(0x532670, [RAM], {RAM: r.bytes(0x20)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
