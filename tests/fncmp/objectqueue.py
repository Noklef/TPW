# The object's queue walks of src/game/world/objectmsg (0x4de130 Object_BackOfQueue, 0x4de110 Object_BackOfQueueTo, 0x4ddf40 Object_QueuePositionOfNone) against the originals:
# an object with a set or unset queue back and a world where the neighbour walk ends or runs on (Back, BackTo); objectmsg.py's queue of persons (None: the position of the
# person with no handle); whether the object takes people into its queue (0x4dd920: flags and states either side of each test, a queue back set or walked to, a queue of
# persons shorter or longer than the room, definitions of kinds 0-3 and a coaster link open or not).
# usage: objectqueue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Back,BackTo,None,Open)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectmsg  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import park  # noqa: E402

B = Bench([(0x4d8300, 0x4d8340), (0x4d96f0, 0x4d97f0), (0x4d99c0, 0x4d99f0), (0x4ddf40, 0x4de200), (0x4de040, 0x4de100), (0x4de670, 0x4de7f0), (0x502430, 0x502470), (0x50b350, 0x50b370),
           (0x522770, 0x5227a0), (0x522850, 0x522870), (0x536300, 0x53644c), (0x5da3c0, 0x5da3c1),
           (0x4dd920, 0x4dda20), (0x412e90, 0x412eea), (0x441970, 0x4419b0), (0x436860, 0x436880)])
OUT = RAM + 0x7000


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


CASES = {'Back': back, 'BackTo': back_to, 'None': none, 'Open': open_}


def case(r):
    return CASES[r.choice(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
