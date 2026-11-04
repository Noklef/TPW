# Two methods nothing names (src/game/ui/windowgroup: 0x47de00, 0x47e250) against the originals: the object a vtable of recorders or none (the word argument logged as a word: its upper half is the caller's ESI), the windows a recorder that a count of any sign runs.
# usage: windowgroup.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, vtable  # noqa: E402

TELL, OPEN = 0x47de00, 0x47e250
OPEN_TREE, SLOT = 1, 2
B = Bench([(0x47de00, 0x47de24), (0x47e250, 0x47e286)], hooks={0x65c707: _recorder(OPEN_TREE, 0)})
SELF, OBJECT, VT, WINDOWS = RAM, RAM + 0x200, RAM + 0x240, RAM + 0x300


def case(r):
    log = log_region()
    me = bytearray(r.bytes(0x180))
    op = r.choice([TELL, OPEN])
    regions = {LOG: log, SELF: me, OBJECT: struct.pack('<I', VT), VT: vtable({SLOT: 3}), WINDOWS: r.bytes(0x40)}
    if op == TELL:
        struct.pack_into('<I', me, 0x9c, r.choice([OBJECT, OBJECT, OBJECT, 0]))
        struct.pack_into('<H', me, 0xe0, r.choice([0, 1, 0x7fff, 0xffff, r.getrandbits(16)]))
        struct.pack_into('<I', log, LOG_BYTES + 4 * SLOT, 1 << (16 + 2))
    else:
        struct.pack_into('<I', me, 0x148, r.choice([WINDOWS, WINDOWS, WINDOWS, 0]))
        struct.pack_into('<h', me, 0x172, r.choice([0, 1, 2, 5, 16, -1, -0x8000]))
    return Call(op, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
