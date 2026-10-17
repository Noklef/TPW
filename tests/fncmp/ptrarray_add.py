# The pointer array's sprite frame append (src/game/core/ptrlist: 0x56b4c0 PtrArray_AddSpriteFrame) against the original: an array object whose
# items (+8) and header (+4) are apart or (now and then) inside the object itself, so the stores' order shows; fncmp's heap failing now and then (the
# error then pushed onto the engine's list at 0x879ad0, as in enginecreate.py).
# usage: ptrarray_add.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, heap, log_region, put, strings  # noqa: E402

B = Bench([(0x56b4c0, 0x56b56e), (0x588d70, 0x588d90), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000)], hooks={0x564650: _cdecl_log})
OBJ, ITEMS, HEADER = RAM, RAM + 0x1000, RAM + 0x2000


def case(r):
    log = log_region()
    strings(log, 0x3f, 1)
    obj = bytearray(r.bytes(0x100))
    count = r.choice([0, 1, 2, 5, 0x1f, r.randrange(0x40)])
    put(obj, 0x1c, 'I', count)
    items = OBJ if r.random() < 0.15 else ITEMS
    if items == OBJ:
        put(obj, 0x1c, 'I', r.choice([1, 3, 6, 7, 8]))
    put(obj, 8, 'I', items)
    put(obj, 4, 'I', r.choice([HEADER, HEADER, HEADER, OBJ, OBJ + 0x18]))
    errors = bytearray(r.bytes(0x204))
    struct.pack_into('<i', errors, 0x200, r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    flags = r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0)
    regions = {OBJ: obj, ITEMS: r.bytes(0x100), HEADER: r.bytes(0x10), HEAP: heap(0x44 + 0x40, 1 if r.random() < 0.3 else 0), 0x879ad0: errors,
               0x8bd508: struct.pack('<I', flags), LOG: log}
    return Call(0x56b4c0, [OBJ, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
