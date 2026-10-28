# A visitor's state set (0x501db0 Visitor_SetState, src/game/world/person) against the original. The visitor 0x240 bytes, its ring +0x30's
# position 0-31, the reference +0xc 0 or set; the world [0x80239c] (its seed +0x1da708, tick +0x1da70c, handles +0x1da720 / +0x1da736) and
# gWorldPtr [0x7cf83c] the same or another; the thing table 0x7cfb90 with the visitor's object (handle +0x1dc), whose definition (one of
# three in gObjectDefs, the cache hit or not) has +0x100 and +0x15c 0, 1 or any.
#   Plain: the states 0-2, 5, 6, 8-10, 12, 13, 19, 20 and past 0x15 (0x16, 0xff, any); the word 0x75c7f0 any.
#   Rand (3); Stamp (11): the byte +0x1f1 any (times 1.2f). Booth (4): the ticket booths 0x78559c (A, B) at the visitor's cell or not.
#   Walk (7): navpath.py's moving person and destination (its words +0x18 / +0x1a), +0xd0 0 or set.
#   Roll (14): the object's chance +0x190 and ride id +0x24 (the ride list of 1-3 records, the system on or off; element 0xb there or not).
#   Enter (15) and Leave (16): personsprite.py's sprite and particle surroundings (the script list ready, folder 10's sprites with 1-5
#   animations); Enter: +0x214 0 or set, the reference 0 now and then (ScriptRef_Start); Leave: the object's flag +0x32 bit 0x20, the
#   slots object of +0x1da736 (index 0-3 or out of range, on or off, the slot's id one of 0x11 / 0x14 / 0x16 or not, its word the object's
#   +0xe or not, the limit 0, the handle or another).
#   Exit (17), Ring (18): the scripts +0xb4 / +0x210 set or not (the list ready or not), the ring's last pair the one pushed now and then.
#   Days (21): +0x204 0 or set, funnytime.py's clock and day ring, the arrival +0x1d4 about the tick.
# usage: visitorstate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Plain,Rand,Booth,Walk,Stamp,Roll,Enter,Leave,Exit,Ring,Days)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import funnytime  # noqa: E402
import navpath  # noqa: E402
import personsprite  # noqa: E402
from fncmp import RAM, Bench, Call, put  # noqa: E402
from personscripts import PSPRITES  # noqa: E402

RANGES = []
for a, b in sorted([(0x402d70, 0x403220), (0x4033a0, 0x4033f0), (0x412e90, 0x412f00), (0x4217f0, 0x421810), (0x4527f0, 0x452a70),
                    (0x475480, 0x4755a0), (0x4758f0, 0x475c60), (0x4c7600, 0x4c7690), (0x4d27a0, 0x4d2850), (0x4d4140, 0x4d41a0),
                    (0x4d8610, 0x4d8660), (0x4d8c50, 0x4d8c70), (0x4d8ca0, 0x4d8ce0), (0x4dd4e0, 0x4dd510), (0x4de670, 0x4de7f0),
                    (0x4e2670, 0x4e2720), (0x4f8730, 0x4f8780), (0x4f9f00, 0x4fa2b0), (0x4fa470, 0x4fa540), (0x4fa5f0, 0x4fa680),
                    (0x4fa770, 0x4fa880), (0x4fd950, 0x4fd980), (0x4fe900, 0x4fe990), (0x501db0, 0x502440), (0x50b350, 0x50b370),
                    (0x50c0d0, 0x50c1a0), (0x50f8e0, 0x50fd50), (0x510100, 0x510190), (0x516330, 0x516390), (0x519410, 0x519440),
                    (0x519510, 0x519560), (0x519610, 0x519660), (0x520560, 0x520e10), (0x521d60, 0x5222f0), (0x522360, 0x522500),
                    (0x536320, 0x536350), (0x53c8d0, 0x53c9d0), (0x540c60, 0x540da0), (0x541f70, 0x541fb0), (0x541fd0, 0x542020),
                    (0x55a070, 0x55a110), (0x55a0b0, 0x55a100), (0x5f5660, 0x5f56a0), (0x5f5e20, 0x5f5e90), (0x5f5fa0, 0x5f5fc0),
                    (0x67c320, 0x67c3e0), (0x67c7f0, 0x67c840), (0x5da3c0, 0x5da3c1)]
                   + personsprite.RANGES + navpath.RANGES + funnytime.RANGES):
    if RANGES and a <= RANGES[-1][1]:
        RANGES[-1] = (RANGES[-1][0], max(RANGES[-1][1], b))
    else:
        RANGES.append((a, b))
B = Bench(RANGES)
P = personsprite.THING                       # the visitor (RAM + 0x20000)
ITEMS, OBJ, SLOTS_OBJ = RAM + 0x60000, RAM + 0x68000, RAM + 0x6c000
NODES, ARRAYS = RAM + 0x6a000, RAM + 0x6b000
TAIL, TAIL2 = RAM + 0x70000, RAM + 0x70100  # the worlds' +0x1da700-+0x1da740
W, W2 = TAIL - 0x1da700, TAIL2 - 0x1da700
H_OBJ, H_SLOTS, H_AN = 5, 9, 12               # the handles of the object, the slots object and the analyser in 0x7cfb90
CALLS = {'Plain': None, 'Rand': 3, 'Booth': 4, 'Walk': 7, 'Stamp': 11, 'Roll': 14, 'Enter': 15, 'Leave': 16, 'Exit': 17, 'Ring': 18,
         'Days': 21}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def word(r):
    return r.choice([0, 1, 5, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])


def worlds(r, regions):
    """the world [0x80239c] and gWorldPtr [0x7cf83c] (the same one mostly), their tails"""
    for tail in (TAIL, TAIL2):
        t = bytearray(r.bytes(0x40))
        put(t, 0x20, 'H', H_AN)
        put(t, 0x36, 'H', H_SLOTS)
        regions[tail] = t
    regions[0x80239c] = struct.pack('<I', W)
    regions[0x7cf83c] = struct.pack('<I', r.choice([W, W, W, W2]))


def table(r, regions, analyser=0):
    """the thing table: the object, the slots object and the analyser at their handles"""
    t = bytearray(r.bytes(20 * 16))
    for h, at in ((H_OBJ, OBJ), (H_SLOTS, SLOTS_OBJ), (H_AN, analyser)):
        put(t, 20 * h, 'I', at)
    regions[0x7cfb90] = t


def defs(r, regions):
    """three definitions (+0x100, +0x15c 0, 1 or any) in gObjectDefs and the object (0x200 bytes) of one of them; returns the object"""
    items = bytearray(r.bytes(3 * 0x500))
    ids = r.sample(range(1, 0x40), 3)
    for i in range(3):
        put(items, 0x500 * i, 'H', ids[i])
        put(items, 0x500 * i + 0x100, 'I', r.choice([0, 0, 1, word(r)]))
        put(items, 0x500 * i + 0x15c, 'I', r.choice([0, 1, 1, 2, word(r)]))
    dtab = bytearray(r.bytes(0x740))
    put(dtab, 0, 'III', ITEMS, ITEMS + 0x500, ITEMS + 0xa00)
    put(dtab, 0x528, 'i', 3)
    k = r.randrange(3)
    put(dtab, 0x730, 'HxxI', ids[k] if r.random() < 0.4 else 0, ITEMS + 0x500 * k)
    regions[0x7890a0] = dtab
    regions[ITEMS] = items
    obj = bytearray(r.bytes(0x200))
    put(obj, 0xe, 'H', r.choice(ids))
    regions[OBJ] = obj
    return obj


def visitor(r, regions, thing=None):
    """the visitor at P (sprite_env's when given), its ring position, handle +0x1dc the object's (or another now and then)"""
    v = thing if thing is not None else bytearray(r.bytes(0x240))
    put(v, 0x30 + 0x88, 'I', r.randrange(32))
    put(v, 0x1dc, 'H', H_OBJ)
    if thing is None:
        put(v, 0xc, 'I', r.choice([0, 0, 1, 2]))
        regions[0x7b49f0] = bytearray(0x2c)   # (the sprite script list not ready)
    regions[P] = v
    return v


def ready_sprites(r, regions):
    """personsprite.py's sprite surroundings with the script list ready; folder 10's sprites with 1-5 animations"""
    while True:
        regions.clear()
        thing = personsprite.sprite_env(r, regions)
        if regions[0x7b49f0][0x24] == 1:
            break
    sprites = bytearray(regions[PSPRITES])
    for k in range(4):
        put(sprites, 0x240 * k + 0x21e, 'H', r.choice([1, 2, 3, 5]))
    regions[PSPRITES] = sprites
    return thing


def ride_list(r, regions, obj):
    count = r.choice([1, 2, 3])
    ids = r.sample(range(1, 9), count)
    nodes = bytearray(r.bytes(0x100 * count))
    for i, rid in enumerate(ids):
        put(nodes, 0x100 * i, 'I', NODES + 0x100 * (i + 1) if i + 1 < count else 0)
        put(nodes, 0x100 * i + 8, 'I', rid)
        put(nodes, 0x100 * i + 0x1c, 'I', ARRAYS + 0x40 * i)
        put(nodes, 0x100 * i + 0x8c, 'i', r.choice([12, 12, 11, 4, 0x10]))
    regions.update({NODES: nodes, ARRAYS: r.bytes(0x40 * count), 0x8791a0: struct.pack('<I', r.choice([1, 1, 1, 0])),
                    0x8791b0: struct.pack('<I', NODES)})
    put(obj, 0x24, 'I', r.choice(ids + [99]))
    obj[0x190] = r.choice([0, 1, 50, 99, 100, 255, r.getrandbits(8)])


def slots_object(r, regions, obj, v):
    s = bytearray(r.bytes(0x3e4))
    index = r.choice([0, 1, 2, 3, 3, -1, 20])
    put(s, 0x3cc, 'i', index)
    s[0x3d9] = r.choice([1, 1, 1, 0])
    kind = struct.unpack_from('<H', obj, 0xe)[0]
    for i in range(4):
        put(s, 0xc + 48 * i, 'I', r.choice([0x11, 0x14, 0x16, 0x11, 0x14, 0x16, 0x12, word(r)]))
        put(s, 0xc + 48 * i + 0xc, 'H', kind if r.random() < 0.8 else r.getrandbits(16))
    put(s, 0x3e0, 'H', r.choice([0, 0, H_OBJ, r.getrandbits(16)]))
    regions[SLOTS_OBJ] = s


def walk(r):
    x, y = r.getrandbits(16), r.getrandbits(16)
    c = navpath.person_case(r, (x, y))
    regions = dict(c.regions)
    v = bytearray(regions[RAM]) + bytearray(r.bytes(0x40))
    put(v, 0x18, 'HH', x, y)
    put(v, 0xd0, 'I', r.choice([0, 1, 1, 1, word(r)]))
    regions[RAM] = v
    return Call(0x501db0, [RAM, 7], regions, conv='t', ret='v')


def days(r):
    regions = {funnytime.CLOCK: put(bytearray(r.bytes(0x40)), 0x1c, 'I', funnytime.u32(r))}
    funnytime.clock_regions(r, regions)   # ([0x80239c] its world: only the ticks and the handle +0x1da720 are read)
    ticks = bytearray(regions[funnytime.TICKS]) + bytearray(r.bytes(0x10))
    put(ticks, 0x14, 'H', H_AN)
    regions[funnytime.TICKS] = ticks
    regions[0x7cf6ec] = struct.pack('<I', funnytime.CLOCK_AT)
    regions[funnytime.CLOCK_AT] = struct.pack('<I', funnytime.CLOCK - 0x2a0)
    ring = bytearray(r.bytes(0xd4))
    size = r.choice([50, 50, 0, 1, r.randrange(60)])
    put(ring, 0xc8, 'ii', r.choice([size - 1, size, 0, r.randrange(-1, 50)]), size)
    regions[funnytime.RING] = ring
    table(r, regions, funnytime.RING - 0x21604)
    v = visitor(r, regions)
    put(v, 0x204, 'I', r.choice([0, 1, 1, 1, word(r)]))
    tick = struct.unpack_from('<I', ticks, 0)[0]
    put(v, 0x1d4, 'I', r.choice([tick, tick - 4, tick - 4000, tick + 8, word(r)]) & 0xffffffff)
    return Call(0x501db0, [P, 21], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Walk':
        return walk(r)
    if op == 'Days':
        return days(r)
    regions = {}
    thing = None
    if op in ('Enter', 'Leave'):
        thing = ready_sprites(r, regions)
    elif op == 'Exit' and r.random() < 0.6:
        thing = personsprite.sprite_env(r, regions)
    worlds(r, regions)
    obj = defs(r, regions)
    table(r, regions)
    v = visitor(r, regions, thing)
    state = CALLS[op]
    if op == 'Plain':
        state = r.choice([0, 1, 2, 5, 6, 8, 9, 10, 12, 13, 19, 20, 0x16, 0xff, word(r)])
        regions[0x75c7f0] = r.bytes(4)
    elif op == 'Stamp':
        v[0x1f1] = r.choice([0, 1, 5, 10, 100, 255, r.getrandbits(8)])
    elif op == 'Booth':
        col, row = r.randrange(128), r.randrange(128)
        v[5], v[7] = col, row
        here = (col, row)
        a = here if r.random() < 0.4 else (r.randrange(128), r.randrange(128))
        b = here if r.random() < 0.5 else (r.randrange(128), r.randrange(128))
        regions[0x78559c] = struct.pack('<4I', *a, *b)
        regions[0x8023a0] = r.bytes(4)
    elif op == 'Roll':
        ride_list(r, regions, obj)
    elif op in ('Enter', 'Leave'):
        item = struct.unpack_from('<H', obj, 0xe)[0]
        items = regions[ITEMS]
        at = [i for i in range(3) if struct.unpack_from('<H', items, 0x500 * i)[0] == item][0]
        if struct.unpack_from('<I', items, 0x500 * at + 0x100)[0]:
            personsprite.particle_env(r, regions)
        elif op == 'Enter' and r.random() < 0.4:
            put(v, 0xc, 'I', 0)
        if op == 'Enter':
            put(v, 0x214, 'I', r.choice([0, 1, 1, word(r)]))
        else:
            obj[0x32] = r.choice([0x20, 0, 0xff, 0xdf, r.getrandbits(8)])
            slots_object(r, regions, obj, v)
    elif op == 'Ring' or op == 'Exit':
        pos = struct.unpack_from('<I', v, 0xb8)[0]
        if r.random() < 0.3:
            put(v, 0x30 + (pos * 4 if pos else 0x80), 'HH', 0x10 if op == 'Exit' else 0x13, 0)
        if thing is None:
            put(v, 0x210, 'I', r.choice([0, 0, 1, 2]))
    return Call(0x501db0, [P, state], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
