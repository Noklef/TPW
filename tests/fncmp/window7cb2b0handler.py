# The handler of the window [0x7cb2b0] (src/game/ui/window7cb2b0handler: 0x488a00) against the original. Recorders as driver.cpp's kWindow7cb2b0HandlerEnv; the key map answers in turn.
# usage: window7cb2b0handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, set_return  # noqa: E402

B = Bench([(0x488a00, 0x488ba0)],
          hooks={0x46b420: _recorder(10, 0, cdecl=True), 0x40c990: _recorder(11, 2), 0x40c900: _recorder(12, 2), 0x40c870: _recorder(13, 2), 0x65f11d: _recorder(14, 1),
                 0x42ae70: _recorder(15, 0, cdecl=True), 0x42a190: _recorder(16, 0, cdecl=True), 0x4a9380: _recorder(17, 0, cdecl=True), 0x42a760: _recorder(18, 3, cdecl=True),
                 0x65f66e: _recorder(19, 3)})
FOUND = RAM + 0x1000
MESSAGES = [0x14, 0x13, 0x15, 0x100, 0x100, 0x10003, 0x10004, 0x10005, 0x10006, 0x10006, 0x10006, 0x10007, 0x10009, 0x1000a, 0x1000b, 0x1000b, 0x1000b, 0x1000b, 0x1000c, 0xffffffff, 0x20]


def case(r):
    log = log_region()
    set_return(log, 10, r.getrandbits(32))
    set_return(log, 19, r.getrandbits(32))
    answers(log, 13, FOUND)
    regions = {LOG: log, FOUND: struct.pack('<40I', *[r.choice([0x10, 0xf, 0x10010, 0x1000f, 5, 0xffffffff]) for _ in range(40)]), 0x7cb2b0: r.bytes(4), 0x787c14: r.bytes(4), 0x78721c: r.bytes(4),
               0x78d911: bytes([r.choice([0, 1, 0x80])]), 0x790ab0: bytes([r.choice([0, 1, 2, 4, 6, 0x10, 0x12, 0x14, 0x16, 0xe9, r.getrandbits(8)])])}
    message = r.choice(MESSAGES)
    a = r.choice([0x1b, 1, 1, 0x10, 0x11, r.getrandbits(32)])
    return Call(0x488a00, [r.getrandbits(32), message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
