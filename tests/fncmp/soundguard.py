# The sound cleanups' holders (src/game/core/soundguard) against the originals: the held object a recorder vtable (slots 0, 2 of one argument, slot 3 of none) or null; the freed block one of the heap's or null.
# usage: soundguard.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, vtable  # noqa: E402

CALLS = {'slot2': 0x6b70b0, 'free': 0x6b70c0, 'array': 0x6bafd0, 'one': 0x6bafe0, 'release': 0x6bb210}
B = Bench([(0x6b70b0, 0x6b70be), (0x6b70c0, 0x6b70cc), (0x6bafd0, 0x6bafdd), (0x6bafe0, 0x6bafed), (0x6bb210, 0x6bb21f)])
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
HOLDER, OBJ, VT = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    name = r.deal(OPS)
    regions = {LOG: log_region(), OBJ: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({0: 1, 2: 1})}
    if name == 'free':
        regions[HEAP] = heap(0x40)
        held = r.choice([0, HEAP + 0x10, HEAP + 0x10])
    elif name == 'release':   # (the flag says the object is there: null only when it is clear)
        flag = r.choice([0, 1, 0x80, r.getrandbits(8)])
        held = OBJ if flag or r.random() < 0.5 else 0
        regions[HOLDER] = struct.pack('<IB', held, flag) + r.bytes(3)
        return Call(CALLS[name], [HOLDER], regions, conv='t', ret='v')
    else:
        held = r.choice([0, OBJ, OBJ])
    regions[HOLDER] = struct.pack('<I', held) + r.bytes(4)
    return Call(CALLS[name], [HOLDER], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
