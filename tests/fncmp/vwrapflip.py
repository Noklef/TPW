# The pointer's flip begun (src/game/core/vwrap: 0x47e9b0 Pointer_BeginFlip) against the original: the pointer [0x7b4b84] or none, AsyncPointer_BeginFlip a recorder (k 7).
# usage: vwrapflip.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

B = Bench([(0x47e9b0, 0x47e9c0)], hooks={0x5fbd40: _recorder(7, 0)})


def case(r):
    regions = {LOG: log_region(), RAM: r.bytes(0x10), 0x7b4b84: struct.pack('<I', r.choice([0, RAM, RAM]))}
    return Call(0x47e9b0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
