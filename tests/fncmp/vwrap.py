# The small virtual-call wrappers of src/game/core/vwrap against the originals: objects whose vtables point at the recorders (see tools/fncmp.py recorder()),
# so the comparison covers which slot is called, on which object, with which arguments (the log) and what is done with the result (the hash the recorder returns).
# usage: vwrap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, vtable  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'S18': (0x5c5510, 't', 'a', 0x20), 'Pass': (0x617dd0, 't', 'a', 0x20), 'R24': (0x5d60d0, 't', 'a', 0x30), 'C24': (0x5d60f0, 't', 'a', 0x30),
         'G14': (0x46c330, 's', 'a', 0x30), 'G1e0': (0x498b90, 'c', 'v', 0x30), 'G1c8': (0x4987d0, 'c', 'v', 0x30), 'G1d8': (0x578390, 'c', 'v', 0x30),
         'Sub14': (0x5c5530, 't', 'a', 0x30), 'Sub28': (0x5c5560, 't', 'a', 0x30)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()])
OBJ, OBJ2, VT = RAM, RAM + 0x200, RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    regions = {OBJ: obj, LOG: log_region()}
    if name == 'S18':
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({6: 1})
        return Call(address, [OBJ, r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'Pass':
        put(obj, 0, 'I', VT)
        other = bytearray(r.bytes(0x40))
        put(other, 0, 'I', VT + 0x100)
        regions.update({OBJ2: other, VT + 0x100: vtable({4: 1})})
        return Call(address, [OBJ, OBJ2], regions, conv=conv, ret=ret)
    if name in ('R24', 'C24'):
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({9: 0})
        return Call(address, [OBJ], regions, conv=conv, ret=ret)
    if name == 'G14':
        regions[0x7b05d8] = struct.pack('<I', OBJ)
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({5: 2})
        return Call(address, [r.getrandbits(32), r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name in ('G1e0', 'G1c8', 'G1d8'):
        glob = {'G1e0': 0x7ca1e0, 'G1c8': 0x7ca1c8, 'G1d8': 0x87b1d8}[name]
        slot = 0 if name == 'G1d8' else 2
        regions[glob] = struct.pack('<I', OBJ if r.random() < 0.7 else 0)
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({slot: 1})
        return Call(address, [], regions, conv=conv, ret=ret)
    sub = 0x14 if name == 'Sub14' else 0x28
    put(obj, sub, 'I', VT)
    regions[VT] = vtable({6: 1})
    return Call(address, [OBJ, r.getrandbits(32)], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
