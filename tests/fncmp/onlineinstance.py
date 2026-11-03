# Online_Instance (src/game/online/session, 0x48b390) against the original: [0xf7d88c] set or none, the heap (its failure now and then), the constructor 0x5b50b0 a recorder (agent B's, unported) answering any value or 0.
# usage: onlineinstance.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, _recorder, heap, log_region, set_return  # noqa: E402

B = Bench([(0x48b390, 0x48b3fd), (0x67acc0, 0x67acd0), (0x67ae00, 0x67af10)], hooks={0x5b50b0: _recorder(10, 0)})


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x1000, r.choice([0] * 4 + [1, 2])), 0xf7d88c: struct.pack('<I', r.choice([0, 0, 0, r.getrandbits(32)]))}
    if r.random() < 0.5:
        set_return(log, 10, r.choice([0, r.getrandbits(32)]))
    return Call(0x48b390, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
