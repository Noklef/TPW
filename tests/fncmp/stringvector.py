# The string vector's insert of count copies (src/game/core/string: 0x5ed130 StringVector_InsertN) against the original: vectors of 0 to 8 strings
# (gamestring.py's strings, but with a vtable of recorders so the destructor (slot 0xc) and assignment (slot 0x18) calls are logged) with 0 to 4 spare
# places or none at all (an empty vector with no block), any position, counts of 0 to 8, the value now and then one of the vector's own strings; the
# new strings (String_CtorCopy) and the grown block come from fncmp's heap.
# usage: stringvector.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, vtable  # noqa: E402

B = Bench([(0x5ed130, 0x5ed430), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)])
VEC, VT, VALUE, ELEMS, DATA, VDATA = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000


def string_at(r, regions, base, data):
    """A gamestring string at base (block at data) given the recorders' vtable; returns its 0x14 bytes."""
    gamestring.string(r, 0, base, data, regions)
    obj = bytearray(regions.pop(base)[:0x14])
    struct.pack_into('<I', obj, 0, VT)
    return obj


def case(r):
    regions = {VT: vtable({3: 1, 6: 1}, 8), HEAP: heap(0x4000), LOG: log_region()}
    size = r.choice([0, 1, 2, 3, 4, 5, 8])
    empty = size == 0 and r.random() < 0.5
    spare = r.choice([0, 0, 1, 2, 4])
    elems = bytearray(r.bytes(0x14 * (size + spare + 1)))
    for i in range(size):
        elems[0x14 * i:0x14 * (i + 1)] = string_at(r, regions, RAM + 0x5000, DATA + 0x40 * i)
    regions[ELEMS] = elems
    first = 0 if empty else ELEMS
    vec = bytearray(r.bytes(0x10))
    struct.pack_into('<3I', vec, 4, first, first + 0x14 * size, first + 0x14 * (size + spare) if first else 0)
    regions[VEC] = vec
    regions[VALUE] = string_at(r, regions, RAM + 0x5000, VDATA)
    value = ELEMS + 0x14 * r.randrange(size) if size and r.random() < 0.15 else VALUE
    position = first + 0x14 * r.randrange(size + 1)
    count = r.choice([0, 1, 1, 2, 3, 4, 5, 8])
    return Call(0x5ed130, [VEC, position, count, value], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
