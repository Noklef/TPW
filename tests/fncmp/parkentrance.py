# The park entrance laid (src/game/world/parkentrance: 0x5323f0) against the original, with Map_EntrancePos and MapCell_ClearFlags as they are; GameMode_Set (k 1), FocusCell_Update (k 2),
# GameMode_MapClick (k 3), CellRun_Mark (k 4), TrackLine_Prune (k 5), CellRun_Lay (k 6), MapCell_ApplyPathModel (k 7) and Terrain_ShadeNormals (k 8) are recorders. Random places for the two
# entrances, row 0 now and then (the cell id wraps), the flag's two ways, a table with a stop early or none.
# usage: parkentrance.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

B = Bench([(0x5323f0, 0x532640), (0x4d8690, 0x4d86d0), (0x53a530, 0x53a540)],
          hooks={0x52f200: _recorder(1, 2, cdecl=True), 0x5276d0: _recorder(2, 1, cdecl=True), 0x524960: _recorder(3, 4, cdecl=True), 0x531040: _recorder(4, 1, cdecl=True),
                 0x5311c0: _recorder(5, 0, cdecl=True), 0x52ba50: _recorder(6, 6, cdecl=True), 0x5365d0: _recorder(7, 0), 0x450d10: _recorder(8, 0, cdecl=True)})
MAP = RAM
CELL = 0x44
ROWS = 0x108


def case(r):
    log = log_region()
    row = r.deal([0, 1, 2, 3, 5, 7])
    placed = (r.randrange(0, 0x80), r.randrange(0, 8), r.deal([0, 0x7f, r.randrange(0, 0x80), r.randrange(0, 0x80)]), row)
    first = max(0, (row << 7) - 3)
    cells = r.bytes(ROWS * CELL)

    table = bytearray(r.bytes(0x208))
    for k in range(0, 0x208, 8):
        struct.pack_into('<ii', table, k, r.randrange(-20, 20), r.randrange(-20, 20))
    stop = r.deal([0, 1, 2, 5, 12, 0x40])
    if stop < 0x40:
        struct.pack_into('<i', table, stop * 8, 0x200)

    regions = {LOG: log, 0x7cf83c: struct.pack('<I', MAP), 0x7855bc: struct.pack('<4I', *placed), 0x7639f0: r.bytes(0x10), 0x81b4cc: r.bytes(1), 0x818c50: r.bytes(1),
               0x79d86c: struct.pack('<I', r.getrandbits(32) & ~0x400 | r.deal([0, 0x400])), 0x784e3c: table, MAP + 0x294 + first * CELL: cells}
    return Call(0x5323f0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
