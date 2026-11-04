# The texture cache's frame work (src/game/gfx/texcache: 0x58cf40, 0x58ccc0, 0x58e110, 0x58e390, 0x58f190, 0x58f940, 0x58fcf0) against the originals, all the cache's children real, the surfaces' slots
# recorders: six sizes of 0-7 entries (surfaces 0x100 >> size wide now and then a little off, flags 0-7, kept around half the area), chains of 0-4 items on them, a pending list of 0-30 items (now and then
# 0x1003), sizes around powers of two, levels (flags 0x30, 0xc0), the driver filter on or off, bands and limits, splits within the counts or (shares far negative) past them; slots 4 and 5 answering 0 now and then.
# Arrange now and then with the room exactly 1.5 times the wanted area. Spread: 5-7 entries each, every other size's share 8-17 below its neighbours' and its surfaces skipped (its own short list's size is negative and would run off the stack), so that the others' short lists hold 3-5 (their smear, sic).
# usage: texcacheupdate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Rank,Trace,Arrange,Next,Pending,Rebalance,Spread,Update)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fail_mask, fb, log_fold, log_region, put, set_return, strings, vtable  # noqa: E402

FOPEN, FWRITE, FFLUSH, FCLOSE = 11, 12, 13, 14   # driver.cpp's kTexCacheTraceEnv
HOOKS = {0x67b8a0: _recorder(FOPEN, 2, cdecl=True), 0x67b6d0: _recorder(FWRITE, 4, cdecl=True), 0x67d070: _recorder(FFLUSH, 1, cdecl=True), 0x67b620: _recorder(FCLOSE, 1, cdecl=True)}
B = Bench([(0x58b330, 0x58b3a0), (0x58cb40, 0x58cfaa), (0x58dc50, 0x58dd5e), (0x58e110, 0x58e159), (0x58e1a0, 0x58e74d), (0x58e790, 0x58f938), (0x58f940, 0x590341), (0x590350, 0x590692), (0x590cb0, 0x590d40),
           (0x590f80, 0x590fe0), (0x590ff0, 0x59118c), (0x4217e0, 0x4217e4), (0x56af20, 0x56afa0), (0x56b650, 0x56b690), (0x57cbc0, 0x57d0d0), (0x67a830, 0x67a860), (0x67b110, 0x67b140)], hooks=HOOKS)
OPS = os.environ.get('OPS', 'Rank,Trace,Arrange,Arrange,Next,Next,Next,Pending,Rebalance,Rebalance,Spread,Update').split(',')
CACHE, ENTRIES, SURFACES, VT, OBJ, VBT, ITEMS = RAM, RAM + 0x2000, RAM + 0x2800, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200, RAM + 0x4000
ITEM = 0x80
MAX_ITEMS = 0xa8   # (the entries' chains)
SORT = 0x8bcb98
FILE = 0x12340000


def make_item(r, items, i, link, filtered):
    """Item i of the region: virtual base at +0x40 or +0x48, wanted sizes about the powers of two, current ones rounded up or halved, a level, the flags that matter."""
    at = ITEM * i
    put(items, at, 'II', VBT + 8 * (i % 0x40), link)
    flags = r.getrandbits(32) & ~0xf0f7 & ~0xb0000
    flags |= r.choice([0, 0x10, 0x20, 0x30, 0x30]) | r.choice([0, 0x40, 0x80, 0xc0]) | r.randrange(7)
    for bit, p in ((0x4000, 0.3), (0x80000, 0.15), (0x40000000, 0.6), (0x8000, 0.05), (0x2000, 0.3), (0x1000, 0.2)):
        flags = flags | bit if r.random() < p else flags & ~bit
    if filtered and r.random() < 0.6:
        flags |= r.choice([0x10000, 0x20000, 0x30000])
    put(items, at + 8, 'I', flags & 0xffffffff)
    want = [r.choice([4, 8, 8, 9, 16, 16, 31, 32, 64, 64, 128, 256, 300, r.randrange(1, 300)]) for _ in range(2)]
    put(items, at + 0x10, 'hh', *want)
    cur = [(w + 7) & ~7 if r.random() < 0.7 else (w + 1) >> 1 for w in want]
    put(items, at + 0x14, 'hh', *cur)
    put(items, at + 0x1c, 'I', r.getrandbits(32))
    put(items, at + 0x30, 'f', r.choice([r.uniform(0, 1.5), r.uniform(0, 1.5), 0.0, 1.5, 1.0, -0.5, 3.0]))
    if r.random() < 0.02:
        put(items, at + 0x30, 'I', r.choice([0x7fc00000, 0xffc00000, 0x7fa00000, 0x7f800000, r.f()]))
    put(items, at + 0x38, 'i', r.choice([r.randrange(-3, 40), r.getrandbits(31)]))
    put(items, at + 0x3c, 'I', r.choice([0, r.getrandbits(32)]))


def world(r, pending=None, negative=False, spread=False):
    """The regions, and the cache's bytes (at CACHE, to set more in)."""
    log = log_region()
    log_fold(log)
    strings(log, FOPEN, 3)
    blob(log, FWRITE, 0, 0x1)
    filtered = r.random() < 0.6
    cache = bytearray(r.bytes(0x1930))
    entries, surfaces = bytearray(r.bytes(0x20 * 42)), bytearray(r.bytes(0x20 * 42))
    n = r.choice([0, 1, 2, 3, 5, 8, 14, 30]) if pending is None else pending
    items = bytearray(r.bytes(ITEM * (n + MAX_ITEMS)))
    vbt = bytearray(r.bytes(8 * 0x40))
    for i in range(0x40):
        put(vbt, 8 * i + 4, 'i', r.choice([0x40, 0x48]))
    next_item = 0
    area_answer = r.choice([0x100, 0x400, 0x1000, 0x4000, 0x10000])

    def chain(count):
        nonlocal next_item
        link = 0
        for _ in range(count):
            make_item(r, items, next_item, link, filtered)
            link = ITEMS + ITEM * next_item
            next_item += 1
        return link

    head = chain(n)
    put(cache, 0x18c4, 'I', head)
    ne = 0
    odd = r.randrange(2)
    for size in range(6):
        count = r.choice([5, 6, 7]) if spread else r.choice([0, 1, 2, 3, 4, 6, 7])
        put(cache, 0x1800 + 4 * size, 'i', count)
        if spread:
            shared = int(count * (r.uniform(-50, -44) if (size + odd) & 1 else r.uniform(-36, -33)))
        elif negative and count:
            shared = int(count * r.uniform(-40, -33)) if r.random() < 0.9 else r.randrange(-40 * count, -33 * count)
        elif count:
            shared = r.choice([0, count, count // 2, r.randrange(count + 1), r.randrange(count + 1), -1])
        else:
            shared = r.choice([0, 0, 1, -1])
        put(cache, 0x1818 + 4 * size, 'i', shared)
        put(cache, 0x1830 + 4 * size, 'i', r.randrange(count + 1))
        put(cache, 0x400 * size + 4 * count, 'I', ENTRIES)   # (the slot past the last, a next entry when the split passes the count)
        for i in range(count):
            at = 0x20 * ne
            side = 0x100 >> size
            put(surfaces, at, 'III', VT, (6 if (size + odd) & 1 else r.choice([0, 0, 0, 4, 1])) if spread else r.choice([0, 1, 2, 3, 4, 5, 7, r.getrandbits(32)]), r.choice([side, side, side, side >> 1, side * 2]))
            put(surfaces, at + 0xc, 'I', r.choice([side, side, side >> 1]))
            put(entries, at, 'II', SURFACES + at, chain(r.choice([0, 0, 1, 1, 2, 3, 4])))
            kept = r.choice([side * side // 2 - 1, 0]) if spread else r.choice([area_answer // 2, area_answer // 2 + 1, r.randrange(area_answer), side * side // 2, side * side // 2 - 1, 0])
            put(entries, at + 8, 'iii', r.randrange(0x8000), kept, r.randrange(-100, 100))
            put(entries, at + 0x14, 'i', r.getrandbits(31))
            put(cache, 0x400 * size + 4 * i, 'I', ENTRIES + at)
            ne += 1
    put(cache, 0x18cc, 'I', r.getrandbits(32) & ~7 | r.choice([0, 0, 1, 2, 3, 4, 4, 5, 6]) )
    a, b = r.randrange(0x40000), r.randrange(0x40000)
    put(cache, 0x18d0, 'iii', r.randrange(0x10000), a, b)
    put(cache, 0x18e4, 'I', r.choice([7, 7, 3, 0]))
    put(cache, 0x18e8, 'I', r.choice([0, 0, 1]))
    put(cache, 0x18f8, 'i', r.choice([0, 1, 2, 5, 8, 0xb, 0xc]))
    put(cache, 0x18fc, 'i', r.choice([0, 0x100, 0x1000, 0x8000, r.randrange(0x40000)]))
    put(cache, 0x1900, 'I', r.choice([fb(1.0), fb(256.0)]))
    put(cache, 0x1878, 'i', r.choice([0, 1, 2, 4, 16, 20]))
    set_return(log, 3, area_answer)
    fail_mask(log, 4, r.choice([0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0, r.getrandbits(32)]))
    set_return(log, 5, r.choice([1, 1, 1, 1, 0, 2]))
    set_return(log, FOPEN, r.choice([FILE, FILE, 0]))
    g = r.getrandbits(32)
    regions = {CACHE: cache, ENTRIES: entries, SURFACES: surfaces, VT: vtable({7: 3}, 8), OBJ: r.bytes(8), VBT: vbt, ITEMS: items, LOG: log,
               0x87a248: struct.pack('<I', OBJ), SORT: bytes(0xc), 0x879d48: bytes([r.randrange(8), 0, 0, 0]),
               0x8bd508: struct.pack('<I', g | 0x40000000 if r.random() < 0.3 else g & 0x3effffff), 0x8bd520: r.bytes(4) + struct.pack('<I', r.choice([1, 1, 1, 0, 4])),
               0x8bd550: bytes([r.getrandbits(8)]), 0x8bcc40: struct.pack('<I', r.getrandbits(31)), 0x8c4b50: r.bytes(0x24),
               0x8c1628: struct.pack('<6I', *[r.choice([fb(0.0), fb(0.5)]) for _ in range(6)]), 0x879ad0: put(bytearray(r.bytes(0x204)), 0x200, 'i', r.choice([0, 3, 0x40]))}
    return regions, cache


IGNORE = [(SORT + 8, 4)]


def rank(r):
    regions, _ = world(r)
    return Call(0x58ccc0, [CACHE], regions, conv='t', ret='v')


def trace(r):
    regions, _ = world(r, 0)
    return Call(0x58e110, [r.getrandbits(32)], regions, conv='t', ret='v')


def arrange(r):
    regions, cache = world(r, 0x1003 if r.randrange(150) == 0 else None)
    if r.random() < 0.25:   # (the room just 1.5 times the area: no shrinking yet)
        link, area = struct.unpack_from('<I', cache, 0x18c4)[0], 0
        while link:
            at = link - ITEMS
            w, h = struct.unpack_from('<hh', regions[ITEMS], at + 0x10)
            area += w * h
            link = struct.unpack_from('<I', regions[ITEMS], at + 4)[0]
        put(cache, 0x18cc, 'I', 0)
        put(cache, 0x18d4, 'ii', 0x1000, 0x1000 + area * 3 // 2)
        put(cache, 0x18f8, 'i', 0)
    return Call(0x58e390, [CACHE], regions, conv='t', ret='v', ignore=IGNORE, steps=60_000_000)


def place_next(r):
    regions, cache = world(r, r.choice([1, 1, 2, 3, 5, 8, 14]))
    return Call(0x58f190, [CACHE, CACHE + 0x18c4], regions, conv='t', ret='a', ignore=IGNORE)


def pending(r):
    regions, cache = world(r, r.choice([0, 1, 2, 3, 5, 8, 14]))
    return Call(0x58f940, [CACHE], regions, conv='t', ret='a', ignore=IGNORE)


def rebalance(r):
    regions, cache = world(r, r.choice([0, 1, 3]), negative=r.random() < 0.2)
    if r.random() < 0.8:
        regions[0x8bd520] = regions[0x8bd520][:4] + struct.pack('<I', 1)
    return Call(0x58fcf0, [CACHE], regions, conv='t', ret='a', ignore=IGNORE)


def spread(r):
    regions, cache = world(r, 0, spread=True)
    regions[0x8bd520] = regions[0x8bd520][:4] + struct.pack('<I', 1)
    return Call(0x58fcf0, [CACHE], regions, conv='t', ret='a', ignore=IGNORE)


def update(r):
    regions, cache = world(r, r.choice([0, 1, 2, 3, 5, 8]))
    return Call(0x58cf40, [CACHE], regions, conv='t', ret='a', ignore=IGNORE)


FUNCS = {'Rank': rank, 'Trace': trace, 'Arrange': arrange, 'Next': place_next, 'Pending': pending, 'Rebalance': rebalance, 'Spread': spread, 'Update': update}


def case(r):
    return FUNCS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
