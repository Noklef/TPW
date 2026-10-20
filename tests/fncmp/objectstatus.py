# An object's status (src/game/world/objectstatus: 0x485f60 Object_Status) against the original. Most cases on objectqueue.py's CanOpen world (states, flags,
# track types and coaster links, the entrance against the path) with tracked_blocks.py's tracking fields on its things, the world's words +0x1da73c /
# +0x1da73e (0 or not) and the game table entry of +0x1da718 with its indexed dwords set or not; definitions that never reach the exit test there. The rest
# on objectexit.py's world (the exit open or not), the object past every earlier test, definitions of kind 0 with +0x284 / +0x288 0, 2 or other.
# The count 0, 1, 2 or any.
# usage: objectstatus.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectexit  # noqa: E402
import objectqueue  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import ITEM, ITEMS  # noqa: E402


def merged(*lists):
    spans = sorted(s for spans in lists for s in spans)
    out = [list(spans[0])]
    for lo, hi in spans[1:]:
        if lo <= out[-1][1]:
            out[-1][1] = max(out[-1][1], hi)
        else:
            out.append([lo, hi])
    return [tuple(s) for s in out]


OWN = [(0x485f60, 0x4861c0), (0x441910, 0x441980), (0x4e0220, 0x4e03a0), (0x4e03d0, 0x4e0400), (0x4e0440, 0x4e0460), (0x4e2710, 0x4e2750),
       (0x506580, 0x506660), (0x509990, 0x5099b0), (0x519450, 0x5194a0), (0x5d46d0, 0x5d47a0)]
QUEUE = [(0x4d8300, 0x4d8340), (0x4d8c00, 0x4d8c30), (0x4d96f0, 0x4d99f0), (0x4de4a0, 0x4de5a0), (0x4df290, 0x4df310), (0x4de840, 0x4dede8), (0x516330, 0x516380),
         (0x4ddf40, 0x4de200), (0x4de040, 0x4de100), (0x4de670, 0x4de7f0), (0x502430, 0x502470), (0x50b350, 0x50b370), (0x522770, 0x5227a0), (0x522850, 0x522870),
         (0x536300, 0x53644c), (0x5da3c0, 0x5da3c1), (0x4dd920, 0x4dda20), (0x412e90, 0x412f00), (0x441970, 0x4419b0), (0x436860, 0x436880)]   # objectqueue.py's
EXIT = [(0x4de5a0, 0x4de670), (0x4d8c00, 0x4d8c1f), (0x4d97e0, 0x4d9918), (0x4d99c0, 0x4d99d3), (0x4d96f0, 0x4d97d7), (0x4d8300, 0x4d8328), (0x4d8750, 0x4d8b29),
        (0x4d0af0, 0x4d0b06), (0x522700, 0x5228b5), (0x536300, 0x536484), (0x53ad20, 0x53aefc), (0x4dd4e0, 0x4dd510), (0x42a440, 0x42a4f0), (0x436840, 0x436870),
        (0x436880, 0x4368d0), (0x44b220, 0x44b280), (0x549d30, 0x549dc0), (0x55a070, 0x55a0c0), (0x55a360, 0x55a3a0), (0x55a570, 0x55a5b0)]   # objectexit.py's
B = Bench(merged(OWN, QUEUE, EXIT))
COUNTS = [0, 0, 1, 1, 2, 3, 0x80000000, 0xffffffff]
WORLD, THING, SPARE = RAM + 0x10000, RAM + 0x50000, RAM + 0x60000   # objectqueue's world (0x7cf83c, 0x80239c), the game table entry's thing


def item_fields(r, items, n, front):
    """the definitions' kind (+0x4ac), +0x284 and +0x288: in front cases a kind-0 one never reaches the exit test (+0x284 set or +0x288 2)"""
    for k in range(n):
        at = ITEM * k
        kind = r.choice([0, 0, 1, 2, 3]) if front else 0
        put(items, at + 0x4ac, 'I', kind)
        word = r.choice([0, 0, 1, 0x100])
        last = r.choice([0, 2, 1, 0x102])
        if front and kind == 0 and word == 0:
            last = 2
        put(items, at + 0x284, 'H', word)
        put(items, at + 0x288, 'I', last)
        if not front:
            put(items, at + 0x9c, 'I', r.choice([0, 4, 5]))


def front(r):
    """objectqueue's CanOpen case, the tracking fields, the world's words and the game table entry"""
    c = objectqueue.can_open(r)
    obj = c.regions[RAM]
    put(obj, 0x19c, 'I', r.choice([0, 1, 1, 2, 2, 3, 4, 5]))
    put(obj, 0x64, 'I', r.choice([0, 0, 1, 0x80000000]))
    put(obj, 0x68, 'I', r.choice([0, 0, 1]))
    put(obj, 0x5e, 'H', r.choice([0, 1, 2, 5, 11]))
    put(obj, 0x60, 'I', r.choice([0, 100, 5000, 0xfffffff0, r.getrandbits(32)]))
    items = bytearray(c.regions[ITEMS])
    n = struct.unpack_from('<i', c.regions[0x7890a0], 0x528)[0]   # (CanOpen's object may name a definition past the count the find searches)
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, ITEM * r.randrange(max(1, min(n, len(items) // ITEM))))[0])
    item_fields(r, items, len(items) // ITEM, True)
    c.regions[ITEMS] = items
    table = bytearray(c.regions[0x7cfb90])
    put(table, 20 * 11, 'I', THING)
    c.regions[0x7cfb90] = table
    for h in range(12):   # the things the tracked handle names: their kind and words (+0x218, +0x21a) the object's handle or not, their state
        thing = struct.unpack_from('<I', table, 20 * h)[0]
        for base, data in c.regions.items():
            if base <= thing and thing + 0x220 <= base + len(data):
                data = bytearray(data)
                off = thing - base
                data[off + 2] = r.choice([0, 4, 4, 5, 5, 6])
                mine = struct.unpack_from('<H', obj, 0)[0]
                put(data, off + 0x218, 'HH', mine if r.random() < 0.5 else r.getrandbits(16), mine if r.random() < 0.5 else r.getrandbits(16))
                put(data, off + 0x19c, 'I', r.choice([0xb, 0xd, 1, 2]))
                c.regions[base] = data
                break
        else:   # an entry the case left pointing nowhere: a thing of its own
            put(table, 20 * h, 'I', SPARE + 0x300 * h)
            spare = c.regions.setdefault(SPARE, bytearray(r.bytes(0x300 * 12)))
            spare[0x300 * h + 2] = r.choice([0, 4, 5])
            put(spare, 0x300 * h + 0x218, 'HH', *[struct.unpack_from('<H', obj, 0)[0] if r.random() < 0.5 else r.getrandbits(16) for _ in range(2)])
            put(spare, 0x300 * h + 0x19c, 'I', r.choice([0xb, 0xd, 1, 2]))
    thing = bytearray(r.bytes(0x220))
    put(thing, 0x28, 'II', r.choice([0, 0, 1]), 0)
    put(thing, 0x34, 'I', r.choice([0, 0, 1]))
    thing[2] = 4
    c.regions[THING] = thing
    tail = bytearray(r.bytes(0x40))   # world + 0x1da700: the tick (+0xc), the entry's index (+0x18), the words +0x3c, +0x3e
    stamp = struct.unpack_from('<I', obj, 0x60)[0]
    put(tail, 0xc, 'I', (stamp + r.choice([0, 0x63, 0x64, 0x65, 1000, r.getrandbits(32)])) & 0xffffffff)
    put(tail, 0x18, 'H', 11)
    put(tail, 0x3c, 'HH', r.choice([0, 0, 1, 7]), r.choice([0, 0, 1, 7]))
    c.regions[WORLD + 0x1da700] = tail
    return c


def back_end(r):
    """objectexit's case with the object past the earlier tests: state 0, 3 or 5, +0x64 0, bit 8 of +0x32 clear, track type 0, 4 or 5"""
    c = objectexit.case(r)
    obj = bytearray(c.regions[objectexit.OBJ]) + bytearray(r.bytes(0x200))
    put(obj, 0x19c, 'I', r.choice([0, 3, 5]))
    put(obj, 0x64, 'I', 0)
    obj[0x32] = r.choice([0, 1, 0xf7])
    put(obj, 0x68, 'I', r.choice([0, 0, 1]))
    c.regions[objectexit.OBJ] = obj
    items = bytearray(c.regions[ITEMS])
    n = struct.unpack_from('<i', c.regions[0x7890a0], 0x528)[0]   # (one of the definitions the find searches: a null one faults in both)
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, ITEM * r.randrange(max(1, min(n, len(items) // ITEM))))[0])
    item_fields(r, items, len(items) // ITEM, False)
    c.regions[ITEMS] = items
    return c


def case(r):
    c = front(r) if r.random() < 0.7 else back_end(r)
    count = r.choice(COUNTS)
    return Call(0x485f60, [c.args[0], count], c.regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
