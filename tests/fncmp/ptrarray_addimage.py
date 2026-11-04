# The pointer array's sprite image added from a file (src/game/core/ptrlist: 0x56b380 PtrArray_AddImage) against the original: the array full
# or not, items (+0xc) apart or inside the object, fncmp's heap failing now and then (the error then pushed onto the engine's list), the
# file's load (k 1) failing now and then and the image then destroyed (k 2) and freed.
# usage: ptrarray_addimage.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, heap, log_region, put, set_return, strings  # noqa: E402

LOAD, DTOR = 1, 2
B = Bench([(0x56b380, 0x56b448), (0x588320, 0x588350), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000)],
          hooks={0x564650: _cdecl_log, 0x588820: _recorder(LOAD, 1), 0x588350: _recorder(DTOR, 0)})
OBJ, ITEMS = RAM, RAM + 0x1000


def case(r):
    log = log_region()
    strings(log, 0x3f, 1)
    set_return(log, LOAD, r.choice([0, 1, 1, r.getrandbits(32)]))
    obj = bytearray(r.bytes(0x100))
    count = r.choice([0, 1, 2, 5, 0x1f, r.randrange(0x40)])
    put(obj, 0x18, 'I', count)
    put(obj, 0x14, 'I', count if r.random() < 0.15 else r.getrandbits(32))
    items = OBJ if r.random() < 0.15 else ITEMS
    if items == OBJ:
        put(obj, 0x18, 'I', r.choice([1, 2, 3, 6, 7]))
    put(obj, 0xc, 'I', items)
    errors = bytearray(r.bytes(0x204))
    struct.pack_into('<i', errors, 0x200, r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    flags = r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0)
    regions = {OBJ: obj, ITEMS: r.bytes(0x100), HEAP: heap(0x18 + 0x40, 1 if r.random() < 0.25 else 0), 0x879ad0: errors, 0x8bd508: struct.pack('<I', flags), LOG: log}
    return Call(0x56b380, [OBJ, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
