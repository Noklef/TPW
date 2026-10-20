# A staff member's state set (0x5054d0 Staff_SetState, src/game/world/staff) against the original: the state 0-7 mostly (past 7 now and
# then: only stored), the old state +0x19c 1 or not (state 0's stamp), the world's tick [0x80239c] + 0x1da70c any. The scripts and, for
# state 7 (the staff dropped: an MMStaffDrop mode of the folder +0x24 and sprite +0x20, its Assign starting a sprite script), mousemode.py's
# assign() surroundings for its AssignDrop (the cursor windows, the sprite folders, the script table and heap; an allocation failing now and
# then); the reference +0xc's slot and the +0x30 object's (+0xb4) one of the table's or 0; the current mode [0x7b05d8] a recorder object or
# none, the holder [0x7b05e8] there or not (a CMMOnline base made then, replaced by a CMMOnline: the setting 0xfb3b7c 1, its guard set).
# usage: staffstate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import wctimage  # noqa: E402
from fncmp import HEAP, LOG, Bench, Call, RAM, heap, put, vtable  # noqa: E402
from focus import MODES  # noqa: E402
from mousemode import assign  # noqa: E402

B = Bench(wctimage.merged(MODES + [
    (0x402d70, 0x403200), (0x40f350, 0x40f370), (0x4217f0, 0x421810), (0x46c350, 0x46c460), (0x46c6a0, 0x46c6c0), (0x46c710, 0x46c740),
    (0x46cbc0, 0x46cc50), (0x46cdc0, 0x46cef0), (0x46cfb0, 0x46cfd0), (0x475550, 0x4755a0), (0x4758f0, 0x475b90), (0x4814a0, 0x481500),
    (0x4a2aa0, 0x4a2ad0), (0x4d4140, 0x4d41a0), (0x4da5b0, 0x4da750), (0x4dad20, 0x4dadd0), (0x4fa460, 0x4fa480), (0x4fa930, 0x4fa9a0),
    (0x5054d0, 0x5056f0), (0x505ea0, 0x505f60), (0x509af0, 0x509b50), (0x50b350, 0x50b370), (0x50c0d0, 0x50c110), (0x519450, 0x5194e0),
    (0x540c60, 0x540da0), (0x550ca0, 0x550d90), (0x5f5660, 0x5f56a0), (0x6590f7, 0x6591e4), (0x65f11d, 0x65f160), (0x67a740, 0x67a800),
    (0x67fa20, 0x67fa40), (0x5d1410, 0x5d1416), (0x5d1750, 0x5d1751), (0x5da3c0, 0x5da3c1)]))
STAFF, WORLD, MODE, MODE_VT, HOLDER = RAM + 0x20000, RAM + 0x21000, RAM + 0xc000, RAM + 0xc100, RAM + 0xc200


def case(r):
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
    return Call(0x5054d0, [STAFF, state], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
