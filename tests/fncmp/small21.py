# The small helpers of src/game/core/small21 against the originals: a pointer variable freed when set, a dword fill, the two keyboard-map wrappers (the five maps of
# keymap.py) and the global-memory wrappers (the heap region serves GlobalAlloc / GlobalFree).
# usage: small21.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import keymap  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, global_heap, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Pfree': (0x598760, 't', 'v', 0x10), 'Zero': (0x598770, 'c', 'a', 0x10), 'One': (0x5f4ea0, 'c', 'a', 0x10), 'KeyE': (0x486b60, 'c', 'v', 0x10),
         'KeyR': (0x486b70, 'c', 'v', 0x10), 'Salloc': (0x51b4a0, 'c', 'a', 0x30), 'Sfree': (0x51b4d0, 'c', 'a', 0x20), 'Chk': (0x51f630, 'c', 'a', 0x30),
         'Flog': (0x556ab0, 'c', 'a', 0x40), 'Fchk': (0x51f660, 'c', 'a', 0x20)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x40c870, 0x40d06a), (0x5f5660, 0x5f5690), (0x5da3c0, 0x5da3c1), (0x67acb0, 0x67acb1)])


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    regions = {HEAP: heap(), **global_heap()}
    if name == 'Pfree':
        obj = bytearray(r.bytes(0x20))
        put(obj, 0, 'I', r.choice([0, HEAP + 0x40, RAM + 0x200]))
        regions[RAM] = obj
        return Call(address, [RAM], regions, conv=conv, ret=ret)
    if name == 'Zero':
        regions[RAM + 0x100] = r.bytes(0x60)
        return Call(address, [RAM + 0x100 + r.choice([0, 4, 8]), r.choice([0, 1, 2, 5, 8])], regions, conv=conv, ret=ret)
    if name == 'One':
        return Call(address, [], regions, conv=conv, ret=ret)
    if name in ('KeyE', 'KeyR'):
        for k, g in enumerate(keymap.GLOBALS):
            keymap.keymap(r, RAM + k * 0x2000 + 0x40, regions)
            regions[g] = (RAM + k * 0x2000 + 0x40).to_bytes(4, 'little')
        regions[0x7c24d0] = r.bytes(0x10)
        return Call(address, [], regions, conv=conv, ret=ret)
    regions[0x802bd8] = r.bytes(0x10)
    if name in ('Sfree', 'Fchk'):
        return Call(address, [r.choice([0, 0, HEAP + 0x40, HEAP + 0x80])], regions, conv=conv, ret=ret)
    return Call(address, [r.choice([0, 1, 7, 0x40, 0x100, 0x1000])], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
