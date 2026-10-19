# The weather's lightning bolt ends (src/game/world/weather: 0x512c50) against the original: the map's size in cells ([0x7a0314] /
# [0x7a0374]: 0, 1, the game's 128-256, odd ones, past 2^24 / 10 and any), any world seed (World_Rand on [0x7cf83c]), the end apart
# from the start or overlapping it.
# usage: weather.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x512c50, 0x512e01), (0x516330, 0x516390)])
WEATHER, START, END, WORLD = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000


def size(r):
    return r.choice([0, 1, 2, 128, 128, 192, 256, 256, r.randrange(1, 300), 0x1999999, 0x2000000, 0xffffffff, r.getrandbits(32)])


def case(r):
    regions = {WEATHER: r.bytes(0x70), RAM + 0x100: r.bytes(0x200), WORLD: r.bytes(0x10),
               0x7cf83c: struct.pack('<I', WORLD - 0x1da708), 0x7a0314: struct.pack('<I', size(r)), 0x7a0374: struct.pack('<I', size(r))}
    end = r.choice([END, END, END, END, START, START + 4, START + 8, START - 4])
    return Call(0x512c50, [WEATHER, START, end, r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
