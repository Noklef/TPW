# The advisor's frame tick (src/game/world/advisor: 0x599800) against the original, its four callees recorders: the two flags, the queued bank from Layer_GetQueued.
# usage: advisorframe.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, fill, log_region, set_return, strings  # noqa: E402

HOLDER, QUEUED, HIGHLIGHT, PARTICLES = 13, 14, 15, 16
B = Bench([(0x599800, 0x599873)], hooks={0x429fe0: _recorder(HOLDER, 1, cdecl=True), 0x473ff0: _recorder(QUEUED, 4, cdecl=True), 0x598dc0: _recorder(HIGHLIGHT, 0, cdecl=True), 0x5ed770: _recorder(PARTICLES, 0, cdecl=True)})


def case(r):
    log = log_region()
    set_return(log, HOLDER, r.getrandbits(32))
    strings(log, QUEUED, 0, 2 | 4)
    fill(log, QUEUED, 1, struct.pack('<I', r.choice([0xc, 0xc, 0xc, 0, 0xb, 0xd, 0x10c, r.getrandbits(32)])))
    flag = lambda: struct.pack('<I', r.choice([0, 0, 1, 0x100, r.getrandbits(32)]))  # noqa: E731
    return Call(0x599800, [], {LOG: log, 0xf79674: r.bytes(4), 0xf7968c: flag(), 0xf79690: flag()}, ret='v')


if __name__ == '__main__':
    B.main(case)
