# The panels' dressers (src/game/ui/panelparts' steps: each panel's slot 3 in src/game/ui/panel7cc*, the dialog [0x7cc1f8]'s stats in
# src/game/ui/dialog7cc1f8) against the originals: dialogbuild.py's chain() surroundings (listrows.py's of a text part's construction, the
# heap, the fonts of group 0-2, the GUI's size words, the renderer; the text parts' unported slots 3 / 9 recorders), the window dressed
# (the panel's +0x14, now and then none; the dialog's child 0x1ed among 0-2 decoys, now and then missing) with the children the dresser
# looks up, each a dialogbuild.py kid() (random bytes, a recorder vtable for slots 5 / 7, its old impl +0x9c none or an old one, +0x98 /
# +0x4c none or objects, shown now and then) with its rectangle tiny (the parts' pixels), shuffled among 0-2 decoys (now and then one of
# them missing: a fault in both, but for Panel7cc408_Dress's 0x3732, which it checks, missing a quarter of the time); the language texts
# the dresser asks for (see texts()); the bars' meshes as barmesh.py's Overlay builds them (global_heap(), the texture directories and holder logged) with their
# vtable 0x6feeb8's slots 3 / 5 (Window_SetImpl's) recorders. The heap fails one allocation now and then (a text part's or a bar's:
# none set; one inside a mesh's construction faults in both). The windows', parts' and meshes' bytes after the call are compared.
# usage: panelparts.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Panel7cc218_Dress,...)
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
from fncmp import HEAP, Bench, Call, global_heap, heap, put, recorder  # noqa: E402
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
    'Dialog7cc1f8_DressStats': (0x4ab6b0, (0x1f4, 0x1f3, 0x1f1, 0x1f6, 0x1f7, 0x1f5, 0x1f2, 0x1ee, 0x1ef, 0x1f0), (0x7d, 0x7e, 0x7f, 0x80,
        0x81, 0x88), 'dialog'),
}
CALLS = {k: v[0] for k, v in DRESSERS.items()}
B = Bench(sorted(set(window.RANGES + listrows.PART_RANGES + listrows.REG_RANGES + animatormesh.RANGES + [
    (0x424ec0, 0x425450), (0x46f150, 0x46f190), (0x477c00, 0x477c40), (0x477ff0, 0x478380), (0x485a70, 0x485ab0), (0x485b00, 0x485b50),
    (0x485e30, 0x485ed0), (0x48f150, 0x48f290), (0x48f6a0, 0x48f750), (0x48fde0, 0x48fe90), (0x490250, 0x490300), (0x490e60, 0x490f10),
    (0x4ab6b0, 0x4abb10), (0x4ad890, 0x4ade50), (0x4afc10, 0x4b01e0), (0x4b1040, 0x4b16d0), (0x4b53c0, 0x4b58c0), (0x4b6c20, 0x4b72b0),
    (0x5775d0, 0x577610), (0x588320, 0x588360), (0x588d70, 0x588da0), (0x6597b0, 0x659896), (0x659d00, 0x659d53), (0x659d90, 0x659dbc),
    (0x65a43e, 0x65a4a3), (0x65c388, 0x65c438), (0x65cfd4, 0x65d00f), (0x65d1c8, 0x65d333), (0x65d42e, 0x65d4b0), (0x65d773, 0x65d832),
    (0x65f5b7, 0x65f60e), (0x6616b0, 0x661710), (0x6617f0, 0x661820), (0x6ace60, 0x6aceb0), (0x6adc80, 0x6add40), (0x6ade00, 0x6adea0)])),
    hooks=animatormesh.HOOKS)
KIDS, PANEL, DIALOG, TEXTS = 0x31000000, 0x31010000, 0x31020000, 0x31100000
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


def case(r):
    op = r.deal(OPS)
    address, ids, indexes, reach = DRESSERS[op]
    regions = dialogbuild.chain(r, op, 0)
    regions[HEAP] = heap(0x8000, r.choice([0] * 12 + [r.randrange(1, 15)]))
    regions.update(global_heap())
    regions[MESH_VTABLE + 0xc] = struct.pack('<I', recorder(3, 2))
    regions[MESH_VTABLE + 0x14] = struct.pack('<I', recorder(5, 1))
    texts(r, regions, indexes)
    root = dialogbuild.ROW
    items = [(KIDS + KID * k, ident) for k, ident in enumerate(ids)]
    if r.random() < 0.03:
        items.remove(r.choice(items))
    elif op == 'Panel7cc408_Dress' and r.random() < 0.25:   # (its 0x3732 is looked for: a bar only when found)
        items.remove((KIDS + KID * ids.index(0x3732), 0x3732))
    items += [(KIDS + KID * (len(ids) + k), r.choice([0, 1, ids[0] + 0x100, r.getrandbits(32)])) for k in range(r.choice([0, 0, 1, 2]))]
    r.shuffle(items)
    for i, (at, ident) in enumerate(items):
        w = dialogbuild.kid(r, root)
        left, top = r.randrange(-5, 20), r.randrange(-5, 20)
        put(w, 0x30, '4h', left, top, left + r.randrange(0, 8), top + r.randrange(0, 8))
        put(w, 0x8c, 'I', ident)
        w[0x48] = (w[0x48] & 0xfe) | (r.random() < 0.25)   # (shown less often than kid()'s: each shown child's new impl is told, room in the log)
        put(w, 0x54, 'I', items[i + 1][0] if i + 1 < len(items) else 0)
        regions[at] = w
    put(regions[root], 0x5c, 'I', items[0][0] if items else 0)
    if reach == 'panel':
        regions[PANEL] = put(bytearray(r.bytes(0x20)), 0x14, 'I', root if r.random() < 0.95 else 0)
        return Call(address, [PANEL], regions, conv='t', ret='v')
    # the dialog [0x7cc1f8]: a window whose children are the window dressed (its id 0x1ed; now and then missing) among 0-2 decoys
    dialog = regions[DIALOG] = dialogbuild.kid(r, 0)
    put(regions[root], 0x8c, 'I', 0x1ed)
    tops = [(root, 0x1ed)] if r.random() < 0.95 else []
    tops += [(DIALOG + 0x200 * (k + 1), r.choice([0, 0x1ec, 0x1f4, r.getrandbits(32)])) for k in range(r.choice([0, 1, 2]))]
    r.shuffle(tops)
    for i, (at, ident) in enumerate(tops):
        if at != root:
            regions[at] = put(dialogbuild.kid(r, DIALOG), 0x8c, 'I', ident)
        put(regions[at], 0x54, 'I', tops[i + 1][0] if i + 1 < len(tops) else 0)
    put(dialog, 0x5c, 'I', tops[0][0] if tops else 0)
    regions[0x7cc1f8] = struct.pack('<I', DIALOG)
    return Call(address, [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
