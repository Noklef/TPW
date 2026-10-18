# The texture cache's eviction (src/game/gfx/texcache: 0x58faf0) against the original: six sizes of 0-8 entries (a few counts
# negative), each entry's items 0-3 (flags 0x4000 and 0x40000000 set or not) and its +0x1c and key around the bounds the size's area
# sets (and past them: the << 2 overflowing, keys negative, ties), the same entry twice now and then; kept 16 (any count up to 0x7fffffff
# capped) or -1-6. Now and then one size gets 15-20 entries that nearly all qualify (exactly 16 of them often, the near miss), with
# kept 16 or more: a 17th qualifying one writes past both arrays, over entries[0] and the return address (sic: a crash in both).
# usage: texcacheevict.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x58faf0, 0x58fcef), (0x590f80, 0x590f8a), (0x58b330, 0x58b344)])
CACHE, ENTRIES, OWNERS, ITEMS = RAM, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000
SIDES = [0x100, 0x80, 0x40, 0x20, 0x10, 8]


def used(r, area):
    q = area // 4
    return r.choice([0, -1, 1, q - 1, q, q + 1, r.randrange(1, q), r.randrange(1, q), 0x20000000, 0x40000000, r.getrandbits(32)])


def key(r, area, ties):
    h = area // 2
    return r.choice([h - 1, h, -5, 0x80000000, r.getrandbits(32), r.randrange(0, h), r.choice(ties), r.choice(ties)])


def case(r):
    cache = bytearray(r.bytes(0x1900))
    entries, owners, items = bytearray(r.bytes(0x20 * 80)), bytearray(r.bytes(0x10 * 80)), bytearray(r.bytes(0x20 * 0x100))
    ne = ni = 0
    big = r.randrange(6) if r.random() < 0.35 else -1
    for size, side in enumerate(SIDES):
        area = side * side
        ties = [r.randrange(0, area // 2) for _ in range(3)]
        count = r.choice([0, 1, 2, 3, 4, 5, 6, 8, r.randrange(9)]) if r.random() > 0.03 else r.choice([-1, -0x80000000])
        qualify = r.choice([16, 16, 17, 18, 15, 20]) if size == big else 0
        if size == big:
            count = max(qualify, r.choice([qualify, qualify, qualify + 1, 20]))
        put(cache, 0x1800 + 4 * size, 'i', count)
        made = []
        for i in range(max(count, 0)):
            if made and r.random() < 0.08:
                entry = r.choice(made)
            else:
                entry = ENTRIES + 0x20 * ne
                put(entries, 0x20 * ne, 'I', OWNERS + 0x10 * ne)
                put(entries, 0x20 * ne + 0xc, 'I', key(r, area, ties) & 0xffffffff)
                put(entries, 0x20 * ne + 0x1c, 'I', used(r, area) & 0xffffffff)
                if i < qualify:   # (a candidate: some items, +0x1c and the key under the bounds)
                    put(entries, 0x20 * ne + 0xc, 'I', r.choice([r.randrange(0, area // 2), r.choice(ties)]))
                    put(entries, 0x20 * ne + 0x1c, 'I', r.randrange(1, area // 4))
                link = 0
                for _ in range(r.choice([1, 1, 2]) if i < qualify else r.choice([0, 1, 1, 2, 3])):
                    flags = r.getrandbits(32)
                    for bit in (0x4000, 0x40000000):
                        flags = flags | bit if r.random() < 0.5 else flags & ~bit
                    put(items, 0x20 * ni + 4, 'II', link, flags)
                    link = ITEMS + 0x20 * ni
                    ni += 1
                put(entries, 0x20 * ne + 4, 'I', link)
                ne += 1
                made.append(entry)
            put(cache, 0x400 * size + 4 * i, 'I', entry)
    limit = r.choice([-1, 0, 1, 2, 3, 4, 6, 16, 16, 17, 0x7fffffff]) if big < 0 or r.random() < 0.15 else r.choice([16, 16, 17, 0x7fffffff])
    put(cache, 0x1878, 'i', limit)
    regions = {CACHE: cache, ENTRIES: entries, OWNERS: owners, ITEMS: items}
    return Call(0x58faf0, [CACHE], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
