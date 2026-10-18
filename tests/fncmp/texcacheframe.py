# The texture cache's per-frame reset (src/game/gfx/texcache: 0x58d8e0) against the original: [0x8bd508] with a bit of 0xc0000000 or not,
# the slot start [0x8bcc40] any; six sizes of 0-5 entries (a few counts negative), the same entry twice now and then, each entry's
# items 0-3 (flags any, 0x2000 and 0x40000000 often), each item's virtual base at an offset of its own (past the item, inside it, or
# overlapping its fields) with the slot bytes, +0x10 and +0x14 (negative often) random; the list at +0x18c4 of 0-3 items or none.
# Requeue: items put back (0x58e8d0), see requeue(). Shrink: items halved under a budget (0x58e1d0), see shrink().
# usage: texcacheframe.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Frame,Requeue,Shrink)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x58d8e0, 0x58da38), (0x56b650, 0x56b690), (0x58b390, 0x58b3a0), (0x590f80, 0x590fe0), (0x58e8d0, 0x58ea1b), (0x58e1d0, 0x58e3a0), (0x67a830, 0x67a860)])
CACHE, ENTRIES, ITEMS, VBT, LIST = RAM, RAM + 0x2000, RAM + 0x3000, RAM + 0x6000, RAM + 0x6800


def flags(r):
    f = r.getrandbits(32)
    for bit in (0x2000, 0x40000000, 0x4000):
        f = f | bit if r.random() < 0.5 else f & ~bit
    return f


def requeue(r):
    """0x58e8d0: a chain of 0-5 items (flag 0x40000000 often, the words +0x10 / +0x12 around multiples of 8, the float +0x30 about 0..1.5,
    keys +0x38 small and tied) put back into a list of 0-4 items (sorted by key, descending, or not), its head pointer's place given."""
    items, vbt = bytearray(r.bytes(0x80 * 12)), bytearray(r.bytes(8 * 12))
    keys = [r.randrange(-3, 4) for _ in range(3)]

    def item(i, link):
        at = 0x80 * i
        put(items, at, 'II', VBT + 8 * i, link)
        put(items, at + 8, 'I', flags(r) | (0x80000 if r.random() < 0.3 else 0))
        put(items, at + 0x10, 'hh', *(r.choice([0, 1, 7, 8, 9, -1, -8, 0x7ff9, r.randrange(-0x8000, 0x8000)]) for _ in range(2)))
        put(items, at + 0x30, 'f', r.choice([r.uniform(0, 1.5), 0.0, 1.0, 1.5, -1.0, 3.0]))
        put(items, at + 0x38, 'i', r.choice(keys + [r.getrandbits(32) - (1 << 31)]))
        put(vbt, 8 * i + 4, 'i', r.choice([0x40, 0x40, 0x48, 0x30, 8, 0x60]))
        return ITEMS + at

    n, m = r.choice([0, 1, 2, 3, 5]), r.choice([0, 1, 2, 4])
    link = 0
    for i in reversed(range(n)):
        link = item(i, link)
    head = 0
    listed = sorted(range(6, 6 + m), key=lambda i: -keys[i % 3]) if r.random() < 0.8 else list(range(6, 6 + m))
    for i in reversed(listed):
        head = item(i, head)
        if r.random() < 0.8:
            put(items, 0x80 * i + 0x38, 'i', keys[i % 3])
    regions = {ITEMS: items, VBT: vbt, RAM: put(bytearray(r.bytes(8)), 0, 'I', head), 0x879d48: bytes([r.randrange(16), 0, 0, 0])}
    return Call(0x58e8d0, [r.getrandbits(32), RAM, link if r.random() < 0.97 else 0], regions, conv='t', ret='v')


def shrink(r):
    """0x58e1d0: 0-6 items (some listed twice; flags 0x30, 0xc0, 0x80000 and 0x30000 each any, widths and heights around 8 and its
    multiples, the float +0x30 inside or on the bands' bounds), tiers 0-8, the bands' floats ascending mostly, the driver kind 1 often, the
    total at +0x18dc near the limit."""
    cache, items = bytearray(r.bytes(0x1920)), bytearray(r.bytes(0x80 * 6))
    bounds = sorted(r.uniform(0, 1.5) for _ in range(5)) if r.random() < 0.85 else [r.uniform(-1, 2) for _ in range(5)]
    put(cache, 0x1904, 'fffff', *bounds)
    n = r.choice([0, 1, 2, 3, 4, 6])
    for i in range(6):
        at = 0x80 * i
        f = r.getrandbits(32) & ~0x300f0 | r.choice([0, 0x10, 0x20, 0x30, 0x30]) | r.choice([0, 0x40, 0x80, 0xc0])
        f |= (0x80000 if r.random() < 0.2 else 0) | r.choice([0, 0, 0x10000, 0x30000])
        put(items, at + 8, 'I', f & ~0x80000 if r.random() < 0.8 else f)
        put(items, at + 0x14, 'hh', *(r.choice([8, 8, 9, 7, 16, 17, 1, 0, -1, -2, 0x100, 0x7fff, r.randrange(-0x8000, 0x8000)]) for _ in range(2)))
        put(items, at + 0x30, 'f', r.choice([r.uniform(0, 1.5), r.choice(bounds), 0.0, 1.5]))
    listed = [ITEMS + 0x80 * r.randrange(n if r.random() < 0.9 else 6) for _ in range(n)] if r.random() < 0.15 else [ITEMS + 0x80 * i for i in range(n)]
    lst = bytearray(r.bytes(4 * 8))
    for i, p in enumerate(listed):
        put(lst, 4 * i, 'I', p)
    limit = r.randrange(-0x400, 0x10000)
    put(cache, 0x18dc, 'i', limit + r.choice([0, 1, -1, 0x20, 0x100, 0x1000, r.randrange(-0x100, 0x10000)]))
    regions = {CACHE: cache, ITEMS: items, LIST: lst, 0x8bd520: r.bytes(4) + bytes([1 if r.random() < 0.5 else r.choice([0, 2]), 0, 0, 0])}
    count = len(listed) if r.random() < 0.95 else r.choice([-1, 0])
    return Call(0x58e1d0, [CACHE, LIST, count, r.randrange(9), limit], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Requeue':
        return requeue(r)
    if op == 'Shrink':
        return shrink(r)
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


OPS = [k for k in os.environ.get('OPS', 'Frame,Requeue,Shrink').split(',') if k]

if __name__ == '__main__':
    B.main(case)
