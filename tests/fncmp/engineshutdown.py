# Bench: Engine_ShutdownAll 0x572b60: the engine's objects destroyed in order, those of the render mode at 0x8bd524 among them.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region  # noqa: E402

CALLS = [0x564b60, 0x57c4a0, 0x57d1a0, 0x58a490, 0x561a00, 0x58b040, 0x574b10, 0x5761b0, 0x56de90, 0x58d090, 0x580500, 0x5740e0, 0x582280, 0x5667a0, 0x569ef0, 0x573b30, 0x578390, 0x56b760]
B = Bench([(0x572b60, 0x572be2)], hooks={a: _recorder(k + 1, 0, cdecl=True) for k, a in enumerate(CALLS)})


def case(r):
    return Call(0x572b60, [], {LOG: log_region(), 0x8bd524: struct.pack('<I', r.deal([1, 4, 2, 0, 5, r.getrandbits(32)]))}, ret='a')


if __name__ == '__main__':
    B.main(case)
