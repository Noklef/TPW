# A keyword added to the wide scanner's table (src/game/core/wscanner: WKeywords_Add 0x5e49f0) against the original: the text formatter a recorder (k 12; its text by a fill of 0-55
# characters), the rest real (the wstring constructors, the pair, the heap). All (WKeywords_AddAll 0x5e47f0, a quarter of the cases): its 36 adds, texts of one length from up to 4 records. The node's allocator byte (+8) is an unset stack byte of the original, left out of the comparison.
# Construct (0x5e43f0, a third of All's cases): all that on a scanner being built: 0-2 tokens to destroy, the string as stdwstring.py makes one, or none and a text at 0x78a374 to take.
# usage: wkeywords.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, log_region, put, strings  # noqa: E402
import stdwstring  # noqa: E402

# (the wstring helpers as wscanner.py and stdwstring.py run them)
B = Bench([(0x5e45b0, 0x5e45ce), (0x5e47f0, 0x5e49eb), (0x5e49f0, 0x5e4b1a), (0x5e73f0, 0x5e7518), (0x4f0170, 0x4f0237), (0x47ae60, 0x47b060), (0x405280, 0x405390), (0x479150, 0x4791b0), (0x47b750, 0x47bcf0),
           (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0), (0x67acd0, 0x67ad00), (0x4ea740, 0x4ea7e0), (0x47b600, 0x47b750), (0x479230, 0x479256), (0x47cc10, 0x47cd00),
           (0x47b460, 0x47b5b0), (0x47d9d0, 0x47db00), (0x47d620, 0x47d780), (0x47b320, 0x47b460), (0x5e43f0, 0x5e45b0), (0x5e6820, 0x5e6a00), (0x5b1650, 0x5b1680), (0x5b1930, 0x5b1a20),
           (0x5b2af0, 0x5b2f40), (0x5b30b0, 0x5b31f0), (0x5b3210, 0x5b3230), (0x5e7720, 0x5e7740), (0x4051d0, 0x405280), (0x45b380, 0x45b5f0), (0x6918f0, 0x691a30)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x6acd60: _recorder(12, 4, cdecl=True), 0x4ea7e0: _recorder(13, 0)})   # (_Lockit's constructor and destructor)
SELF, HEAD, NEXT, PREV = RAM + 0x800, RAM + 0x900, RAM + 0x940, RAM + 0x980


def node_at(length):
    """Where the node lands: after the text's block, if there is one (63 characters at most here, so one of two sizes)."""
    return HEAP + 0x10 + (0 if length == 0 else 0x50 if length <= 31 else 0x90)


def construct(r, regions, length):
    """The base's blocks (0x20 each: a nil node or none, two heads, eight nodes), the list's sentinel (0x30), the text's block when it has to get one, then the 36 keywords'."""
    nil = r.choice([0, RAM + 0xd00])
    regions[RAM + 0xd00] = put(put(bytearray(r.bytes(0x14)), 0, '3I', 0, r.getrandbits(32), 0), 0x10, 'I', 1)
    regions[0xf7c96c] = struct.pack('<II', r.getrandbits(32), nil)
    blocks = bytearray(r.bytes(4 * stdwstring.SLOT))
    text = r.choice([b'', b'', b'a\0b\0'])
    regions[0x78a374] = text + bytes(2)
    if text:
        out = (0, 0, 0)
    else:
        out = stdwstring.string(r, blocks, 0)
    regions[stdwstring.BLOCKS] = blocks
    regions[RAM + 0x1000] = struct.pack('<4I', r.getrandbits(32), *out)
    n = r.choice([0, 0, 1, 2])
    held = bytearray(r.bytes(0x24 * (n + 1)))
    tokens = bytearray(r.bytes(0x40 * 4))
    for k in range(n):
        for side in (0, 0x10):
            at = 0x20 * (2 * k + (side >> 4)) + 8
            tokens[at - 2:at] = bytes([0, r.choice([0, 1, 2, 0xff])])
            put(held, 0x24 * k + side, '4I', r.getrandbits(32), r.choice([0, RAM + 0x1200 + at]), r.choice([0, 3]), 8)
    regions[RAM + 0x1200] = bytes(tokens)
    regions[RAM + 0x1400] = bytes(held)
    regions[RAM + 0x1100] = struct.pack('<4I', r.getrandbits(32), RAM + 0x1400, RAM + 0x1400 + 0x24 * n, RAM + 0x1400 + 0x24 * (n + 1))
    regions[SELF] = r.bytes(0x60)
    first = HEAP + 0x10 + 0x20 * (10 + (0 if nil else 1)) + 0x30 + (0x50 if text else 0) + (0 if length == 0 else 0x50)
    stride = 0x30 if length == 0 else 0x80
    ignore = [(SELF + 4, 1), (SELF + 0x14, 2), (SELF + 0x24, 2)] + [(first + stride * i + 8, 1) for i in range(36)]
    return Call(0x5e43f0, [SELF, RAM + 0x1100, RAM + 0x1000], regions, conv='t', ret='a', ignore=ignore)


def case(r):
    log = log_region()
    if r.randrange(8) == 0:   # (WScanner_DeletingDtor 0x5e45b0: the destructor 0x4ea7e0 a recorder)
        return Call(0x5e45b0, [SELF, r.choice([0, 1, 2, 3, r.getrandbits(32)])], {LOG: log, HEAP: heap(0x40), SELF: r.bytes(0x60)}, conv='t', ret='a')
    length = r.choice([0, 1, 2, r.randrange(1, 31), 30, 31, 32, r.randrange(32, 56), 55])
    chars = [r.choice([r.randrange(0x20, 0x7f), r.randrange(1, 0x10000)]) for _ in range(length)]
    strings(log, 12, 0, out=1 << 1)
    many = r.randrange(4) == 0
    if many:
        length = min(length, 31)
        records = r.randrange(1, 5)
        chars = [r.randrange(1, 0x10000) for _ in range(length * records)]
        data = b''.join(struct.pack('<%dH' % (length + 1), *chars[length * i:length * (i + 1)], 0) for i in range(records))
        fill(log, 12, 1, data[:0x70 // (2 * length + 2) * (2 * length + 2)], records=min(records, 0x70 // (2 * length + 2)))
    else:
        fill(log, 12, 1, struct.pack('<%dH' % (length + 1), *chars, 0))
    self_ = bytearray(r.bytes(0x60))
    struct.pack_into('<I', self_, 0x48, HEAD)
    next_ = bytearray(r.bytes(0x20))
    prev = r.choice([PREV, PREV, NEXT, 0])
    struct.pack_into('<I', next_, 4, prev)
    regions = {LOG: log, 0x78bac8: r.bytes(4), SELF: self_, HEAD: struct.pack('<I', NEXT), NEXT: next_, PREV: r.bytes(0x20), HEAP: heap()}
    if many and r.randrange(3) == 0:
        return construct(r, regions, length)
    if many:
        first = HEAP + 0x10 + (0 if length == 0 else 0x50)
        stride = 0x30 if length == 0 else 0x80
        return Call(0x5e47f0, [SELF], regions, conv='t', ignore=[(first + stride * i + 8, 1) for i in range(36)])
    return Call(0x5e49f0, [SELF, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ignore=[(node_at(length) + 8, 1)])


if __name__ == '__main__':
    B.main(case)
