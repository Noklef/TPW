# The staff dialog [0x7ca168]'s staff list (src/game/ui/dialogs: Dialog7ca168_AddStaffRow 0x496320, Dialog7ca168_FillStaffList
# 0x496400) against the originals. The list is window7059f0.py's add_case() list with five columns (LIST3). Its column bottoms are
# add_case()'s for the first two columns; for the number columns 2-4 they are 1 mostly (always for the sort column), else -1 or 2.
# The staff members (staff()) have:
#   - the model byte +2 one of 4-8 mostly;
#   - +0x19c 0-7 mostly;
#   - a name at +0x1a0 of 0-3 of window7059f0.py's ALPHA;
#   - the grade +0x1e4 0-4 mostly, and its progress byte +0x1e8 any;
#   - the float +0x1f8 0-100 mostly;
#   - the patrol region +0x20a / +0x20c none (1, 0x4000) now and then.
# The wage table 0x785300 is random. The record table [0x78baa4] has 6 short records, some not terminated, some with another kind byte,
# its count 0-6. The language's converter flag [0x78ba80] is 0, so the wide buffer 0x7ba058 (a short string) is returned as it is.
# Row (0x496320): one staff member; the list [0x7ca164] LIST3 (none now and then: a fault in both); the new strings' allocator bytes
# masked as in add_case().
# Fill (0x496400): ridesweep.py's five staff lists over a park of 0-6 staff (no other things); the window [0x7ca168] whose child 0x321
# is the list (among 0-2 decoys, now and then missing: a fault in both); the columns all number ones (no strings made: the allocations
# of several rows are not worked out); the tab 0-4 mostly, else 5, -1 or any; clear 0, 1 or any. A case the emulator runs past its step
# limit is made again (fill_case(): add_case()'s forests can send a few rows added in turn round a free chain for ever, natively too).
# Handler (0x4960a0, the dialog's handler): Fill's world (made again the same way, else with the message 0) with the dialog's child 0x329
# (now and then missing: a fault in both) holding 0x32a / 0x32d (slots 9 / 7 recorders) among 0-2 decoys, the texts 0x6b-0x6f as
# panel7cc298.py's (a record table [0x78ba94] about them, its count 0x70 mostly), the game's +0x1da720 a small slot (its answer unused);
# the window's default handler +0x118 fncmp's logger (its result set now and then); the message 0x101, a one of the tab buttons
# 0x323-0x327 and b 1 mostly.
# usage: stafflist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Row,Fill,Handler)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import window7059f0 as w7  # noqa: E402
from dialog7cc5f4 import merged  # noqa: E402
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, log_region, put, set_return, vtable  # noqa: E402
from thingenv import THING, park  # noqa: E402

CALLS = {'Row': 0x496320, 'Fill': 0x496400, 'Handler': 0x4960a0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(merged(w7.LIST_RANGES + [
    (0x485ac0, 0x485b10), (0x496320, 0x4963f1), (0x496400, 0x4964d0), (0x4f0170, 0x4f0260), (0x5064f0, 0x506590), (0x506860, 0x506900),
    (0x506eb0, 0x506ee0), (0x658ccc, 0x658e04), (0x659710, 0x659740), (0x659d43, 0x659dbc), (0x65c672, 0x65c717), (0x65d547, 0x65d56a),
    (0x65d56b, 0x65d58c), (0x65d773, 0x65d832), (0x65d9dd, 0x65da9d), (0x65f697, 0x65f6e1), (0x66316b, 0x663564), (0x6636f6, 0x663728),
    (0x663a93, 0x663c21), (0x663cb8, 0x663d66), (0x663dfa, 0x6644fa), (0x66525c, 0x6654a4), (0x6697e3, 0x6698b4), (0x66b47d, 0x66b518),
    (0x67c290, 0x67c300), (0x67fff0, 0x680060), (0x6800f0, 0x6802f8), (0x6ace60, 0x6aceb0), (0x6adc80, 0x6add40), (0x5da3c0, 0x5da3c1),
    (0x47db20, 0x47db90), (0x481bc0, 0x4824e0), (0x4cd300, 0x4cd350), (0x4dd4e0, 0x4dd510), (0x4e2710, 0x4e2750), (0x516130, 0x5161c0),
    (0x663085, 0x66317b), (0x6649d5, 0x664b78), (0x67a830, 0x67a860),
    (0x4960a0, 0x496320), (0x496400, 0x496630), (0x485b00, 0x485b50), (0x4c9cc0, 0x4c9d90), (0x519510, 0x519560), (0x65f66e, 0x65f6a7),
    (0x663324, 0x663564), (0x663ba2, 0x663c21), (0x664495, 0x6644fa), (0x50b350, 0x50b370)]))
STAFF, HEADS, DIALOG, DECOYS = RAM + 0x60000, RAM + 0x68000, RAM + 0x68100, RAM + 0x68400
RECORDS = RAM + 0x69000   # (the table, its pointers at +0x20, the records at +0x100, 0x20 apart)


def staff(r, buf, at):
    """a staff member's fields at buf[at:] (see the module comment); returns its name's length"""
    buf[at + 2] = r.choice([4, 5, 6, 7, 8] * 3 + [0, 3, r.getrandbits(8)])
    put(buf, at + 0x19c, 'I', r.choice(list(range(8)) * 2 + [8, r.getrandbits(32)]))
    name = [r.choice(w7.ALPHA) for _ in range(r.choice([0, 1, 2, 3]))]
    struct.pack_into('<%dH' % (len(name) + 1), buf, at + 0x1a0, *name, 0)
    put(buf, at + 0x1e4, 'i', r.choice([0, 1, 2, 3, 4] * 3 + [5, -1]))
    buf[at + 0x1e8] = r.getrandbits(8)
    put(buf, at + 0x1f8, 'f', r.choice([0.0, 1.0, 9.99, 50.0, 99.9, 100.0, r.uniform(0, 100)]) if r.random() < 0.9 else
        r.choice([-1.0, 255.9, 256.0, 300.0, 1e10]))
    put(buf, at + 0x20a, 'HH', *((1, 0x4000) if r.random() < 0.3 else (r.getrandbits(16), r.getrandbits(16))))
    return len(name)


def texts(r, regions):
    """the record table [0x78baa4], the converter flag [0x78ba80] 0 and the wide buffer 0x7ba058; returns the buffer's length"""
    table = put(bytearray(r.bytes(0x20)), 8, 'II', r.choice([6, 6, 6, r.randrange(7)]), RECORDS + 0x20)
    ptrs = struct.pack('<6I', *[RECORDS + 0x100 + 0x20 * i for i in range(6)])
    recs = bytearray()
    for _ in range(6):
        body = bytes(r.choice(b'abcdef') for _ in range(r.choice([1, 2, 3, 5]))) + b'\0'
        rec = bytearray(struct.pack('<I', r.choice([1] * 6 + [2, 0x101]) | len(body) << 8) + body + bytes(0x20 - 4 - len(body)))
        if r.random() < 0.15:
            rec[4 + ((len(body) + 3) & ~3)] = 1   # (not terminated)
        recs += rec
    regions[RECORDS] = table + ptrs
    regions[RECORDS + 0x100] = bytes(recs)
    regions[0x78baa4] = struct.pack('<I', RECORDS)
    regions[0x78ba7c] = struct.pack('<II', r.getrandbits(32), 0)
    wide = [r.choice(w7.ALPHA) for _ in range(r.choice([0, 1, 2, 3]))]
    regions[0x7ba058] = struct.pack('<%dH' % (len(wide) + 1), *wide, 0) + bytes(0x40)
    regions[0x785300] = r.bytes(0x1100)
    return len(wide)


def list_env(r, strings):
    """add_case()'s list with five columns (see the module comment; strings: the first two columns' bottoms add_case()'s, else 1);
    returns (regions, the bottoms)"""
    regions = w7.add_case(r, columns=(5,)).regions
    window = regions[w7.LIST3]
    sort = struct.unpack_from('<h', window, 0x168)[0]
    old = [struct.unpack_from('<h', regions[w7.COLS3], 0xc * k + 0xa)[0] for k in range(5)]
    bottoms = old[:2] if strings else [1, 1]
    for k in range(2):
        if sort == k and bottoms[k] == -1:   # (numbers sorted as text: a fault in both)
            bottoms[k] = 1
    bottoms += [1 if k == sort or not strings or r.random() < 0.7 else r.choice([-1, 2]) for k in range(2, 5)]
    regions[w7.COLS3] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', v) for v in bottoms)
    return regions, bottoms


def row_case(r):
    regions, bottoms = list_env(r, True)
    window = regions[w7.LIST3]
    buf = bytearray(r.bytes(0x220))
    n = staff(r, buf, 0)
    regions[STAFF] = buf
    wide = texts(r, regions)
    regions[0x7ca164] = struct.pack('<I', w7.LIST3 if r.random() < 0.97 else 0)
    # (the text is the wide buffer unless the record is missing: which one the mode picks is not worked out here, so the second
    # string's allocator byte is masked either way only when the first column makes no string)
    mode = struct.unpack_from('<I', buf, 0x19c)[0]
    index = [0, 1, 3, 3, 4, 4, 2, 5][mode] if mode < 8 else 2
    if index == 1 and struct.unpack_from('<HH', buf, 0x20a) == (1, 0x4000):
        index = 0
    count, base = struct.unpack_from('<II', regions[RECORDS], 8)
    rec = regions[RECORDS + 0x100][0x20 * index:0x20 * index + 0x20]
    head = struct.unpack_from('<I', rec)[0]
    found = index < count and head & 0xff == 1 and struct.unpack_from('<I', rec, 4 + ((head >> 8) + 3 & ~3))[0] == 0
    lengths = [n if bottoms[0] == 0 else None, wide if bottoms[1] == 0 and found else None]
    fail = struct.unpack_from('<I', regions[HEAP])[0] - HEAP - 0x10
    count, dwords = struct.unpack_from('<i', window, 0x150)[0], struct.unpack_from('<i', window, 0x158)[0]
    grows = count * 9 >= dwords
    masks = w7.allocator_masks(fail, lengths, (dwords + (20 if dwords == 0 else 10) * 9) * 4 if grows else None)
    return Call(CALLS['Row'], [STAFF], regions, ret='a', masks=masks)


def fill_case(r, once=None):
    """fill_once() (or once()), made again (up to 8 times, else with the tab 5 / the message 0: no rows) while the emulator runs it past
    its step limit: add_case()'s forests are made for one AddItem, and a few rows added in turn can walk a free chain round for ever
    (natively too)"""
    for _ in range(8):
        c = (once or fill_once)(r)
        probe = Call(c.address, c.args, dict(c.regions), conv=c.conv, ret=c.ret)
        probe.regions.update({k: v for k, v in B.extra.items() if k not in probe.regions})
        if B.original(probe) != 'step limit':
            return c
    c.args[1 if once else 0] = 0 if once else 5
    return c


def fill_once(r):
    regions, _ = list_env(r, False)
    count = r.choice([0, 1, 2, 3, 4, 6])
    env, handles, addrs = park(r, count=count, kinds=(4, 5, 6, 7, 8), defs=False, pool=STAFF)
    regions.update(env)
    things = regions[STAFF]
    for i in range(count):
        staff(r, things, THING * i)
    lists = [[] for _ in range(5)]
    for i in range(count):
        k = r.randrange(6)
        if k < 5:
            lists[k].append(i)
    heads = [0] * 5
    for k, members in enumerate(lists):
        r.shuffle(members)
        for j, i in enumerate(members):
            struct.pack_into('<H', things, THING * i + 0x210, handles[members[j + 1]] if j + 1 < len(members) else 0)
        heads[k] = handles[members[0]] if members else 0
    regions[0x7cf83c] = struct.pack('<I', HEADS - 0x1da73c)
    regions[HEADS] = struct.pack('<5H', *heads) + r.bytes(6)
    texts(r, regions)
    kids = [DECOYS + 0x100 * k for k in range(r.choice([0, 0, 1, 2]))]
    for k in kids:
        regions[k] = put(bytearray(r.bytes(0x100)), 0x8c, 'I', r.choice([0x320, 0x322, r.getrandbits(32)]))
    if r.random() < 0.97:
        kids.insert(r.randrange(len(kids) + 1), w7.LIST3)
    put(regions[w7.LIST3], 0x8c, 'I', 0x321)
    for i, k in enumerate(kids):
        put(regions[k], 0x54, 'I', kids[i + 1] if i + 1 < len(kids) else 0)
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', kids[0] if kids else 0)
    regions[0x7ca164] = struct.pack('<3I', r.getrandbits(32), DIALOG, r.getrandbits(32))   # (0x7ca164, the dialog 0x7ca168, 0x7ca16c)
    tab = r.choice([0, 1, 2, 3, 4] * 3 + [5, 0xffffffff, r.getrandbits(32)])
    return Call(CALLS['Fill'], [tab, r.choice([0, 1, 1, r.getrandbits(32)])], regions, ret='v')


GROUP, WINDOW, VT, TEXTS = RAM + 0x6a000, RAM + 0x6a800, RAM + 0x6ac00, RAM + 0x6b000
TABS = {0x323: 0, 0x324: 4, 0x325: 2, 0x326: 1, 0x327: 3}


def handler_once(r):
    """Handler's case on fill_once()'s world (see the module comment)"""
    c = fill_once(r)
    regions = c.regions
    log = regions.setdefault(LOG, log_region())
    regions[VT] = vtable({7: 1, 9: 1})
    ids = [0x32a, 0x32d] + [r.choice([0x329, 0x32b, 0x32c, r.getrandbits(32)]) for _ in range(r.choice([0, 0, 1, 2]))]
    r.shuffle(ids)
    for i, ident in enumerate(ids):
        w = put(bytearray(r.bytes(0x100)), 0, 'I', VT)
        put(w, 0x54, 'I', GROUP + 0x100 * (i + 2) if i + 1 < len(ids) else 0)
        regions[GROUP + 0x100 * (i + 1)] = put(w, 0x8c, 'I', ident)
    group = put(put(bytearray(r.bytes(0x100)), 0x5c, 'I', GROUP + 0x100), 0x8c, 'I', 0x329)
    dialog = regions[DIALOG]
    put(group, 0x54, 'I', struct.unpack_from('<I', dialog, 0x5c)[0])
    regions[GROUP] = group
    if r.random() < 0.97:
        put(dialog, 0x5c, 'I', GROUP)
    # the record table [0x78ba94] about the texts 0x6b-0x6f (the converter off: texts() sets [0x78ba80] 0)
    table = put(bytearray(r.bytes(0x20)), 8, 'II', r.choice([0x70, 0x70, 0x70, 0x6d, 5]), TEXTS + 0x100 - 4 * 0x6b)
    recs = bytearray()
    for _ in range(5):
        body = bytes(r.choice(b'ghijk') for _ in range(r.choice([1, 2, 4]))) + b'\0'
        recs += struct.pack('<I', r.choice([1] * 6 + [2]) | len(body) << 8) + body + bytes(0x20 - 4 - len(body))
    regions[TEXTS] = table
    regions[TEXTS + 0x100] = struct.pack('<5I', *[TEXTS + 0x200 + 0x20 * i for i in range(5)])
    regions[TEXTS + 0x200] = bytes(recs)
    regions[0x78ba94] = struct.pack('<I', TEXTS)
    regions[HEADS - 0x1c] = struct.pack('<H', r.randrange(12))   # (the game's +0x1da720: GameTable7cfb90_Get1da720_519510, its answer unused)
    win = put(bytearray(r.bytes(0x120)), 0x118, 'I', CDECL_LOG4)
    regions[WINDOW] = win
    message = 0x101 if r.random() < 0.9 else r.choice([0x100, 0x14, r.getrandbits(32)])
    a = r.choice(list(TABS) * 3 + [0x322, 0x328, r.getrandbits(32)])
    b = 1 if r.random() < 0.9 else r.choice([0, 2, r.getrandbits(32)])
    if r.random() < 0.2:
        set_return(log, 0x3d, r.getrandbits(32))
    return Call(CALLS['Handler'], [WINDOW, message, a, b], regions, ret='a')


def case(r):
    op = r.deal(OPS)
    return row_case(r) if op == 'Row' else fill_case(r) if op == 'Fill' else fill_case(r, handler_once)


if __name__ == '__main__':
    B.main(case)
