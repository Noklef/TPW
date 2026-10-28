# The dialog at 0x7cb25c (src/game/ui/dialog7cb25c: 0x49f860) against the original. SetTexts: the dialog [0x7cb25c] (or none now and then)
# with its children 0x12c4bd (the panel) and 0x12c4ca (the title: its slot 9 a recorder) among 0-2 decoys, the panel's children 0x11, 0x12
# and 0x15 among 0-2 decoys (now and then one of the five missing: a fault in both), each a window of random bytes whose impl +0x9c is a
# recorder object (slot 3: its event's second event's dword +0 hashed, the text) or none; the flag [0x7cb260] 0, 1 or any; the language
# texts as panelparts.py's texts() makes them. FillList (0x49fa00): window7059f0.py's list_env() list of 1-6 columns (the first a
# string column mostly, the rest number columns), its sort column one of them (a string or number one); the money record (the table 0x7cfb90's entry for the world
# [0x7cf83c]'s word +0x1da726) of eight offers: amounts about the credit (the analyser's kept score, slot 0, fresh: no scan) or any, terms,
# rates, flags (taken) and months paid any, texts (+0x1c) records of the language's table [0x78bac4] (panelparts.py's texts(), their
# converter off mostly: then the wide buffer 0x7ba058 a short string of its own, else the first column a number one); the flag [0x7cb260] 0
# or 1 mostly. The new strings' allocator bytes masked (the items added, the list's growth and the strings worked out as the original's).
# Refresh (0x4a0770): FillList's, the list the child 0x12c4bd of the dialog [0x7cb25c] (among 0-2 decoys; none now and then), or no dialog.
# usage: dialog7cb25c.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetTexts,FillList,Refresh)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import panelparts  # noqa: E402
import window7059f0 as wl  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, Mask, blob_bytes, log_region, put, vtable  # noqa: E402

CALLS = {'SetTexts': 0x49f860, 'FillList': 0x49fa00, 'Refresh': 0x4a0770}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(sorted(set(wl.LIST_RANGES + [
    (0x485b00, 0x485b40), (0x49f860, 0x49f9ff), (0x65d7bc, 0x65d832), (0x65f5b7, 0x65f5fe), (0x6ace60, 0x6aceb0), (0x6adc80, 0x6add40),
    (0x41a990, 0x41a9c0), (0x47db20, 0x47db90), (0x485ac0, 0x485b00), (0x49fa00, 0x49fb40), (0x4a0770, 0x4a07a0), (0x4c7db0, 0x4c8160),
    (0x4d06b0, 0x4d0760), (0x4f0240, 0x4f0260), (0x519510, 0x519560), (0x5195d0, 0x519620), (0x535db0, 0x535de0), (0x536300, 0x536330),
    (0x658ccc, 0x658e04), (0x659710, 0x659740), (0x65d773, 0x65d7bc), (0x65d9dd, 0x65da9d), (0x6649d5, 0x664b78), (0x6697e3, 0x6698b4),
    (0x66b47d, 0x66b518), (0x5da3c0, 0x5da3c1)])))   # (tools/ranges.py 4a0770, window7059f0.py's)
ROOT, PANEL, NODES, VT, IMPL, IMPLVT = RAM + 0x1000, RAM + 0x1200, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200
NODE = 0x120


def window(r, ident):
    buf = bytearray(r.bytes(NODE))
    put(buf, 0, 'I', VT)
    put(buf, 0x54, 'I', 0)
    put(buf, 0x5c, 'I', 0)
    put(buf, 0x8c, 'I', ident)
    put(buf, 0x9c, 'I', r.choice([IMPL, IMPL, IMPL, 0]))
    return buf


def kids(r, regions, parent, ids, at):
    """the children `ids` of `parent` (one now and then missing) among 0-2 decoys, laid out from `at`"""
    ids = list(ids)
    if r.random() < 0.04:
        ids.remove(r.choice(ids))
    ids += [r.choice([0, 0x13, 0x12c4be, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(ids)
    head = 0
    for i, ident in reversed(list(enumerate(ids))):
        address = PANEL if ident == 0x12c4bd else at + NODE * i
        regions[address] = put(window(r, ident), 0x54, 'I', head)
        head = address
    put(regions[parent], 0x5c, 'I', head)


def case(r):
    op = r.deal(OPS)
    if op != 'SetTexts':
        return fill_case(r, op)
    log = log_region()
    regions = {LOG: log}
    regions[ROOT] = window(r, 0)
    kids(r, regions, ROOT, (0x12c4bd, 0x12c4ca), NODES)
    if PANEL in regions:
        kids(r, regions, PANEL, (0x11, 0x12, 0x15), NODES + 0x800)
    regions[VT] = vtable({9: 1})
    regions[IMPL] = struct.pack('<I', IMPLVT)
    regions[IMPLVT] = vtable({3: 2})
    blob_bytes(log, 3, 1, 0, (0x10, 0xf))
    regions[0x7cb25c] = struct.pack('<I', ROOT if r.random() < 0.95 else 0)
    regions[0x7cb260] = struct.pack('<I', r.choice([0, 0, 1, 1, r.getrandbits(32)]))
    regions[0x78b530] = r.bytes(0x400)
    panelparts.texts(r, regions, list(range(0xaa, 0xb5)))
    return Call(CALLS['SetTexts'], [], regions, ret='v')


SW, SCORE_AT, MONEY = RAM + 0x400000, RAM + 0x20000, RAM + 0x60000   # (the world [0x80239c] / [0x7cf83c]; the analyser; the money record)
DIALOG2, DECOYS = RAM + 0x18000, RAM + 0x18200   # (Refresh: the dialog and its other children)
WIDE = 0x7ba058   # (Language_Widen's buffer)


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def peek(regions, address, fmt):
    for base, data in regions.items():
        if base <= address and address + struct.calcsize('<' + fmt) <= base + len(data):
            return struct.unpack_from('<' + fmt, data, address - base)
    return None


def record(regions, index):
    """Records_DataIfTerminated (0x6ace60) of the table [0x78bac4]: whether the record `index` is there"""
    table = peek(regions, 0x78bac4, 'I')[0]
    count, array = peek(regions, table + 8, 'II')
    if index >= count:
        return False
    rec = peek(regions, (array + 4 * index) & 0xffffffff, 'I')
    head = peek(regions, rec[0], 'I') if rec else None   # (none: a fault in both)
    end = peek(regions, rec[0] + 4 + 4 * (((head[0] >> 8) + 3) >> 2), 'I') if head and head[0] & 0xff == 1 else None
    return bool(end) and end[0] == 0


def fill_env(r):
    """FillList's list and money: see above. Returns (regions, masks, the flag)."""
    window, regions, _ = wl.list_env(r, columns=(1, 2, 3, 4, 5, 6, 6, 6))
    ncols = struct.unpack_from('<h', window, 0x164)[0]
    window[0x48] = (window[0x48] & ~0x10) | r.choice([0, 0x10])
    if struct.unpack_from('<i', window, 0x150)[0] == 0:   # (Clear leaves a list of no items as it is, and list_env()'s links may run in a
        put(window, 0x160, 'i', -1)                       # ring that AddItem's walk to the end follows for ever: a list of none is a
        dwords = struct.unpack_from('<i', window, 0x158)[0]   # reset one, as Window7059f0_ResetItems leaves it)
        stride = 4 + ncols
        region = bytearray(regions[wl.LINKS3 - 0x20])
        n = dwords // stride
        for k in range(n):
            put(region, 0x20 + 4 * stride * k, 'i', stride * (k + 1) if k + 1 < n else -1)
            put(region, 0x20 + 4 * stride * k + 8, 'ii', -1, -2)
        regions[wl.LINKS3 - 0x20] = region
    regions[0x78b530] = b''
    panelparts.texts(r, regions, list(range(0x20, 0x30)))
    lang = regions[0x78b530]
    put(lang, 0x594, 'I', panelparts.TEXTS + 0xa00)   # ([0x78bac4]: the same records as [0x78ba94])
    converts = struct.unpack_from('<I', lang, 0x550)[0] != 0
    if converts and r.random() < 0.5:
        put(lang, 0x550, 'I', 0)
        converts = False
    bottoms = [r.choice([1, -1]) if converts else r.choice([0, 0, 0, 1])] + [r.choice([1, 1, -1, 2]) for _ in range(5)]
    regions[wl.COLS3] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', v) for v in bottoms)
    sortable = [c for c in range(ncols) if bottoms[c] in (0, 1)] or [0]   # (a number in a column sorted as text: a fault in both)
    put(window, 0x168, 'hh', r.choice(sortable), r.choice([0, 0, 1, 1]))
    length = r.choice([0, 1, 3, 31, 32, 40])
    if not converts:
        regions[WIDE] = struct.pack('<%dH' % length, *[r.choice(wl.ALPHA) for _ in range(length)]) + bytes(0x40)
    # the analyser's kept score (fresh, worked out) and the money record: the slots 0 and h of the table 0x7cfb90
    ticks = r.getrandbits(32) | 0x10000
    h = r.randrange(1, 12)
    tail = bytearray(r.bytes(0x3c))
    put(tail, 0, 'I', ticks)
    put(tail, 0x14, 'H', 0)
    put(tail, 0x1a, 'H', h)
    regions[SW + 0x1da70c] = tail
    regions[0x80239c] = regions[0x7cf83c] = struct.pack('<I', SW)
    score = r.choice([0, 7, 100, 5000, 250000, 0x1000001, 0x7fffffff, -5, r.randrange(-0x80000000, 0x80000000)])
    regions[SCORE_AT + 0x1f0e8] = struct.pack('<IiB', (ticks - r.choice([0, 4, 0x14 * 4])) & 0xffffffff, score, r.choice([1, 0x40])) + r.bytes(3)
    slots = bytearray(r.bytes(20 * 12))
    put(slots, 0, 'I', SCORE_AT)
    put(slots, 20 * h, 'I', MONEY)
    regions[0x7cfb90] = slots
    money = bytearray(r.bytes(0x200))
    for i in range(8):
        e = 0x14 + 0x20 * i
        put(money, e + 8, 'i', r.choice([0, 1, 7, -1, r.getrandbits(31)]))
        put(money, e + 0xc, 'iiii', r.choice([0, 1, 12, 36, 60, -3]), r.choice([0, 1, 100, 500, 3000, -7]), r.choice([0, 0, 1, 0x100]),
            r.choice([0, 1, 5, 12, 40, 70]))
        put(money, e + 0x1c, 'I', r.choice([0x20 + i] * 8 + [r.randrange(0x20, 0x30), 0x30, 0x40, r.getrandbits(32)]))
    owed = 0
    for i in range(8):
        term, rate, taken, paid = struct.unpack_from('<4I', money, 0x20 + 0x20 * i)
        owed += (term - paid) * rate if taken else 0
    credit = (int(f32(float(score))) - owed) & 0xffffffff
    signed = credit - (1 << 32) if credit >> 31 else credit
    offers = []
    for i in range(8):
        near = [signed - 1, signed, signed + 1] if -0x80000000 < signed < 0x7fffffff else [signed]
        put(money, 0x18 + 0x20 * i, 'i', r.choice(near * 2 + [r.randrange(-0x80000000, 0x80000000), 0, 1000]))
        offers.append(struct.unpack_from('<8i', money, 0x14 + 0x20 * i))
    regions[MONEY] = money
    flag = r.choice([0, 0, 1, 1, 2, r.getrandbits(32)])
    regions[0x7cb260] = struct.pack('<I', flag)
    return regions, (window, ncols, bottoms, offers, signed, converts, length), flag


def masks_of(regions, flag, env):
    """the items the original adds, the list's growth and its strings: the new strings' allocator bytes (allocator_masks' heap walk)"""
    window, ncols, bottoms, offers, signed, converts, length = env
    stride, dwords, count = 4 + ncols, struct.unpack_from('<i', window, 0x158)[0], 0
    sizes = []
    for o in offers:
        added = o[5] == 0 and o[1] <= signed if flag == 0 else flag == 1 and o[5] != 0
        if not added:
            continue
        if count * stride >= dwords:
            n = 0x14 if dwords == 0 else 0xa
            sizes.append(((dwords + n * stride) * 4, False))
            dwords += n * stride
        if bottoms[0] == 0 and record(regions, o[7] & 0xffffffff):
            sizes.append((0x10, True))
            if length:
                sizes.append((((length | 31) + 2) * 2, False))
        count += 1
    nxt, offsets = HEAP + 0x10, []
    for size, string in sizes:
        if string:
            offsets.append(nxt - HEAP)
        nxt += (size + 15) & ~15
    return [Mask(HEAP, offsets, "the new strings' allocator byte: a stack byte the original never sets")] if offsets else []


def fill_case(r, op):
    regions, env, flag = fill_env(r)
    if op == 'FillList':
        return Call(CALLS['FillList'], [wl.LIST3], regions, ret='v', masks=masks_of(regions, flag, env))
    flag = r.choice([1, 1, 1, flag])
    regions[0x7cb260] = struct.pack('<I', flag)
    ids = [0x12c4bd] if r.random() < 0.97 else []
    ids += [r.choice([0, 0x12c4be, 0x11, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(ids)
    head = 0
    for i, ident in reversed(list(enumerate(ids))):
        address = wl.LIST3 if ident == 0x12c4bd else DECOYS + NODE * i
        w = regions[address] if address == wl.LIST3 else bytearray(r.bytes(NODE))
        put(w, 0x8c, 'I', ident)
        regions[address] = put(w, 0x54, 'I', head)
        head = address
    regions[DIALOG2] = put(bytearray(r.bytes(NODE)), 0x5c, 'I', head)
    dialog = DIALOG2 if r.random() < 0.9 else 0
    regions[0x7cb25c] = struct.pack('<I', dialog)
    masks = masks_of(regions, flag, env) if dialog and flag == 1 and 0x12c4bd in ids else []
    return Call(CALLS['Refresh'], [], regions, ret='v', masks=masks)


if __name__ == '__main__':
    B.main(case)
