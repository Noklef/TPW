# A panel screen's message handler (src/game/ui/panelkeys: 0x5e41c0) against the original: the escape key's message (0x1000b, 0x1b) or others; 0 to 3 objects listed, whose slot 9 (up or not) and slot 6 (took the key or not)
# answer from tables; slot 5 a recorder with the message. Panel7c2534_Open (0x48c830) is a recorder, with a bench of its own.
# usage: panelkeys.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put, vtable  # noqa: E402

OPEN, SLOT, TOOK, UP = 1, 5, 6, 9
B = Bench([(0x5e41c0, 0x5e424e)], hooks={0x48c830: _recorder(OPEN, 1, cdecl=True)})
PANEL, OWNER, NODES, OBJECTS, VT, TABLE_UP, TABLE_TOOK = RAM, RAM + 0x80, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x500, RAM + 0x600


def case(r):
    log = log_region()
    answers(log, UP, TABLE_UP)
    answers(log, TOOK, TABLE_TOOK)
    n = r.choice([0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x40))
    for i in range(n):
        put(nodes, 8 * i, '2I', OBJECTS + 4 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
    panel = bytearray(r.bytes(0x40))
    put(panel, 0x18, 'I', OWNER)
    regions = {LOG: log, PANEL: panel, OWNER: struct.pack('<II', r.getrandbits(32), NODES if n else 0), NODES: nodes, OBJECTS: struct.pack('<3I', VT, VT, VT), VT: vtable({SLOT: 3}),
               TABLE_UP: struct.pack('<48I', *[r.choice([0, 1, 1, 0x100, r.getrandbits(32)]) for _ in range(48)]),
               TABLE_TOOK: struct.pack('<48I', *[r.choice([0, 0, 0, 1, 0x100, r.getrandbits(32)]) for _ in range(48)])}
    message = r.choice([0x1000b, 0x1000b, 0x1000b, 0x1000a, 0x1000c, 0xb, r.getrandbits(32)])
    a = r.choice([0x1b, 0x1b, 0x1b, 0x1a, 0x11b, r.getrandbits(32)])
    return Call(0x5e41c0, [PANEL, message, a, r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
