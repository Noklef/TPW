# The sound system's save / load chain against the originals, on one fixture for the saved records.
# The names' map (src/game/core/soundnames: 0x6c10f0, 0x6c0d90, 0x6c1050, 0x6c0f50): the singleton category at 0xfb1f28 (soundcategory.py's,
# its guard set) with deque A's records named as soundcategory.py's findrec case; its map at +4 a count and a block (0 now and then). The load
# reads from a memory stream at STREAM over DATA: a size (small, now and then short of the names or past the stream), the count (0-5, the
# names' count or not) and the block: the count's dwords, then each name a length byte (its text's length + 1, 0, or the length alone) and
# its text (one of the records' names or another). The block comes from the heap (failing now and then: a fault in both).
# usage: soundsave.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, RAM, Bench, Call, heap, memory_stream  # noqa: E402

CALLS = {'mapfree': 0x6c10f0, 'freemap': 0x6c0d90, 'mapload': 0x6c1050, 'namesload': 0x6c0f50}
B = Bench([(0x47db00, 0x47db30), (0x5ec210, 0x5ec230), (0x5f61a0, 0x5f61f0), (0x5f7980, 0x5f79f0), (0x5f8240, 0x5f8260), (0x632310, 0x632360),
           (0x6338b0, 0x6338d0), (0x67a740, 0x67a800), (0x67ad90, 0x67ae10), (0x67c250, 0x67c2a0), (0x67dcd0, 0x67dd50), (0x67efc0, 0x67f1b0),
           (0x67fa20, 0x67fa50), (0x681740, 0x681970), (0x682720, 0x6827d0), (0x682b40, 0x682e50), (0x682fc0, 0x6830a0), (0x688710, 0x688740),
           (0x6bead0, 0x6bed90), (0x6c0d90, 0x6c1120), (0x6c1240, 0x6c1280), (0x6c15f0, 0x6c1aa0), (0x6c2140, 0x6c2200), (0x6c2860, 0x6c29a0),
           (0x6c2ca0, 0x6c2d30), (0x6c31a0, 0x6c31f0)])
SINGLETON, GUARD = 0xfb1f28, 0xfb1ff8
OUT, STREAM, DATA, MAPAT, BLOCK = RAM + 0xa000, RAM + 0xa100, RAM + 0xa200, RAM + 0xa800, RAM + 0xa900
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def singleton(r):
    """The singleton category (its guard set) with deque A's records named (soundcategory.py's findrec case); returns (regions, names)."""
    import soundcategory as sc
    regions = sc.category(r, 4, holes=True, many=r.random() < 0.1)
    names = [r.choice(sc.NAMES_GIVEN + [b'Bank0', b'ab']) for _ in range(4)]
    recs = bytearray(regions[sc.RECS])
    for i in range(len(recs) // 0x40):
        struct.pack_into('<I', recs, 0x40 * i + 8, r.choice([0, 1, 1, r.getrandbits(32)]))
        recs[0x40 * i + 0xc:0x40 * i + 0x20] = sc.string(r, names[i], sc.NAMES + 0x40 * i)
        regions[sc.NAMES + 0x40 * i] = names[i] + b'\0' + r.bytes(8)
    regions[sc.RECS] = recs
    regions[SINGLETON] = regions.pop(sc.CAT)
    regions[GUARD] = bytes([r.choice([1, r.getrandbits(8) | 1])])
    return regions, names


def names_block(r, names):
    """A saved names' block: (count, the bytes after the size and count)."""
    count = r.choice([0, 1, 2, 3, 5])
    body = bytearray(r.bytes(4 * count))
    for _ in range(count):
        text = r.choice(names + [b'', b'Bank', b'x', b'Bank0.bnk'])
        n = r.choice([len(text) + 1] * 4 + [0, len(text)])
        body += bytes([n]) + (text + b'\0')[:n] + r.bytes(max(0, n - len(text) - 1))
    return count, bytes(body)


def names_case(r, name):
    regions, names = singleton(r)
    regions[HEAP] = heap(0x800, fail=r.choice([0] * 12 + [1]))
    cat = bytearray(regions[SINGLETON])
    struct.pack_into('<II', cat, 4, r.getrandbits(32), r.choice([0, BLOCK]))
    regions[SINGLETON] = cat
    regions[BLOCK] = r.bytes(0x10)
    regions[OUT] = r.bytes(4)
    if name == 'mapfree':
        regions[MAPAT] = struct.pack('<II', r.getrandbits(32), r.choice([0, BLOCK]))
        return Call(CALLS[name], [r.choice([MAPAT, SINGLETON + 4])], regions, conv='t', ret='v')
    if name == 'freemap':
        return Call(CALLS[name], [SINGLETON], regions, conv='t', ret='v')
    count, body = names_block(r, names)
    if r.random() < 0.15:
        count = r.choice([count + 1, max(0, count - 1), r.randrange(8)])
    size = r.choice([len(body)] * 6 + [max(0, len(body) - r.randrange(1, 8)), len(body) + r.randrange(1, 0x10)])
    data = struct.pack('<II', size, count) + body + r.bytes(0x20)
    left = r.choice([len(data)] * 8 + [r.randrange(len(data))])
    regions[STREAM] = memory_stream(STREAM, DATA, left)
    regions[DATA] = data
    if name == 'mapload':
        regions[MAPAT] = struct.pack('<II', r.getrandbits(32), r.choice([0, BLOCK]))
        return Call(CALLS[name], [r.choice([MAPAT, SINGLETON + 4]), OUT, STREAM], regions, conv='t', ret='a')
    return Call(CALLS[name], [SINGLETON, OUT, STREAM], regions, conv='t', ret='a')


def case(r):
    name = r.deal(OPS)
    return names_case(r, name)


if __name__ == '__main__':
    B.main(case)
