# The focus cell's update (src/game/world/focuscell: 0x5276d0) against the original, with its callees run (Global81ae2c_Equals, the 0x7c2654
# accessors and their wrappers, Fn_4e0df0, ObjectDefs_Find on thingenv.py's park, Position_StepA / B, ActionRec_WriteW on actionrec.py's record
# stream): the game mode (0x81ae2c) 0x19, 0x1a or other with the set cell (0x7639f0 / 0x7639f4) or none (-1); the selected object (0x7c2654) none, an
# object of its own or one of the park's things, its handle word (+0x30) none or a thing's and its definition's +0x288 1 or not; its column, row and
# direction (+5, +7, +0x10) about the steps' cases; the previous focus (0x8186ac) none or a thing; the old cell word the new one or not; the flag to
# record the move 0, 1 or with a zero low byte, the recorder recording or not.
# usage: focuscell.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import actionrec  # noqa: E402
from fncmp import RAM, Bench, Call, put  # noqa: E402
from thingenv import DEFS, ITEM, ITEMS, NITEMS, POOL, park  # noqa: E402

B = Bench([(0x5276d0, 0x527925), (0x52f860, 0x52f872), (0x4815a0, 0x4815be), (0x48c930, 0x48c94a), (0x4e0df0, 0x4e0dfe), (0x412e90, 0x412f00),
           (0x52a3a0, 0x52a490), (0x4d8c00, 0x4d8c1f), (0x403d00, 0x403d54), (0x5da3c0, 0x5da3c1)])
OBJ = RAM + 0x40000
FOCUS = 0x818698  # to 0x8186c8: the cell word (+0), the old one (+8), the focus object (+0x14), its cell's entry (+0x2c)
STEPS = 0x7639a0  # to 0x7639f8: the cell and direction (+4..), the two stepped positions (+0x40..), the set cell (+0x50, +0x54)
DIRS = [0, 0, 0x5a, 0xb4, 0x10e, 1, 0x168, 0xffffffff]


def thing(r, t, handles):
    """A thing's column, row and direction; its handle word (+0x30) none or a thing's."""
    t[5], t[7] = r.choice([0, 1, 0x7e, 0x7f, 0x80, 0xff, r.getrandbits(8)]), r.choice([0, 1, 0x7f, 0xff, r.getrandbits(8)])
    put(t, 0x10, 'I', r.choice(DIRS))
    put(t, 0x30, 'H', r.choice(handles + [0, 0]))


def case(r):
    regions, handles, addrs = park(r, count=r.choice([1, 2, 3, 4]), defs=True)
    things = bytearray(regions[POOL])
    for a in addrs:
        t = bytearray(things[a - POOL:a - POOL + 0x40])
        thing(r, t, handles)
        things[a - POOL:a - POOL + 0x40] = t
    regions[POOL] = bytes(things)
    items = bytearray(regions[ITEMS])
    n = struct.unpack_from('<i', regions[DEFS], 0x528)[0]
    ids = [struct.unpack_from('<H', items, ITEM * i)[0] for i in range(n)]
    for i in range(NITEMS):
        put(items, i * ITEM + 0x288, 'I', r.choice([1, 1, 0, 2, r.getrandbits(32)]))
    regions[ITEMS] = bytes(items)
    obj = bytearray(r.bytes(0x40))
    thing(r, obj, handles)
    put(obj, 0xe, 'H', r.choice(ids))
    regions[OBJ] = bytes(obj)
    selected = r.choice([OBJ, OBJ, OBJ, OBJ, 0, addrs[0]])
    regions[0x7c2654] = struct.pack('<I', selected)
    regions[0x81ae2c] = struct.pack('<I', r.choice([0x19, 0x1a, 0x19, 0x1a, 0, 5, r.getrandbits(32)]))
    steps = bytearray(r.bytes(0x58))
    if r.random() < 0.7:
        put(steps, 0x50, 'II', r.randrange(0x80), r.randrange(0x80))
    else:
        put(steps, 0x50, 'I', 0xffffffff)
    regions[STEPS] = bytes(steps)
    focus = bytearray(r.bytes(0x30))
    put(focus, 0x14, 'I', r.choice([0, 0, OBJ, addrs[-1]]))
    if r.random() < 0.4:  # the cell word already where the object (or the set cell) is
        src = obj if selected == OBJ else bytes(0x40)
        put(focus, 0, 'H', ((src[7] << 7) + src[5] + 1) & 0xffff if r.random() < 0.6 else ((steps[0x54] << 7) + steps[0x50] + 1) & 0xffff)
    regions[FOCUS] = bytes(focus)
    regions[0x818c50] = r.bytes(4)
    rec = actionrec.writer(r)
    regions[0x785a08] = rec.pop(actionrec.THIS)
    regions.update(rec)
    flag = r.choice([0, 1, 1, 1, 0x100, r.getrandbits(32)])
    return Call(0x5276d0, [flag], regions)


if __name__ == '__main__':
    B.main(case)
