# The sound settings' setters and flag, and the sound commands' base constructor
# (src/game/core/soundset: 0x51bb60-0x51bc20, 0x51e6f0, 0x51e710, 0x51c1c0, 0x51c6f0, 0x51bb40)
# against the originals.
# usage: soundset.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x51bb40, 0x51bb52), (0x51bb60, 0x51bc34), (0x51e6f0, 0x51e704), (0x51e710, 0x51e726),
           (0x51c1c0, 0x51c1cc), (0x51c6f0, 0x51c6f6)])


def case(r):
    value = r.choice([0, 1, 100, 0xffff, 0x10000, r.getrandbits(32)])
    fn = r.choice([0x51bb60, 0x51bb80, 0x51bba0, 0x51bbc0, 0x51bbe0, 0x51bc00, 0x51bc20, 0x51e6f0, 0x51e710,
                   0x51c1c0, 0x51c6f0, 0x51bb40])
    if fn == 0x51c6f0:
        return Call(fn, [], {0x802bc0: r.bytes(0x10)}, ret='a')
    if fn == 0x51bb40:
        return Call(fn, [RAM + 4, value], {RAM: r.bytes(0x10)}, conv='t', ret='a')
    return Call(fn, [value], {0x803ab0: r.bytes(0x40)})


if __name__ == '__main__':
    B.main(case)
