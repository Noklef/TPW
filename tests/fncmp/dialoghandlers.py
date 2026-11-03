# The dialogs' handlers that only call on (src/game/ui/dialog7ca2e8: 0x49a0b0; dialog7ca1bc: 0x497a70) against the originals: all their calls recorders, numbered as in tools/fncmp/driver.cpp.
# usage: dialoghandlers.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x49a0b0, 0x49a1c3), (0x497a70, 0x497b19)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x486b40: _recorder(11, 1, cdecl=True), 0x485b00: _recorder(12, 1, cdecl=True), 0x485d20: _recorder(13, 3, cdecl=True), 0x488ba0: _recorder(14, 4, cdecl=True),
                 0x49fb30: _recorder(15, 0, cdecl=True), 0x4b2750: _recorder(16, 0, cdecl=True), 0x498d80: _recorder(17, 0, cdecl=True), 0x4b7b00: _recorder(18, 0, cdecl=True), 0x4a25f0: _recorder(19, 1, cdecl=True), 0x65f66e: _recorder(20, 3)})
WINDOW = RAM
A = {0x49a0b0: [0xfffffffd, 0xfffffffe, 0xffffffff, 0, 0x1227e, 0x1227f, 0x12280, 0x12281, 0x12282, 0x1b5a], 0x497a70: [0x1b58, 0x1b59, 0x1b5a, 0x1b5b, 0x1b5c, 0x1227f]}
M = {0x49a0b0: [4, 5, 6, 0x13, 0x14, 0x15, 0x16, 0xff, 0x100, 0x100, 0x100, 0x100, 0x101], 0x497a70: [0x13, 0x14, 0x15, 0xff, 0x100, 0x100, 0x100, 0x100, 0x101]}


def case(r):
    op = r.deal([0x49a0b0, 0x497a70])
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 19, 0b1 << 0)   # (Dialog7cb2b8_ResetChild21's value: a byte)
    for k in (10, 12, 14, 20):
        set_return(log, k, r.getrandbits(32))
    regions = {LOG: log, WINDOW: r.bytes(16), 0x7ca2e8: r.bytes(4), 0x7ca1bc: r.bytes(4)}
    return Call(op, [WINDOW, r.deal(M[op]), r.choice(A[op] + [r.getrandbits(32)]), r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
