# The park as a whole, for the benches of code that asks several of its parts at once (the advisor criteria: advisorcriteria.py). One game
# object G ([[0x7cf6ec]], [0x80239c] and [0x7cf83c]) and, consistent with each other:
# - thingenv.py's things (kinds 1, 3 and the staff 4-8 mostly; their byte +3 0 mostly) on the first eight map cells of a compact grid
#   ([0x8023a0]: types 0-0xb mostly); the people's floats +0x19c 0..100 mostly, the others' +0x19c states 1 mostly; patrol regions
#   (+0x20a / +0x20c) none (1, 0x4000) or not;
# - three chains of some of them: the booths' (+0xa, from the two ticket booths' cells: the positions 0x78559c, the cells G + 0x294),
#   the objects list (+0xc, from G's word +0x1da746) and the sum chain (+0x210, from G's word +0x1da742; the index +0x1e4 into
#   0x78545c's 12-byte entries);
# - the object definitions (0x7890a0: 1 to 8, now and then none; sparse: only their ids, +0xc4 and +0x4ac set), the things' ids among
#   theirs (now and then not: a fault in both), and the game's control records for the ids (G + 0x1d943c: one each, now and then one
#   missing; their byte +0x10 and dwords +0x14 / +0x18 about 0, signed) with the find caches hit or not;
# - G's time +0x1da70c, its flag +0x1da710 (0 or not), and the slot words +0x1da720 (the park analyser) and +0x1da722 (the research lab);
# - the park analyser: its kept count of the people on plain cells (+0x1f0b8: kept or not, for the time's quarter or older), the
#   booths' count +0x1f0d0 and its tallies +0x21bfc / +0x21c00 / +0x21c08;
# - the research lab: 0-12 projects (or a negative count) of categories 0-5 mostly, done or not; its groups' dwords (Table_ScaledBySum's
#   +0x1410 + 16 * index) 0 mostly; and the globals 0x7852f8 and 0x78548c (their product Table_ScaledBySum's total) 0 now and then.
# park(r) returns (regions, info): info['things'] the things' addresses, info['kinds'] their kinds.
import struct

from fncmp import RAM, put
import thingenv

SLOTS, POOL, THING = thingenv.SLOTS, thingenv.POOL, thingenv.THING
GAME = RAM + 0x100000
GAMEPTR = RAM + 0x1000                  # [0x7cf6ec] points here; the dword here is G
FIELDS = GAME + 0x1d943c                # the control table, then G's fields from +0x12c0
TAIL = FIELDS + 0x12c0
BOOTHS = GAME + 0x294                   # the world's map cells the booths read (the first row's)
CELLS = RAM + 0x30000                   # gMapCells (0x8023a0): the things' cells
ANALYSER, LAB = RAM + 0x200000, RAM + 0x40000
ASLOT, LSLOT = 12, 13                   # their slots at 0x7cfb90
DEFS, ITEMS, ITEM = 0x7890a0, RAM + 0x8000, 0x500
IDS = [1, 2, 3, 4, 5, 9]
KINDS = (1, 1, 1, 3, 3, 3, 4, 5, 6, 7, 8, 0)
TYPES = [0, 1, 2, 3, 4, 5, 8, 9, 0xa, 0xb, 0x109]
FLOATS = [0.0, 5.0, 29.0, 29.9, 30.0, 31.0, 50.0, 99.0, 100.0]


def small(r, *extra):
    return r.choice([-1, 0, 0, 1, 1, 2, 3, 5] + list(extra) + [r.getrandbits(32)]) & 0xffffffff


def chain(r, handles, things, link, pool):
    """Links a random subset of the things (by their handles) through the word at link; returns the head's handle (or 0)."""
    order = r.sample(range(len(handles)), r.randrange(len(handles) + 1)) if handles else []
    for k, i in enumerate(order):
        put(pool, THING * i + link, 'H', handles[order[k + 1]] if k + 1 < len(order) else 0)
    return handles[order[0]] if order else 0


def park(r, kinds=KINDS, count=None):
    regions, handles, things = thingenv.park(r, count=count, kinds=kinds, defs=False)
    pool = regions[POOL]
    known = r.sample(IDS, r.choice([1, 2, 4, 6, 6]))
    n = r.choice([1, 2, 4, 6, 8, 8]) if r.random() < 0.97 else 0
    ids = [r.choice(known) for _ in range(n)]
    for i in range(len(handles)):
        t = THING * i
        pool[t + 3] = r.choice([0, 0, 0, 0, 1])
        pool[t + 5], pool[t + 7] = r.randrange(8), 0
        if pool[t + 2] == 1:
            put(pool, t + 0x19c, 'f', r.choice(FLOATS) if r.random() < 0.9 else r.choice([-1.0, 255.5, 300.0, float('nan')]))
        else:
            put(pool, t + 0x19c, 'I', r.choice([1, 1, 1, 0, 2, 0x101]))
        put(pool, t + 0xe, 'H', r.choice(ids) if ids and r.random() < 0.97 else r.choice(IDS))
        put(pool, t + 0x1e4, 'I', r.randrange(4))
        put(pool, t + 0x220, 'I', r.choice([3, 3, 0, 2, 0x103]))
        put(pool, t + 0x20a, 'HH', *r.choice([(1, 0x4000), (1, 0x4000), (1, 0x4001), (2, 0x4000)]))
        put(pool, t + 0x212, 'H', r.choice([0, 0, 0, 1, 0x3000]))
    booth = chain(r, handles, things, 0xa, pool)
    objects = chain(r, handles, things, 0xc, pool)
    sums = chain(r, handles, things, 0x210, pool)
    # The slots: the things' (thingenv), the analyser's and the lab's.
    slots = bytearray(regions[SLOTS]) + bytearray(r.bytes(20 * 4))
    put(slots, 20 * ASLOT, 'I', ANALYSER)
    put(slots, 20 * LSLOT, 'I', LAB)
    regions[SLOTS] = slots
    for g in thingenv.WORLD_PTRS:
        regions[g] = struct.pack('<I', GAME)
    regions[0x7cf6ec] = struct.pack('<I', GAMEPTR)
    regions[GAMEPTR] = struct.pack('<I', GAME)
    # The map cells.
    grid = bytearray(r.bytes(0x44 * 8))
    for c in range(8):
        put(grid, 0x44 * c + 8, 'I', r.choice(TYPES))
    regions[CELLS] = grid
    regions[0x8023a0] = struct.pack('<II', CELLS, 0)
    spots = [r.randrange(8) for _ in range(2)]
    regions[0x78559c] = struct.pack('<4I', spots[0], 0, spots[1], 0)
    cells = bytearray(r.bytes(0x250))
    heads = [booth, booth, 0] + ([h for h in handles] if handles else [])
    for x in spots:
        put(cells, 0x44 * (x + 1) + 0x24, 'H', r.choice(heads))
    regions[BOOTHS] = cells
    # The definitions (sparse) and their control records.
    dtab = bytearray(r.bytes(0x20))
    for i in range(8):
        put(dtab, 4 * i, 'I', ITEMS + ITEM * i)
        if i < n:
            regions[ITEMS + ITEM * i] = struct.pack('<H', ids[i]) + r.bytes(2)
            regions[ITEMS + ITEM * i + 0xc4] = struct.pack('<i', r.choice([0, 0, 0, 1, -1, 7]))
            regions[ITEMS + ITEM * i + 0x4ac] = struct.pack('<I', r.choice([0, 0, 1, 2, 3, 4, 0x100]))
    regions[DEFS] = dtab
    regions[DEFS + 0x528] = struct.pack('<i', n)
    regions[DEFS + 0x72c] = struct.pack('<iHHI', r.choice([-1, 0, 3]), r.choice(IDS + [0, 0x77]), 0, ITEMS + ITEM * r.randrange(max(n, 1)))
    table = bytearray(r.bytes(0xc0))
    recs = r.sample(IDS, len(IDS))
    if r.random() < 0.03:
        recs.pop()
    for k, i in enumerate(recs):
        put(table, 0x20 * k, 'H', i)
        table[0x20 * k + 0x10] = r.choice([0, 1, 1, 0x80])
        put(table, 0x20 * k + 0x14, 'I', small(r, 0x80000000))
        put(table, 0x20 * k + 0x18, 'I', small(r, 0x80000000))
    regions[FIELDS] = table
    # G's fields: the table's count and cache, the time, the flag, the slot words, the chain heads.
    now = r.getrandbits(32)
    tail = bytearray(r.bytes(0x50))
    put(tail, 0, 'iHHI', len(recs), r.choice(IDS + [0, 0x77]), 0, FIELDS + 0x20 * r.randrange(len(recs)))
    put(tail, 0x10, 'II', now, r.choice([0, 0, 0, 1, 0x100, 0xffffffff]))
    put(tail, 0x24, 'HH', ASLOT, LSLOT)
    put(tail, 0x46, 'H', sums)
    put(tail, 0x4a, 'H', objects)
    regions[TAIL] = tail
    # The park analyser.
    kept = bytearray(r.bytes(0x30))
    put(kept, 8, 'III', (now - r.choice([0, 0, 0, 1, 3, 4, 400])) & 0xffffffff, r.choice([0, 1, 2, 3, 60, 199, 200, 201, 250, -1]) & 0xffffffff,
        r.choice([0, 1, 1, 1, 0xff]))
    regions[ANALYSER + 0x1f0b0] = kept
    tallies = bytearray(r.bytes(0x20))
    put(tallies, 0xc, 'II', small(r), small(r))
    put(tallies, 0x18, 'I', small(r))
    regions[ANALYSER + 0x21bf0] = tallies
    # The research lab and the staff globals.
    projects = r.choice([0, 1, 2, 3, 5, 8, 12, 12, -1])
    lab = bytearray(r.bytes(0x100))
    for i in range(12):
        p = 0xc + 20 * i
        put(lab, p + 4, 'I', r.choice([0, 1, 2, 3, 0, 1, 2, 3, 4, 5]))
        cost = r.choice([0.0, 1.0, 50.0, 100.0])
        put(lab, p + 0xc, 'ff', cost, r.choice([cost, cost, 0.0, 10.0, 100.0, float('nan')]))
    regions[LAB] = lab
    groups = bytearray(r.bytes(0x60))
    for g in range(5):
        put(groups, 0x10 + 16 * g, 'I', r.choice([0, 0, 0, 1, 50, r.getrandbits(32)]))
    put(groups, 0x5c, 'i', projects)
    regions[LAB + 0x1400] = groups
    regions[0x7852f8] = struct.pack('<I', r.choice([0, 1, 2, 5]))
    regions[0x78545c] = b''.join(struct.pack('<I', r.choice([0, 1, 3, 10])) for _ in range(12)) + struct.pack('<I', r.choice([0, 1, 3, 100]))
    return regions, {'things': things, 'kinds': [pool[THING * i + 2] for i in range(len(handles))]}
