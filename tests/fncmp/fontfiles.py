# The font files' vector of src/game/files/fontfiles against the original: Destroy (0x412bc0) over the global vector at 0x789c1c (begin,
# end, capacity): 0-4 Strings of 0x14 bytes whose vtables point at the recorders (slot 0xc logged with its argument), now and then an
# empty vector with a null begin; the capacity any. The begin's freeing is hashed by the heap region.
# usage: fontfiles.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x412bc0, 0x412c12)])
NAMES, VT = RAM, RAM + 0x1000


def case(r):
    n = r.choice([0, 1, 2, 3, 4])
    names = bytearray(r.bytes(0x14 * 5))
    for k in range(5):
        put(names, 0x14 * k, 'I', VT)
    begin = 0 if n == 0 and r.random() < 0.5 else NAMES + 0x14 * r.choice([0, 0, 1])
    regions = {LOG: log_region(), HEAP: heap(), NAMES: names, VT: vtable({3: 1}),
               0x789c1c: struct.pack('<3I', begin, begin + 0x14 * n, r.choice([begin + 0x14 * n, NAMES + 0x64, r.getrandbits(32)]))}
    return Call(0x412bc0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
