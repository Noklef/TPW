# A GUI text token read as a wide string (src/game/ui/guitokenwide: 0x66de4c) against the original: strchr real, strlen and swprintf recorders as driver.cpp's kGuiTokenWideEnv (strlen's answer small, so the count decides).
# usage: guitokenwide.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

LENGTH, PRINT = 10, 11
TEXT, OUT = RAM, RAM + 0x200
B = Bench([(0x66de4c, 0x66ded6), (0x67bfc0, 0x67c090)], hooks={0x67ff70: _recorder(LENGTH, 1, cdecl=True), 0x67af00: _recorder(PRINT, 3, cdecl=True)})


def case(r):
    log = log_region()
    length = r.deal([0, 1, 2, 7, 8, 0x20, 0x41, 0x80000000])
    set_return(log, LENGTH, length)
    text = bytes(r.choice(b'abc XY') for _ in range(r.deal([0, 1, 5, 12]))) + b'\0'
    regions = {LOG: log, TEXT: text + r.bytes(8), OUT: r.bytes(0x10)}
    count = r.deal([0, 1, 3, 4, 0x10, 0x20, 0x21, 0x8000, 0xffff, r.getrandbits(32)])
    return Call(0x66de4c, [TEXT, r.deal([OUT, OUT, OUT, 0]), count], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
