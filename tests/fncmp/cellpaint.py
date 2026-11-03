# A record's paint put on its cell (src/game/world/queuepieces: CellPaint_Apply 0x5366e0) against the original: QueuePiece_Place (k 1, its answer stored) and Terrain_PaintAt (k 2, the paint logged as a hash) recorders. Any cell id, 0 too.
# usage: cellpaint.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, log_region, put, set_return  # noqa: E402

B = Bench([(0x5366e0, 0x53677b)], hooks={0x5229e0: _recorder(1, 6, cdecl=True), 0x450470: _recorder(2, 3, cdecl=True)})
RECORD = RAM


def case(r):
    log = log_region()
    set_return(log, 1, r.getrandbits(32))
    blob(log, 2, 2, 7)
    record = bytearray(r.bytes(0x30))
    put(record, 0, 'H', r.choice([0, 1, 0x80, 0x81, r.getrandbits(16)]))
    put(record, 0x14, 'I', r.choice([2, 2, 0, 1, 3, r.getrandbits(32)]))
    return Call(0x5366e0, [RECORD], {LOG: log, RECORD: record}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
