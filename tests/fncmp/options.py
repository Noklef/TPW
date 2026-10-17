# The game options' revert / keep and setters (src/game/core/options: 0x423ad0-0x423ba0) against the
# originals, over junk with +0x94 either sign and values equal to the field or not; the sound settings applied (0x423dd0); the defaults (0x423690).
# usage: options.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x423690, 0x423714), (0x423ad0, 0x423afc), (0x423b00, 0x423b36), (0x423b40, 0x423b4a), (0x423b60, 0x423b71),
           (0x423b80, 0x423b91), (0x423ba0, 0x423bb1), (0x423dd0, 0x423e89), (0x51b3c0, 0x51b3e0), (0x51bb80, 0x51bc50)])
OPT = RAM + 0x100
SETS = {0x423b40: 0x30, 0x423b60: 4, 0x423ba0: 8, 0x423b80: 0xc}
OPTIONS = 0x78d8d8   # the game's options (0x423dd0 applies their sound settings)


def apply_sound(r):
    """0x423dd0: each of the four sound switches (+0x10, +0x18, +0x20, +0x28 bytes) on, off or junk above the low byte,
    the volumes (+0x14, +0x1c, +0x24, +0x2c) about 0 to 100, either sign and wild; the sound globals and the scale (0x785914) junk."""
    options = bytearray(r.bytes(0x40))
    for at in (0x10, 0x18, 0x20, 0x28):
        put(options, at, 'I', r.choice([0, 1, 1, 0x100, 0xff, r.getrandbits(32)]))
        put(options, at + 4, 'I', r.choice([0, 1, 50, 99, 100, 101, -1, -100, r.randrange(0, 101), r.getrandbits(32)]) & 0xffffffff)
    scale = put(bytearray(r.bytes(8)), 4, 'I', r.choice([0, 50, 100, r.randrange(0, 101), r.getrandbits(32)]))
    return Call(0x423dd0, [], {OPTIONS: options, 0x803ab0: r.bytes(0x40), 0x785910: scale, 0x802ba0: r.bytes(0x10)})


def defaults(r):
    """0x423690: on the game's options or elsewhere; the defaults (0x78591c-0x785934) junk, the switch at 0x785924 zero or not."""
    at = r.choice([OPTIONS, OPT])
    balance = bytearray(r.bytes(0x28))
    put(balance, 0x14, 'I', r.choice([0, 0, 1, 0x100, r.getrandbits(32)]))
    for k in (0x18, 0x1c, 0x20, 0x24):
        put(balance, k, 'I', r.choice([0, 50, 100, r.randrange(0, 101), r.getrandbits(32)]))
    regions = {OPTIONS: r.bytes(0x80), 0x785910: balance, 0x803ab0: r.bytes(0x40), 0x802ba0: r.bytes(0x10)}
    if at == OPT:
        regions[RAM] = r.bytes(0x200)
    return Call(0x423690, [at], regions, conv='t')


def case(r):
    if r.random() < 0.2:
        return defaults(r)
    if r.random() < 0.25:
        return apply_sound(r)
    mem = put(r.bytes(0x400), 0x194, 'i', r.choice([-2, -1, 0, 1, 2, r.getrandbits(31), -r.getrandbits(31)]))
    at = r.choice([0x423ad0, 0x423b00] + list(SETS))
    if at in SETS:
        cur = struct.unpack_from('<I', mem, 0x100 + SETS[at])[0]
        return Call(at, [OPT, r.choice([cur, r.getrandbits(32)])], {RAM: mem}, conv='t')
    return Call(at, [OPT], {RAM: mem}, conv='t')


if __name__ == '__main__':
    B.main(case)
