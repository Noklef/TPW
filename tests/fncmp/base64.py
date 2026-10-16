# The base64 encoder (src/game/core/base64: 0x5c92f0) against the original: lengths around the
# 57-byte lines and the 1-2-3 byte endings, any bytes, null destination or source.
# usage: base64.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5c92f0, 0x5c93bb)])
DST, SRC = RAM + 0x1000, RAM + 0x2000


def case(r):
    length = r.choice([0, 1, 2, 3, 4, 5, 56, 57, 58, 59, 113, 114, 115, 171, 228, r.randrange(0, 230), r.randrange(-2, 3)])
    dst = r.choice([DST, DST, DST, DST, 0])
    src = r.choice([SRC, SRC, SRC, SRC, 0])
    return Call(0x5c92f0, [dst, src, length & 0xffffffff], {DST: bytes([0x55]) * 0x400, SRC: r.bytes(0x100)})


if __name__ == '__main__':
    B.main(case)
