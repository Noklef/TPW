# The panel [0x7cc258]'s opening (src/game/ui/panel7cc258: 0x4b0640, slot 0 of the vtable 0x6ffc68) against the original: panelparts.py's
# surroundings (dialogbuild.py's chain(): the heap, the fonts, the text parts' unported slots 3 / 9 recorders; the language texts 0x27 /
# 0x28) with the panel's window +0x10 the row whose children are the ones it looks up (0xc069 / 0xc06a hidden when the group counts under
# 2, the sliders 0xc06b / 0xc086 (0x705b20: their range and step bytes past a kid()'s), 0xc085 (0x705af0: its range and value), the texts
# 0xc07e / 0xc084 / 0xc083, the hover label 0xc06e (its object +0x98 set: Attach's rectangle path would message its new handler 0x48d2e0,
# which the driver cannot call)), each a dialogbuild.py kid(), shuffled among 0-2 decoys, now and then one missing (the checked ones are
# skipped; 0xc069 / 0xc06a / 0xc06e missing fault in both); the hidden windows' handlers +0x114 a logger or none, the manager's slot 13 a
# recorder, the hover as panelparts.py's tip; the panel's group +4 the staff (-1: the things of thingenv.py's park, their kinds staff or
# not, counted) mostly, else one that counts none; the selected thing [0x7cc288] one of the park's, its definition's ingredient kind +0x158
# (0-5, -1) from a small definition table at 0x7890a0, the ingredient records [0x78ba8c] (terminated or not) widened.
# usage: panel7cc258.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
import ridesweep  # noqa: E402
from fncmp import CDECL_LOG4, RAM, Bench, Call, put, recorder, vtable  # noqa: E402
from thingenv import park  # noqa: E402

CALLS = {'Open': 0x4b0640}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def merged(ranges):
    out = []
    for lo, hi in sorted(set(ranges)):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(panelparts.ALL_RANGES + ridesweep.SWEEP_RANGES + [
    (0x4b0640, 0x4b0947), (0x4e2200, 0x4e22a0), (0x4d91f0, 0x4d93c0), (0x412e90, 0x412f00), (0x485ac0, 0x485b00),
    (0x485d20, 0x485e30), (0x48d290, 0x48d36d), (0x48f690, 0x48f6b0), (0x65d9dd, 0x65da8d), (0x65e6d5, 0x65e73d), (0x66a5c4, 0x66abcf),
    (0x66b47d, 0x66b508), (0x65c5d5, 0x65c682), (0x659d00, 0x659d53), (0x659d90, 0x659df5), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b),
    (0x65db25, 0x65dc00), (0x65f5fe, 0x65f655), (0x6589f9, 0x658abb), (0x658ccc, 0x658e04), (0x659710, 0x659740), (0x659e3e, 0x659e93),
    (0x65aa20, 0x65aaa0), (0x65c388, 0x65c4c9), (0x65c7fe, 0x65c989), (0x65c9bd, 0x65cda5), (0x65d070, 0x65d1d8), (0x65e526, 0x65e559),
    (0x65f66e, 0x65f6e1), (0x6616b0, 0x661780), (0x6697e3, 0x6698b4)]))
ROOT = dialogbuild.ROW
KIDS, PANEL, MANAGER = 0x31000000, 0x31010000, 0x31030000
POOL = RAM + 0x40000
DEFTAB, ITEMS, ITEM = RAM + 0x48000, RAM + 0x48800, 0x200
RECTABLE, RECARRAY, RECORDS = RAM + 0x49800, RAM + 0x49900, RAM + 0x49a00
LOOKED = (0xc069, 0xc06a, 0xc06b, 0xc086, 0xc085, 0xc07e, 0xc084, 0xc083, 0xc06e)
CHECKED = {0xc06b, 0xc086, 0xc085, 0xc07e, 0xc084, 0xc083}
BIG = 0x160   # (the sliders' fields up to +0x148)


def things(r, regions):
    """the park (no definitions of its own: the table below), the staff group's walk, the selected thing and its definition"""
    env, handles, addrs = park(r, count=r.choice([0, 1, 2, 2, 3, 4]), kinds=(3, 3, 1, 2, 0xff, 0), defs=False, pool=POOL)
    regions.update(env)
    ids = [r.getrandbits(16) for _ in range(4)]
    for a in addrs:
        put(regions[POOL], a - POOL + 0xe, 'H', r.choice(ids))
    table = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(ITEM * 4))
    for i in range(4):
        put(table, 4 * i, 'I', ITEMS + ITEM * i)
        put(items, ITEM * i, 'H', ids[i])
        put(items, ITEM * i + 0x158, 'I', r.choice([0, 1, 2, 3, 4, 5, 0xffffffff]))
    put(table, 0x528, 'i', 4)
    put(table, 0x730, 'HxxI', r.getrandbits(16), ITEMS + ITEM * r.randrange(4))
    regions[0x7890a0], regions[ITEMS] = table, items
    regions[0x7cc288] = struct.pack('<I', r.choice(addrs) if addrs and r.random() < 0.97 else POOL)
    if not addrs:   # (a thing anyway: its id one of the table's or not)
        regions[POOL] = put(bytearray(r.bytes(0x400)), 0xe, 'H', r.choice(ids + [r.getrandbits(16)]))
    n = r.choice([0, 2, 4, 4])
    rt = bytearray(r.bytes(0x10))
    put(rt, 8, 'II', n, RECARRAY)
    array = bytearray(r.bytes(0x20))
    records = bytearray(r.bytes(0x40 * 4))
    for k in range(4):
        length = r.choice([0, 1, 4, 5, 8])
        put(array, 4 * k, 'I', RECORDS + 0x40 * k)
        put(records, 0x40 * k, 'I', (length << 8) | r.choice([1, 1, 1, 0, 2]))
        put(records, 0x40 * k + 4 + 4 * ((length + 3) >> 2), 'I', r.choice([0, 0, 1]))
    regions.update({0x78ba8c: struct.pack('<I', RECTABLE), RECTABLE: rt, RECARRAY: array, RECORDS: records})


def case(r):
    r.deal(OPS)
    regions = dialogbuild.chain(r, 'Open', 0)
    panelparts.surroundings(r, regions)
    panelparts.texts(r, regions, [0x27, 0x28])
    things(r, regions)
    ids = list(LOOKED)
    if r.random() < 0.06:
        ids.remove(r.choice(ids))
    ids += [r.choice([0, 1, 0xc06c, 0xc087, r.getrandbits(32)]) for _ in range(r.choice([0, 0, 1, 2]))]
    r.shuffle(ids)
    items = [(KIDS + BIG * k, ident) for k, ident in enumerate(ids)]
    panelparts.kids(r, regions, items, ROOT)
    for at, ident in items:
        w = regions[at] = bytearray(regions[at]) + bytearray(r.bytes(BIG - len(regions[at])))
        put(w, 0x114, 'I', r.choice([0, CDECL_LOG4]))
        put(w, 0x5c, 'I', 0)   # (childless: a hidden window's children are told too)
        if ident in (0xc06b, 0xc086):   # (0x705b20: range +0x138 / +0x13c, step +0x144)
            put(w, 0x138, 'iiii', r.choice([2, 100, 0, r.getrandbits(31)]), r.choice([0, 0, 1, r.getrandbits(31)]), r.getrandbits(31),
                r.choice([1, -1, 5, -5, 0, r.getrandbits(31)]))
        elif ident == 0xc085:   # (0x705af0: low +0x138, high +0x13c, value +0x140, step +0x148)
            put(w, 0x138, 'iii', r.choice([0, 5, -5]), r.choice([500, 100, 0]), r.choice([0, 250, 600, -3, r.getrandbits(31)]))
        elif ident == 0xc06e:
            put(w, 0x98, 'I', dialogbuild.listrows.OBJ98)
    put(regions[ROOT], 0x114, 'I', r.choice([0, CDECL_LOG4]))
    regions[0x77c48c] = struct.pack('<I', MANAGER)   # (the manager told of a hidden window: its slot 13 a recorder)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    regions[0xfaa540] = r.bytes(8)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([0, ROOT]), r.getrandbits(32), 0, r.getrandbits(32), r.randrange(1, 1 << 32))
    regions[0x6ff320 + 0xc] = struct.pack('<I', recorder(3, 2))   # (the new text parts told: their slot 3 a recorder too)
    regions[0x7c2658] = r.bytes(0x10)   # (the count's value and count)
    panel = bytearray(r.bytes(0x30))
    put(panel, 4, 'i', r.choice([-1] * 5 + [4, 0x3e9, -3]))
    put(panel, 0x10, 'I', ROOT)
    regions[PANEL] = panel
    return Call(CALLS['Open'], [PANEL], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
