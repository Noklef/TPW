# The choice between the panel [0x7c2534]'s menus (src/game/ui/panel7c2534menus: 0x48c830) against the original. Every callee a recorder as driver.cpp's kPanel7c2534OpenEnv.
# usage: panel7c2534open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region  # noqa: E402

MAIN, ONLINE, RESET, LEAVE, REFRESH, SHOW = range(10, 16)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x48c830, 0x48c8a7)], hooks={0x48c150: C(MAIN, 0), 0x48c600: C(ONLINE, 0), 0x4a25f0: C(RESET, 1), 0x4092a0: _recorder(LEAVE, 2), 0x662411: C(REFRESH, 0), 0x493130: _recorder(SHOW, 1)})


def case(r):
    regions = {LOG: log_region()}
    regions[0x7c2534] = struct.pack('<I', r.choice([0, 0, 0, 0, r.getrandbits(32)]))
    regions[0x786ba4] = struct.pack('<I', r.choice([1, 1, 0, r.getrandbits(32)]))
    regions[0x7c2518] = r.bytes(1)
    return Call(0x48c830, [r.choice([0, 0, 1, r.getrandbits(32)])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
