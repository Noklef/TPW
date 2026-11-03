# A keyword added to the wide scanner's table (src/game/core/wscanner: WKeywords_Add 0x5e49f0) against the original: the text formatter a recorder (k 12; its text by a fill of 0-55
# characters), the rest real (the wstring constructors, the pair, the heap). The node's allocator byte (+8) is an unset stack byte of the original, left out of the comparison.
# usage: wkeywords.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, log_region, strings  # noqa: E402

# (the wstring helpers as wscanner.py and stdwstring.py run them)
B = Bench([(0x5e49f0, 0x5e4b1a), (0x5e73f0, 0x5e7518), (0x4f0170, 0x4f0237), (0x47ae60, 0x47b060), (0x405280, 0x405390), (0x479150, 0x4791b0), (0x47b750, 0x47bcf0),
           (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0), (0x67acd0, 0x67ad00), (0x4ea740, 0x4ea7e0), (0x47b600, 0x47b750), (0x479230, 0x479256), (0x47cc10, 0x47cd00),
           (0x47b460, 0x47b5b0), (0x47d9d0, 0x47db00), (0x47d620, 0x47d780), (0x47b320, 0x47b460)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x6acd60: _recorder(12, 4, cdecl=True)})   # (_Lockit's constructor and destructor)
SELF, HEAD, NEXT, PREV = RAM + 0x800, RAM + 0x900, RAM + 0x940, RAM + 0x980


def node_at(length):
    """Where the node lands: after the text's block, if there is one (63 characters at most here, so one of two sizes)."""
    return HEAP + 0x10 + (0 if length == 0 else 0x50 if length <= 31 else 0x90)


def case(r):
    log = log_region()
    length = r.choice([0, 1, 2, r.randrange(1, 31), 30, 31, 32, r.randrange(32, 56), 55])
    chars = [r.choice([r.randrange(0x20, 0x7f), r.randrange(1, 0x10000)]) for _ in range(length)]
    strings(log, 12, 0, out=1 << 1)
    fill(log, 12, 1, struct.pack('<%dH' % (length + 1), *chars, 0))
    self_ = bytearray(r.bytes(0x60))
    struct.pack_into('<I', self_, 0x48, HEAD)
    next_ = bytearray(r.bytes(0x20))
    prev = r.choice([PREV, PREV, NEXT, 0])
    struct.pack_into('<I', next_, 4, prev)
    regions = {LOG: log, 0x78bac8: r.bytes(4), SELF: self_, HEAD: struct.pack('<I', NEXT), NEXT: next_, PREV: r.bytes(0x20), HEAP: heap()}
    return Call(0x5e49f0, [SELF, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ignore=[(node_at(length) + 8, 1)])


if __name__ == '__main__':
    B.main(case)
