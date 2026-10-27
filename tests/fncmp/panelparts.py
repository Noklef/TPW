# The panels' dressers (src/game/ui/panelparts' steps: each panel's slot 3 in src/game/ui/panel7c*, the dialog [0x7cc1f8]'s stats in
# src/game/ui/dialog7cc1f8) against the originals: dialogbuild.py's chain() surroundings (listrows.py's of a text part's construction, the
# heap, the fonts of group 0-2, the GUI's size words, the renderer; the text parts' unported slots 3 / 9 recorders), the window dressed
# (the panel's +0x14, now and then none; the dialog's child 0x1ed among 0-2 decoys, now and then missing) with the children the dresser
# looks up, each a dialogbuild.py kid() (random bytes, a recorder vtable for slots 5 / 7, its old impl +0x9c none or an old one, +0x98 /
# +0x4c none or objects, shown now and then) with its rectangle tiny (the parts' pixels), shuffled among 0-2 decoys (now and then one of
# them missing: a fault in both, but for Panel7cc408_Dress's 0x3732, which it checks, missing a quarter of the time, and Panel7ca228_Dress's and Panel7cc3e0_Dress's, which check each: one missing 30% of the time); the language texts
# the dresser asks for (see texts()); the bars' meshes as barmesh.py's Overlay builds them (global_heap(), the texture directories and holder logged) with their
# vtable 0x6feeb8's slots 3 / 5 (Window_SetImpl's) recorders. The heap fails one allocation now and then (a text part's or a bar's:
# none set; one inside a mesh's construction faults in both). The windows', parts' and meshes' bytes after the call are compared.
# The rows' initialisers of src/game/ui/listrows (ROWS below): the same surroundings with the row dialogbuild.ROW (its first child +0x5c
# and their siblings +0x54 the kids as above, one too few now and then: a fault in both), their font dword (7 mostly, 0-12), for
# List7ca158_InitRow the state [0x7ca154] (0-3 mostly), for List7ca140_InitRow the GUI's registered windows of listrows.py's register()
# with "ti_hungry" among them or not.
# The panels' slot 4 (Panel*_DressTip: 0x496f90, 0x4ad720, 0x4afaa0, 0x4b0ed0, the same code): the same surroundings, the window the
# panel's +0x1c (now and then none) with only decoy children.
# usage: panelparts.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Panel7cc218_Dress,List7ca158_InitRow,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import animatormesh  # noqa: E402
import dialogbuild  # noqa: E402
import listrows  # noqa: E402
import window  # noqa: E402
from fncmp import CDECL_LOG4, HEAP, Bench, Call, global_heap, heap, put, recorder, vtable  # noqa: E402
from language import text  # noqa: E402

# each dresser: (its address, the ids of the children it looks up, the language texts it asks for, how the window is reached)
DRESSERS = {
    'Panel7cc218_Dress': (0x4ad890, (0x3e20, 0x3e1a, 0x3e1c, 0x3e1e, 0x3e1f, 0x3e1d, 0x3e22, 0x3e21, 0x3e1b, 0x3e16, 0x3e18, 0x3e19, 0x3e17,
        0x3e23), (0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17), 'panel'),
    'Panel7cc258_Dress': (0x4afc10, (0xc075, 0xc071, 0xc072, 0xc073, 0xc074, 0xc07a, 0xc07b, 0xc070, 0xc076, 0xc077, 0xc078, 0xc079, 0xc07d,
        0xc07c), (0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26), 'panel'),
    'Panel7cc298_Dress': (0x4b1040, (0xa09c, 0xa095, 0xa09a, 0xa098, 0xa09b, 0xa097, 0xa0a1, 0xa09f, 0xa09d, 0xa096, 0xa093, 0xa092, 0xa099,
        0xa094, 0xa0a0, 0xa09e), (0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30), 'panel'),
    'Panel7cc3b0_Dress': (0x4b53c0, (0x51a, 0x51d, 0x519, 0x518, 0x51c, 0x516, 0x51b, 0x513, 0x514, 0x51e, 0x515, 0x517), (0x40, 0x41, 0x42,
        0x43, 0x44, 0x45), 'panel'),
    'Panel7cc408_Dress': (0x4b6c20, (0x372c, 0x3725, 0x3726, 0x3727, 0x3728, 0x3729, 0x372a, 0x372b, 0x3724, 0x372d, 0x372e, 0x372f, 0x3730,
        0x3731, 0x3732, 0x3733), (0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d), 'panel'),
    'Panel7ca178_Dress': (0x497100, (0x15bbe, 0x15bbc, 0x15bbf, 0x15bbd, 0x15bbb, 0x15bc0), (0x3c, 0x3d, 0x3f), 'panel'),
    'Dialog7cc1f8_DressStats': (0x4ab6b0, (0x1f4, 0x1f3, 0x1f1, 0x1f6, 0x1f7, 0x1f5, 0x1f2, 0x1ee, 0x1ef, 0x1f0), (0x7d, 0x7e, 0x7f, 0x80,
        0x81, 0x88), 'dialog'),
    'Panel7ca228_Dress': (0x499890, (0x1587, 0x1586, 0x1585, 0x1584), (0x35, 0x36), 'panel'),
    'Panel7cc3e0_Dress': (0x4b6540, (0x5211, 0x5210, 0x520f, 0x520e), (0x37, 0x38), 'panel'),
    'Panel7cc328_Dress': (0x4b46a0, (0x1e8b, 0x1e8a), (0x38,), 'panel'),
    'Dialog7cb2fc_DressRow': (0x4a43b0, (0x1d4c9,), (), 'row7cb2fc'),
    'Dialog7cb2fc_DressSlider': (0x4a4490, (0x1d4c9,), (), 'row7cb2fc'),
    'Panel7ca178_DressTip': (0x496f90, (), (), 'tip'),
    'Panel7cc218_DressTip': (0x4ad720, (), (), 'tip'),
    'Panel7cc258_DressTip': (0x4afaa0, (), (), 'tip'),
    'Panel7cc298_DressTip': (0x4b0ed0, (), (), 'tip'),
}
CHECKED = {'Panel7ca228_Dress', 'Panel7cc3e0_Dress'}   # (they dress only the children found: any missing now and then)
# each row initialiser: (its address, its children, its font dword, the name it looks up)
ROWS = {'List7ca158_InitRow': (0x494690, 4, 0x750d54, None), 'List7ca140_InitRow': (0x4939d0, 5, 0x7508b4, 'ti_hungry'),
        'List7ca170_InitRow': (0x496d20, 4, 0x751184, None), 'List7cc338_InitRow': (0x4b4d40, 1, 0x756f38, None)}
CALLS = {k: v[0] for k, v in DRESSERS.items()}
CALLS.update({k: v[0] for k, v in ROWS.items()})
ALL_RANGES = sorted(set(window.RANGES + listrows.PART_RANGES + listrows.REG_RANGES + animatormesh.RANGES + [
    (0x424ec0, 0x425450), (0x46f150, 0x46f190), (0x477c00, 0x477c40), (0x477ff0, 0x478380), (0x485a70, 0x485ab0), (0x485b00, 0x485b50),
    (0x485e30, 0x485ed0), (0x48f150, 0x48f290), (0x48f6a0, 0x48f750), (0x48fde0, 0x48fe90), (0x490250, 0x490300), (0x490e60, 0x490f10),
    (0x497100, 0x497391), (0x496f90, 0x497056), (0x4ad720, 0x4ad7e6), (0x4afaa0, 0x4afb66), (0x4b0ed0, 0x4b0f96), (0x65f16b, 0x65f246), (0x494690, 0x494c60), (0x4939d0, 0x493c03), (0x496d20, 0x496f21), (0x490c30, 0x490ce0), (0x4b4d40, 0x4b4e22),
    (0x65c388, 0x65c55a), (0x65d547, 0x65d58c), (0x4ab6b0, 0x4abb10), (0x4ad890, 0x4ade50), (0x4afc10, 0x4b01e0), (0x4b1040, 0x4b16d0), (0x4b53c0, 0x4b58c0), (0x4b6c20, 0x4b72b0),
    (0x499890, 0x499a6a), (0x4b6540, 0x4b671a), (0x4b46a0, 0x4b479c), (0x4a43b0, 0x4a4484), (0x4a4490, 0x4a457b), (0x66b47d, 0x66b508),
    (0x5775d0, 0x577610), (0x588320, 0x588360), (0x588d70, 0x588da0), (0x6597b0, 0x659896), (0x659d00, 0x659d53), (0x659d90, 0x659dbc),
    (0x65a43e, 0x65a4a3), (0x65c388, 0x65c438), (0x65cfd4, 0x65d00f), (0x65d1c8, 0x65d333), (0x65d42e, 0x65d4b0), (0x65d773, 0x65d832),
    (0x65f5b7, 0x65f60e), (0x6616b0, 0x661710), (0x6617f0, 0x661820), (0x6ace60, 0x6aceb0), (0x6adc80, 0x6add40), (0x6ade00, 0x6adea0)]))
B = Bench(ALL_RANGES, hooks=animatormesh.HOOKS)
KIDS, PANEL, DIALOG, TEXTS, MANAGER = 0x31000000, 0x31010000, 0x31020000, 0x31100000, 0x31030000
KID = dialogbuild.KID
MESH_VTABLE = 0x6feeb8


def texts(r, regions, indexes):
    """the language object's converter (on or off) and record table [0x78ba94] about the indexes (their pointers from the lowest on, the
    count about the highest), a record of language.py's for each index (its own: Language_WideText's result is the wide buffer or none, so
    which index a child is given shows only in whether its record is terminated and counted)"""
    lang = regions[0x78b530] = bytearray(regions[0x78b530]) + bytearray(r.bytes(0x600 - len(regions[0x78b530])))
    conv = bytearray(r.bytes(0x10))
    put(conv, 8, 'BBHI', r.choice([0x80, 0x81, 0xa0, 0, 0xff, r.getrandbits(8)]), r.getrandbits(8), r.choice([0, 3, 0x80, 0x400, 0x7ff]), TEXTS + 0x4000)
    put(lang, 0x54c, 'II', TEXTS + 0x800, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    low, high = min(indexes), max(indexes)
    table = bytearray(r.bytes(0x20))
    put(table, 8, 'II', r.choice([high + 1] * 4 + [r.randrange(low, high + 2), 0]), TEXTS + 0xb00 - 4 * low)
    put(lang, 0x564, 'I', TEXTS + 0xa00)
    pool = bytearray()
    for i in range(high + 1 - low):
        body = text(r) + bytes([0]) * r.choice([1, 1, 2])
        head = r.choice([1] * 6 + [2, 0x101]) | (len(body) << 8 if r.random() < 0.9 else r.randrange(0x40) << 8)
        rec = bytearray(struct.pack('<I', head) + body + bytes(0x40 - 4 - len(body)))
        if r.random() < 0.15:
            rec[4 + ((len(body) + 3) & ~3)] = 1   # (not terminated)
        pool += rec
    regions.update({TEXTS + 0x800: conv, TEXTS + 0x4000: r.bytes(0x1000), 0x7ba058: bytes([0x55]) * 0x404, TEXTS + 0xa00: table,
                    TEXTS + 0xc00: bytes(pool),
                    TEXTS + 0xb00: struct.pack('<%dI' % (high + 1 - low), *[TEXTS + 0xc00 + 0x40 * i for i in range(high + 1 - low)])})


def kids(r, regions, items, root):
    """the windows `items` (their addresses and ids) children of `root` in that order (+0x5c, +0x54), each a dialogbuild.py kid()"""
    for i, (at, ident) in enumerate(items):
        w = dialogbuild.kid(r, root)
        left, top = r.randrange(-5, 20), r.randrange(-5, 20)
        put(w, 0x30, '4h', left, top, left + r.randrange(0, 8), top + r.randrange(0, 8))
        put(w, 0x8c, 'I', ident)
        w[0x48] = (w[0x48] & 0xfe) | (r.random() < 0.25)   # (shown less often than kid()'s: each shown child's new impl is told, room in the log)
        put(w, 0x54, 'I', items[i + 1][0] if i + 1 < len(items) else 0)
        regions[at] = w
    put(regions[root], 0x5c, 'I', items[0][0] if items else 0)


def surroundings(r, regions):
    regions[HEAP] = heap(0x8000, r.choice([0] * 12 + [r.randrange(1, 15)]))
    regions.update(global_heap())
    regions[MESH_VTABLE + 0xc] = struct.pack('<I', recorder(3, 2))
    regions[MESH_VTABLE + 0x14] = struct.pack('<I', recorder(5, 1))


def row_case(r, op):
    address, n, font, name = ROWS[op]
    regions = dialogbuild.chain(r, op, 0, name=name, font=font)
    surroundings(r, regions)
    if op == 'List7ca158_InitRow':
        regions[0x7ca154] = struct.pack('<I', r.choice([0, 1, 2, 3] * 3 + [4, r.getrandbits(32)]))
        n += 1   # (its state 2's)
    n -= r.random() < 0.03
    kids(r, regions, [(KIDS + KID * k, r.getrandbits(32)) for k in range(n)], dialogbuild.ROW)
    return Call(address, [dialogbuild.ROW], regions, ret='v')


def case(r):
    op = r.deal(OPS)
    if op in ROWS:
        return row_case(r, op)
    address, ids, indexes, reach = DRESSERS[op]
    regions = dialogbuild.chain(r, op, 0, font=0x7537b4 if reach == 'row7cb2fc' else None)
    surroundings(r, regions)
    if indexes:
        texts(r, regions, indexes)
    root = dialogbuild.ROW
    items = [(KIDS + KID * k, ident) for k, ident in enumerate(ids)]
    if items and r.random() < (0.3 if op in CHECKED else 0.03):
        items.remove(r.choice(items))
    elif op == 'Panel7cc408_Dress' and r.random() < 0.25:   # (its 0x3732 is looked for: a bar only when found)
        items.remove((KIDS + KID * ids.index(0x3732), 0x3732))
    items += [(KIDS + KID * (len(ids) + k), r.choice([0, 1, (ids[0] if ids else 0x1ed) + 0x100, r.getrandbits(32)])) for k in range(r.choice([0, 0, 1, 2]))]
    r.shuffle(items)
    kids(r, regions, items, root)
    if reach == 'tip':   # (the panel's window +0x1c the one dressed, with only decoys for children, childless (the depth goes down to them); now and then none)
        for at in [root] + [at for at, _ in items]:   # (hidden: each told by its handler +0x114, a logger or none)
            put(regions[at], 0x114, 'I', r.choice([0, CDECL_LOG4]))
            if at != root:
                put(regions[at], 0x5c, 'I', 0)
        regions[0x77c48c] = struct.pack('<I', MANAGER)   # (the manager told of the hidden window: its slot 13 a recorder)
        regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
        regions[MANAGER + 0x40] = vtable({13: 1})
        regions[0xfaa540] = r.bytes(8)   # (the hover: no root, or one busy (+0x10))
        regions[0xfaa5d8] = struct.pack('<5I', r.choice([0, root]), r.getrandbits(32), 0, r.getrandbits(32), r.randrange(1, 1 << 32))
        regions[0x6ff320 + 0xc] = struct.pack('<I', recorder(3, 2))   # (the new text part told: its slot 3 a recorder too)
        regions[PANEL] = put(bytearray(r.bytes(0x20)), 0x1c, 'I', root if r.random() < 0.95 else 0)
        return Call(address, [PANEL], regions, conv='t', ret='v')
    if reach == 'panel':
        regions[PANEL] = put(bytearray(r.bytes(0x20)), 0x14, 'I', root if r.random() < 0.95 else 0)
        return Call(address, [PANEL], regions, conv='t', ret='v')
    # the dialog [0x7cc1f8] (or [0x7cb2fc]): a window whose children are the window dressed (its id 0x1ed, or the row asked for; now
    # and then missing) among 0-2 decoys
    rowed = reach == 'row7cb2fc'
    ident = r.choice([0x1d4cb, 0x1d4cc, 0x1d4cd, 0x1d4d0, r.getrandbits(32)]) if rowed else 0x1ed
    if rowed:   # (the row a 0x705b20 slider: its range +0x138 / +0x13c and step +0x144 past a kid()'s bytes)
        regions[root] = put(bytearray(regions[root]) + bytearray(r.bytes(0x160 - len(regions[root]))), 0x138, 'iiii',
                            r.choice([100, 100, 0, r.getrandbits(31)]), r.choice([0, 0, 1, r.getrandbits(31)]), r.getrandbits(31),
                            r.choice([1, -1, 5, -5, 0, r.getrandbits(31)]))
    dialog = regions[DIALOG] = dialogbuild.kid(r, 0)
    put(regions[root], 0x8c, 'I', ident)
    tops = [(root, ident)] if r.random() < 0.95 else []
    tops += [(DIALOG + 0x200 * (k + 1), r.choice([0, ident - 1, ident + 7, r.getrandbits(32)])) for k in range(r.choice([0, 1, 2]))]
    r.shuffle(tops)
    for i, (at, top) in enumerate(tops):
        if at != root:
            regions[at] = put(dialogbuild.kid(r, DIALOG), 0x8c, 'I', top)
        put(regions[at], 0x54, 'I', tops[i + 1][0] if i + 1 < len(tops) else 0)
    put(dialog, 0x5c, 'I', tops[0][0] if tops else 0)
    if rowed:
        regions[0x7cb2fc] = struct.pack('<I', DIALOG)
        return Call(address, [ident] + ([r.choice([r.randrange(0, 101), r.getrandbits(32)])] if op.endswith('Slider') else []), regions,
                    ret='v')
    regions[0x7cc1f8] = struct.pack('<I', DIALOG)
    return Call(address, [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
