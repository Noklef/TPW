# The wide text scanner's text setter (src/game/core/wscanner: 0x5e6b30) against the original: its wstring at +4 and the text passed by
# value made as stdwstring.py makes strings (null, empty, short and long, count bytes 0-2, 0xfd-0xff, sometimes the same block), any
# flag bytes and context; the copies and frees from the heap region.
# usage: wscanner.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402
import stdwstring  # noqa: E402

# (with stdwstring.py's helpers: _Grow, erase, the counted buffer reset, the traits' copy, the length setter)
B = Bench([(0x405280, 0x405390), (0x479150, 0x4791b0), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0),
           (0x67acd0, 0x67ad00), (0x4ea740, 0x4ea7e0), (0x47ae60, 0x47b060), (0x47b600, 0x47b750), (0x479230, 0x479256),
           (0x47cc10, 0x47cd00), (0x47b460, 0x47b5b0), (0x47d9d0, 0x47db00), (0x47d620, 0x47d780), (0x47b320, 0x47b460),
           (0x5e6b30, 0x5e6c87)])
SCANNER, FLAGS = RAM + 0x800, RAM + 0x900


def case(r):
    blocks = bytearray(r.bytes(4 * stdwstring.SLOT))
    s = stdwstring.string(r, blocks, 0)
    together = s[0] and r.random() < 0.2
    x = s if together else stdwstring.string(r, blocks, 1)
    if together:
        blocks[s[0] - stdwstring.BLOCKS - 1] = r.choice([1, 2, 0xfd, 0xfe])
    scanner = put(bytearray(r.bytes(0x50)), 8, '3I', *s)
    regions = {SCANNER: scanner, FLAGS: r.bytes(4), stdwstring.BLOCKS: blocks, HEAP: heap()}
    return Call(0x5e6b30, [SCANNER, r.getrandbits(32), *x, r.getrandbits(32), FLAGS], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
