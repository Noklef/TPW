# The texture cache's per-frame reset (src/game/gfx/texcache: 0x58d8e0) against the original: [0x8bd508] with a bit of 0xc0000000 or not,
# the slot start [0x8bcc40] any; six sizes of 0-5 entries (a few counts negative), the same entry twice now and then, each entry's
# items 0-3 (flags any, 0x2000 and 0x40000000 often), each item's virtual base at an offset of its own (past the item, inside it, or
# overlapping its fields) with the slot bytes, +0x10 and +0x14 (negative often) random; the list at +0x18c4 of 0-3 items or none.
# usage: texcacheframe.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x58d8e0, 0x58da38), (0x56b650, 0x56b690), (0x58b390, 0x58b3a0), (0x590fa0, 0x590fb0)])
CACHE, ENTRIES, ITEMS, VBT = RAM, RAM + 0x2000, RAM + 0x3000, RAM + 0x6000


def flags(r):
    f = r.getrandbits(32)
    for bit in (0x2000, 0x40000000, 0x4000):
        f = f | bit if r.random() < 0.5 else f & ~bit
    return f


def case(r):
    cache = bytearray(r.bytes(0x1900))
    entries, items, vbt = bytearray(r.bytes(0x20 * 40)), bytearray(r.bytes(0x80 * 0x60)), bytearray(r.bytes(8 * 0x60))
    ni = ne = 0

    def item(link):
        nonlocal ni
        at = 0x80 * ni
        put(items, at, 'II', VBT + 8 * ni, link)
        put(items, at + 8, 'I', flags(r))
        # (never below 8: a base over the link at +4 would have its slot byte cleared there, a list walking off anywhere)
        put(vbt, 8 * ni + 4, 'i', r.choice([0x40, 0x40, 0x40, 0x48, 0x30, 8, 0x60]))
        if r.random() < 0.5:
            put(items, at + 0x40 + 0x14, 'i', r.randrange(-0x100, 0x100))
        ni += 1
        return ITEMS + at

    for size in range(6):
        count = r.choice([0, 1, 2, 3, 4, 5]) if r.random() > 0.04 else r.choice([-1, -0x80000000])
        put(cache, 0x1800 + 4 * size, 'i', count)
        made = []
        for i in range(max(count, 0)):
            if made and r.random() < 0.08:
                entry = r.choice(made)
            else:
                link = 0
                for _ in range(r.choice([0, 1, 1, 2, 3])):
                    link = item(link)
                put(entries, 0x20 * ne, 'II', ENTRIES + 0x20 * ne + 0x10, link)
                entry = ENTRIES + 0x20 * ne
                ne += 1
                made.append(entry)
            put(cache, 0x400 * size + 4 * i, 'I', entry)
    link = 0
    for _ in range(r.choice([0, 1, 2, 3])):
        link = item(link)
    put(cache, 0x18c4, 'I', link)
    g = r.getrandbits(32)
    g = g | r.choice([0x40000000, 0x80000000, 0xc0000000]) if r.random() < 0.5 else g & 0x3fffffff
    regions = {CACHE: cache, ENTRIES: entries, ITEMS: items, VBT: vbt, 0x8bd508: put(bytearray(4), 0, 'I', g),
               0x8bcc40: r.bytes(4), 0x879d08: r.bytes(0x44)}
    return Call(0x58d8e0, [CACHE], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
