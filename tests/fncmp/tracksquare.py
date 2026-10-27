# The track's 2x2 square test (src/game/world/tracksquare: TrackSquare_Check 0x5286a0) against the original: (x, y) on the map mostly, its
# edges and beyond now and then; the map [0x7cf83c]'s four cells; the kind being built 0x81ecac (0-3; the flag 0x8187f8 that hides it set
# now and then); skip and strict 0 or not, the direction a bit or any dword. Agent B's unported cell test 0x532fc0 a thiscall recorder (k 9
# of 3, driver.cpp's kTrackSquareEnv) answering from a table (0 a quarter of the time).
# usage: tracksquare.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region  # noqa: E402

B = Bench([(0x5286a0, 0x528a70), (0x4d8300, 0x4d8340), (0x4d8c20, 0x4d8c60), (0x52faf0, 0x52fb10), (0x5da3c0, 0x5da3c1)],
          hooks={0x532fc0: _recorder(9, 3)})
MAP, TABLE = RAM + 0x100000, RAM + 0x1000


def case(r):
    x, y = r.randrange(127), r.randrange(127)
    if r.random() < 0.2:
        x, y = r.choice([-1, 0, 126, 127, 128, x]), r.choice([-1, 0, 126, 127, 128, -0x80000000, y])
    log = log_region()
    regions = {LOG: log, 0x7cf83c: struct.pack('<I', MAP), 0x81ecac: struct.pack('<I', r.choice([0, 1, 2, 3, 1, 2, r.getrandbits(32)])),
               0x8187f8: struct.pack('<I', r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))}
    if 0 <= x < 127 and 0 <= y < 127:
        for row in (y, y + 1):
            regions[MAP + 0x294 + (((row << 7) + x + 1) & 0xffff) * 0x44] = r.bytes(0x88)
    regions[TABLE] = struct.pack('<48I', *[r.choice([0, 1, 1, 0x100, 0x80000000, r.getrandbits(32)]) for _ in range(48)])
    answers(log, 9, TABLE)
    flag = lambda: r.choice([0, 0, 1, 0x100, r.getrandbits(32)])  # noqa: E731
    direction = r.choice([1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0, r.getrandbits(8), r.getrandbits(32)])
    return Call(0x5286a0, [x & 0xffffffff, y & 0xffffffff, r.getrandbits(32), flag(), flag(), direction], regions, ret='b')


if __name__ == '__main__':
    B.main(case)
