# A cell object's queue piece removed (src/game/world/cellkind: 0x536780 CellObject_RemovePiece) against the original: the piece at +4 or none, QueuePiece_Remove
# a recorder (k 3).
# usage: cellkindremove.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

REMOVE = 3
B = Bench([(0x536780, 0x53679c)], hooks={0x522a90: _recorder(REMOVE, 1, cdecl=True)})
SELF = RAM


def case(r):
    me = bytearray(r.bytes(0x10))
    put(me, 4, 'I', r.choice([0, r.getrandbits(32)]))
    return Call(0x536780, [SELF], {LOG: log_region(), SELF: me}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
