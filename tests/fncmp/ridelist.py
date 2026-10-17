# The ride list accessors (src/game/world/ridelist) against the originals: the system flag set or not, an empty list or up to four records
# (ids, link ids, arrays of up to four dwords), records and indexes in and outside the list and the arrays.
# usage: ridelist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Find,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from thingenv import POOL, THING, park  # noqa: E402

# name: (address, size, arguments: id, rec, rec+index, rec+index+value, recword, linked)
CALLS = {'Find': (0x55a070, 0x40, 'id'), 'Set': (0x55a0b0, 0x50, 'rec2v'), 'Get': (0x55a390, 0x50, 'rec2'), 'SetW': (0x55a300, 0x30, 'recv'),
         'GetW': (0x55a330, 0x30, 'rec'), 'G9c': (0x55a360, 0x30, 'rec'), 'Link': (0x55a3e0, 0x90, 'id2'), 'Clr4': (0x4e03f0, 0x30, 'this'), 'E7': (0x4e0410, 0x30, 'this'), 'E0': (0x4e0a70, 0x40, 'this'),
         'Spd': (0x4dd6e0, 0x40, 'thisv'), 'Dur': (0x4dd720, 0xd0, 'thisv'), 'GameE': (0x51a290, 0x60, 'this'), 'E41': (0x4e0db0, 0x30, 'this'), 'Fill': (0x4396e0, 0xa0, 'this'), 'Roll': (0x4e2670, 0xa0, 'thisv'), 'SetId': (0x436f60, 0x40, 'this'), 'RideId': (0x4685c0, 0x40, 'this'), 'Take': (0x51a690, 0xd0, 'this'),
         'Clr0': (0x4e0ac0, 0xd0, 'thisv')}
B = Bench([(a, a + n) for (a, n, _s) in CALLS.values()] + [(0x5da3c0, 0x5da3c1), (0x412e90, 0x412eea), (0x516330, 0x516370), (0x50b350, 0x50b360)])
NODES, ARRAYS = RAM + 0x1000, RAM + 0x4000


def case(r):
    name = r.choice(OPS)
    address, _n, shape = CALLS[name]
    count = r.choice([0, 1, 2, 3, 4])
    ids = r.sample(range(1, 9), count)
    nodes = bytearray(r.bytes(0x100 * 4))
    arrays = bytearray(r.bytes(0x40 * 4))
    for i, rid in enumerate(ids):
        o = 0x100 * i
        put(nodes, o, 'I', NODES + 0x100 * (i + 1) if i + 1 < count else 0)
        put(nodes, o + 8, 'I', rid)
        put(nodes, o + 0x14, 'I', r.choice(ids + [99]))
        put(nodes, o + 0x1c, 'I', ARRAYS + 0x40 * i)
        put(nodes, o + 0x8c, 'i', r.choice([0, 1, 2, 4, -1]))
    regions = {NODES: nodes, ARRAYS: arrays, 0x8791a0: struct.pack('<I', r.choice([0, 1, 1, 1])),
               0x8791b0: struct.pack('<I', NODES if count else 0)}
    rec = r.choice([0] + [NODES + 0x100 * i for i in range(count)]) if count else 0
    ident = r.choice(ids + [0, 99, 5]) if ids else r.choice([0, 1, 5])
    index = r.choice([0, 1, 2, 3, 4, 5, -1, 0x7fffffff, 0x80000000])
    value = r.choice([0, 1, 0x7fff, 0x8000, 0xffff, 0x12345678, 0xffffffff])
    obj = bytearray(r.bytes(0x60))
    put(obj, 0x24, 'I', ident)
    put(obj, 0x3c, 'H', r.choice([0, 1, 0x7fff, r.getrandbits(16)]))
    regions[RAM] = obj
    if name == 'Dur':  # the object definitions the limits come from, the object one of them
        defs, _h, _a = park(r, count=0, defs=True)
        regions.update({k: v for k, v in defs.items() if k in (0x7890a0, 0x30008000)})
        items = defs[0x30008000]
        put(obj, 0xe, 'H', struct.unpack_from('<H', items, 0x500 * r.randrange(16))[0])
    if name == 'Take':  # the game object: the handle at +0x1da72a, things that name ride ids; element 1 often 6
        for i in range(count):
            put(arrays, 0x40 * i + 4, 'I', r.choice([6, 6, 6, 0, 1, 2, 0xffffffff]))
        things, handles, _a = park(r, defs=False)
        for i in range(len(handles)):
            put(things[POOL], THING * i + 0x24, 'I', r.choice(ids + [0, 99]))
        regions.update(things)
        game = RAM + 0x40000
        regions[game] = struct.pack('<H', r.choice(handles + [0, 0]) if handles else 0)
        return Call(address, [game - 0x1da72a], regions, conv='t', ret='a')
    if name == 'GameE':  # the game object: the handle at +0x1da732, a thing table where the things name ride ids
        things, handles, _a = park(r, defs=False)
        for i in range(len(handles)):
            put(things[POOL], THING * i + 0x24, 'I', r.choice(ids + [0, 99]))
        regions.update(things)
        game = RAM + 0x40000
        regions[game] = struct.pack('<H', r.choice(handles + [0, 0]) if handles else 0)
        return Call(address, [game - 0x1da732], regions, conv='t', ret='a')
    if name == 'Fill':  # the slots: counts around the 0 test of each element
        obj.extend(bytes(0x400))
        count = r.choice([0, 1, 2, 3])
        put(obj, 0xf4, 'I', count)
        put(obj, 0x128, 'I', RAM + 0x800)
        slots = bytearray(r.bytes(0x80 * 4 + 0x60))
        for i in range(count):
            for off in (0x54, 0x58, 0x5c, 0x60):
                if r.random() < 0.5:
                    put(slots, 0x80 * i + off, 'I', 0)
        regions[RAM + 0x800] = slots
        regions[RAM] = obj
        return Call(address, [RAM if r.random() < 0.9 else 0], regions, conv='c', ret='v')
    if name in ('SetId', 'RideId'):  # a list of up to three objects with ids, each with slots for the fill
        n = r.choice([0, 1, 2, 3])
        objs = bytearray(r.bytes(0x300 * 3))
        slots = bytearray(r.bytes(0x400))
        wanted = r.choice([1, 2, 3, 9])
        for i in range(n):
            o = 0x300 * i
            put(objs, o + 0x10, 'I', r.choice([1, 2, 3]))
            put(objs, o + 0x18, 'I', RAM + 0x1000 + 0x300 * (i + 1) if i + 1 < n else 0)
            put(objs, o + 0xf4, 'I', r.choice([0, 1, 2]))
            put(objs, o + 0x128, 'I', RAM + 0x800 + 0x100 * i)
        regions.update({RAM + 0x1000: objs, RAM + 0x800: slots, 0x790fe0: struct.pack('<I', RAM + 0x1000 if n else 0)})
        if name == 'RideId':   # the ride table: records with the flag 0x40000 set or clear
            rides = bytearray(r.bytes(0x100 * 4))
            for i in range(4):
                put(rides, 0x100 * i + 4, 'I', (r.getrandbits(32) & ~0x40000) | (0x40000 if r.random() < 0.6 else 0))
            regions[RAM + 0x2000] = rides
            regions[0x7a4610] = b''.join(struct.pack('<I', RAM + 0x2000 + 0x100 * k) for k in range(4))
            return Call(address, [r.choice([0, 1, 2, 3, 3]) if r.random() < 0.9 else wanted, r.choice([0, 1, 2, 3, 5, 99, ident])], regions, conv='c', ret='v')
        return Call(address, [wanted, r.choice([0, 1, 2, 3, 5, 99, ident])], regions, conv='c', ret='v')
    if name == 'Clr0':  # the handle element 0 of the record holds (or another), the word at +0x6c to clear
        obj.extend(r.bytes(0x20))
        held = r.getrandbits(32)
        if ident in ids:  # element 0 of the object's record: often a small value
            at = 0x40 * ids.index(ident)
            if r.random() < 0.5:
                put(arrays, at, 'I', r.choice([0, 1, 0xffff, 0x10000, r.getrandbits(16)]))
            held = struct.unpack_from('<I', arrays, at)[0]
        value = r.choice([held, held, held & 0xffff, held | 0x10000, held ^ 1, r.getrandbits(32)])
        put(obj, 0x6c, 'H', r.choice([value & 0xffff, r.getrandbits(16)]))
        regions[RAM] = obj
        return Call(address, [RAM, value], regions, conv='t', ret='v')
    if name == 'Roll':  # the object whose ride record gets the roll, the world's random state
        obj.extend(bytes(0x400))
        put(obj, 0x190, 'B', r.choice([0, 1, 50, 99, 100, 101, 255]))
        defs, _h, _a = park(r, count=0, defs=True)
        regions.update({k: v for k, v in defs.items() if k in (0x7890a0, 0x30008000)})
        put(obj, 0xe, 'H', struct.unpack_from('<H', defs[0x30008000], 0x500 * r.randrange(16))[0])
        world = RAM + 0x40000
        regions[0x7cf83c] = struct.pack('<I', world - 0x1da708)
        regions[world] = r.bytes(4)
        return Call(address, [RAM, r.getrandbits(32)], regions, conv='t', ret='a')
    args = {'this': [RAM], 'id': [ident], 'rec2v': [rec, index, value], 'rec2': [rec, index], 'recv': [rec, value], 'rec': [rec], 'id2': [ident, index], 'thisv': [RAM, r.choice([0, 1, 5, 99, 0x100, 0x1ff, 0x80, 0xffffffff])]}[shape]
    return Call(address, args, regions, conv='t' if shape.startswith('this') else 'c', ret='v' if name in ('Set', 'SetW', 'Clr4', 'Spd', 'Dur', 'E41') else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
