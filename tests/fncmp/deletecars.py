# The coaster editor's car deletions (src/game/coaster/edittools: CoasterEdit_DeleteLastCar 0x444f30, CoasterEdit_DeleteSpan 0x446740)
# against the original, with the
# editor's tool calls they make. The world is settools.py's (coaster.py's edit-tool world: the link at +0x58 a chain of four pieces through
# +0x68, +0x60 one of them, their trains (+0x100) the cells block; the recorder; endtool.py's rides; the 0x7ca1c8 dialog up or not). Their
# deletions' calls are recorders (cdecl, as the driver's EditToolsEnv has them): Coaster_DeleteCar 0x444cb0 (18 of 2),
# Coaster_DeleteFlaggedEnd 0x435530 (19 of 1), Coaster_SetState 0x435730 (20 of 2), MouseMode_EnterTile15 0x497d20 (21 of 0).
# Last: the link's +0x3c flags 1 / 2 either way, its +4 byte flag 2 mostly, +0x58 / +0x60 a piece or now and then none, the pieces' +8
# flags 0x40 / 0x80 either way, their trains' +4 byte flag 2 now and then; 0x785a2c set now and then.
# Span (CoasterEdit_DeleteSpan 0x446740): the pieces a list both ways from the link's +0x58, their handles rising (from 0 now and then),
# the span's ends (0x79c640 / 0x79c6dc) and the selection (0x79c628 / 0x79c62c) pieces or none, 0x79c658 flag 0x80 mostly, +0x40 set mostly.
# usage: deletecars.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Last,Span)
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
                                                                         (0x444f30, 0x445212), (0x446740, 0x446c37)]))
B = Bench(RANGES, sines=256, hooks=HOOKS)
GAME, PIECES, CELLS, LTYPE = settools.GAME, settools.PIECES, RAM + 0x7000, RAM + 0x6100
HOLD, PARTS, EXTRA = settools.endtool.HOLD, RAM + 0xb000, RAM + 0xb400


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
    if op == 'Last':
        return Call(0x444f30, [], regions, ret='v')
    # Span: the pieces a list both ways (+0x68 from piece 0, the link's last, to 3; +0x64 back), their handles rising from 3 to 0 (from
    # 0 now and then: an end with handle 0); the ends (0x79c640, 0x79c6dc) pieces, the selection (0x79c628 / 0x79c62c) two or none
    # (and a piece either side, EXTRA's low one (handle base, +0x68 none) and high one (base + 5, +0x64 none) mostly, else the four
    # pieces' ends are the list's)
    pieces = bytearray(regions[PIECES])
    base = r.choice([0, 1, 1, 7, r.getrandbits(31)])
    ends = r.random() < 0.85
    for k in range(4):
        put(pieces, 0x200 * k, 'I', base + 4 - k)
        put(pieces, 0x200 * k + 0x64, 'I', PIECES + 0x200 * (k - 1) if k else EXTRA + 0x200 if ends else 0)
    put(pieces, 0x600 + 0x68, 'I', EXTRA if ends else 0)
    extra = bytearray(r.bytes(0x400))
    for k, (prev, nxt) in enumerate(((0, PIECES + 0x600), (PIECES, 0))):
        o = 0x200 * k
        put(extra, o, 'I', base + 5 * k)
        put(extra, o + 8, 'I', r.getrandbits(32) & ~0xc0 | r.choice([0, 0, 0, 0x40, 0x80]))
        put(extra, o + 0x64, 'II', nxt, prev)
        put(extra, o + 0x70, 'I', r.choice([3, 4]))
        put(extra, o + 0x100, 'I', CELLS + 0x20 * r.randrange(4))
    regions[EXTRA] = extra
    if r.random() < 0.9:   # (the low end without 0x40 mostly: the walk back past 0x40 ends on it, not on null)
        lo = (EXTRA, 8) if ends else (PIECES, 0x608)
        buf = extra if ends else pieces
        put(buf, lo[1], 'I', struct.unpack_from('<I', buf, lo[1])[0] & ~0x40)
    regions[PIECES] = pieces
    # the part entry's +0x34: the cells' (+0x100) +0x18 one of four types (PARTS, 0x40 apart) whose +0x2c indexes (0x14 each) the
    # entries (PARTS + 0x110) at the ride holders' objects' +0x28 (PARTS + 0x100) -> +4, an entry's +4 a value block (PARTS + 0x180) or none
    cells = bytearray(regions[CELLS])
    for k in range(4):
        put(cells, 0x20 * k + 0x18, 'I', PARTS + 0x40 * r.randrange(4))
    regions[CELLS] = cells
    parts = bytearray(r.bytes(0x200))
    for k in range(4):
        put(parts, 0x40 * k + 0x2c, 'I', r.randrange(4))
        put(parts, 0x110 + 0x14 * k + 4, 'I', r.choice([PARTS + 0x180, PARTS + 0x180, 0]))
    put(parts, 0x104, 'I', PARTS + 0x110)
    r.floats(parts, [0x180 + 0x34])
    regions[PARTS] = parts
    hold = bytearray(regions[HOLD])
    for k in range(2):
        put(hold, 0x200 + 0x40 * k + 0x28, 'I', PARTS + 0x100)
    regions[HOLD] = hold
    put(game, 0x58, 'I', EXTRA + 0x200 if ends and r.random() < 0.7 else PIECES)   # (the link's last: the list's, or piece 0)
    put(game, 0x40, 'I', r.choice([1, 1, 1, 1, 0, r.getrandbits(32)]))
    regions[GAME] = game
    glob = bytearray(regions[0x79c600])
    one = lambda: r.choice([PIECES + 0x200 * r.randrange(4)] * 6 + [EXTRA] * ends)  # noqa: E731   (the low end: handle 0 now and then)
    put(glob, 0x28, 'II', *(r.choice([(0, 0), (one(), one()), (one(), one())])))
    put(glob, 0x40, 'I', r.choice([one(), one(), one(), 0]))
    put(glob, 0xdc, 'I', one() if r.random() < 0.95 else 0)
    put(glob, 0x58, 'I', struct.unpack_from('<I', glob, 0x58)[0] & ~0x80 | r.choice([0x80, 0x80, 0x80, 0]))
    regions[0x79c600] = glob
    regions[0x7925d0] = r.bytes(4)
    return Call(0x446740, [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', 'Last,Span').split(',') if k]

if __name__ == '__main__':
    B.main(case)
