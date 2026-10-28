# The park opened or closed (src/game/world/parkopen: Game_OpenPark 0x519ef0) against the original, over objectqueue.py's CanOpen world: its
# object (RAM) one of 1-4 objects on the game object's list (the word +0x1da746, linked by +0xc through the thing table 0x7cfb90, the
# objects in the slots 12-15: the bit 2 of +0x32 set or not, the others' states 1, 2 or 4); the game object's flag +0x1da710 0, 1 or
# any, the gate (the word +0x1da732: slot 17's thing whose +0x24 names a ride record, or 0 now and then: slot 0's thing, another such
# block, read but for a close) and the
# ride list (0x8791b0, loaded or not at 0x8791a0: the gate's and the objects' records, their counts 0-8, element 1 1 mostly); the open and
# gateOnly arguments 0, 1 or any. The park's persons (objectmsg's queue, kind 1) stand in the walk world's rows 0-2 (cells of types 1, 3,
# 9, 0 or 0xa: plain or not) for Things_CountKind1OnPlainCell. RestartRide's strip objects ([0x7a4610 + 4 * +0x20]) have no strips (+0xb0
# 0); the message centre [0x788d3c] is made. Agent B's unported 0x40fb10 is thiscall recorder 21 (the message's 12 bytes logged; driver.cpp's
# kObjectEnv), agent C's unported 0x4df390 thiscall recorder 13 of none (driver.cpp's kParkOpenEnv).
# usage: parkopen.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectqueue  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put  # noqa: E402
from thingenv import POOL, THING  # noqa: E402


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged([(0x4d8300, 0x4d8340), (0x4d8c00, 0x4d8c30), (0x4d96f0, 0x4d99f0), (0x4de4a0, 0x4de5b0), (0x4df290, 0x4df310), (0x4de840, 0x4dede8),
                  (0x516330, 0x516380), (0x4ddf40, 0x4de200), (0x4de040, 0x4de100), (0x4de670, 0x4de7e0), (0x4de7e0, 0x4de83e), (0x502430, 0x502470),
                  (0x50b350, 0x50b370), (0x522770, 0x5227a0), (0x522850, 0x522870), (0x536300, 0x53644c), (0x4dd920, 0x4dda20), (0x412e90, 0x412f00),
                  (0x441970, 0x4419c0), (0x436860, 0x436890),   # (objectqueue.py's)
                  (0x40f380, 0x40f3b0), (0x40f3d0, 0x40f400), (0x40f420, 0x40f450), (0x40f630, 0x40f660), (0x40ff80, 0x4104c0), (0x410610, 0x410730),
                  (0x410ac0, 0x410b30), (0x454550, 0x454680), (0x4547c0, 0x454800), (0x4a8bd0, 0x4a8c00), (0x4c9130, 0x4c91d0), (0x4cd300, 0x4cd350),
                  (0x4d97e0, 0x4d99d0), (0x4de130, 0x4de200), (0x4df300, 0x4df390), (0x4fa990, 0x4faa10), (0x516130, 0x5161c0), (0x519ef0, 0x51a290),
                  (0x536310, 0x536330), (0x55a070, 0x55a110), (0x55a390, 0x55a3f0), (0x67acf0, 0x67ada0), (0x5da3c0, 0x5da3c1)]),
          hooks={0x40fb10: _recorder(21, 1), 0x4df390: _recorder(13, 0)})
OBJS, GATE, RIDES, ELEMS, STRIP, CENTRE, GATE0 = (RAM + 0x40000, RAM + 0x41000, RAM + 0x42000, RAM + 0x43000, RAM + 0x44000, RAM + 0x45000,
                                                 RAM + 0x46000)
GAME = RAM + 0x50000 - 0x1da700   # (the game object: its fields +0x1da700-+0x1da760 at RAM + 0x50000)


def case(r):
    c = objectqueue.can_open(r)
    regions = c.regions
    obj = bytearray(regions[RAM])
    pool = bytearray(regions[POOL])
    for i in range(len(pool) // THING):   # (the persons in rows 0-2 of the walk world's cells)
        pool[THING * i + 7] = r.randrange(3)
    regions[POOL] = pool
    if r.random() < 0.6:   # (no things on the list the count walks)
        regions[0x7cf56c] = bytes(4)
    cells =next(v for k, v in regions.items() if len(v) == 0x44 * 0x1c0)
    for idx in range(0x1c0):
        if r.random() < 0.5:
            put(cells, 0x44 * idx + 8, 'I', r.choice([1, 3, 9, 0, 0xa, 2, 4, 5]))
    slots = bytearray(regions[0x7cfb90]) + bytearray(r.bytes(20 * 6))
    n = r.randrange(1, 5)
    order = r.sample(range(n), n)   # (position k of the list: object order[k]; object 0 the CanOpen one)
    handles = [12 + j for j in range(n)]
    others = bytearray(r.bytes(0x400 * 3))
    addrs = [RAM] + [OBJS + 0x400 * j for j in range(n - 1)]
    rides = [r.choice([1, 2, 3, 7]) for _ in range(n)]
    for k, i in enumerate(order):
        nxt = handles[order[k + 1]] if k + 1 < n else 0
        if i == 0:
            put(obj, 0xc, 'H', nxt)
            put(obj, 0x24, 'I', rides[0])
            if r.random() < 0.8:
                obj[0x32] |= 4
            if r.random() < 0.7:   # (a state Object_CanOpen lets through, +0x64 clear)
                put(obj, 0x19c, 'i', r.choice([0, 3, 5]))
                put(obj, 0x64, 'I', 0)
        else:
            b = 0x400 * (i - 1)
            put(others, b + 0xc, 'H', nxt)
            others[b + 0x32] = r.choice([4, 4, 0, 0xff, r.getrandbits(8)])
            put(others, b + 0x19c, 'i', r.choice([1, 2, 4]))   # (Object_CanOpen: no at once)
            put(others, b + 0x20, 'I', r.randrange(16))
            put(others, b + 0x24, 'I', rides[i])
    for i in range(n):
        put(slots, 20 * handles[i], 'I', addrs[i])
    put(obj, 0x20, 'I', struct.unpack_from('<I', obj, 0x20)[0] & 0xf)
    gate = bytearray(r.bytes(0x40))
    gid = r.choice([4, 5, 1, 9])
    put(gate, 0x24, 'I', gid)
    ghandle = 17
    put(slots, 20 * ghandle, 'I', GATE)
    gate0 = bytearray(r.bytes(0x40))   # (slot 0, the gate handle 0's: the table's first dword, a thing too)
    put(gate0, 0x24, 'I', r.choice([4, 5, 1, 9]))
    put(slots, 0, 'I', GATE0)
    regions[0x7cfb90] = slots
    # the ride list: records 1-9 (some missing), their element arrays of 8 dwords
    ids = [i for i in range(1, 10) if r.random() < 0.8]
    nodes = bytearray(r.bytes(0x100 * max(len(ids), 1)))
    elems = bytearray(r.bytes(0x20 * 10))
    for k, rid in enumerate(ids):
        put(nodes, 0x100 * k, 'I', RIDES + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(nodes, 0x100 * k + 8, 'I', rid)
        put(nodes, 0x100 * k + 0x1c, 'I', ELEMS + 0x20 * rid)
        put(nodes, 0x100 * k + 0x8c, 'i', r.choice([8, 8, 8, 2, 7, 0, -1]))
        put(elems, 0x20 * rid + 4, 'I', r.choice([1, 1, 1, 1, 1, 0, 2, r.getrandbits(32)]))
    game = bytearray(r.bytes(0x60))
    put(game, 0x32, 'H', r.choice([ghandle] * 6 + [0]))
    put(game, 0x46, 'H', handles[order[0]] if r.random() < 0.95 else 0)
    log = log_region()
    blob_bytes(log, 21, 0, 0xfff)
    regions.update({RAM: obj, OBJS: others, GATE: gate, GATE0: gate0, RIDES: nodes, ELEMS: elems, GAME + 0x1da700: game, LOG: log,
                    0x8791b0: struct.pack('<I', RIDES if ids else 0), 0x8791a0: struct.pack('<I', r.choice([1, 1, 1, 1, 0])),
                    0x7a4610: b''.join(struct.pack('<I', STRIP) for _ in range(16)), STRIP: put(bytearray(r.bytes(0xc0)), 0xb0, 'I', 0),
                    0x788d3c: struct.pack('<I', CENTRE), CENTRE: r.bytes(0x40)})
    open_ = r.choice([0, 1, 0, 1, r.getrandbits(32)])
    only = r.choice([0] * 8 + [1, r.getrandbits(32)])
    # (the flag mostly the other way: a change of state)
    put(game, 0x10, 'I', r.choice([r.choice([1, r.getrandbits(32)]) if open_ else 0] * 3 + [0, 1, r.getrandbits(32)]))
    regions[GAME + 0x1da700] = game
    return Call(0x519ef0, [GAME, open_, only], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
