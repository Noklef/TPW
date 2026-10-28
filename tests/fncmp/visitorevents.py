# A visitor's arrival and its message (src/game/world/person: 0x4feb50 Visitor_Arrive, 0x4fb360 Visitor_HandleMessage) against the
# originals. 0x501db0 (Visitor_SetState, benched by visitorstate.py) is the thiscall recorder 26 (driver.cpp's kPersonEnv), the post
# 0x40fb10 the recorder 21 (the objects' env; the message's bytes 0-9 logged). The sounds on or off (soundplay.py's globals, the manager's
# slot 2 a recorder of the command's 7 dwords), the sprite script list not ready.
#   Arrive: navpath.py's moving person (objectblocks.py's cells and block values, the limit +0x198 about the counter), arriving at its own
#   cell mostly (else a neighbour, or column & 0x7f for one past 127: its old cell's chain unlinked and the new one's head replaced, the
#   handle +0 and the links +8 / +0xa among the table 0x7cfb90's 16 slots); the crossing's park side 0x7855cc (column A / row / column B)
#   that cell mostly (so the route is that one cell), else a neighbour or any; the world's seed; the kind +2 one Thing_Delete sends nowhere
#   (0, 2, 0x13), the delete queue 0x801f38 with 0-1 handles and the heap.
#   Message: the message's type (its vtable's slot 0 a recorder, k 19) 0xa mostly, its word +8 the handle +0x1dc mostly (and the words
#   +0x1de, +0x1e0-+0x1e6 it now and then); the state +0x220 0x10, 11-14 or others; the reference +0xc 0 or set; the ring +0x30's position
#   0-31 (its last pair the one pushed now and then); happiness +0x19c about 0-100, the bytes 0x785058; the handle +0's low 3 bits 0 or not.
# usage: visitorevents.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Arrive,Message)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import navpath  # noqa: E402
import objectblocks  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, blob_bytes, fb, global_heap, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
from wctimage import merged  # noqa: E402

B = Bench(merged([
    (0x4feb50, 0x4fec10), (0x4fb360, 0x4fb4e3), (0x4fb4f0, 0x4fb560), (0x4fe980, 0x4fe9bb), (0x502430, 0x502454), (0x5012f0, 0x50138d),
    (0x4217f0, 0x421810), (0x4d4140, 0x4d41a0), (0x4d86d0, 0x4d8710), (0x516330, 0x516390), (0x4fa530, 0x4fa600),
    (0x475480, 0x4755a0), (0x4758f0, 0x475c60), (0x4d27a0, 0x4d2850), (0x4fa470, 0x4fa540), (0x412e90, 0x412f00), (0x4033a0, 0x4033f0),
    (0x402d70, 0x403220), (0x4fa930, 0x4fa9a0), (0x50b6a0, 0x50b790), (0x5100e0, 0x510110), (0x510160, 0x510190), (0x4d91f0, 0x4d93c0),
    # (visitorleave.py's: the ring, the sound, Visitor_Lower19c)
    (0x4dd4e0, 0x4dd510), (0x4ddd20, 0x4ddf50), (0x4e0a70, 0x4e0ab0), (0x4e2670, 0x4e2720), (0x4faa00, 0x4faa70), (0x4754e0, 0x475560),
    (0x4fea70, 0x4feb20), (0x50b350, 0x50b370), (0x50c0d0, 0x50c1a0), (0x519610, 0x519660), (0x51bfc0, 0x51c060), (0x55a070, 0x55a110),
    (0x55a390, 0x55a3f0), (0x5da3c0, 0x5da3c1),
    # (thingdetach.py's: Thing_Delete with no per-kind call, the post, the delete queue)
    (0x50b780, 0x50b917), (0x415e00, 0x415e80), (0x40f630, 0x40f650), (0x40f900, 0x40f91b), (0x516310, 0x516322), (0x51ad40, 0x51ad90),
    (0x409250, 0x409260), (0x5f5660, 0x5f56a0), (0x4d4170, 0x4d4186), (0x475550, 0x47558b)] + navpath.RANGES),
    hooks={0x501db0: _recorder(26, 1), 0x40fb10: _recorder(21, 1)})
CALLS = {'Arrive': 0x4feb50, 'Message': 0x4fb360}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
VISITOR, MANAGER, VT = RAM + 0x40000, RAM + 0x41000, RAM + 0x41100
MSG, THINGS, QNODES = RAM + 0x42000, RAM + 0x43000, RAM + 0x44000


def word(r):
    return r.choice([0, 1, 5, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])


def sound(r, regions, log):
    """visitorleave.py's: the sounds on or off, the manager's slot 2 a recorder of the command's 7 dwords, the category [0x803a24]."""
    g = bytearray(r.bytes(0x20))   # 0x802bc0.. (soundplay.py's)
    put(g, 8, 'I', r.choice([0, 1, 1, 1]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1]))
    blob(log, 2, 1, 0x7f)
    if r.random() < 0.5:
        set_return(log, 2, r.getrandbits(32))
    regions.update({0x802bc0: g, MANAGER: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({2: 2}, 4), 0x803a24: r.bytes(4)})
    script = bytearray(r.bytes(0x2c))   # (the sprite script list not ready)
    put(script, 0, 'Ii', 0, 0)
    put(script, 0x24, 'I', 0)
    regions[0x7b49f0] = script


def patch(regions, address, fmt, *values):
    """Writes into the region holding address (objectblocks.py's rows of cells)."""
    for k, v in regions.items():
        if k <= address < k + len(v):
            regions[k] = put(bytearray(v), address - k, fmt, *values)
            return
    raise AssertionError(hex(address))


def cell_at(column, row):
    return objectblocks.WORLD + 0x294 + (((row << 7) + column + 1) & 0xffff) * 0x44


def arrive(r):
    while True:   # (a row past 127 makes the walk's words negative: a route search over no map)
        c = navpath.person_case(r)
        if c.regions[RAM][7] <= 0x7f:
            break
    regions = dict(c.regions)
    v = bytearray(regions[RAM])
    v += bytearray(r.bytes(max(0, 0x240 - len(v))))
    col, row = v[5], v[7]
    if col > 0x7f:
        new = col & 0x7f   # (the row before's cells of objectblocks.py hold it)
    elif r.random() < 0.85:
        new = col
    else:
        new = col + 1 if col < 0x7f else col - 1
    if new != col:   # the chains: the table's 16 slots all things (the visitor at its handle)
        h = r.randrange(1, 16)
        put(v, 0, 'H', h)
        put(v, 8, 'HH', r.choice([0, 0, r.randrange(16)]), r.choice([0, r.randrange(16)]))
        table = bytearray(r.bytes(20 * 16))
        for k in range(16):
            put(table, 20 * k, 'I', RAM if k == h else THINGS + 0x10 * k)
        regions[0x7cfb90] = table
        regions[THINGS] = r.bytes(0x100)
        patch(regions, cell_at(col, row) + 0x24, 'H', r.choice([h, r.randrange(16)]))
        patch(regions, cell_at(new, row) + 0x24, 'H', r.choice([0, 0, r.randrange(16)]))
    cell = (row << 7) + new + 1
    arg = cell | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)
    dest = [new, row]
    if r.random() < 0.03:   # (another cell: a route search over no map, which faults in both)
        dest = [r.choice([new + 1, new - 1, r.randrange(128)]) & 0x7f, r.choice([row, r.randrange(128)])]
    other = new if r.random() < 0.9 else r.randrange(128)
    regions[0x7855cc] = struct.pack('<III', dest[0], dest[1], other) if r.random() < 0.5 else struct.pack('<III', other, dest[1], dest[0])
    v[2] = r.choice([0, 2, 0x13])
    regions[RAM] = v
    regions[objectblocks.MAP + 0x1da708] = r.bytes(8)
    log = log_region()
    blob_bytes(log, 21, 0, 0x3ff)
    sound(r, regions, log)
    regions[LOG] = log
    n = r.choice([0, 0, 1])
    regions[QNODES] = put(bytearray(r.bytes(0x18)), 0, 'HHII', r.randrange(16), r.getrandbits(16), 0, 0)
    regions[0x801f38] = struct.pack('<IIII', r.getrandbits(32), QNODES if n else 0, QNODES if n else 0, n)
    regions[0x788d3c] = struct.pack('<I', RAM + 0x45000)
    regions.update({HEAP: heap(), **global_heap()})
    return Call(0x4feb50, [RAM, arg], regions, conv='t', ret='v')


def message(r):
    log = log_region()
    regions = {}
    sound(r, regions, log)
    v = bytearray(r.bytes(0x240))
    put(v, 0, 'H', r.choice([8, 16, 0x18, 1, 3, 5, r.getrandbits(16)]))
    h = r.choice([1, 13, r.getrandbits(16)])
    put(v, 0x1dc, 'H', h)
    state = r.choice([0x10] * 4 + [11, 12, 13, 14] * 2 + [0, 6, 0xf, 0x15, word(r)])
    put(v, 0x220, 'I', state)
    pos = r.randrange(32)
    put(v, 0x30 + 0x88, 'I', pos)
    if r.random() < 0.3:   # (the last pair the one pushed: (0xd, the handle) in state 0x10, (6, the handle) when sent away)
        put(v, 0x30 + (pos * 4 if pos else 0x80), 'HH', 0xd if state == 0x10 else 6, h)
    put(v, 0x19c, 'I', fb(r.choice([0.0, 1.0, 50.0, 100.0, r.uniform(-10, 110)])))
    put(v, 0xc, 'I', r.choice([0, 1, 5]))
    said = h if r.random() < 0.8 else r.choice([0, r.getrandbits(16)])
    for off in (0x1de, 0x1e0, 0x1e2, 0x1e4, 0x1e6):
        if r.random() < 0.3:
            put(v, off, 'H', said)
    kind = r.choice([0xa] * 6 + [9, 0xc, 0, r.getrandbits(32)])
    regions[MSG + 0x40] = struct.pack('<I', recorder(19, 0))
    set_return(log, 19, kind)
    regions[MSG] = put(put(bytearray(r.bytes(0x10)), 0, 'I', MSG + 0x40), 8, 'H', said)
    regions[VISITOR] = v
    regions[0x785058] = bytes([r.choice([0, 1, 5, 10, 255])]) + r.bytes(3)
    regions[LOG] = log
    return Call(0x4fb360, [VISITOR, MSG], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    return arrive(r) if op == 'Arrive' else message(r)


if __name__ == '__main__':
    B.main(case)
