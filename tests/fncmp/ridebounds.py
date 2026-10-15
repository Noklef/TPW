# A ride's box in the world (src/game/coaster/ridebounds: 0x466b70) against the original:
# rides (or none) in the 0x7a4610 table, with or without their +0xcc block, tile corners of
# any 16 bits, heights and the root's y as floats.
# usage: ridebounds.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x466b70, 0x466c7f)])
RIDE, BLOCK, HOLDER, MODEL, ROOT, OUT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x220, RAM + 0x300, RAM + 0x400


def case(r):
    ram = bytearray(r.bytes(0x500))
    put(ram, 0xcc, 'I', r.choice([BLOCK, BLOCK, BLOCK, 0]))
    corner = lambda: r.choice([r.randrange(256), r.getrandbits(16), 0, 0xffff])  # noqa: E731
    put(ram, 0xc0, 'HHHH', corner(), corner(), corner(), corner())
    put(ram, 8, 'I', HOLDER)
    put(ram, HOLDER - RAM + 4, 'I', MODEL)
    put(ram, MODEL - RAM + 0x78, 'I', ROOT)
    scale = r.choice([1, 100, None])
    r.floats(ram, [BLOCK - RAM + 0x1c, BLOCK - RAM + 0x28, ROOT - RAM + 0x44], scale)
    index = r.randrange(8)
    table = bytearray(r.bytes(32))
    put(table, 4 * index, 'I', r.choice([RIDE, RIDE, RIDE, 0]))
    return Call(0x466b70, [index, OUT], {RAM: ram, 0x7a4610: table}, ret='a')


if __name__ == '__main__':
    B.main(case)
