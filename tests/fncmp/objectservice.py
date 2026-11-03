# The object after a mechanic's visit (src/game/world/object: 0x4df8f0 Object_MechanicDone) against the original: objectqueue.py's Close world, the state 2 mostly
# (an upgrade: then angles other than quarter turns too, and +0x50's levels with the definition's rows of cost, capacity, price and +0xc0 word either side of 0, the
# limits +0x12c / +0x130 either side of the price), the money real (the record money.py's, on the table 0x7cfb90 by [0x80239c] + 0x1da726). A definition of kind 3 with
# its link closed now and then (the restart). 0x475550, 0x475a10 and 0x55a100 have benches of their own (script.py's, ridesound.py): cdecl recorders 27, 28 and 29
# (driver.cpp's kObjectServiceEnv), the script's slot random.
# usage: objectservice.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import money  # noqa: E402
import objectqueue  # noqa: E402
from fncmp import LOG, RAM, Bench, _recorder, log_region, put, set_return  # noqa: E402

RANGES = objectqueue.RANGES + [(0x4df8f0, 0x4dfd77), (0x4dd7f0, 0x4dd920), (0x55a300, 0x55a32e), (0x55a390, 0x55a3f0), (0x5195d0, 0x51960d), (0x519510, 0x519550),
                               (0x4d01f0, 0x4d0260)]
B = Bench(RANGES, hooks={0x475550: _recorder(27, 1, cdecl=True), 0x475a10: _recorder(28, 7, cdecl=True), 0x55a100: _recorder(29, 1, cdecl=True)})
RECORD, TOTALS = RAM + 0x48000, RAM + 0x49000
LEVELS = 6


def case(r):
    c = objectqueue.close(r)
    obj, items = c.regions[RAM], c.regions[0x30008000]
    state = r.choice([2, 2, 2, 2, 2, 2, 0, 1, 3, 4, 5])
    put(obj, 0x19c, 'I', state)
    obj[0x50] = r.choice([0, 0, 0, 1, 1, 1, 2, 3, 0xff])
    put(obj, 0x54, 'I', r.choice([0, 0, 0x1234, r.getrandbits(32)]))
    if state == 2:  # (other states read the entrance's cells, where another angle gives a stale stack byte: see objectmsg.h)
        put(obj, 0x10, 'i', r.choice([0, 0x5a, 0xb4, 0x10e, 0x2d, 1, -0x5a, 0x168]))
    found = struct.unpack_from('<i', c.regions[0x7890a0], 0x528)[0]   # (the definitions searched: one of them is the object's)
    chosen = r.randrange(found)
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, 0x500 * chosen)[0])
    for k in range(len(items) // 0x500):
        item = 0x500 * k
        put(items, item + 0x12c, 'i', r.choice([0, 0, 10, 60, -1]))
        put(items, item + 0x130, 'i', r.choice([0xff, 0xff, 20, 60, 0]))
        for level in range(LEVELS):
            row = item + 0x40 * level
            put(items, row + 0x198, 'i', r.choice([0, 0, -1, 3, 0x104, 0xff]))
            put(items, row + 0x1a0, 'i', r.choice([0, -1, 5, 50, 200, 0x1ff]))
            put(items, row + 0x1a8, 'i', r.choice([0, -1, 7, 0x12345]))
            put(items, row + 0x1b8, 'i', r.choice([0, 100, -5, r.getrandbits(31)]))
    if r.random() < 0.3:  # (kind 3, the link open or not: no restart or the restart)
        links = c.regions[RAM + 0x20000]
        for k in range(len(items) // 0x500):
            put(items, 0x500 * k + 0x9c, 'I', 3)
        put(obj, 0x20, 'I', 1)
        put(links, 0x140, 'I', r.choice([0, 1]))
        put(links, 0x3c, 'I', 1)
        c.regions.update({0x790fe0: struct.pack('<I', RAM + 0x20000), 0x790fec: struct.pack('<I', 0)})
    if struct.unpack_from('<I', items, 0x500 * chosen + 0x9c)[0] == 3:  # (a restart's grow walks the strips' owners, random in Close's world: no strips then)
        for i in range(10):
            strip = bytearray(c.regions[objectqueue.STRIPS + 0x100 * i])
            c.regions[objectqueue.STRIPS + 0x100 * i] = put(strip, 0xb0, 'I', 0)
    world = RAM + 0x10000
    handles = bytearray(r.bytes(0x40))
    put(handles, 0x20, 'H', 1)
    put(handles, 0x26, 'H', 2)
    slots = bytearray(r.bytes(20 * 4))
    put(slots, 20 * 1, 'I', TOTALS)
    put(slots, 20 * 2, 'I', RECORD)
    log = log_region()
    set_return(log, 28, r.getrandbits(32))
    c.regions.update({world + 0x1da700: handles, 0x7cfb90: slots, RECORD: money.record(r), TOTALS + 0x1f5a0: r.bytes(4), LOG: log})
    c.address, c.args, c.ret = 0x4df8f0, [RAM, r.getrandbits(32)], 'v'
    return c


if __name__ == '__main__':
    B.main(case)
