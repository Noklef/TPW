# The engine's start (src/game/render/enginestart: 0x44df20) against the original, its five callees recorders answering each its turn from a table: each driver kind, each of the open, the device and the engine failing on each try.
# usage: enginestart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region  # noqa: E402

OPEN, DEVICE, INIT, DOWN, FREE = 0, 1, 2, 3, 4
B = Bench([(0x44df20, 0x44e077)], hooks={0x5632b0: _recorder(OPEN, 6, cdecl=True), 0x5836d0: _recorder(DEVICE, 1, cdecl=True), 0x572780: _recorder(INIT, 3, cdecl=True), 0x583700: _recorder(DOWN, 0, cdecl=True), 0x5633a0: _recorder(FREE, 0, cdecl=True)})
ROUTES = [(a, b, c) for a in (0, 1) for b in (0, 1, 2) for c in (0, 1, 2)]
KINDS = [1, 1, 1, 4, 4, 0, 2]


def turns(r, ok_first):
    """40 answers: ok_first failures, then a pass (any nonzero); later ones whatever."""
    fails = [0] * ok_first
    return fails + [r.choice([1, 1, 2, 0x80000000])] + [r.choice([0, 1]) for _ in range(39 - ok_first)]


def case(r):
    log = log_region()
    opens, devices, inits = r.deal(ROUTES)
    table = struct.pack('<40I', *([0] * opens + [1] * (40 - opens))) + struct.pack('<40I', *turns(r, devices)) + struct.pack('<40I', *turns(r, inits))
    for i, k in enumerate((OPEN, DEVICE, INIT)):
        answers(log, k, RAM + 0xa0 * i)
    regions = {LOG: log, RAM: table, 0x8bd524: struct.pack('<I', r.choice(KINDS)), 0x7a130c: r.bytes(4), 0x7a1968: r.bytes(4), 0x7a1930: r.bytes(4)}
    return Call(0x44df20, [r.getrandbits(32) for _ in range(4)], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
