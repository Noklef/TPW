# A staff member's state set (0x5054d0 Staff_SetState, src/game/world/staff) against the original: the state 0-7 mostly (past 7 now and
# then: only stored), the old state +0x19c 1 or not (state 0's stamp), the world's tick [0x80239c] + 0x1da70c any. The scripts and, for
# state 7 (the staff dropped: an MMStaffDrop mode of the folder +0x24 and sprite +0x20, its Assign starting a sprite script), mousemode.py's
# assign() surroundings for its AssignDrop (the cursor windows, the sprite folders, the script table and heap; an allocation failing now and
# then); the reference +0xc's slot and the +0x30 object's (+0xb4) one of the table's or 0; the current mode [0x7b05d8] a recorder object or
# none, the holder [0x7b05e8] there or not (a CMMOnline base made then, replaced by a CMMOnline: the setting 0xfb3b7c 1, its guard set).
# Mechanic (0x4da370): Staff_SetState's cases now and then (any state), else the states 0xc and 0xd: the grade +0x1e4 small or any
# into the skill table 0x78542c, the handle +0x218's thing (its +0x19c 2 or not, byte +0x50 small or any, float +0x44 0-100, past it,
# negative, huge or NaN) and its definition (gObjectDefs as object.py's: 1-3 of 0x4000 bytes, the cache hit or not).
# Handyman (0x4d7330): Staff_SetState's cases now and then, else the states 8-0xb: the word 0x75c7f2 and the world's tick any.
# Researcher (0x502c20): Staff_SetState's cases now and then, else the state 0xf: the world's tick any.
# PickUp (0x505c50): SetState's surroundings (the old state +0x19c 4, 5, 7, 0xd or 0x12 mostly, else one whose debug print reads the
# staff member's cell: the cells [0x8023a0] random, its column +5 and row +7 within them), the current mode's slot 9 answering 6 mostly
# (else 8, 0, 7 or any), the world's handle +0x1da718 into the thing table 0x7cfb90 (a record whose word +0x60 is set).
# Settle (0x505f50): PickUp's surroundings with the state +0x19c 0-7, 0xd, 0x12 or others, the type +8 of the staff member's cell
# plain (0, 1, 3, 9, 0xa) or not, its byte +2 7 mostly.
# Head (0x506300): see head().
# LeaveRest (0x506d10): SetState's surroundings for state 0, the message centre [0x788d3c] made, agent B's unported 0x40fb10 thiscall recorder 21
# (driver.cpp's kObjectEnv) logging the message's bytes 0-9 and 0xc-0xf (0xa-0xb are the original's stack); the rest area +0x208 a
# handle 0-15 whose slot of 0x7cfb90 holds a thing with the ride id +0x24 (the ride list ready or not, its id matching mostly; element 0
# 0, 1, any or the count 0).
# Message (0x504c70): the message's type (its vtable's slot 0 a recorder, k 19) 0xc, 0xa mostly, 0x1b or 9, its word +8 the rest area +0x208 mostly. Pay
# (0xc): the staff kind byte +2 4-8 mostly, the grade +0x1e4 0-4, the wage tables 0x78532c / 0x785380 small, the world's handles +0x1da726
# (a money record, +0x114 set mostly) and +0x1da720 (a record whose +0x1f5a0 and +0x1f7f0 take the wage). 0xa: the state +0x19c 3, 2 or
# others; in state 3 LeaveRest's surroundings and the world's rest areas (head +0x1da746, next +0xc, flags +0x32 bit 2, cell +0x36): none,
# or found with navpath.py's person as the staff member (as Head; the destination the area's cell) so that the walk there works.
# WalkRest (0x505fe0): LeaveRest's surroundings, Sprite_WalkNav a thiscall recorder (28 of 0, driver.cpp's StaffWalkEnv) answering 0, 2
# or another; +0x1f8 about 0-100 (or past them, a NaN, an infinity, huge), the int 0x785328 small, negative or huge.
# usage: staffstate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetState,Mechanic,Handyman,PickUp,Settle,Head,Researcher,LeaveRest,
#   Message,WalkRest)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import navpath  # noqa: E402
import wctimage  # noqa: E402
from fncmp import HEAP, LOG, Bench, Call, RAM, _recorder, blob_bytes, fb, heap, put, recorder, set_return, vtable  # noqa: E402
from focus import MODES  # noqa: E402
from mousemode import assign  # noqa: E402
from movingperson import cell_xy  # noqa: E402

B = Bench(wctimage.merged(MODES + [
    (0x402d70, 0x403200), (0x40f350, 0x40f370), (0x4217f0, 0x421810), (0x46c350, 0x46c460), (0x46c6a0, 0x46c6c0), (0x46c710, 0x46c740),
    (0x46cbc0, 0x46cc50), (0x46cdc0, 0x46cef0), (0x46cfb0, 0x46cfd0), (0x475550, 0x4755a0), (0x4758f0, 0x475b90), (0x4814a0, 0x481500),
    (0x4a2aa0, 0x4a2ad0), (0x4d4140, 0x4d41a0), (0x4da5b0, 0x4da750), (0x4dad20, 0x4dadd0), (0x4fa460, 0x4fa480), (0x4fa930, 0x4fa9a0),
    (0x5054d0, 0x5056f0), (0x502c20, 0x502c64), (0x505ea0, 0x505f60), (0x509af0, 0x509b50), (0x50b350, 0x50b370), (0x50c0d0, 0x50c110), (0x519450, 0x5194e0),
    (0x540c60, 0x540da0), (0x550ca0, 0x550d90), (0x5f5660, 0x5f56a0), (0x6590f7, 0x6591e4), (0x65f11d, 0x65f160), (0x67a740, 0x67a800),
    (0x67fa20, 0x67fa40), (0x5d1410, 0x5d1416), (0x5d1750, 0x5d1751), (0x5da3c0, 0x5da3c1),
    (0x4da370, 0x4da483), (0x4e03e0, 0x4e03f0), (0x4dd4e0, 0x4dd4fb), (0x412e90, 0x412eea), (0x4d7330, 0x4d73b4),
    (0x505c50, 0x505ea0), (0x509ae0, 0x509aec), (0x4fa990, 0x4fa9fa), (0x536300, 0x53644c), (0x505f50, 0x505fe0),
    (0x506300, 0x5064a0), (0x4c9130, 0x4c91d0), (0x4fa530, 0x4fa600), (0x509990, 0x5099d0), (0x516130, 0x5161c0), (0x516330, 0x516380),
    (0x519510, 0x519560), (0x51a280, 0x51a2a0), (0x4d4170, 0x4d41a0), (0x506d10, 0x506e41), (0x55a070, 0x55a100), (0x55a390, 0x55a3e0),
    (0x415e00, 0x415e80), (0x504c70, 0x504dd2), (0x40f440, 0x40f446), (0x40f490, 0x40f496), (0x40f510, 0x40f516), (0x40f920, 0x40f926),
    (0x506490, 0x5064d4), (0x506910, 0x506a38), (0x5195d0, 0x51960d), (0x4d01f0, 0x4d0259)] + navpath.RANGES + [(0x505fe0, 0x506183)]), hooks={0x40fb10: _recorder(21, 1), 0x4fa2a0: _recorder(28, 0)})
STAFF, WORLD, MODE, MODE_VT, HOLDER = RAM + 0x20000, RAM + 0x21000, RAM + 0xc000, RAM + 0xc100, RAM + 0xc200
RECORD, CELLS = RAM + 0x21400, RAM + 0x50000


CALLS = {'SetState': 0x5054d0, 'Mechanic': 0x4da370, 'Handyman': 0x4d7330, 'PickUp': 0x505c50, 'Settle': 0x505f50, 'Head': 0x506300,
         'Researcher': 0x502c20, 'LeaveRest': 0x506d10, 'Message': 0x504c70, 'WalkRest': 0x505fe0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
TARGET, ITEMS, AREA, RIDES = RAM + 0x38000, RAM + 0x40000, RAM + 0x39000, RAM + 0x3a000
MSG, MONEY, PAID, RESTS = RAM + 0x3c000, RAM + 0x3d000, RAM + 0x10000, RAM + 0x3e000


def mechanic(r):
    staff = bytearray(r.bytes(0x240))
    put(staff, 0x1e4, 'I', r.choice([0, 1, 2, 3, 4, 5, 7, r.getrandbits(32) & 0xff]))
    h = r.randrange(0x400)
    put(staff, 0x218, 'H', h)
    target = bytearray(r.bytes(0x200))
    put(target, 0x19c, 'I', r.choice([2, 2, 0, 1, 3, r.getrandbits(32)]))
    target[0x50] = r.choice([0, 1, 2, 3, r.getrandbits(8)])
    put(target, 0x44, 'f', r.choice([0.0, 100.0, 50.5, r.uniform(0, 100), r.uniform(0, 100), 255.9, 300.0, -1.5, -300.0, 1e12, float('nan')]))
    table = bytearray(r.bytes(0x740))
    n = r.randrange(1, 4)
    ids = r.sample(range(0x10000), n)
    regions = {STAFF: staff, TARGET: target, 0x7cfb90 + 20 * h: struct.pack('<I', TARGET), 0x78542c: r.bytes(0x800)}
    for i in range(n):
        put(table, 4 * i, 'I', ITEMS + 0x4000 * i)
        regions[ITEMS + 0x4000 * i] = put(bytearray(r.bytes(0x4000)), 0, 'H', ids[i])
    put(table, 0x528, 'i', n)
    k = r.randrange(n)
    put(target, 0xe, 'H', ids[k])
    put(table, 0x730, 'HHI', r.choice([ids[k], r.getrandbits(16)]), 0, ITEMS + 0x4000 * k)
    regions[0x7890a0] = table
    return Call(0x4da370, [STAFF, r.choice([0xc, 0xd, 0xd, 0xd])], regions, conv='t', ret='v')


SLOTS, THING, REC = RAM + 0x60000, RAM + 0x60400, RAM + 0x60800


def head(r):
    """Head (0x506300): navpath.py's person (so the walk to the entrance's next cell works), the staff kind 4-8 mostly; the world's
    0x1da708-0x1da722 (the random seed, tick, 0x1da710 0 mostly, the handles 0x1da718 / 0x1da720 into the thing table: the first a record
    whose kind entries (+0x28, 12 each) are 0 now and then); the thing list [0x7cf56c] empty or one thing on the staff member's cell, of
    kind 1 mostly; the entrance 0x7855bc / 0x7855c0 the destination's cell (any now and then)."""
    cid = r.randrange(1, 0x4001)
    regions = dict(navpath.person_case(r, cell_xy(cid)).regions)
    staff = bytearray(regions[RAM])
    staff += bytearray(r.bytes(max(0, 0x240 - len(staff))))
    staff[2] = r.choice([4, 5, 6, 7, 8] * 3 + [1, 0, r.getrandbits(8)])
    put(staff, 0x1c, 'I', r.choice([0, 0x40, 0x7f, 0x80, 0x7c0, 0x7ff, 0x800, 0xffffffff, r.getrandbits(32)]))
    put(staff, 0x19c, 'I', r.choice([1, 1, 0, 2, r.getrandbits(32)]))
    regions[RAM] = staff
    world = struct.unpack('<I', regions[0x80239c])[0]
    h1, h2, h3 = r.sample(range(1, 16), 3)
    w = bytearray(r.bytes(0x20))
    put(w, 8, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    put(w, 0x10, 'H', h1)
    put(w, 0x18, 'H', h2)
    regions[world + 0x1da708] = w
    rec = bytearray(r.bytes(0x80))
    for k in range(6):
        if r.random() < 0.5:
            put(rec, 0x28 + 12 * k, 'I', 0)
    regions[REC] = rec
    table = bytearray(r.bytes(20 * 16))
    put(table, 20 * h1, 'I', REC)
    put(table, 20 * h3, 'III', THING, h3, 0)
    regions[0x7cfb90] = table
    thing = bytearray(r.bytes(0x40))
    put(thing, 0, 'H', h3)
    thing[2] = r.choice([1, 1, 1, 4, 0])
    thing[5], thing[7] = staff[5], staff[7]
    regions[THING] = thing
    regions[0x7cf56c] = struct.pack('<I', r.choice([0, 0x7cfb90 + 20 * h3]))
    column, row = (cid - 1) & 0x7f, (cid - 1) >> 7
    if r.random() < 0.05:
        column, row = r.getrandbits(32), r.getrandbits(32)
    regions[0x7855bc] = struct.pack('<II', column, row)
    regions[0x75c7f0] = r.bytes(4)
    return Call(0x506300, [RAM], regions, conv='t', ret='v')


def leave_rest(r, regions, table, h):
    """LeaveRest's surroundings: the rest area h's thing (its ride id +0x24) in table, the ride list, the message centre."""
    regions[0x788d3c] = struct.pack('<I', RAM + 0x3b000)
    rid = r.choice([1, 2, 7])
    put(table, 20 * h, 'I', AREA)
    regions[AREA] = put(bytearray(r.bytes(0x40)), 0x24, 'I', r.choice([rid, rid, rid, 3]))
    node = bytearray(r.bytes(0x100))
    put(node, 0, 'I', 0)
    put(node, 8, 'I', rid)
    put(node, 0x1c, 'I', RIDES + 0x100)
    put(node, 0x8c, 'i', r.choice([1, 1, 2, 0]))
    regions[RIDES] = node
    regions[RIDES + 0x100] = put(bytearray(r.bytes(0x10)), 0, 'I', r.choice([0, 1, 5, r.getrandbits(32)]))
    regions[0x8791a0] = struct.pack('<I', r.choice([0, 1, 1, 1]))
    regions[0x8791b0] = struct.pack('<I', RIDES)


def message(r, regions, staff):
    """Message (0x504c70): see the header."""
    kind = r.choice([0xc] * 3 + [0xa] * 6 + [0x1b, 9])
    h = r.randrange(1, 16)
    state = r.choice([3, 3, 3, 3, 2, 2, 0, 1, 7])
    regions.pop(WORLD)
    put(staff, 0x20, 'I', r.choice([0, 1, 2, 3]))   # (LeaveRest's sprite: a whole dword)
    found = kind == 0xa and state == 3 and r.random() < 0.6
    me = STAFF
    if found:   # (navpath's person as the staff member, the destination the rest area's cell)
        cid = r.randrange(1, 0x4001)
        nav = dict(navpath.person_case(r, cell_xy(cid)).regions)
        for k in (RAM, RAM + 0x100, RAM + 0x200):
            regions.pop(k, None)
        regions.update(nav)
        person = bytearray(nav[RAM])
        person += bytearray(r.bytes(max(0, 0x240 - len(person))))
        for off in (0xc, 0xb4, 0x20, 0x24):
            put(person, off, 'I', struct.unpack_from('<I', staff, off)[0])
        staff, me = person, RAM
        base = struct.unpack('<I', regions[0x80239c])[0] + 0x1da708
    else:
        cid = r.randrange(1, 0x4001)
        base = WORLD - 4
        regions[0x80239c] = struct.pack('<I', WORLD - 0x1da70c)
    regions[0x7cf83c] = regions[0x80239c]
    put(staff, 0x208, 'H', h)
    put(staff, 0x19c, 'I', state)
    staff[2] = r.choice([4, 5, 6, 7, 8] * 2 + [0, 1, r.getrandbits(8)])
    put(staff, 0x1e4, 'I', r.choice([0, 1, 2, 3, 4]))
    regions[me] = staff
    regions[MSG + 0x40] = struct.pack('<I', recorder(19, 0))
    set_return(regions[LOG], 19, kind)
    regions[MSG] = put(put(bytearray(r.bytes(0x10)), 0, 'I', MSG + 0x40), 8, 'H', h if r.random() < 0.85 else r.randrange(16))
    world = bytearray(r.bytes(0x40))   # (the world's +0x1da708-0x1da747)
    table = bytearray(r.bytes(20 * 16))
    h_money, h_paid = r.sample([k for k in range(1, 16) if k != h], 2)
    put(world, 0x1e, 'H', h_money)
    put(world, 0x18, 'H', h_paid)
    put(table, 20 * h_money, 'I', MONEY)
    put(table, 20 * h_paid, 'I', PAID)
    regions[MONEY] = put(bytearray(r.bytes(0x130)), 0x114, 'I', r.choice([1, 1, 1, 0]))
    regions[PAID + 0x1f5a0] = r.bytes(4)
    regions[PAID + 0x1f7f0] = r.bytes(4)
    regions[0x78532c] = b''.join(struct.pack('<i', r.choice([0, 1, 50, 300, -20, r.getrandbits(16)])) for _ in range(0x54 // 4 + 8))
    leave_rest(r, regions, table, h)
    others = [k for k in range(1, 16) if k not in (h, h_money, h_paid)]
    rests = r.sample(others, r.choice([1, 1, 2, 3])) if found else r.sample(others, r.choice([0, 0, 1, 2]))
    blob = bytearray(r.bytes(0x40 * 4))
    for i, k in enumerate(rests):
        put(table, 20 * k, 'I', RESTS + 0x40 * i)
        put(blob, 0x40 * i + 0xc, 'H', rests[i + 1] if i + 1 < len(rests) else 0)
        blob[0x40 * i + 0x32] = r.choice([4, 4, 0xff, 0, 3]) if not found else 4 | r.getrandbits(8)
        put(blob, 0x40 * i + 0x36, 'H', cid)
    regions[RESTS] = blob
    put(world, 0x3e, 'H', rests[0] if rests else 0)
    regions[base] = world
    regions[0x7cfb90] = table
    blob_bytes(regions[LOG], 21, 0, 0xf3ff)
    return Call(0x504c70, [me, MSG], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Head':
        return head(r)
    if op == 'Mechanic' and r.random() < 0.7:
        return mechanic(r)
    if op == 'Handyman' and r.random() < 0.7:
        staff = bytearray(r.bytes(0x240))
        regions = {STAFF: staff, 0x75c7f0: r.bytes(8), 0x80239c: struct.pack('<I', WORLD - 0x1da70c), WORLD: r.bytes(4)}
        return Call(0x4d7330, [STAFF, r.choice([8, 9, 0xa, 0xb])], regions, conv='t', ret='v')
    if op == 'Researcher' and r.random() < 0.6:
        regions = {STAFF: r.bytes(0x240), 0x80239c: struct.pack('<I', WORLD - 0x1da70c), WORLD: r.bytes(4)}
        return Call(0x502c20, [STAFF, 0xf], regions, conv='t', ret='v')
    regions = dict(assign(r, 'AssignDrop').regions)
    regions[HEAP] = heap(fail=r.choice([0] * 8 + [1, 2, 3]))
    regions.pop(RAM)   # (assign()'s mode)
    n = struct.unpack_from('<I', regions[0x7b49f0], 4)[0]
    staff = bytearray(r.bytes(0x240))
    for off in (0xc, 0xb4):
        put(staff, off, 'I', r.choice([0, 0, r.randrange(n), r.randrange(n)]))
    put(staff, 0x19c, 'I', r.choice([1, 1, 0, 2, 7, 8, r.getrandbits(32)]))
    put(staff, 0x24, 'I', r.randrange(10))   # (the folder: one of assign()'s 0-9)
    staff[0x20] = r.choice([0, 1, 2, 3, 0, 1, r.getrandbits(8)])
    regions[STAFF] = staff
    regions[0x80239c] = struct.pack('<I', WORLD - 0x1da70c)
    regions[WORLD] = r.bytes(4)
    regions[MODE_VT] = vtable({0: 1})
    regions[MODE] = put(bytearray(r.bytes(0x10)), 0, 'I', MODE_VT)
    regions[0x7b05d4] = put(put(bytearray(r.bytes(0x18)), 4, 'I', r.choice([MODE, MODE, 0])), 0x14, 'I', r.choice([0, HOLDER]))
    regions[0xfb3518] = bytes([r.choice([1, 3, 0xff])])
    regions[0xfb3b7c] = struct.pack('<I', 1)
    state = r.choice([0, 1, 2, 3, 4, 5, 6, 7, 7, 7]) if r.random() < 0.95 else r.choice([8, 9, 0xffffffff, r.getrandbits(32)])
    assert LOG in regions
    if op in ('PickUp', 'Settle'):
        if op == 'Settle':
            put(staff, 0x19c, 'I', r.choice([0, 1, 2, 3, 4, 5, 6, 7, 0xd, 0x12, 8, 0x11, r.getrandbits(32)]))
            staff[2] = r.choice([7, 7, 7, 0, 1, r.getrandbits(8)])
        else:
            put(staff, 0x19c, 'I', r.choice([4, 5, 7, 0xd, 0x12] * 3 + [0, 1, 2, 3, 6, 8, r.getrandbits(32)]))
        staff[5], staff[7] = r.randrange(128), r.randrange(4)
        regions[CELLS] = r.bytes(0x44 * 4 * 128)
        regions[0x8023a0] = struct.pack('<I', CELLS)
        regions[MODE_VT] = vtable({0: 1, 9: 0})
        set_return(regions[LOG], 9, r.choice([6] * 5 + [8, 0, 7, r.getrandbits(32)]))
        h = r.randrange(16)
        regions[WORLD] = put(bytearray(r.bytes(0x10)), 0xc, 'H', h)
        regions[0x7cfb90 + 20 * h] = struct.pack('<I', RECORD)
        regions[RECORD] = r.bytes(0x80)
        if op == 'Settle':
            cells = bytearray(regions[CELLS])
            put(cells, 0x44 * (staff[7] * 128 + staff[5]) + 8, 'I', r.choice([0, 1, 3, 9, 0xa, 2, 4, 0xc, 0x11, r.getrandbits(32)]))
            regions[CELLS] = cells
        return Call(CALLS[op], [STAFF], regions, conv='t', ret='v')
    if op == 'Message':
        return message(r, regions, staff)
    if op == 'WalkRest':
        h = r.randrange(16)
        put(staff, 0x208, 'H', h)
        table = bytearray(r.bytes(20 * 16))
        leave_rest(r, regions, table, h)
        regions[0x7cfb90] = table
        blob_bytes(regions[LOG], 21, 0, 0xf3ff)
        set_return(regions[LOG], 28, r.choice([0, 0, 2, 2, 1, 3]))
        put(staff, 0x1f8, 'I', r.choice([fb(r.choice([0.0, 50.0, 100.0, 100.5, 101.0, 5.0, -0.0, 3e38, -3e38, r.uniform(-50, 200)])),
                                          0x7fc00000, 0x7f800001, 0xff800000, 0x7f800000]))
        regions[0x785328] = struct.pack('<i', r.choice([0, 1, 5, 10, 50, -5, -50, 0x7fffffff, -0x80000000, r.getrandbits(31)]))
        return Call(CALLS[op], [STAFF], regions, conv='t', ret='v')
    if op == 'LeaveRest':
        h = r.randrange(16)
        put(staff, 0x208, 'H', h)
        put(staff, 0x20, 'I', r.choice([0, 1, 2, 3]))   # (the sprite the script starts with: a dword here, so the upper bytes clear)
        regions[0x788d3c] = struct.pack('<I', RAM + 0x3b000)
        blob_bytes(regions[LOG], 21, 0, 0xf3ff)
        rid = r.choice([1, 2, 7])
        regions[0x7cfb90 + 20 * h] = struct.pack('<I', AREA)
        regions[AREA] = put(bytearray(r.bytes(0x40)), 0x24, 'I', r.choice([rid, rid, rid, 3]))
        node = bytearray(r.bytes(0x100))
        put(node, 0, 'I', 0)
        put(node, 8, 'I', rid)
        put(node, 0x1c, 'I', RIDES + 0x100)
        put(node, 0x8c, 'i', r.choice([1, 1, 2, 0]))
        regions[RIDES] = node
        regions[RIDES + 0x100] = put(bytearray(r.bytes(0x10)), 0, 'I', r.choice([0, 1, 5, r.getrandbits(32)]))
        regions[0x8791a0] = struct.pack('<I', r.choice([0, 1, 1, 1]))
        regions[0x8791b0] = struct.pack('<I', RIDES)
        return Call(CALLS[op], [STAFF], regions, conv='t', ret='v')
    return Call(CALLS[op], [STAFF, state], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
