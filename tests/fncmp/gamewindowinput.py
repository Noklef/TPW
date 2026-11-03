# The game window's handler (src/game/ui/gamewindowinput: 0x4a2840) against the original. Every callee a recorder as driver.cpp's kGameWindowInputEnv.
# usage: gamewindowinput.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, set_return  # noqa: E402

CAPTURE, RELEASE, CAMERA, MODS, UP, DOWN, DEFAULT, OPEN, FIND, TOGGLE, KEY15 = range(10, 21)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4a2840, 0x4a29ab)],
          hooks={0x65e549: _recorder(CAPTURE, 0), 0x65e56e: _recorder(RELEASE, 0), 0x42a760: C(CAMERA, 3), 0x46b420: C(MODS, 0), 0x40c990: _recorder(UP, 2), 0x40c900: _recorder(DOWN, 2),
                 0x65f66e: _recorder(DEFAULT, 3), 0x48c830: C(OPEN, 1), 0x40c870: _recorder(FIND, 2), 0x4a29d0: C(TOGGLE, 0), 0x4a9380: C(KEY15, 0)})
WINDOW, MODEWORD, FOUND = RAM + 0x1000, RAM + 0x1100, RAM + 0x1200


def case(r):
    log = log_region()
    regions = {LOG: log, WINDOW: r.bytes(0x40)}
    regions[MODEWORD] = struct.pack('<I', r.choice([4, 0, 3, 3, r.getrandbits(32)]))
    regions[0x7cf83c] = struct.pack('<I', (MODEWORD - 0x1da738) & 0xffffffff)
    regions[0x787c14] = r.bytes(4)
    regions[0x787a60] = r.bytes(4)
    regions[0x78721c] = r.bytes(4)
    high = lambda: r.choice([0, 0x10000, r.getrandbits(16) << 16])  # noqa: E731
    regions[FOUND] = struct.pack('<40I', *[r.choice([4, 0xf, 0, 1, 0x104, 0x10f]) | high() for _ in range(40)])
    answers(log, FIND, FOUND)
    set_return(log, MODS, r.getrandbits(32))
    set_return(log, DEFAULT, r.getrandbits(32))
    message = r.choice([0x10004, 0x10005, 0x1000a, 0x1000b, 0x1000b, 0x1000b, 0x10003, 0x10006, 0x10009, 0x1000c, r.getrandbits(32)])
    a = r.choice([1, 1, 0x1b, 0x1b, 0x10, r.randrange(0, 0x100), r.getrandbits(32)])
    b = r.choice([0, 0, 0x800000, r.getrandbits(16), r.getrandbits(8) << 16, r.getrandbits(32)])
    return Call(0x4a2840, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
