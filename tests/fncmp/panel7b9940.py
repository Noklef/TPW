# The 0x7b9940 object's detach (src/game/ui/panel7b9940: 0x48a3d0, and the jump 0x481410 to it) against the original: the GUI manager
# [0x77c48c] with a recorder vtable (slot 4 logged), the object's vtable giving its virtual base an offset of 0, 8 or 0x20, the renderer
# [0x87a6d0] whose tag 0x20 list (+0x18) holds 0-4 nodes keyed by that base or not, its free list and count, the scene flags 0x8bd508.
# usage: panel7b9940.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Detach,Jump)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Detach': 0x48a3d0, 'Jump': 0x481410}
B = Bench([(0x481410, 0x481415), (0x48a3d0, 0x48a40a), (0x659607, 0x65961c), (0x576540, 0x5765d0)])
MANAGER, VTABLE, TABLE, RENDERER, NODES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
OBJECT = 0x7b9940


def case(r):
    op = r.deal(OPS)
    log = log_region()
    if r.random() < 0.5:
        set_return(log, 4, r.getrandbits(32))
    d = r.choice([0, 8, 0x20])
    base = OBJECT + d
    renderer = bytearray(r.bytes(0x40))
    nodes = bytearray(r.bytes(0x40))
    head = 0
    for i in range(r.choice([0, 1, 2, 3, 4])):
        put(nodes, 8 * i, 'II', r.choice([base, base, OBJECT, r.getrandbits(32)]), head)
        head = NODES + 8 * i
    put(renderer, 0x18, 'I', head)
    put(renderer, 4, 'I', r.choice([0, NODES + 0x30]))
    put(renderer, 0xc, 'I', r.randrange(8))
    regions = {LOG: log, VTABLE: vtable({}), MANAGER: put(bytearray(r.bytes(0x10)), 0, 'I', VTABLE),
               0x77c48c: struct.pack('<II', MANAGER, MANAGER), TABLE: struct.pack('<II', r.getrandbits(32), d),
               OBJECT: put(bytearray(r.bytes(0x30)), 0, 'I', TABLE), RENDERER: renderer, NODES: nodes,
               0x87a6d0: struct.pack('<I', RENDERER), 0x8bd508: r.bytes(8)}
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
