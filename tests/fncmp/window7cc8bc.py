# The window [0x7cc8bc]'s name list and the handler of [0x7cc8c4] / [0x7cc8c8] (src/game/ui/window7cc8bc: 0x4beb60-0x4bedd1 and the
# stdcall wrappers 0x481820-0x4818b2) against the originals. The window [0x7cc8bc] (now and then none) has the child 0xc3123 whose child
# 0xc3124 is window7059f0.py's list_env() list (each among 0-2 decoys, now and then missing: a fault in both); the mode [0x7cc8d0] the
# op's mostly, else the other or 2. Add / Add1 (and the wrappers): window7059f0.py's add_case() with at most one column (the callers'
# texts are one stack slot: a second column would read the return address), its texts' first the text, its data the data. Remove /
# SetData (and 1, and the wrappers): list_env()'s cells strings of window7059f0.py's ALPHA, 0-3 long (the buffers' counts kept), the
# first column's bottom 0 mostly; the text one of them or another. Handler (0x4bed60): window.py's shown_env tree with its previous
# siblings +0x58 set as windowlinks.py's (Window_Unlink), the windows [0x7cc8cc] and [0x7cc8bc] of it (now and then none: a fault in
# both), [0x7cc8c4] the window or another, the window's default handler +0x118 fncmp's logger; the message 0x14, 0x10006 or another.
# usage: window7cc8bc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Add,Remove,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cc5f4  # noqa: E402
import window7059f0 as w7  # noqa: E402
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, log_region, put  # noqa: E402
from window import shown_env  # noqa: E402

CALLS = {'Add': 0x4beb60, 'Remove': 0x4bebb0, 'SetData': 0x4bec00, 'Add1': 0x4bec60, 'Remove1': 0x4becb0, 'SetData1': 0x4bed00,
         'AddStd': 0x481820, 'RemoveStd': 0x481840, 'SetDataStd': 0x481850, 'Add1Std': 0x481870, 'Remove1Std': 0x481890,
         'SetData1Std': 0x4818a0, 'Handler': 0x4bed60}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(dialog7cc5f4.merged(w7.LIST_RANGES + [(0x4beb60, 0x4bedd1), (0x481820, 0x4818b3), (0x65f538, 0x65f5b7), (0x65e59b, 0x65e60a),
                                                (0x65f66e, 0x65f6e1)]))
TOP, MIDDLE, DECOYS, TEXT = RAM + 0x30000, RAM + 0x30200, RAM + 0x30400, RAM + 0x30800


def among(r, regions, want, base):
    """want (0x100 bytes from it on, its +0x54 set here) among 0-2 decoy windows at base (0x100 bytes each), linked by +0x54, now and then
    missing; returns the first or 0"""
    kids = [base + 0x100 * k for k in range(r.choice([0, 0, 1, 2]))]
    for k in kids:
        regions[k] = put(bytearray(r.bytes(0x100)), 0x8c, 'I', r.choice([1, 0xc3125, r.getrandbits(32)]))
    if r.random() < 0.98:
        kids.insert(r.randrange(len(kids) + 1), want)
    for i, k in enumerate(kids):
        put(regions[k], 0x54, 'I', kids[i + 1] if i + 1 < len(kids) else 0)
    return kids[0] if kids else 0


def top(r, regions, mode):
    """[0x7cc8bc] (mostly TOP) and the mode [0x7cc8d0] (mostly mode); TOP's child 0xc3123 MIDDLE, whose child 0xc3124 the list"""
    regions[0x7cc8bc] = struct.pack('<I', TOP if r.random() < 0.92 else 0)
    regions[0x7cc8d0] = struct.pack('<I', mode if r.random() < 0.85 else r.choice([1 - mode, 2]))
    put(regions[w7.LIST3], 0x8c, 'I', 0xc3124)
    regions[MIDDLE] = put(bytearray(r.bytes(0x100)), 0x8c, 'I', 0xc3123)
    put(regions[MIDDLE], 0x5c, 'I', among(r, regions, w7.LIST3, DECOYS))
    regions[TOP] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', among(r, regions, MIDDLE, DECOYS + 0x200))


def named(r):
    """list_env() with its cells strings of ALPHA (the counted buffers' chars at +3 rewritten, their lengths +8 0-3) and the first
    column's bottom mostly 0, at least one column and each item's first cell an object (taken from the other columns' cells when none is
    unused: FindText reads it unchecked); returns (regions, the text: one of the strings or another)"""
    while True:
        window, regions, idx = w7.list_env(r)
        ncols = struct.unpack_from('<h', window, 0x164)[0]
        if ncols > 0:
            break
    links = list(struct.unpack('<%dI' % (len(regions[w7.LINKS3 - 0x20]) // 4), regions[w7.LINKS3 - 0x20]))
    used = {links[8 + i + 4 + c] for i in idx for c in range(ncols)}
    spare = [w7.OBJS3 + 0x10 * j for j in range(16) if w7.OBJS3 + 0x10 * j not in used]
    for i in idx:   # (an object from another column's cell when none is spare)
        for c in range(1, ncols):
            if spare:
                break
            for k in idx:
                if links[8 + k + 4 + c]:
                    spare.append(links[8 + k + 4 + c])
                    links[8 + k + 4 + c] = 0
                    break
        if links[8 + i + 4] == 0 and spare:
            links[8 + i + 4] = spare.pop()
    regions[w7.LINKS3 - 0x20] = struct.pack('<%dI' % len(links), *links)
    if r.random() < 0.9:   # (the count at most the items the walk from the first shown one reaches: past them it reads links as cells)
        put(window, 0x150, 'i', min(struct.unpack_from('<i', window, 0x150)[0], reach(links[8:], struct.unpack_from('<i', window, 0x160)[0])))
    texts = [tuple(r.choice(w7.ALPHA) for _ in range(r.choice([0, 1, 2, 3]))) for _ in range(8)]
    bufs, objs = bytearray(regions[w7.BUFS3]), bytearray(regions[w7.OBJS3])
    for k in range(8):
        t = texts[k]
        struct.pack_into('<%dH' % (len(t) + 1), bufs, 0x20 * k + 3, *t, 0)
    for j in range(16):
        p = struct.unpack_from('<I', objs, 0x10 * j + 4)[0]
        put(objs, 0x10 * j + 8, 'I', len(texts[(p - w7.BUFS3) // 0x20]) if p else 0)
    regions[w7.BUFS3], regions[w7.OBJS3] = bufs, objs
    cols = bytearray(regions[w7.COLS3])
    if r.random() < 0.8:
        put(cols, 0xa, 'h', 0)
    regions[w7.COLS3] = cols
    text = r.choice(texts + [(0x42, 0x41, 0x41, 0x41)])
    regions[TEXT] = struct.pack('<%dH' % (len(text) + 1), *text, 0)
    return regions, TEXT


def reach(links, item):
    """the items NextItem (0x663a93) steps through from item (as window7059f0.py's list_case for Clear)"""
    links = [v - (1 << 32) if v & 0x80000000 else v for v in links]
    n = 0
    while item >= 0 and n < 64:
        n += 1
        if links[item + 2] >= 0 or links[item] >= 0:
            item = links[item + 2] if links[item + 2] >= 0 else links[item]
        elif links[item + 3] < 0:
            item = -2
        else:
            item = links[item + 3]
            for _ in range(64):
                if not (links[item] < 0 and links[item + 3] >= 0):
                    break
                item = links[item + 3]
            item = links[item]
    return n


def handler_case(r):
    log = log_region()
    regions = {LOG: log}
    wins = shown_env(r, regions, log)
    for w in wins:
        put(regions[w], 0x58, 'I', 0)
    for w in wins:   # (each child list's previous links, as windowlinks.py's)
        prev, c = 0, struct.unpack_from('<I', regions[w], 0x5c)[0]
        while c:
            put(regions[c], 0x58, 'I', prev if r.random() < 0.95 else 0)
            prev, c = c, struct.unpack_from('<I', regions[c], 0x54)[0]
    window = r.choice(wins)
    put(regions[window], 0x118, 'I', CDECL_LOG4)
    regions[0xfaa5ec] = struct.pack('<I', r.choice(wins + [0]))
    regions[0x7cc8cc] = struct.pack('<I', r.choice(wins) if r.random() < 0.97 else 0)
    regions[0x7cc8bc] = struct.pack('<I', r.choice(wins) if r.random() < 0.97 else 0)
    regions[0x7cc8c4] = struct.pack('<II', r.choice([window, window, r.choice(wins), 0]), r.getrandbits(32))
    message = r.choice([0x14] * 4 + [0x10006] * 4 + [0x10005, 0x15, 4, r.getrandbits(32)])
    return Call(CALLS['Handler'], [window, message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Handler':
        return handler_case(r)
    mode = 1 if '1' in op else 0
    conv = 's' if op.endswith('Std') else 'c'
    if op.startswith('Add'):
        while True:
            c = w7.add_case(r, columns=(0, 1, 1, 1))
            if c.args[1]:
                break
        regions = c.regions
        top(r, regions, mode)
        text = struct.unpack_from('<I', regions[w7.ATEXTS])[0]
        return Call(CALLS[op], [text, c.args[2]], regions, conv=conv, ret='v', masks=c.masks)
    regions, text = named(r)
    top(r, regions, mode)
    args = [text] if 'Remove' in op else [text, r.getrandbits(32)]
    return Call(CALLS[op], args, regions, conv=conv, ret='v')


if __name__ == '__main__':
    B.main(case)
