# The screen grab's taking of the front surface (src/game/gfx/screengrab: 0x561610) against the original: display sizes small, depths of 8-32 bits and odd ones, the tracked block got or not (fncmp's
# heap; the error pushed), the front surface's Lock (a COM recorder filling a description with a pitch of either sign) and Unlock answering 0, a positive result or a failure (its message logged through
# enginedebug.py's file recorders).
# usage: screengrabtake.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import enginedebug  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, fill, global_heap, heap, log_region, put, set_return, strings, vtable  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged([(0x561610, 0x56175d), (0x579b40, 0x579c13), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000), enginedebug.RANGE]), hooks=enginedebug.HOOKS)
COUNT, TABLE = 0x8bb214, 0x87b1f8   # the tracked blocks: their count and (block, block) pairs
GRAB, HOLDER, SURFACE, VT, BITS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000
LOCK, UNLOCK = 25, 32
FAILURES = [0x80004005, 0x887601c2, 0x88760096]


def case(r):
    log = log_region()
    enginedebug.texts(log)
    strings(log, LOCK, 0, 1 << 2)
    desc = bytearray(r.bytes(0x70))
    pitch = r.choice([24, 32, 40, 0, -32, 17])
    put(desc, 0x10, 'i', pitch)
    put(desc, 0x24, 'I', BITS + 0x200)
    fill(log, LOCK, 2, bytes(desc))
    set_return(log, LOCK, r.choice([0, 0, 0, 0, 1, *FAILURES]))
    set_return(log, UNLOCK, r.choice([0, 0, 0, 1, *FAILURES]))
    width, height = r.choice([0, 1, 2, 3, 5, 6]), r.choice([0, 1, 2, 4, 5, -1])
    depth = r.choice([8, 16, 16, 24, 32, 32, 15, 0])
    errors = bytearray(r.bytes(0x204))
    struct.pack_into('<i', errors, 0x200, r.randrange(0x40))
    regions = {HEAP: heap(0x800, r.choice([0, 0, 0, 0, 1])), **global_heap(), LOG: log, GRAB: r.bytes(0x10), 0x879a24: struct.pack('<I', HOLDER), HOLDER: r.bytes(0x14) + struct.pack('<I', SURFACE),
               SURFACE: struct.pack('<I', VT), VT: vtable({}, 33, com={LOCK: 5, UNLOCK: 2}), BITS: r.bytes(0x400), 0x8bcbd0: struct.pack('<ii', width, height), 0x8bcbe4: struct.pack('<I', depth),
               0x879cd4: r.bytes(4), 0x879ad0: errors, 0x8bd508: struct.pack('<I', r.getrandbits(32)), TABLE: bytes(8 * 8), COUNT: struct.pack('<I', r.choice([0, 1, 3])), 0x879a2c: struct.pack('<I', r.choice([0, 1]))}
    return Call(0x561610, [GRAB], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
