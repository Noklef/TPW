# The dialog [0x7cc1f8]'s stats shown or hidden (src/game/ui/dialog7cc1f8: 0x4abfb0) against the original. ShowStats: window.py's shown_env
# tree, its root the stats child (id 0x1ed mostly, else another) and its root's children given the eight stats ids mostly (0x1ed, 0x1f4, a
# high bit or any now and then; repeats and misses); deeper windows any id or one of them. The dialog's children: the root, a decoy (id any or
# 0x1ed, childless) then the root, the decoy alone or none. The mode 0-3 mostly, 4, -1 or any.
# ItemTip (0x4abc80, cdecl (item)): the dialog's one child the list 0x1f8 (its saved rectangle +0x2c), the mouse (0xfaa540 / 0xfaa544)
# inside it mostly; the tip state [0x75513c] -1 to 4 or any, the id tipped [0x7cc1e0] the item's mostly. The item -1, -2, below them (then
# only the one tipped: another leaves the stack buffer as it is, sic) or an object id: the object definitions 0x7890a0 (1-3, objectunlock.py's,
# the item's found by the cache or the list; its +0xc4 0 or not, its name +0x90 any text id but 0x94-0x96), the control table (the game's
# +0x1d943c, as panel7cc218choose.py's Tile: the cost +4 about the balance of its money()), the global save info (globalsave.py's count()
# for CountLeft and objectunlock.py's key_set(), holding the id or not; moved past window.py's addresses). The help background as
# helpbg.py's SetText (notify_env(), slot 9 the recorder hashing the text's first 18 bytes: the formatter's 8 hex digits). The formatter
# 0x6acd60 the log stub, its one argument hashed for the texts 0x94-0x96 only (text_args(); the name's text that argument), its buffer (the
# stack's) logged as 0.
# usage: dialog7cc1f8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ShowStats,ItemTip)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cc5f4  # noqa: E402
import globalsave  # noqa: E402
import helpbg  # noqa: E402
import objectunlock  # noqa: E402
import panel7cc218choose as choose  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _cdecl_log, _cdecl_log4, blob_bytes, log_region, put, strings, text_args, vtable  # noqa: E402
from window import RANGES, VTABLE, notify_blobs, notify_env, shown_env  # noqa: E402

CALLS = {'ShowStats': 0x4abfb0, 'ItemTip': 0x4abc80}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(dialog7cc5f4.merged(RANGES + [
    (0x4abfb0, 0x4ac25e),
    (0x401030, 0x401290), (0x4013e0, 0x401710), (0x4017a0, 0x402730), (0x405710, 0x405760), (0x409410, 0x409490), (0x412e90, 0x412f00),
    (0x4924b0, 0x492540), (0x4abc80, 0x4abfc0), (0x4d3d10, 0x4d3d90), (0x4d4b70, 0x4d4bd0), (0x5195d0, 0x519620), (0x5aef50, 0x5af0f0),
    (0x5af5d0, 0x5af690), (0x5af780, 0x5af800), (0x5b11c0, 0x5b12f0), (0x5b1680, 0x5b16c0), (0x5b2480, 0x5b2540), (0x5b2fd0, 0x5b3000),
    (0x5b30b0, 0x5b3140), (0x5f5660, 0x5f56a0), (0x5f5710, 0x5f5960), (0x5f5ad0, 0x5f5bc0), (0x5f5c70, 0x5f5ce0), (0x5f5e20, 0x5f5e60),
    (0x6589f9, 0x658abb), (0x658ccc, 0x658daf), (0x65958a, 0x6595b8), (0x659710, 0x659740), (0x659d00, 0x659d53), (0x659d90, 0x659dbc),
    (0x659e83, 0x659ef2), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x65c428, 0x65c4c9), (0x65d4f6, 0x65d531), (0x65d773, 0x65d832),
    (0x65d90b, 0x65da9d), (0x65db25, 0x65dc00), (0x6697e3, 0x6698b4), (0x66bff7, 0x66c01b), (0x67a520, 0x67a600), (0x67a650, 0x67a6d0),
    (0x67acd0, 0x67ad00), (0x67ae00, 0x67af10), (0x67c2f0, 0x67c330), (0x680ad0, 0x6816b0), (0x683aa0, 0x683ad0), (0x686440, 0x6864d0),
    (0x686a20, 0x686ab0), (0x687140, 0x6871c0), (0x687530, 0x6875a0), (0x68c320, 0x68c450), (0x6acb70, 0x6accb0), (0x6ad810, 0x6ad830),
    (0x6b00e0, 0x6b0140), (0x5da3c0, 0x5da3c1)]),
    hooks={0x6acd60: _cdecl_log4, 0x691de0: globalsave.lock, 0x691ea0: lambda e: None, 0x67a7d0: _cdecl_log})
DIALOG, DECOY = RAM + 0xb000, RAM + 0xb200
LIST, ITEM, DEFS = RAM + 0xb400, RAM + 0xb600, RAM + 0x56000
SAVE_MOVE = 0x50000   # (the global save's regions, RAM to RAM + 0x10000 in globalsave.py / objectunlock.py, moved by this)
IDS = [0x1f3, 0x1f1, 0x1f6, 0x1f7, 0x1ee, 0x1f2, 0x1ef, 0x1f0]


def moved(regions, delta, lo=RAM, hi=RAM + 0x10000):
    """regions with those at lo..hi moved by delta and every aligned dword pointing there too"""
    out = {}
    for at, data in regions.items():
        b = bytearray(data)
        for i in range(0, len(b) - 3 if at % 4 == 0 else 0, 4):
            v = struct.unpack_from('<I', b, i)[0]
            if lo <= v < hi:
                put(b, i, 'I', v + delta)
        out[at + delta if lo <= at < hi else at] = b
    return out


def item_tip(r):
    log = log_region()
    regions = {LOG: log}
    wins = notify_env(r, regions, log)
    notify_blobs(log, 0, 0x30)
    regions[VTABLE] = vtable({5: 1, 3: 2, 0: 1, 9: 1})
    blob_bytes(log, 9, 0, 0x3ffff)   # (the text: the formatter's 8 hex digits and the terminator, on the stack)
    text_args(log, 1, (0x94, 0x96))
    strings(log, 0x3d, 0, 2)   # (the formatter's buffer: a stack address, logged as 0)
    bg =put(bytearray(r.bytes(0x20)), 0, 'I', helpbg.BG_VTABLE)
    for off in (4, 0xc):
        put(bg, off, 'H', r.choice([1, 1, 1, 1, 0x100, 0, r.getrandbits(16)]))
    window = r.choice(wins)
    put(regions[window], 0, 'I', VTABLE)
    put(bg, 0x18, 'I', window)
    regions[0x7ca0c0] = bg
    regions[helpbg.BG_VTABLE] = vtable({}, size=8)
    # the list and the mouse
    regions[0x7cc1f8] = struct.pack('<I', DIALOG)
    regions[DIALOG] = put(bytearray(r.bytes(0x120)), 0x5c, 'I', LIST)
    left, top = r.choice([0, 10, 300, 0x7e00, -20]), r.choice([0, 10, 200, -5])
    right, bottom = left + r.choice([1, 50, 200, 0]), top + r.choice([1, 40, 300, 0])
    lst = put(put(put(bytearray(r.bytes(0x120)), 0x2c, 'I', 0x705920), 0x30, 'hhhh', left, top, right, bottom), 0x54, 'I', 0)
    regions[LIST] = put(lst, 0x8c, 'I', 0x1f8)
    if r.random() < 0.8:
        x, y = left + r.randrange(max(right - left, 1)), top + r.randrange(max(bottom - top, 1))
    else:
        x, y = r.choice([left - 1, right, left, r.getrandbits(15)]), r.choice([top - 1, bottom, top, r.getrandbits(15)])
    regions[0xfaa540] = struct.pack('<hH', x, r.getrandbits(16))
    regions[0xfaa544] = struct.pack('<hH', y, r.getrandbits(16))
    regions[0x78ba98] = struct.pack('<I', r.getrandbits(32))
    regions[0x78bad0] = struct.pack('<I', r.getrandbits(32))
    # the item
    n = r.randrange(1, 4)
    ids = r.sample(range(0x8000), n)
    k = r.randrange(n)
    kind = r.choice(['object'] * 6 + ['-1', '-2', 'below'])
    item = {'object': ids[k], '-1': -1, '-2': -2, 'below': r.choice([-3, -4, -0x8000])}[kind]
    regions[ITEM] = struct.pack('<hH', item, r.getrandbits(16))
    tipped = item if kind == 'below' or r.random() < 0.4 else r.choice([-1, -2, ids[k] ^ 1, r.getrandbits(16)])
    regions[0x7cc1e0] = struct.pack('<hH', (tipped + 0x8000) % 0x10000 - 0x8000, r.getrandbits(16))
    regions[0x75513c] = struct.pack('<i', r.choice([-1, 0, 1, 2, 3, 4, 2, 3, r.getrandbits(32) - (1 << 31)]))
    # the definitions, the control, the money
    table = bytearray(r.bytes(0x740))
    for i in range(n):
        put(table, 4 * i, 'I', DEFS + 0x100 * i)
        d = put(bytearray(r.bytes(0x100)), 0, 'H', ids[i])
        put(d, 0xc4, 'i', r.choice([0, 0, 0, 1, 2, 4, 8, -1, r.getrandbits(31), -r.getrandbits(31)]))
        name = r.choice([0x93, 0x97, 0x1000, r.getrandbits(32)])
        put(d, 0x90, 'I', name if not 0x94 <= name <= 0x96 else 0x97)
        regions[DEFS + 0x100 * i] = d
    put(table, 0x528, 'i', n)
    put(table, 0x730, 'HHI', r.choice([ids[k], ids[k], r.getrandbits(16)]), 0, DEFS + 0x100 * k)
    regions[0x7890a0] = table
    balance = choose.money(r, regions)
    control = bytearray(0x12d0)
    count = r.choice([1, 2, 4])
    for i in range(count):
        put(control, 0x20 * i, 'H', r.getrandbits(16))
    if r.random() < 0.6:
        put(control, 0x12c4, 'HHI', ids[k], 0, choose.CTL)
    else:
        put(control, 0x12c4, 'H', (ids[k] + 1) & 0xffff)
        put(control, 0x20 * r.randrange(count), 'H', ids[k])
    put(control, 0x12c0, 'i', count)
    regions[choose.GAME + 0x1d943c] = control
    regions[choose.CTL] = put(bytearray(r.bytes(0x20)), 4, 'i', choose.cost(r, balance, r.random() < 0.5))
    # the global save
    save = dict(globalsave.count(r, 'CountLeft').regions)
    save[0xf7c58c] = struct.pack('<I', globalsave.INFO)
    save[LOG] = log
    objectunlock.key_set(r, save, ids[k] if r.random() < 0.97 else r.getrandbits(16))
    del save[LOG]
    regions.update(moved(save, SAVE_MOVE))
    return Call(CALLS['ItemTip'], [ITEM], regions, ret='v')


def case(r):
    if r.deal(OPS) == 'ItemTip':
        return item_tip(r)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    wins = shown_env(r, regions, log)
    root = wins[0]
    put(regions[root], 0x8c, 'I', 0x1ed if r.random() < 0.9 else r.choice([0x1ec, 0x101ed, r.getrandbits(32)]))
    c = struct.unpack_from('<I', regions[root], 0x5c)[0]
    while c:
        put(regions[c], 0x8c, 'I', r.choice(IDS * 3 + [0x1ed, 0x1f4, 0x101f3, r.getrandbits(32)]))
        c = struct.unpack_from('<I', regions[c], 0x54)[0]
    for w in wins[1:]:
        if r.random() < 0.3:
            put(regions[w], 0x8c, 'I', r.choice(IDS))
    decoy = put(put(put(bytearray(r.bytes(0x120)), 0x8c, 'I', r.choice([0x1ed, r.getrandbits(32)])), 0x5c, 'I', 0), 0x54, 'I', root)
    shape = r.choice(['root'] * 6 + ['decoy'] * 2 + ['lone', 'none'])
    if shape == 'lone':
        put(decoy, 0x54, 'I', 0)
    regions[DECOY] = decoy
    dialog = bytearray(r.bytes(0x120))
    put(dialog, 0x5c, 'I', {'root': root, 'decoy': DECOY, 'lone': DECOY, 'none': 0}[shape])
    regions[DIALOG] = dialog
    regions[0x7cc1f8] = struct.pack('<I', DIALOG)
    mode = r.choice([0, 1, 2, 3] * 3 + [4, 0xffffffff, r.getrandbits(32)])
    return Call(CALLS['ShowStats'], [mode], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
