# The channel volumes (src/game/core/volumes: 0x503870, 0x503900) against the originals: volumes and
# master about 0, 100 and beyond, sums of zero, large volumes that make the percentage overflow,
# channels 0 to 4 and a few outside.
# usage: volumes.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x503870, 0x5038f7), (0x503900, 0x503972)])


def case(r):
    obj = bytearray(r.bytes(0x1500))
    pick = lambda: r.choice([0, 0, 1, 50, 99, 100, 101, 0x28f5c3, 0x28f5c4, 0xffffffff, r.randrange(0x200), r.getrandbits(32)])
    for i in range(5):
        put(obj, 0x140c + 0x10 * i, 'I', pick())
    put(obj, 0x1460, 'I', pick())
    if r.random() < 0.5:
        return Call(0x503900, [RAM, pick()], {RAM: obj}, conv='t')
    channel = r.choice([0, 1, 2, 3, 4, 4, 5, 0xf])
    return Call(0x503870, [RAM, channel, pick()], {RAM: obj}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
