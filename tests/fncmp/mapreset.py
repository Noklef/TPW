# The map's cells reset (Map_ResetCells 0x52f8d0, src/game/world/mapreset) against the original: MapCell_Reset (k 1) and TrackCell_Reset (k 2) recorders, the map's pointer random; bounds at
# and below 0, small ones, the game's 0x80 by 0x80, and rows past 0x200 for the id's wrap at 16 bits.
# usage: mapreset.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region  # noqa: E402

B = Bench([(0x52f8d0, 0x52f949)], hooks={0x536490: _recorder(1, 1), 0x53af00: _recorder(2, 0)})


def case(r):
    columns, rows = r.choice([(0, 3), (3, 0), (-1, 3), (3, -1), (1, 1), (2, 3), (r.randrange(6), r.randrange(6)), (0x80, 2), (0x82, 3), (1, 0x203), (0x80, 0x80), (2, 0x1ff)])
    return Call(0x52f8d0, [columns, rows], {LOG: log_region(), 0x7cf83c: struct.pack('<I', r.getrandbits(32))}, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
