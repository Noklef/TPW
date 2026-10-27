# The object's queue walks of src/game/world/objectmsg (0x4de130 Object_BackOfQueue, 0x4de110 Object_BackOfQueueTo, 0x4ddf40 Object_QueuePositionOfNone) against the originals:
# an object with a set or unset queue back and a world where the neighbour walk ends or runs on (Back, BackTo); objectmsg.py's queue of persons (None: the position of the
# person with no handle); whether the object takes people into its queue (0x4dd920: flags and states either side of each test, a queue back set or walked to, a queue of
# persons shorter or longer than the room, definitions of kinds 0-3 and a coaster link open or not); whether the entrance meets the path (0x4de4a0 Object_EntranceOnPath,
# Path: the back of the queue's cell or the cell ahead of a quarter-turn angle) and whether the object can be opened (0x4df290 Object_CanOpen: Open's states with Path's
# cells). The angles are quarter turns only: another leaves the original's direction bit a stale stack byte (see objectmsg.h). Where a person at a place in the
# queue stands (0x4de840 Object_QueueSpot, Spot: Path's world, places in the first cells or past the end, the world's random state, now and then the one drawing 0x80000000); EntSpot (0x4dec30 Object_EntranceQueueSpot) on Spot's cases.
# AnySpot (0x4de7e0 Object_AnyQueueSpot): Spot's cases, the object's bit 3 of +0x32 set or clear. Close (0x4df390 Object_Close): see close().
# Flag64 (0x4dfe30 Object_SetFlag64): see flag64().
# usage: objectqueue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Back,BackTo,None,Open,Path,CanOpen,Spot,EntSpot,AnySpot,Close,Flag64)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectmsg  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import park  # noqa: E402

B = Bench([(0x4d8300, 0x4d8340), (0x4d8c00, 0x4d8c30), (0x4d96f0, 0x4d99f0), (0x4de4a0, 0x4de5b0), (0x4df290, 0x4df310), (0x4de840, 0x4dede8), (0x516330, 0x516380),
           (0x4ddf40, 0x4de200), (0x4de040, 0x4de100), (0x4de670, 0x4de7e0), (0x4de7e0, 0x4de83e), (0x502430, 0x502470), (0x50b350, 0x50b370),
           (0x522770, 0x5227a0), (0x522850, 0x522870), (0x536300, 0x53644c), (0x5da3c0, 0x5da3c1),
           (0x4dd920, 0x4dda20), (0x412e90, 0x412eea), (0x441970, 0x4419b0), (0x436860, 0x436880),
           (0x4df390, 0x4df450), (0x454550, 0x454800), (0x4e0e60, 0x4e11e4), (0x55a070, 0x55a110), (0x4dfe30, 0x4e004b)])
OUT = RAM + 0x7000
RIDES, STRIP = RAM + 0x24000, RAM + 0x26000   # (Close's)


def back(r):
    obj = bytearray(r.bytes(0x60))
    put(obj, 0x3a, 'H', r.choice([0, 0, 0, 5, 0x81]))
    put(obj, 0x36, 'H', r.choice([1, 2, 3, 0x81, r.randrange(1, 0x100)]))
    cells = bytearray(r.bytes(0x44 * 0x1c0))
    for idx in range(0x1c0):
        put(cells, 0x44 * idx, 'H', min(idx + 1, 0x17f) if r.random() < 0.9 else r.randrange(1, 0x17f))
        put(cells, 0x44 * idx + 8, 'I', r.choice([3, 3, 9, 0, 1, 3, 4, 0xa]))
        cells[0x44 * idx + 0xd] = r.choice([0x10, 1, 0x40, 4, 0x10, 1, 0x40, 4, 0x80, 2])
    world = RAM + 0x10000
    regions = {RAM: obj, world + 0x294: cells, 0x8023a0: struct.pack('<I', world + 0x294), 0x7cf83c: struct.pack('<I', world), 0x80239c: struct.pack('<I', world), OUT: r.bytes(4)}
    return Call(0x4de130, [RAM, OUT], regions, conv='t', ret='a')


def back_to(r):
    c = back(r)
    c.address = 0x4de110
    return c


def none(r):
    c = objectmsg.queue(r, 'Pos')
    c.address, c.args, c.ret = 0x4ddf40, c.args[:1], 'a'
    return c


def open_(r):
    """0x4dd920: the queue of persons (objectmsg's) with the walk world of back(), the definitions and the coaster links."""
    c = objectmsg.queue(r, 'Pos')
    walk = back(r)
    obj = bytearray(c.regions[RAM]) + bytearray(r.bytes(0x100))
    for at, size in ((0x36, 2), (0x3a, 2)):
        obj[at:at + size] = walk.regions[RAM][at:at + size]
    c.regions.update({k: v for k, v in walk.regions.items() if k != RAM})
    if r.random() < 0.3:  # rows facing each other in turn: the walk goes round two cells until its budget runs out (the back is then 0)
        cells = next(v for k, v in walk.regions.items() if len(v) == 0x44 * 0x1c0)
        for idx in range(0x1c0):
            put(cells, 0x44 * idx + 8, 'I', 3)
            cells[0x44 * idx + 0xd] = (0x10, 1)[idx // 128 & 1]
    put(obj, 0x32, 'H', r.choice([4, 0xc, 0xfffc, 0, 8, 0xfffb]))
    put(obj, 0x19c, 'I', r.choice([0, 2, 3, 3, 3, 1, 4, 5]))
    put(obj, 0x68, 'I', r.choice([1, 1, 1, 0, 0x80000000]))
    put(obj, 0x40, 'I', r.choice([0, 1, 2, 0x40000000, 0x3fffffff]))
    put(obj, 0x2c, 'I', r.choice([0, 1, 0x100]))
    defs, _h, _a = park(r, count=0, defs=True)
    items = bytearray(defs[0x30008000])
    for k in range(len(items) // 0x500):
        put(items, 0x500 * k + 0x9c, 'I', r.choice([0, 1, 2, 3, 3, 4]))
    c.regions.update({k: v for k, v in defs.items() if k in (0x7890a0, 0x30008000)})
    c.regions[0x30008000] = items
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, 0x500 * r.randrange(len(items) // 0x500))[0])
    links = bytearray(r.bytes(0x200 * 3))
    ids = [1, 2, 3]
    for i in range(3):
        put(links, 0x200 * i + 0x10, 'I', ids[i])
        put(links, 0x200 * i + 0x18, 'I', RAM + 0x20000 + 0x200 * (i + 1) if i < 2 else 0)
        put(links, 0x200 * i + 0x140, 'I', r.choice([0, 0, 1]))
        put(links, 0x200 * i + 0x3c, 'I', r.choice([1, 1, 3, 0, 5]))
    put(obj, 0x20, 'I', r.choice(ids + [9]))
    c.regions.update({RAM + 0x20000: links, 0x790fe0: struct.pack('<I', RAM + 0x20000 if r.random() < 0.9 else 0),
                      0x790fec: struct.pack('<I', r.choice([0, RAM + 0x20000, RAM + 0x20200]))})
    c.regions[RAM] = obj
    c.address, c.args, c.ret = 0x4dd920, c.args[:1], 'a'
    return c


def entrance(r, c):
    """The fields 0x4de4a0 reads, on a case with back()'s world: flag 8 of +0x32 or not, a quarter-turn angle, the object's cell in rows 1-2 (a step stays in the
    cells; x 0 or 0x7f can step off the map: null, read in both), the cells mostly of type 1, a direction equal to the neighbour bits or not."""
    obj = c.regions[RAM]
    obj[0x32] = r.choice([8, 0xc, 0xff, 0, 4, 0xf7])
    put(obj, 0x10, 'i', r.choice([0, 0x5a, 0xb4, 0x10e]))
    put(obj, 0x36, 'H', r.randrange(0x81, 0x181))
    cells = next(v for k, v in c.regions.items() if len(v) == 0x44 * 0x1c0)
    for idx in range(0x1c0):
        put(cells, 0x44 * idx + 8, 'I', r.choice([1, 1, 1, 3, 0, 9]))
        if r.random() < 0.3:
            cells[0x44 * idx + 0xd] = cells[0x44 * idx + 0xc]
    c.regions[0x81b4cc] = bytes([r.choice([0, 0, 0, 1])])
    return c


def path(r):
    c = back(r)
    c.regions[RAM] = bytearray(c.regions[RAM])
    c = entrance(r, c)
    c.address, c.args = 0x4de4a0, c.args[:1]
    return c


def can_open(r):
    """0x4df290: open_()'s states, definitions and links with entrance()'s fields and +0x64 set or not."""
    c = entrance(r, open_(r))
    put(c.regions[RAM], 0x64, 'I', r.choice([0, 0, 0, 1, 0x80000000]))
    c.address = 0x4df290
    return c


def spot(r):
    """0x4de840: back()'s world with entrance()'s cells, a place in the first cells of the queue or far beyond its end, the world's random state."""
    c = path(r)
    world = RAM + 0x10000
    # (0x2b625fe5: the one state whose next draw is 0x80000000, the draw that stays negative: % 0x1c then takes it unsigned)
    c.regions[world + 0x1da708] = struct.pack('<I', 0x2b625fe5) if r.random() < 0.15 else r.bytes(4)
    place =r.choice([0, 1, 2, 3, 3, 3, 4, 6, 7, 8, 11, 15, 0x23, 0x200, 0x200, 0x201, 0x80000003, r.getrandbits(32)])
    if r.random() < 0.3:  # rows facing each other in turn (as Open's): the walk goes on, so a far place (512: along exactly 0x80) still ends on a cell
        cells = next(v for k, v in c.regions.items() if len(v) == 0x44 * 0x1c0)
        for idx in range(0x1c0):
            put(cells, 0x44 * idx + 8, 'I', 3)
            cells[0x44 * idx + 0xd] = (0x10, 1)[idx // 128 & 1]
        place = r.choice([0, 3, 4, 0x23, 0x200, 0x200, 0x201, 0x3ff])  # (a walk of a huge place never ends in the emulator's step budget)
    c.address, c.args, c.ret = 0x4de840, [RAM, 0x800, OUT, OUT + 2, OUT + 3], 'a'
    if place > 0x800:   # (the random cells' facing walk can go round a ring too: probed in the emulator with 0x200 steps, more than the cells,
        probe = Call(c.address, c.args, dict(c.regions), conv=c.conv, ret='a')   # a cell still under way means a ring, and a far place one
        probe.regions.update({k: v for k, v in B.extra.items() if k not in probe.regions})   # the emulator cannot walk: a near one then)
        out = B.original(probe)
        if not isinstance(out, str) and struct.unpack('<H', bytes.fromhex(out[3 + list(probe.regions).index(OUT)])[:2])[0]:
            place = r.choice([0x200, 0x201, 0x3ff])
    c.args[1] = place
    return c


def entrance_spot(r):
    """0x4dec30: Spot's cases (its places, world and random state)."""
    c = spot(r)
    c.address = 0x4dec30
    return c


def any_spot(r):
    """0x4de7e0: Spot's cases, the object's +0x32 bit 3 either way."""
    c = spot(r)
    c.address = 0x4de7e0
    obj = bytearray(c.regions[RAM])
    obj[0x32] = obj[0x32] & ~8 | r.choice([0, 8])
    c.regions[RAM] = obj
    return c


def close(r):
    """0x4df390: CanOpen's cases, then the strip object of 0x7a4610[+0x20] (its strips +0xb0 set or not, its flags random) and the ride
    record of the id +0x24 (the rides' system on mostly, one record of that id or another, its element count 0-8 or -1: element 6 there
    or not)."""
    c = can_open(r)
    obj = c.regions[RAM]
    if r.random() < 0.3:   # (else the coaster link's id, 1-3 or 9)
        put(obj, 0x20, 'I', r.randrange(10))
    k = struct.unpack_from('<I', obj, 0x20)[0]
    table = bytearray(r.bytes(0x28))
    put(table, 4 * k, 'I', STRIP)
    c.regions[0x7a4610] = table
    c.regions[STRIP] = put(bytearray(r.bytes(0xc0)), 0xb0, 'I', r.choice([0, r.getrandbits(32) | 1]))
    rid = r.choice([1, 2, 7])
    node = put(put(put(put(bytearray(r.bytes(0x100)), 0, 'I', 0), 8, 'I', rid), 0x1c, 'I', RIDES + 0x100), 0x8c, 'i', r.choice([7, 7, 8, 6, 0, -1]))
    put(obj, 0x24, 'I', r.choice([rid, rid, rid, 3]))
    c.regions.update({RIDES: node, RIDES + 0x100: r.bytes(0x20), 0x8791a0: struct.pack('<I', r.choice([0, 1, 1, 1])),
                      0x8791b0: struct.pack('<I', RIDES)})
    c.address, c.ret = 0x4df390, 'v'
    return c


def flag64(r):
    """0x4dfe30: close()'s world, +0x64 0, 1 or any, the argument 0, 1 or any"""
    c = close(r)
    put(c.regions[RAM], 0x64, 'I', r.choice([0, 0, 1, 1, r.getrandbits(32)]))
    c.address, c.ret, c.args = 0x4dfe30, 'a', c.args[:1] + [r.choice([0, 0, 1, 1, r.getrandbits(32)])]
    return c


CASES = {'Back': back, 'BackTo': back_to, 'None': none, 'Open': open_, 'Path': path, 'CanOpen': can_open, 'Spot': spot, 'EntSpot': entrance_spot, 'AnySpot': any_spot,
         'Close': close, 'Flag64': flag64}


def case(r):
    return CASES[r.choice(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
