# StaffPool_TakeCandidate (src/game/world/staffcandidate, 0x507bf0) against the original. The pool: 32 entries of kinds 0-4 mostly, most in use, +0xb now and then, the sprite bytes
# small; no things, so Staff_CountTypes clears the flags, set by nothing (a thing list of its own is staffcandidate.py's). The vtables of CMMPlaceStaff (0x6fea40) and CMMOnline (0x6fea10)
# recorders: their slots 9 and 10 are the game's, not all ported (0x40f2d0, 0x46c6b0), and mousemode.py has Assign. Slot 9's answer 1 now and then, so the mode is replaced. The setting 1
# or not, its guard set; the current mode a recorder object or none, the holder made or not, the heap failing now and then. 0x481550 the thiscall recorder 20 of 1 (driver.cpp's
# StaffCandidateEnv).
# usage: stafftake.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x507bf0, 0x507fec), (0x508000, 0x508170), (0x516130, 0x5161c0), (0x46c6a0, 0x46c6b0), (0x46c6d0, 0x46c702), (0x46c710, 0x46c72e), (0x46c450, 0x46c453), (0x46c650, 0x46c653),
           (0x46ced0, 0x46ced7), (0x46cfb0, 0x46cfd0), (0x46cff0, 0x46d000), (0x5da3c0, 0x5da3c1), (0x67a400, 0x6902c0), (0x46ce80, 0x46ceca), (0x46d550, 0x46d59d), (0x46c350, 0x46c448),
           (0x4814a0, 0x4814e3), (0x4a2aa0, 0x4a2abf), (0x6590f7, 0x6591d4), (0x65f11d, 0x65f14e), (0x66190a, 0x66191b), (0x661d96, 0x661dca),
           (0x5d1750, 0x5d1751)],
          hooks={0x481550: _recorder(20, 1)})
POOL, HOLDER, CURRENT, CURRENTVT = RAM + 0x100000, RAM + 0x8300, RAM + 0x8400, RAM + 0x8500


def case(r):
    log = log_region()
    set_return(log, 9, r.choice([0, 0, 1, 5]))
    pool = bytearray(r.bytes(0x2a0))
    for i in range(0x20):
        put(pool, 0x14 * i, 'I', r.randrange(5) if r.random() < 0.9 else r.choice([5, 0xff]))
        put(pool, 0x14 * i + 8, 'BB', r.randrange(4), r.randrange(4))
        pool[0x14 * i + 0xa] = r.choice([1, 1, 1, 0xff, 0])
        pool[0x14 * i + 0xb] = r.choice([0, 0, 0, 1])
    regions = {LOG: log, POOL: pool, 0x80239c: struct.pack('<I', POOL), 0x7cf56c: bytes(4), HEAP: heap(fail=r.choice([0, 0, 0, 0, 1, 2, 3])),
               0x7b05e8: struct.pack('<I', r.choice([0, HOLDER])), 0x7b05d8: struct.pack('<I', r.choice([0, CURRENT])), CURRENT: struct.pack('<I', CURRENTVT),
               CURRENTVT: vtable({0: 1, 11: 0}, size=16), 0x6fea10: vtable({0: 1, 9: 0, 10: 0}, size=12), 0x6fea40: vtable({0: 1, 9: 0, 10: 0}, size=12),
               0xfb3518: b'\x01', 0xfb3b7c: struct.pack('<I', r.choice([0, 1, 2]))}
    return Call(0x507bf0, [POOL, r.choice([r.randrange(0x20)] * 6 + [0x20, 0x21, r.getrandbits(32)])], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
