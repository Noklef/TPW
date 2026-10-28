# The coaster editor's car deletions (src/game/coaster/edittools: CoasterEdit_DeleteLastCar 0x444f30) against the original, with the
# editor's tool calls they make. The world is settools.py's (coaster.py's edit-tool world: the link at +0x58 a chain of four pieces through
# +0x68, +0x60 one of them, their trains (+0x100) the cells block; the recorder; endtool.py's rides; the 0x7ca1c8 dialog up or not). Their
# deletions' calls are recorders (cdecl, as the driver's EditToolsEnv has them): Coaster_DeleteCar 0x444cb0 (18 of 2),
# Coaster_DeleteFlaggedEnd 0x435530 (19 of 1), Coaster_SetState 0x435730 (20 of 2), MouseMode_EnterTile15 0x497d20 (21 of 0).
# Last: the link's +0x3c flags 1 / 2 either way, its +4 byte flag 2 mostly, +0x58 / +0x60 a piece or now and then none, the pieces' +8
# flags 0x40 / 0x80 either way, their trains' +4 byte flag 2 now and then; 0x785a2c set now and then.
# usage: deletecars.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Last)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import settools  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

HOOKS = {0x444cb0: _recorder(18, 2, cdecl=True), 0x435530: _recorder(19, 1, cdecl=True), 0x435730: _recorder(20, 2, cdecl=True),
         0x497d20: _recorder(21, 0, cdecl=True)}
RANGES = sorted(set(settools.coaster.RANGES + settools.endtool.RANGES + [(0x444770, 0x444ab2), (0x442880, 0x442b23), (0x445220, 0x4454ab),
                                                                         (0x444f30, 0x445212)]))
B = Bench(RANGES, sines=256, hooks=HOOKS)
GAME, PIECES, CELLS, LTYPE = settools.GAME, settools.PIECES, RAM + 0x7000, RAM + 0x6100


def world(r):
    """settools.py's world (its op dealt as its own), the pieces' flags 0x40 / 0x80 and their trains' flag 2 dealt, the link's +4 byte."""
    regions = dict(settools.case(r).regions)
    regions.setdefault(LOG, log_region())
    pieces = bytearray(regions[PIECES])
    for k in range(4):
        o = 0x200 * k + 8
        put(pieces, o, 'I', struct.unpack_from('<I', pieces, o)[0] & ~0xc0 | r.choice([0, 0x40, 0x80, 0xc0]))
    regions[PIECES] = pieces
    cells = bytearray(regions[CELLS])
    for k in range(4):
        cells[0x20 * k + 4] = cells[0x20 * k + 4] & ~2 | r.choice([0, 0, 0, 2])
    regions[CELLS] = cells
    ltype = bytearray(regions[LTYPE])
    ltype[0] = ltype[0] & ~2 | r.choice([2, 2, 2, 0])
    regions[LTYPE] = ltype
    regions[0x785a2c] = struct.pack('<I', r.choice([0, 0, 0, 0, 1]))
    return regions


def case(r):
    op = r.deal(OPS)
    regions = world(r)
    game = bytearray(regions[GAME])
    put(game, 0x3c, 'I', struct.unpack_from('<I', game, 0x3c)[0] & ~3 | r.choice([0, 1, 2, 3, 2, 0]))
    for off in (0x58, 0x60):
        if r.random() < 0.08:
            put(game, off, 'I', 0)
        elif off == 0x60 or r.random() < 0.3:
            put(game, off, 'I', PIECES + 0x200 * r.randrange(4))
    regions[GAME] = game
    assert op == 'Last'
    return Call(0x444f30, [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', 'Last').split(',') if k]

if __name__ == '__main__':
    B.main(case)
