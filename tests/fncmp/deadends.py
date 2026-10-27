# An object demolished off the map (src/game/world/demolish: Path_CheckDeadEnds 0x527ee0) against the original. The world (the map
# [0x7cf83c] and [0x80239c], object.py's WORLD with its clock) has the cells within seven rows and columns of the object's cell (and
# their 0x28-byte entries), each cell's first word its own id, the types mostly 1, 4, 9 and 0xa (9 and 0xa not on the map's edge, their direction one bit),
# the flag 0x20 half the time; half the queue cells (9 / 0xa) have the path cell they step to (behind / ahead) of type 1 with flag 0x20.
# The directions' steps (0x7cdba0, set at run time) laid out. The cell clicked (on the map or not; of type 4, 9, 0xa or another) or its parent (+0x10) heads a chain
# (+0x24) of a thing of kind 1 and / or the object (kind 3), whose word +0x34 is a cell next to it or the same. The object's slot, its
# definition (gObjectDefs, the cache hit now and then; +0x40, kind +0x9c 0-3, the scrap values), its footprint (-1 to 3 columns and rows,
# cells taken at random), its angle (0x5a, 0xb4, 0x10e or another; for kinds 1 and 2 square, with the
# track runs' first cells track dead ends: track()), its +0x28 and +0x40; the money record and the park's (the handles at
# +0x1da720 / +0x1da726); the rates 0x81ea14, 0x81aebc and 0x81b43c; sound on or off (0x785a2c) and Sound_PlayAt's manager as soundplay.py's
# (slot 2 a recorder, k 2). The unported calls are recorders (driver.cpp's kDemolishEnv): agent B's 0x530120 (k 3), 0x52f200 (k 4),
# 0x52f580 (k 5), 0x52fe50 (k 6, answering runs of not 0 ended by a 0 from a table), agent B's 0x53b280 (k 7), 0x545610 (k 8) and 0x5367a0
# (k 28). Thing_Delete as thingdetach.py's for kind 3 (agent C's 0x4dd0a0 recorder k 10, the post 0x40fb10 k 21, its message centre
# made already (else its slots' allocator bytes are stack bytes), the delete queue).
# usage: deadends.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, blob, blob_bytes, global_heap, heap, put, set_return, vtable  # noqa: E402
from funnytime import RANGES as CLOCK_RANGES  # noqa: E402
from object import WORLD, clock  # noqa: E402
from thingdetach import centre  # noqa: E402

B = Bench([(0x527ee0, 0x5286b0), (0x412e90, 0x412f00), (0x4d01f0, 0x4d0270), (0x4d0af0, 0x4d0b20), (0x4d8300, 0x4d8340),
           (0x4d8c00, 0x4d8c60), (0x4d96f0, 0x4d9a70), (0x4dd4e0, 0x4dd510), (0x50c320, 0x50c5f0), (0x519510, 0x519560),
           (0x5195d0, 0x519620), (0x522770, 0x5227a0), (0x522810, 0x522870), (0x5228a0, 0x5228c0), (0x527d60, 0x527e90),
           (0x52a3a0, 0x52a4a0), (0x52f850, 0x52f870), (0x52fb40, 0x52fba0), (0x52fbd0, 0x52fbf0), (0x530830, 0x530e90),
           (0x535db0, 0x535de0), (0x536340, 0x536370), (0x536380, 0x5363a0), (0x53a530, 0x53a550), (0x53ad30, 0x53ada0),
           (0x53adb0, 0x53aed0), (0x5da3c0, 0x5da3c1), (0x4e2290, 0x4e2410), (0x51bfc0, 0x51c060),
           # (Thing_Delete and what it reaches)
           (0x40f470, 0x40f4a0), (0x40f900, 0x40f930), (0x40ff80, 0x4104c0), (0x410610, 0x410730), (0x475550, 0x4755a0),
           (0x4a8bd0, 0x4a8c00), (0x4d3d10, 0x4d3d90), (0x4d8460, 0x4d8490), (0x4e0210, 0x4e0300), (0x4f7c40, 0x4f8b20),
           (0x4f9460, 0x4f94a0), (0x4fb330, 0x4fb370), (0x502920, 0x502970), (0x5041f0, 0x504220), (0x50b350, 0x50b370),
           (0x50b550, 0x50b6b0), (0x50b780, 0x50b980), (0x512000, 0x512020), (0x516310, 0x516340), (0x519550, 0x5195a0),
           (0x519690, 0x5197d0), (0x519800, 0x519940), (0x519970, 0x519ab0), (0x519ae0, 0x519c20), (0x519c50, 0x519d90),
           (0x519dc0, 0x519f00), (0x67acf0, 0x67ad6f),
           # (thingdetach.py's others)
           (0x405460, 0x405530), (0x415e00, 0x415e80), (0x5e7720, 0x5e7750), (0x5f5e20, 0x5f5e80), (0x67a400, 0x6902c0), (0x409250, 0x409260),
           (0x40f950, 0x40f961), (0x40f970, 0x40fb03), (0x410ac0, 0x410b11), (0x40fe60, 0x40ff80), (0x4107b0, 0x410ac0), (0x410b20, 0x410bf0),
           (0x504090, 0x504132), (0x504140, 0x5041e2), (0x40f630, 0x40f650), (0x51ad40, 0x51ad90), (0x5f5660, 0x5f56a0), (0x50bab0, 0x50bac4),
           (0x4faa60, 0x4fada1), (0x50c190, 0x50c1d0), (0x4d9280, 0x4d93b0), (0x4d4170, 0x4d4186)] + CLOCK_RANGES,
          hooks={0x530120: _recorder(3, 1, cdecl=True), 0x52f200: _recorder(4, 2, cdecl=True), 0x52f580: _recorder(5, 2, cdecl=True),
                 0x52fe50: _recorder(6, 2, cdecl=True), 0x53b280: _recorder(7, 3), 0x545610: _recorder(8, 1, cdecl=True),
                 0x5367a0: _recorder(28, 2), 0x4dd0a0: _recorder(10, 0), 0x40fb10: _recorder(21, 1), 0x53afa0: _recorder(24, 0),
                 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
MAP = WORLD
SLOTS, POOL, THING = 0x7cfb90, RAM + 0x2000, 0x400
OBJ, OTHER, MONEY, AN = POOL, POOL + THING, RAM + 0x3000, RAM + 0x10000
DEFS, ITEMS, ITEM, SHAPE = 0x7890a0, RAM + 0x4000, 0x400, RAM + 0x6000
MANAGER, VT, TABLE, QNODES = RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
TYPES = [1, 1, 1, 4, 4, 4, 9, 9, 9, 0xa, 0xa, 0xa, 0, 3, 2]
BITS = [1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80]


def cell_id(x, y):
    return ((y << 7) + x + 1) & 0xffff


def clip(v):
    return min(127, max(0, v))


def near(r, x, y):
    return clip(x + r.choice([-1, 0, 0, 1])), clip(y + r.choice([-1, 0, 0, 1]))


def init(r, cells, c, cx, cy):
    """A cell: its own id first; near the object types mostly 1, 4, 9, 0xa (9 and 0xa not on the map's edge, their direction one bit)."""
    put(cells, c, 'H', cell_id(cx, cy))
    kind = r.choice(TYPES)
    if kind in (9, 0xa) and (cx in (0, 127) or cy in (0, 127)):
        kind = 4
    put(cells, c + 8, 'I', kind)
    if kind in (9, 0xa) or r.random() < 0.5:
        cells[c + 0xd] = r.choice(BITS)
    put(cells, c + 0xe, 'H', r.getrandbits(16) & ~0x20 | (0x20 if r.random() < 0.6 else 0))
    cells[c + 0x26] = r.getrandbits(8) & ~8 | (8 if r.random() < 0.3 else 0)


STEP_A = {0: (4, 1, 4), 0x5a: (1, -5, 1), 0xb4: (-5, -2, 0x40), 0x10e: (-2, 4, 0x10)}   # (Position_StepA: offset, direction)
STEP_B = {0: (-2, 1, 0x40), 0x5a: (1, 1, 0x10), 0xb4: (1, -2, 4), 0x10e: (-2, -2, 1)}  # (Position_StepB's, the code reversed)
REVERSE = {1: 0x10, 4: 0x40, 0x10: 1, 0x40: 4}
# (the directions' steps (dx, dy) at 0x7cdba0 +, set at run time, as cellrun.py's)
STEPS = {1: (0, -1), 2: (1, -1), 4: (1, 0), 8: (1, 1), 0x10: (0, 1), 0x20: (-1, 1), 0x40: (-1, 0), 0x80: (-1, -1)}
STEPS_AT = {1: 0, 0x10: 8, 0x20: 0x10, 8: 0x18, 0x80: 0x20, 4: 0x28, 0x40: 0x30, 2: 0x38}
DIRECTIONS = bytearray(0x40)
for _d, _at in STEPS_AT.items():
    put(DIRECTIONS, _at, 'ii', *STEPS[_d])


def track(r, entries, clo, obj, ax, ay):
    """Kinds 1 and 2: the object at (ax, ay) square to the map, the 0x28-byte entries TrackRun_TraceA / TraceB start from track (0xb or
    0x10) dead ends (neighbours +0xc none, or only the one behind), so the runs end there (on an entry of another type, or at another
    angle, they would run on for ever)."""
    angle = r.choice(list(STEP_A))
    put(obj, 0x10, 'I', angle)
    obj[5], obj[7] = ax, ay
    for dx, dy, direction in (STEP_A[angle], STEP_B[angle]):
        e = entries[ay + dy]
        c = 0x28 * (ax + dx - clo)
        put(e, c + 8, 'I', r.choice([0xb, 0x10]))
        e[c + 0xc] = e[c + 0x22] = r.choice([0, REVERSE[direction]])


def case(r):
    regions = {HEAP: heap(), **global_heap()}
    obj = bytearray(r.bytes(THING))
    clock(r, regions, obj)          # (the log, the imports, [0x80239c] = WORLD, its clock and ticks; the object's date and level +0x50)
    log = regions[LOG]
    regions[0x7cf83c] = struct.pack('<I', MAP)
    regions[0x8023a0] = struct.pack('<I', MAP + 0x2d8)
    x, y = r.randrange(128), r.randrange(128)
    if r.random() < 0.3:
        x, y = r.choice([0, 1, 126, 127, x]), r.choice([0, 1, 126, 127, y])
    click = (x, y)
    parent = near(r, x, y) if r.random() < 0.2 else click
    ax, ay = near(r, *parent)
    if r.random() < 0.05:
        x, y = r.choice([-1, 128, x]), r.choice([-1, 128, -0x80000000, y])
    lo, hi = max(0, ay - 7), min(127, ay + 7)       # (the cells within seven of the object's: Map_StepCell keeps to the map)
    clo, chi = max(0, ax - 7), min(127, ax + 7)
    width = chi - clo + 1
    rows = {cy: bytearray(r.bytes(0x44 * width)) for cy in range(lo, hi + 1)}
    entries = {}
    for cy in range(lo, hi + 1):
        for cx in range(clo, chi + 1):
            init(r, rows[cy], 0x44 * (cx - clo), cx, cy)
    for cy in range(lo, hi + 1):    # (half the queue cells' path cells (9: behind, 0xa: ahead) of type 1 with flag 0x20)
        for cx in range(clo, chi + 1):
            c = 0x44 * (cx - clo)
            kind = struct.unpack_from('<I', rows[cy], c + 8)[0]
            if kind in (9, 0xa) and r.random() < 0.5:
                d = rows[cy][c + 0xd]
                dx, dy = STEPS[d if kind == 0xa else (d << 4 | d >> 4) & 0xff]
                if clo <= cx + dx <= chi and lo <= cy + dy <= hi:
                    n, path = 0x44 * (cx + dx - clo), rows[cy + dy]
                    put(path, n + 8, 'I', 1)
                    put(path, n + 0xe, 'H', struct.unpack_from('<H', path, n + 0xe)[0] | 0x20)
    handles = r.sample(range(1, 12), 4)
    h_obj, h_other, h_money, h_an = handles
    cells, c = rows[click[1]], 0x44 * (click[0] - clo)
    kind = r.choice([4, 4, 9, 0xa, 1, 0])
    put(cells, c + 8, 'I', 4 if kind in (9, 0xa) and (click[0] in (0, 127) or click[1] in (0, 127)) else kind)
    cells[c + 0xd] = r.choice(BITS)
    put(cells, c + 0x10, 'H', cell_id(*parent))
    head = r.choice([h_obj, h_obj, h_other, 0] if r.random() < 0.95 else [0])
    put(rows[parent[1]], 0x44 * (parent[0] - clo) + 0x24, 'H', head)
    for cy in range(lo, hi + 1):
        regions[MAP + 0x294 + cell_id(clo, cy) * 0x44] = rows[cy]
        # (the 0x28-byte entries of the same cells)
        entries[cy] = regions[MAP + 0x1102b0 + cell_id(clo, cy) * 0x28] = bytearray(r.bytes(0x28 * width))
    # (the things: the object, a kind 1 thing before it in the chain, the money record, the park's)
    other = bytearray(r.bytes(THING))
    put(other, 0, 'H', h_other)
    other[2] = r.choice([1, 0, 4, 0xff])
    put(other, 0xa, 'H', h_obj if r.random() < 0.9 else 0)
    put(obj, 0, 'H', h_obj)
    obj[2] = 3
    obj[5], obj[7] = min(126, max(1, ax)), min(126, max(1, ay))
    put(obj, 0x10, 'I', r.choice([0, 0x5a, 0xb4, 0x10e, 0x5a, 0xb4, 0x10e, 0x2d, r.getrandbits(32)]))
    put(obj, 0x28, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(obj, 0x34, 'H', cell_id(ax, ay))
    put(obj, 0x40, 'i', r.choice([0, 1, 1, 5, -1]))
    slots = bytearray(r.bytes(20 * 12))
    for h, p in ((h_obj, OBJ), (h_other, OTHER), (h_money, MONEY), (h_an, AN)):
        put(slots, 20 * h, 'I', p)
    regions[SLOTS] = slots
    regions[POOL] = obj + other
    money = bytearray(r.bytes(0x130))
    put(money, 0x114, 'I', r.choice([0, 1, 1, 1]))
    put(money, 0xc, 'i', r.choice([0, 1, 1000, -5, r.getrandbits(32) - (1 << 31)]))
    put(money, 0x11c, 'i', r.choice([0, -1, 5, r.getrandbits(32) - (1 << 31)]))
    regions[MONEY] = money
    regions[AN + 0x1f5a0] = r.bytes(4)
    regions[MAP + 0x1da720] = struct.pack('<HHHH', h_an, r.getrandbits(16), r.getrandbits(16), h_money)
    # (the definitions: the object's among 1-3, the cache its id now and then)
    n = r.randrange(1, 4)
    ids = r.sample(range(1, 0x10000), n)
    k = r.randrange(n)
    put(obj, 0xe, 'H', ids[k])
    regions[POOL] = obj + other
    dtab = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(ITEM * n))
    for i in range(n):
        put(dtab, 4 * i, 'I', ITEMS + ITEM * i)
        put(items, ITEM * i, 'H', ids[i])
    put(dtab, 0x528, 'i', n)
    put(dtab, 0x730, 'HxxI', ids[k] if r.random() < 0.3 else ids[k] ^ r.randrange(1, 0x10000), ITEMS + ITEM * k)
    d = ITEM * k
    put(items, d + 0x18, 'I', SHAPE)
    put(items, d + 0x40, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    dkind = r.choice([0, 1, 2, 3, 0, 1, 2, 0xffffffff])
    if dkind in (1, 2) and not (5 <= ax <= 122 and 5 <= ay <= 122):
        dkind = r.choice([0, 3])        # (the track runs' cells kept in the window)
    put(items, d + 0x9c, 'I', dkind)
    if dkind in (1, 2):
        track(r, entries, clo, obj, ax, ay)
        regions[POOL] = obj + other
    for col in range(0x188, 0x388, 4):
        put(items, d + col, 'i', r.choice([0, 1, 50, 100, 250, -3, r.getrandbits(20)]))
    regions[DEFS] = dtab
    regions[ITEMS] = items
    cols, rows = r.choice([1, 2, 3, 3, 0, -1]), r.choice([1, 2, 3, 3, 0, -1])
    shape = bytearray(r.bytes(8 + 0xa0 * max(rows, 1)))
    put(shape, 0, 'ii', cols, rows)
    for row in range(max(rows, 0)):
        for col in range(max(cols, 0)):
            put(shape, 8 + 0xa0 * row + 8 * col, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    regions[SHAPE] = shape
    # (the globals: the saved one, the rates, the flags written, the track run's)
    for g in (0x81af64, 0x81ea14, 0x81aebc, 0x81b43c):
        regions[g] = struct.pack('<i', r.choice([0, 1, 10, 50, 100, -2, r.getrandbits(16)]))
    for g in (0x81d7a8, 0x81b46c, 0x820a8c):
        regions[g] = r.bytes(4)
    regions[0x818c50] = r.bytes(4)
    regions[0x7cdba0] = DIRECTIONS
    regions[0x81b740] = r.bytes(0x20)   # (the track runs' points, and their last)
    regions[0x81ede4] = r.bytes(8)
    regions[0x7639e0] = struct.pack('<ii', r.randrange(-1, 129), r.randrange(-1, 129))
    regions[0x81b4cc] = struct.pack('<I', r.choice([0, 1]))
    # (0x52fe50's answers: runs of not 0 ended by a 0)
    seq = []
    for _ in range(3):
        seq += [r.choice([1, 0x100, 0x80000000, r.getrandbits(32) | 1]) for _ in range(r.choice([0, 0, 1, 2, 3]))] + [0]
    regions[TABLE] = struct.pack('<48I', *(seq + [0] * (48 - len(seq))))
    answers(log, 6, TABLE)
    # (sound: on and started or not, the manager's slot 2 the recorder)
    g = bytearray(r.bytes(0x20))
    put(g, 8, 'I', r.choice([0, 1, 1, 1]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1]))
    regions[0x802bc0] = g
    regions[MANAGER] = struct.pack('<I', VT) + r.bytes(0xc)
    regions[VT] = vtable({2: 2}, 4)
    regions[0x803a2c] = r.bytes(4)
    regions[0x785a2c] = struct.pack('<I', r.choice([0, 0, 0, 1]))
    blob(log, 2, 1, 0x7f)
    if r.random() < 0.5:
        set_return(log, 2, r.getrandbits(32))
    # (Thing_Delete's: thingdetach.py's message centre, the delete queue with 0-2 handles, the post's message bytes 0-9)
    centre(r, regions)
    nq = r.choice([0, 0, 1, 2])
    nodes = bytearray(r.bytes(0x18))
    for q in range(nq):
        put(nodes, 0xc * q, 'HHII', r.getrandbits(16), r.getrandbits(16), QNODES + 0xc * (q + 1) if q + 1 < nq else 0,
            QNODES + 0xc * (q - 1) if q else 0)
    regions[QNODES] = nodes
    regions[0x801f38] = struct.pack('<IIII', r.getrandbits(32), QNODES if nq else 0, QNODES + 0xc * (nq - 1) if nq else 0, nq)
    blob_bytes(log, 21, 0, 0x3ff)
    flag = r.choice([0, 0, 1, r.getrandbits(32)])
    return Call(0x527ee0, [x & 0xffffffff, y & 0xffffffff, flag], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
