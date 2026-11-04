# The C++ library's collate<wchar_t>::do_transform (src/game/core/wcollate) against the original, over the wide strings' ports and the heap's Mem_Alloc and Mem_Free. The out string's allocator byte (the original's, an uninitialized stack byte) is not compared.
# usage: wcollate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region  # noqa: E402

B = Bench([(0x69ffe0, 0x6a0198), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2440), (0x479150, 0x479230), (0x478cf0, 0x478d40), (0x47ba30, 0x47ba50), (0x4f6080, 0x4f6090), (0x405280, 0x405390), (0x479230, 0x479270)])
OUT, TEXT = RAM, RAM + 0x100


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000, fail=r.choice([0] * 20 + [1, 2]))}
    n = r.choice([0, 0, 1, 2, 3, 0x1f, 0x20, 0x21, 0x100])
    regions[0x6fef00] = struct.pack('<I', r.choice([0xffffffff, 0xffffffff, 0xffffffff, n + 1, n + 2]))   # (npos more than n: _Xlen ends the case)
    regions[TEXT] = bytes(r.randrange(256) for _ in range(n * 2)) + r.bytes(0x10)
    regions[OUT] = r.bytes(0x10)
    return Call(0x69ffe0, [r.getrandbits(32), OUT, TEXT, TEXT + n * 2 + r.choice([0, 0, 0, 0, 1])], regions, conv='t', ret='a', ignore=[(OUT, 1)])


if __name__ == '__main__':
    B.main(case)
