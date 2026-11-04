# The dynamic faces' three models made (src/game/render/dynfaces: 0x53bfe0) against the original: the holder's waking and sleeping and VertexBlock_Construct recorders, this one's results any (0 sleeps the holder and stops the later wakes' sleeps, sic), the waking answering 0 now and then; the second and third allocations failing now and then (the first: the result is a stack slot not yet written, sic, not dealt); the random table.
# usage: dynfacesinit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAND_SEED, Bench, Call, _recorder, fill, heap, log_region, set_return, strings  # noqa: E402

CONSTRUCT, WAKE, SLEEP = 1, 2, 3
B = Bench([(0x53bfe0, 0x53c1ef)], hooks={0x53d790: _recorder(CONSTRUCT, 11), 0x424ec0: _recorder(WAKE, 1, cdecl=True), 0x424ed0: _recorder(SLEEP, 1, cdecl=True)})


def case(r):
    log = log_region()
    set_return(log, CONSTRUCT, r.getrandbits(32))
    set_return(log, WAKE, r.choice([1, 1, 1, 1, 1, 1, 1, 2, 0]))
    strings(log, CONSTRUCT, 8, 4)
    fill(log, CONSTRUCT, 2, struct.pack('<3i', *[r.choice([1, 1, 1, 0, 2, -1]) for _ in range(3)]), records=3)
    regions = {LOG: log, HEAP: heap(0x400, r.choice([0, 0, 0, 2, 3])), 0x871d18: r.bytes(0x250), 0x874fc0: r.bytes(0x10), 0x763c98: r.bytes(4), 0x763c78: r.bytes(4), RAND_SEED: r.bytes(4)}
    return Call(0x53bfe0, [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
