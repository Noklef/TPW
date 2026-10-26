# The 0x7cc328 panel (src/game/ui/panel7cc328: 0x4b4670, 0x4b47e0, 0x4b4850) against the originals.
# Kill24 (0x4b4670): any object bytes, the script slot at +0x24 0, small or any; the sprite scripts not ready, so the kill
# (SpriteScript_Kill) does nothing.
# OnMessage (0x4b4850, and through its sweep 0x4b47e0): the message mostly 0x101, b mostly 1, the id mostly one of the tabs 0x1e7d-0x1e81
# (around them, any now and then); the window's default handler +0x118 the driver's four-argument logging stub; the panel [0x7cc338]'s
# children (+0x5c, on through +0x54) another window, then window7059f0.py's list (list_case's Clear surroundings, its id +0x8c 0x1e7b,
# 0-4 rows of row_windows() for RefreshRows: their slot 3 events hashed as refresh_case's);
# ridesweep.py's staff lists over a park of things (the pool moved past the GUI's windows), each thing resting (+0x19c 3) or not, its area
# word +0x208 the area's [0x7c2658] or not, its float +0x1fc small, large, negative or past the dword range; the panel object 0x7cc328 a
# recorder vtable (slot 8). The list's columns are at most two (the texts are two dwords: a third column's would be the stack's), all number
# columns (bottom 1): the rows' cells are the texts themselves (the name's address and the number), so no strings are made (their
# allocator byte is a stack byte the original never sets: window7059f0.py).
# AddStaffRow (0x4b47e0): OnMessage's surroundings, the list empty; one of the park's things.
# Reopen (0x4b4e30), AddStaff (0x4b4f00), RemoveStaff (0x4b4f90): the panel 0x7cc328 itself (in OnMessage's surroundings: its window +0x10
# the panel window [0x7cc338], for AddStaff / RemoveStaff 0 now and then), its recorder vtable (slot 13's answer any), its +0x20 object a
# recorder vtable (slot 9 of one); the tab [0x7cc358] 0-4 mostly (Reopen) or one of the things' group bytes +2 (4-8 and others); AddStaff
# with the list empty, RemoveStaff with OnMessage's list, some of its items' data dwords the thing.
# OnStaff (0x481960): AddStaff's or RemoveStaff's set-up by its flag (0, 1 or any), the staff member in a random slot of the thing table
# (the handle's high half any).
# usage: panel7cc328.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Kill24,OnMessage,AddStaffRow,Reopen,AddStaff,RemoveStaff,OnStaff)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import ridesweep  # noqa: E402
import window7059f0  # noqa: E402
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, put, set_return, vtable  # noqa: E402
from window import VTABLE, notify_blobs  # noqa: E402
from thingenv import THING  # noqa: E402

CALLS = {'Kill24': 0x4b4670, 'OnMessage': 0x4b4850, 'AddStaffRow': 0x4b47e0, 'Reopen': 0x4b4e30, 'AddStaff': 0x4b4f00,
         'RemoveStaff': 0x4b4f90, 'OnStaff': 0x481960}
B = Bench(sorted(set(window7059f0.LIST_RANGES + ridesweep.SWEEP_RANGES + [
    (0x4b4670, 0x4b4695), (0x475550, 0x47558b), (0x4b47e0, 0x4b4850), (0x4b4850, 0x4b4a3c), (0x506650, 0x506675), (0x506860, 0x506867),
    (0x65f66e, 0x65f697), (0x4b4e30, 0x4b4ee9), (0x4b4f00, 0x4b4f8b), (0x4b4f90, 0x4b4fc0), (0x506490, 0x5064d4), (0x5da3c0, 0x5da3c1), (0x481960, 0x4819db)])))
POOL2, PANEL, OTHER, WIN, AREA, PVT = RAM + 0x40000, RAM + 0x50000, RAM + 0x50100, RAM + 0x50200, RAM + 0x50400, RAM + 0x50500
ROWS, ROWWINS, OBJ20, OVT = RAM + 0x50600, RAM + 0x52000, RAM + 0x50580, RAM + 0x50700   # (the rows' windows past the park's definitions)
FLOATS = [0.0, 0.5, 1.0, 50.0, 99.9, 100.0, 255.9, 256.0, 300.5, -1.5, -0.5, 1e10, -1e10]


def env(r, empty=False):
    """OnMessage's surroundings (above), the list empty (as a clear leaves it) when asked: (regions, the area word)"""
    list3 = window7059f0.LIST3
    while True:   # (two columns at most: the texts are two dwords, a third column's would be the stack's)
        regions = dict(window7059f0.list_case(r, 'Clear').regions)
        if struct.unpack_from('<h', regions[list3], 0x164)[0] <= 2:
            break
    window = regions[list3]
    put(window, 0x144, 'I', window7059f0.LINKS3)   # (always an array; the sort column one of the columns, the direction any)
    ncols = struct.unpack_from('<h', window, 0x164)[0]
    put(window, 0x168, 'hh', r.randrange(max(ncols, 1)), r.choice([0, 0, 1, 1, 0x100]))
    if empty:
        put(window, 0x150, 'i', 0)
    if struct.unpack_from('<i', window, 0x150)[0] == 0:   # (an empty list as a clear leaves it, which Clear then leaves alone)
        ncols, dwords = struct.unpack_from('<h', window, 0x164)[0], struct.unpack_from('<i', window, 0x158)[0]
        links = bytearray(regions[window7059f0.LINKS3 - 0x20])
        n, item = max(dwords // (ncols + 4), 1), 0
        for i in range(n):   # (ResetItems: the free chain)
            put(links, 0x20 + 4 * item, 'i', item + ncols + 4 if i < n - 1 else -1)
            put(links, 0x20 + 4 * item + 8, 'ii', -1, -2)
            item += ncols + 4
        regions[window7059f0.LINKS3 - 0x20] = links
        put(window, 0x14c, 'iii', -1, 0, 0)
        put(window, 0x160, 'i', -1)
    regions[window7059f0.COLS3] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', 1) for _ in range(4))
    regions[VTABLE] = vtable({5: 1, 3: 2, 0: 1, 7: 1, 9: 1})   # (the rows' windows: RefreshRows' slots 7 and 9 too)
    notify_blobs(regions[LOG], 0, 0x3c00)
    nrows = r.choice([0, 1, 2, 3, 4])
    rows = window7059f0.row_windows(r, regions, [], nrows, ncols, list3, ROWWINS)
    regions[ROWS] = struct.pack('<%dI' % (nrows + 1), *rows, r.getrandbits(32))
    put(window, 0x148, 'I', ROWS)
    put(window, 0x172, 'h', nrows)
    put(regions[list3], 0x8c, 'I', 0x1e7b)
    put(regions[list3], 0x54, 'I', 0)
    regions[OTHER] = put(put(bytearray(r.bytes(0x100)), 0x8c, 'I', r.choice([0, 1, 0x1e7c, 0x1e7d])), 0x54, 'I', list3)
    regions[PANEL] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', r.choice([list3, OTHER]))
    staff = ridesweep.select(r, pool=POOL2).regions
    staff.pop(LOG)   # (the list's log kept: its blob settings)
    assert not set(staff) & set(regions), sorted(hex(k) for k in set(staff) & set(regions))
    regions.update(staff)
    area = r.choice([0, 1, 2, 0x8001, r.getrandbits(16)])
    regions[AREA] = struct.pack('<HH', area, r.getrandbits(16))
    regions[0x7c2658] = struct.pack('<I', AREA)
    things = regions[POOL2]
    for t in range(0, len(things), THING):
        put(things, t + 0x19c, 'I', r.choice([3, 3, 3, 0, 2, 0x103]))
        put(things, t + 0x208, 'H', r.choice([area, area, area, area ^ 1, area ^ 0x100]))
        put(things, t + 0x1fc, 'f', r.choice(FLOATS + [r.uniform(0, 300)] * 4))
    regions[0x7cc310] = put(put(bytearray(r.bytes(0x50)), 0x18, 'I', PVT), 0x28, 'I', PANEL)
    regions[PVT] = vtable({8: 0})
    return regions, area


def message_case(r):
    regions, _ = env(r)
    regions[WIN] = put(bytearray(r.bytes(0x120)), 0x118, 'I', CDECL_LOG4)
    tab = r.choice(list(range(0x1e7d, 0x1e82)) * 3 + [0x1e7b, 0x1e7c, 0x1e82, r.getrandbits(32)])
    message = r.choice([0x101] * 6 + [0x100, 0x102, 0, r.getrandbits(32)])
    b = r.choice([1] * 6 + [0, 2, 0x101])
    return Call(CALLS['OnMessage'], [WIN, message, tab, b], regions, ret='a')


def panel_case(r, op):
    """Reopen, AddStaff, RemoveStaff on the panel 0x7cc328 (above); OnStaff as either by its flag"""
    flag = None
    if op == 'OnStaff':
        flag = r.choice([0, 1, 0, 1, r.getrandbits(32)])
        op = 'AddStaff' if flag else 'RemoveStaff'
    regions, area = env(r, op == 'AddStaff')
    globs = regions[0x7cc310]
    put(globs, 0x38, 'I', OBJ20)
    regions[OBJ20] = struct.pack('<I', OVT)
    regions[OVT] = vtable({9: 1})
    set_return(regions[LOG], 13, r.choice([0, 1, r.getrandbits(32)]))
    if op == 'Reopen':
        put(globs, 0x48, 'I', r.choice(list(range(5)) * 3 + [5, 0xffffffff, r.getrandbits(32)]))
        return Call(CALLS[op], [0x7cc328], regions, conv='t', ret='v')
    if r.random() < 0.1:
        put(globs, 0x28, 'I', 0)
    things = regions[POOL2]
    for t in range(0, len(things), THING):
        things[t + 2] = r.choice([4, 5, 6, 7, 8] * 2 + [3, 9, r.getrandbits(8)])
    staff = POOL2 + THING * r.randrange(6)
    put(globs, 0, 'H', r.choice([area, area, area, area ^ 1]))
    put(globs, 0x48, 'I', r.choice([{4: 1, 5: 0, 6: 2, 7: 3, 8: 4}.get(things[staff - POOL2 + 2], 5)] * 3 + [0, 1, 2, 3, 4, 5]))
    if op == 'RemoveStaff':
        list3 = regions[window7059f0.LIST3]
        stride = 4 + max(struct.unpack_from('<h', list3, 0x164)[0], 0)
        links = bytearray(regions[window7059f0.LINKS3 - 0x20])
        for j in range(1, (len(links) - 0x20) // 4, stride):
            if r.random() < 0.3:
                put(links, 0x20 + 4 * j, 'I', r.choice([staff, staff, staff + THING]))
        regions[window7059f0.LINKS3 - 0x20] = links
    if flag is not None:
        h = r.randrange(1, 12)
        put(regions[0x7cfb90], 20 * h, 'I', staff)
        return Call(CALLS['OnStaff'], [0x7b51f0, h | r.choice([0, r.getrandbits(16) << 16]), flag], regions, conv='t', ret='v')
    return Call(CALLS[op], [0x7cc328, staff], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'OnMessage':
        return message_case(r)
    if op in ('Reopen', 'AddStaff', 'RemoveStaff', 'OnStaff'):
        return panel_case(r, op)
    if op == 'AddStaffRow':   # (env()'s, the list empty: the area [0x7cc310] the things' or not, the list [0x7cc314]; one of the things)
        regions, area = env(r, True)
        put(regions[0x7cc310], 0, 'H', r.choice([area, area, area, area ^ 1]))
        put(regions[0x7cc310], 4, 'I', window7059f0.LIST3)
        return Call(CALLS[op], [POOL2 + THING * r.randrange(6)], regions, ret='a')
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x24, 'I', r.choice([0, 0, 1, 2, 3, 0xffffffff, r.randrange(8)]))
    return Call(CALLS[op], [RAM], {RAM: obj, 0x7b49f0: bytearray(16)}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
