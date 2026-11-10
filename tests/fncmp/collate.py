# The C++ library's collate<char>::do_transform (src/game/core/collate) against the original, over the strings' ports and the heap's Mem_Alloc and Mem_Free (wcollate.py's, for char).
# usage: collate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region  # noqa: E402

B = Bench([(0x6a7080, 0x6a7237), (0x45b340, 0x45b9a0), (0x47b1c0, 0x47b27c), (0x47b280, 0x47b2f8), (0x67bb90, 0x67bd82)])
OUT, TEXT = RAM, RAM + 0x200


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000, fail=r.choice([0] * 20 + [1, 2]))}
    n = r.choice([0, 0, 1, 2, 3, 0x1f, 0x20, 0x21, 0x100])
    regions[0x6fe5ac] = struct.pack('<I', r.choice([0xffffffff, 0xffffffff, 0xffffffff, n + 1, n + 2]))   # (npos past n: _Xlen would throw)
    regions[TEXT] = bytes(r.randrange(256) for _ in range(n)) + r.bytes(0x10)
    regions[OUT] = r.bytes(0x10)
    text = TEXT + r.choice([0, 0, 1, 2, 3])   # (the out string's allocator byte is the low byte of first, in the original)
    return Call(0x6a7080, [r.getrandbits(32), OUT, text, text + n], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
