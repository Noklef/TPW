# A rectangle's outline laid (src/game/world/parkentrance: 0x5327e0) against the original; GameMode_Set (k 1), GameMode_MapClick (k 3), CellRun_Mark (k 4), TrackLine_Prune (k 5), CellRun_Lay (k 6, its answer stored),
# Terrain_ShadeNormals (k 8) and GameCursor_Set (k 9) are recorders. The corners either way round, the run's four dwords at 0x7639f0.
# usage: parkoutline.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x5327e0, 0x5328d4)],
          hooks={0x52f200: _recorder(1, 2, cdecl=True), 0x524960: _recorder(3, 4, cdecl=True), 0x531040: _recorder(4, 1, cdecl=True), 0x5311c0: _recorder(5, 0, cdecl=True),
                 0x52ba50: _recorder(6, 6, cdecl=True), 0x450d10: _recorder(8, 0, cdecl=True), 0x4a2aa0: _recorder(9, 1, cdecl=True)})


def case(r):
    log = log_region()
    set_return(log, 6, r.getrandbits(32))
    regions = {LOG: log, 0x7639f0: r.bytes(0x10), 0x7639c0: r.bytes(4), 0x81b4cc: r.bytes(1)}
    a = [r.randrange(-5, 300) & 0xffffffff, r.randrange(-5, 300) & 0xffffffff]
    b = [r.choice([a[0], r.randrange(-5, 300) & 0xffffffff]), r.choice([a[1], r.randrange(-5, 300) & 0xffffffff])]
    return Call(0x5327e0, [a[0], a[1], b[0], b[1]], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
