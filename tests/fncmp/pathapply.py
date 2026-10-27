# A path cell's model applied (src/game/world/cellmodel: 0x5365d0 MapCell_ApplyPathModel) against the original: pathmodel.py's cell and
# its neighbours (its kind +8 positive but 5 mostly: MapCell_PickPathModel; else the default model 0x37), at a cell inside scene.py's
# PaintAt terrain mostly (the map's flag 2 either; its sets and grid of up to 16 x 16), its queue ride (+4) none or one (Ride_Destroy
# 0x463c80 a cdecl recorder, 31, as queuepieces.py's); a model of group 2 places a queue piece (the piece the model's index into the table
# at 0x763388; Ride_Create 0x463060 a cdecl recorder, 30, logging the description, its result now and then set) and paints the erasing
# tile (set 0, tile 8), any other paints the model.
# usage: pathapply.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, log_region, put, set_return  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import pathmodel  # noqa: E402
import scene  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

pathmodel.CELL, pathmodel.OUT = RAM + 0x40000, RAM + 0x41000   # (scene.py's terrain uses RAM .. RAM + 0x26000)


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


RANGES = merged(pathmodel.RANGES + scene.RANGES + [(0x5229e0, 0x522a8a), (0x522a90, 0x522a9e), (0x5365d0, 0x5366dd)])
HOOKS = {0x463060: _recorder(30, 10, cdecl=True), 0x463c80: _recorder(31, 1, cdecl=True)}
B = Bench(RANGES, sines=256, hooks=HOOKS)


def case(r, log=None):
    """log: the log region to set up (cellkind.py's, shared with object.py's clock), else a new one."""
    regions = dict(scene.paint(r, 'PaintAt').regions)
    w, h = struct.unpack_from('<2I', regions[scene.GRID], 0x18)
    x = r.choice([r.randrange(w)] * 4 + [r.randrange(128)])
    y = r.choice([r.randrange(h)] * 4 + [r.randrange(128)])
    regions.update(pathmodel.case(r, (x, y)).regions)
    cell = bytearray(regions[pathmodel.CELL])
    put(cell, 4, 'I', r.choice([0, 0, 5, r.getrandbits(32)]))
    regions[pathmodel.CELL] = cell
    log = log_region() if log is None else log
    blob(log, 30, 2, 0xf)
    if r.random() < 0.5:
        set_return(log, 30, r.choice([0, 1, r.getrandbits(32)]))
    regions.update({LOG: log, 0x763388: r.bytes(0x200), 0x816d68: struct.pack('<2I', r.choice([1, 2]), r.choice([1, 2])),
                    0x820aa8: r.bytes(12)})
    return Call(0x5365d0, [pathmodel.CELL], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
