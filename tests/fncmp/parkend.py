# The park ended (src/game/world/parkopen: Game_EndPark 0x5168f0) against the original: thingenv's park (0-6 things, kinds mostly 3,
# +3 and +0x32 any of a few), the gate (the game object's word +0x1da732: slot 13's thing, its +0x24 a ride id), the ride list (0x8791b0,
# loaded or not at 0x8791a0: records 1-9, some missing, element arrays of 8 dwords) for the objects' element 4 and the gate's element 0, the
# delete queue 0x801f38 with 0-3 of the park's handles (repeats too) released into the heap, the sprite flag 0x763f84 and the game's state
# +0x1da738 dwords of their own. Agent C's unported 0x481b50 is thiscall recorder 14 of 1 (driver.cpp's kParkOpenEnv).
# usage: parkend.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put  # noqa: E402
from thingenv import POOL, SLOTS, park  # noqa: E402

B = Bench([(0x5168f0, 0x516b00), (0x4e0db0, 0x4e0df0), (0x50b350, 0x50b370), (0x519ef0, 0x51a290), (0x5408f0, 0x540910),
           (0x55a070, 0x55a110), (0x55a390, 0x55a3f0), (0x5da3c0, 0x5da3c1), (0x4cd300, 0x4cd350), (0x4c9130, 0x4c91d0), (0x4a8bd0, 0x4a8c00)],
          hooks={0x481b50: _recorder(14, 1)})
GATE, RIDES, ELEMS, QNODES = RAM + 0x41000, RAM + 0x42000, RAM + 0x43000, RAM + 0x44000
GAME = RAM + 0x50000 - 0x1da700   # (the game object: its fields +0x1da700-+0x1da760 at RAM + 0x50000)
KINDS = [3, 3, 3, 3, 1, 2, 0, 0xff]


def case(r):
    regions = {HEAP: heap(), LOG: log_region()}
    world, handles, _things = park(r, kinds=KINDS, defs=False)
    regions.update(world)
    pool = bytearray(regions[POOL])
    slots = bytearray(regions[SLOTS]) + bytearray(r.bytes(20 * 2))
    gate = bytearray(r.bytes(0x40))
    put(gate, 0x24, 'I', r.choice([4, 5, 1, 9, 2]))
    put(slots, 20 * 13, 'I', GATE)
    regions[SLOTS] = slots
    for i in range(len(handles)):   # (the objects' bit 2 of +0x32 set more often)
        if r.random() < 0.5:
            pool[0x400 * i + 0x32] |= 4
    regions[POOL] = pool
    # the ride list: records 1-9 (some missing), their element arrays of 8 dwords
    ids = [i for i in range(1, 10) if r.random() < 0.8]
    nodes = bytearray(r.bytes(0x100 * max(len(ids), 1)))
    elems = bytearray(r.bytes(0x20 * 10))
    for k, rid in enumerate(ids):
        put(nodes, 0x100 * k, 'I', RIDES + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(nodes, 0x100 * k + 8, 'I', rid)
        put(nodes, 0x100 * k + 0x1c, 'I', ELEMS + 0x20 * rid)
        put(nodes, 0x100 * k + 0x8c, 'i', r.choice([8, 8, 8, 2, 4, 5, 0, -1]))
    # the delete queue: 0-3 of the park's handles
    n = r.choice([0, 1, 2, 3]) if handles else 0
    qnodes = bytearray(r.bytes(0x24))
    for k in range(n):
        put(qnodes, 0xc * k, 'HHII', r.choice(handles), r.getrandbits(16), QNODES + 0xc * (k + 1) if k + 1 < n else 0, QNODES + 0xc * (k - 1) if k else 0)
    game = bytearray(r.bytes(0x60))
    put(game, 0x32, 'H', 13)
    regions.update({GATE: gate, RIDES: nodes, ELEMS: elems, QNODES: qnodes, GAME + 0x1da700: game,
                    0x8791b0: struct.pack('<I', RIDES if ids else 0), 0x8791a0: struct.pack('<I', r.choice([1, 1, 1, 1, 0])),
                    0x801f38: struct.pack('<IIII', r.getrandbits(32), QNODES if n else 0, QNODES + 0xc * (n - 1) if n else 0, n),
                    0x801bbc: struct.pack('<I', r.choice([0, SLOTS + 20 * 13, r.getrandbits(32) | 0x80000000])),
                    0x763f84: r.bytes(4)})
    return Call(0x5168f0, [GAME], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
