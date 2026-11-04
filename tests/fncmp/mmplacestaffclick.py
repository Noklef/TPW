# The staff member placed with the mouse (src/game/world/mousemode: 0x46c8e0) against the original: a cell (and its handle's entry and parent) of any type and flags, kinds 4-8 and others, the block failing or not; the constructors recorders whose tile and sprite arguments keep only what the callee reads (the original pushes whole registers, their rest left by the code before); MouseMode_Set the game's on a holder there or made, the current mode a recorder one, CMMOnline's vtable (0x6fea10) recorders as stafftake.py's: its slot 10 is the game's cursor code (0x46c6b0, not ported).
# usage: mmplacestaffclick.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, vtable  # noqa: E402

HANDYMAN, ENTERTAINER, GUARD, MECHANIC, RESEARCHER = range(1, 6)
RECORDERS = {k: _recorder(k, 4) for k in (HANDYMAN, ENTERTAINER, GUARD, MECHANIC, RESEARCHER)}


def ctor(k):
    def hook(e):
        tile, kind, sprite = struct.unpack('<3I', e.read(e.r['ESP'], 12))
        e.write(e.r['ESP'], struct.pack('<3I', tile & 0xffff, kind, sprite & 0xff))
        RECORDERS[k](e)
    return hook


B = Bench([(0x46c8e0, 0x46cb9e), (0x46c350, 0x46c448), (0x46c6a0, 0x46c6b0), (0x46d550, 0x46d58b), (0x46c450, 0x46c453), (0x46c460, 0x46c480), (0x46c710, 0x46c72e),
           (0x535db0, 0x535dc3), (0x536300, 0x536484), (0x53ad20, 0x53aefc), (0x53c8d0, 0x53c9c0), (0x5da3c0, 0x5da3c1), (0x5d1750, 0x5d1751), (0x5d1410, 0x5d1420), (0x67b0c0, 0x67b0d0),
           (0x4d0a70, 0x4d0a86), (0x4d0a90, 0x4d0ab1), (0x4d0ac0, 0x4d0ae1), (0x4d0af0, 0x4d0b06), (0x4d0b10, 0x4d0b27)],
          hooks={0x4d6b60: ctor(HANDYMAN), 0x4d4340: ctor(ENTERTAINER), 0x4d5d40: ctor(GUARD), 0x4d9eb0: ctor(MECHANIC), 0x502600: ctor(RESEARCHER)})
HTYPES = [0, 0, 1, 1, 3, 9, 0, 1, 3, 9, 2, 4, 0xb, 0xc, 0xd, 0x10, 0x11, 0x12, 0x19, 0x1e]
SELF, OLD, OLD_VT, HOLDER = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
CELLS = RAM + 0x1000   # the 0x44-byte cells: [0x7cf83c] + 0x294
ENTRIES = CELLS - 0x294 + 0x1102b0   # the 0x28-byte entries of handles 0-5


def case(r):
    self = bytearray(r.bytes(0x20))
    put(self, 0xc, 'I', r.choice([4, 5, 6, 7, 8] * 3 + [3, 9, 0, r.getrandbits(32)]))
    tile = r.randrange(4)
    cells = bytearray(r.bytes(4 * 0x44))
    entries = bytearray(r.bytes(6 * 0x28))
    for c, k in [(cells, tile * 0x44)] + [(entries, i * 0x28) for i in range(6)]:
        put(c, k + 8, 'I', r.choice(HTYPES + [r.getrandbits(32)]))
        put(c, k + 0xe, 'H', r.choice([0, 0, 0, 0, 0x40, 0x400, 8, r.getrandbits(16)]))
        put(c, k + 0x10, 'H', r.choice([0, 1, 2, 5, r.randrange(6)]))
    put(cells, tile * 0x44, 'H', r.randrange(6))
    if r.random() < 0.3:   # a parented entry (parent 0 too) of a parent of type 0x19 or not
        h, p = r.randrange(1, 6), r.choice([0, 0, r.randrange(6)])
        put(entries, h * 0x28 + 8, 'I', r.choice([0xc, 0x11]))
        put(entries, h * 0x28 + 0x10, 'H', p)
        put(entries, p * 0x28 + 8, 'I', r.choice([0x19, 0x19, 0xd, 1]))
        put(cells, tile * 0x44, 'H', h)
    regions = {LOG: log_region(), HEAP: heap(fail=r.choice([0, 0, 0, 1, 2, 3, 4])), SELF: self, CELLS: cells, ENTRIES: entries, HOLDER: r.bytes(4),
               OLD: put(bytearray(r.bytes(0x20)), 0, 'I', OLD_VT), OLD_VT: vtable({0: 1, 11: 0}, size=16), 0x6fea10: vtable({0: 1, 9: 0, 10: 0}, size=12),
               0x7b05cc: put(bytearray(r.bytes(4)), 0, 'H', tile), 0x7cf83c: struct.pack('<I', CELLS - 0x294), 0x8023a0: struct.pack('<I', ENTRIES - 0x10ffd8),
               0x7b05d4: r.bytes(4), 0x7b05d8: struct.pack('<I', r.choice([0, OLD])), 0x7b05e0: r.bytes(4), 0x7b05e8: struct.pack('<I', r.choice([0, HOLDER])),
               0xfb3518: b'\x01', 0xfb3b7c: struct.pack('<I', r.choice([0, 2]))}
    return Call(0x46c8e0, [SELF, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
