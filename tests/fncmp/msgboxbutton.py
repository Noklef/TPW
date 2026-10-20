# The message box's button looks (src/game/ui/msgboxbutton: 0x48e610) against the original: listrows.py's init_case surroundings (the
# window: its object +0x98 none or one released, +0x4c, shown or not up its parents), its vtable a recorder (slot 5, one argument); the
# GUI's registered windows of listrows.py's register() with the kind's name among them or not; the kind 3-27 mostly, around them now and
# then.
# usage: msgboxbutton.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import listrows  # noqa: E402
from fncmp import RAM, Bench, Call, put, vtable  # noqa: E402
from window import RANGES  # noqa: E402

B = Bench(sorted(set(RANGES + listrows.PART_RANGES + listrows.REG_RANGES + [(0x48e610, 0x48e996)])))
WVT = RAM + 0xa600
NAMES = {3: 'mb_locate', 4: 'mb_locate', 18: 'b_loan', 19: 'mb_resrch', 20: 'b_pkinfo', 21: 'b_staffcost', 23: 'mb_callmech',
         24: 'b_entry', 26: 'mb_accept', 27: 'mb_decline'}
NAMES.update({k: 'mb_buy' for k in range(9, 18)})


def case(r):
    regions = dict(listrows.init_case(r, 'HilightList_InitRow').regions)
    kind = r.choice(list(range(3, 28)) * 3 + [0, 1, 2, 28, 29, 0xffffffff, r.getrandbits(32)])
    listrows.register(r, regions, NAMES.get(kind, r.choice(list(NAMES.values()))))
    put(regions[listrows.ROW], 0, 'I', WVT)
    regions[WVT] = vtable({5: 1})
    return Call(0x48e610, [listrows.ROW, kind], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
