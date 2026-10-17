# The ingredient record and stream constructor of src/game/core/small62 (0x4e2200, 0x5667d0) against the originals: a thing whose definition (thingenv's table) has the
# ingredient value 0 to 5 and a table of records (some with a terminating zero word, some not) (Record); an object whose vtable gives the virtual base offset, with the flag
# set or clear (the base class constructor needs the driver quirk globals) (Ctor).
# usage: small62.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Record,Ctor)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, POOL, park  # noqa: E402

B = Bench([(0x412e90, 0x412f00), (0x4e2200, 0x4e22a0), (0x5651c0, 0x565220), (0x5667d0, 0x566850), (0x6ace60, 0x6aceb0)])
TABLE, ARRAY, RECORDS, VT, OBJ = RAM + 0x4000, RAM + 0x4100, RAM + 0x4200, RAM + 0x1000, RAM


def case(r):
    if r.random() < 0.5:
        regions, handles, things = park(r, count=1, defs=True)
        items = bytearray(regions[ITEMS])
        for i in range(NITEMS):
            put(items, i * ITEM + 0x158, 'I', r.choice([0, 1, 2, 3, 4, 5, 0xffffffff]))
        regions[ITEMS] = items
        n = r.choice([0, 2, 4, 4])
        table = bytearray(r.bytes(0x10))
        put(table, 8, 'I', n)
        put(table, 0xc, 'I', ARRAY)
        array = bytearray(r.bytes(0x20))
        records = bytearray(r.bytes(0x40 * 4))
        for k in range(4):
            length = r.choice([0, 1, 4, 5, 8])
            put(array, 4 * k, 'I', RECORDS + 0x40 * k)
            put(records, 0x40 * k, 'I', (length << 8) | r.choice([1, 1, 1, 0, 2]))
            put(records, 0x40 * k + 4 + 4 * ((length + 3) >> 2), 'I', r.choice([0, 0, 1]))
        regions.update({0x78ba8c: struct.pack('<I', TABLE), TABLE: table, ARRAY: array, RECORDS: records})
        return Call(0x4e2200, [things[0]], regions, conv='t', ret='a')
    offset = r.choice([0x2c, 0x30, 0x34, 0x40, 0x20])
    obj = bytearray(r.bytes(0x100))
    put(obj, 0, 'I', VT)
    regions = {OBJ: obj, VT: put(bytearray(r.bytes(0x10)), 4, 'I', offset), 0x879a20: put(bytearray(r.bytes(0x20)), 4, 'I', RAM + 0x2000), 0x879ab0: r.bytes(0x10),
               0x8bd520: r.bytes(4) + struct.pack('<I', r.choice([1, 1, 0, 4])), RAM + 0x2000: put(bytearray(r.bytes(0x500)), 0x428, 'I', r.choice([0x10de, 0x10de, 0x1002, 0]))}
    return Call(0x5667d0, [OBJ, r.choice([0, 1, 1, 0xffffff00, 7])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
