# A consistent park for the benches of the code that walks the thing table (src/game/world/things): the slots in use linked newest first, the
# things behind them, and (for the objects among them) the definitions gObjectDefs (0x7890a0) finds by id. Shared by thingscan.py and the like.
import struct

from fncmp import RAM, put

SLOTS = 0x7cfb90
USED = 0x7cf56c
POOL = RAM + 0x2000       # the things, 0x100 apart
THING = 0x100
DEFS, ITEMS, NITEMS, ITEM = 0x7890a0, RAM + 0x8000, 16, 0x500
WORLD_PTRS = (0x80239c, 0x7cf83c)   # the world pointers the callers load into ECX (the table code ignores them)


def park(r, count=None, kinds=(3, 3, 3, 1, 2, 0xff, 0), defs=True):
    """(regions, handles newest first, thing addresses newest first)."""
    count = r.choice([0, 1, 2, 3, 4, 6]) if count is None else count
    handles = r.sample(range(1, 12), count)
    slots = bytearray(r.bytes(20 * 12))
    things = bytearray(r.bytes(THING * 6))
    n = r.randrange(1, NITEMS + 1)
    ids = [r.choice([i, r.getrandbits(16)]) for i in range(NITEMS)]
    addrs = []
    for i, h in enumerate(handles):
        nxt = SLOTS + 20 * handles[i + 1] if i + 1 < count else 0
        prv = SLOTS + 20 * handles[i - 1] if i else 0
        put(slots, 20 * h, 'IIIII', POOL + THING * i, h, nxt, prv, 0)
        addrs.append(POOL + THING * i)
        t = THING * i
        put(things, t, 'H', h)
        things[t + 2] = r.choice(kinds)
        things[t + 3] = r.choice([0, 0, 0, 1, 5])
        things[t + 0x32] = r.choice([0, 1, 2, 3, 4, 0xff, r.getrandbits(8)])
        put(things, t + 0x68, 'I', r.choice([0, 0, 0, 1, 5]))
        put(things, t + 0xe, 'H', ids[r.randrange(n)])
        things[t + 0x50] = r.choice([0, 1, 5, 0x7f, 0x80, 0xff, r.getrandbits(8)])
    regions = {SLOTS: slots, POOL: things, USED: struct.pack('<I', SLOTS + 20 * handles[0] if count else 0)}
    for g in WORLD_PTRS:
        regions[g] = struct.pack('<I', RAM)
    if defs:
        dtab = bytearray(r.bytes(0x740))
        items = bytearray(r.bytes(NITEMS * ITEM))
        for i in range(NITEMS):
            put(dtab, i * 4, 'I', ITEMS + ITEM * i)
            put(items, i * ITEM, 'H', ids[i])
            put(items, i * ITEM + 0x9c, 'I', r.choice([3, 3, 0, 2]))
            put(items, i * ITEM + 0x284, 'H', r.choice([0, 0, 1, 5]))
            put(items, i * ITEM + 0x288, 'I', r.choice([0, 2, 2, 3]))
            put(items, i * ITEM + 0x4ac, 'I', r.choice([0, 3, 3, 5]))
        put(dtab, 0x528, 'i', n)
        k = r.randrange(n)
        put(dtab, 0x730, 'HxxI', ids[k] if r.random() < 0.3 else ids[k] ^ r.randrange(1, 0x10000), ITEMS + ITEM * r.randrange(NITEMS))
        regions[DEFS] = dtab
        regions[ITEMS] = items
    return regions, handles, addrs
