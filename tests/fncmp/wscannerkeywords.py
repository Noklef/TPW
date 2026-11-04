# The wide text scanner's keyword handlers (src/game/core/wscanner: 0x5e4b20, 0x5e4ca0, 0x5e4f00, 0x5e50a0, 0x5e5240, 0x5e53c0, 0x5e58b0, 0x5e5aa0, 0x5e5c90, 0x5e5e80, 0x5e6160, 0x5e6350, 0x5e6630) against the originals: wscanner.py's scanner, with its word's and rest's sets and text (now and then its next word failing: the position at the end, on a character of the skipped set, or past the end), the tokens a vector of 0-3 pairs with room; the new pairs' allocator bytes (the low byte of a stack address, CountedPair_CopyCtor, sic) left out. The vector's growth is countedpair.py's.
# usage: wscannerkeywords.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, put  # noqa: E402
import stdwstring  # noqa: E402
import wscanner  # noqa: E402

B = Bench([(0x405280, 0x405390), (0x479150, 0x4791b0), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0),
           (0x67acd0, 0x67ad00), (0x4ea740, 0x4ea7e0), (0x47ae60, 0x47b060), (0x47b600, 0x47b750), (0x479230, 0x479256),
           (0x47cc10, 0x47cd00), (0x47b460, 0x47b5b0), (0x47d9d0, 0x47db00), (0x47d620, 0x47d780), (0x47b320, 0x47b460),
           (0x5e7310, 0x5e7319), (0x5e6fc0, 0x5e7093), (0x5b30b0, 0x5b3110), (0x5e70a0, 0x5e72ff), (0x5e7520, 0x5e76b9), (0x5e76c0, 0x5e7720),
           (0x45b380, 0x45b5f0), (0x45b680, 0x45b9b0), (0x4e6410, 0x4e6560), (0x4f5860, 0x4f5988), (0x4f5560, 0x4f5852), (0x4e51a0, 0x4e520e),
           (0x4f67b0, 0x4f67e1), (0x4f67f0, 0x4f6819), (0x4f7410, 0x4f7423), (0x4f36f0, 0x4f3711), (0x4f6ef0, 0x4f7170), (0x4f6e70, 0x4f6ee9), (0x4051d0, 0x405272), (0x4f0150, 0x4f0168),
           (0x5e4b20, 0x5e4c97), (0x5e4ca0, 0x5e4efd), (0x5e4f00, 0x5e5094), (0x5e50a0, 0x5e5234), (0x5e5240, 0x5e53b7), (0x5e53c0, 0x5e5537), (0x5e58b0, 0x5e5a96), (0x5e5aa0, 0x5e5c86), (0x5e5c90, 0x5e5e76),
           (0x5e5e80, 0x5e6066), (0x5e6160, 0x5e6346), (0x5e6350, 0x5e6536), (0x5e6630, 0x5e6716)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})   # (_Lockit's constructor and destructor)
VECTOR, ARRAY = RAM + 0x30000, RAM + 0x30100
NEXT_FAILS = 3   # one word-handler case in this many has its next word fail


def scene(r, op):
    """(regions, ignored): wscanner.py's scanner of `op` ('Next' or 'Rest') with the vector and the out string at +0x50, +0x54."""
    regions = wscanner.span(r, op).regions
    scanner = bytearray(regions[wscanner.SCANNER]) + struct.pack('<2I', VECTOR, wscanner.OUT)
    regions[wscanner.SCANNER] = scanner
    text, length = struct.unpack_from('<2I', scanner, 8)
    if text and not length:   # (an empty text shared: read, it would be freed and read through null, a fault in both)
        regions[stdwstring.BLOCKS][text - stdwstring.BLOCKS - 1] = r.choice([0, 0xff])
    count = r.choice([0, 1, 2, 3])
    first, last = ARRAY, ARRAY + 0x24 * count
    pairs = bytearray(r.bytes(0x24 * (count + 4)))
    for i in range(count):
        for offset in (4, 8, 0xc, 0x14, 0x18, 0x1c):
            put(pairs, 0x24 * i + offset, 'I', 0)
    regions[VECTOR], regions[ARRAY] = struct.pack('<4I', r.getrandbits(32), first, last, last + 0x24 * 4), pairs
    return regions, [(last + 0x24 * i + k, 1) for i in range(4) for k in (0, 0x10)]


def next_fails(regions):
    """The word's skip goes past the text's end: the position at its length, the character there one of the skipped set."""
    scanner, blocks = regions[wscanner.SCANNER], regions[stdwstring.BLOCKS]
    ptr, n = struct.unpack_from('<2I', scanner, 8)
    if ptr:
        root = struct.unpack_from('<I', regions[wscanner.stdtree.HEAD], 4)[0]
        key = struct.unpack_from('<H', regions[wscanner.stdtree.NODES], root - wscanner.stdtree.NODES + 0xc)[0]
        put(blocks, ptr - stdwstring.BLOCKS + 2 * n, 'H', key)
        put(scanner, 0x38, 'I', n)


REST = [0x5e4b20, 0x5e5240, 0x5e53c0]
COUNTED = [0x5e4f00, 0x5e50a0]
WORD = [0x5e58b0, 0x5e5aa0, 0x5e5c90, 0x5e5e80, 0x5e6160, 0x5e6350]
WORD_REST, TOKEN = 0x5e4ca0, 0x5e6630


def case(r):
    address = r.deal(REST + COUNTED + WORD + [WORD_REST, TOKEN])
    fails = r.randrange(NEXT_FAILS) == 0 and address in WORD + [WORD_REST]
    regions, ignored = scene(r, 'Next' if address in WORD or fails else 'Rest')   # (the rest after a next word of 'Next''s text: its skip may pass the end, _Xran)
    if fails:
        next_fails(regions)
        if r.randrange(2):
            put(regions[wscanner.SCANNER], 0x38, 'I', struct.unpack_from('<I', regions[wscanner.SCANNER], 0xc)[0] + r.choice([1, 9]))
    return Call(address, [wscanner.SCANNER], regions, conv='t', ret='a' if address == TOKEN else 'b', ignore=ignored)


if __name__ == '__main__':
    B.main(case)
