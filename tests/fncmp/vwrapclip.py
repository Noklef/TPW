# The 8bba48 object's 0x78 entries (src/game/core/vwrap: 0x57c4d0 Global8bba48_ClipAndTouch) against the original: each
# entry's count (k 1), box (k 2, six integers through two pointers), clip code (k 6, or 0x7c for an empty box), records
# (k 3) with each one's texture touched (k 5), and the records given back (k 4).
# usage: vwrapclip.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, recorder, strings  # noqa: E402

COUNT, BOX, RECORDS, GIVE, TOUCH, CLIP = 1, 2, 3, 4, 5, 6
B = Bench([(0x57c4d0, 0x57c61c)], hooks={0x561a30: _recorder(CLIP, 2), 0x58c9f0: _recorder(TOUCH, 2)})
SELF, TABLES, RECS, ITEMS = RAM + 0x100, RAM + 0x200, RAM + 0x800, RAM + 0xc00


def box(r):
    kind = r.random()
    if kind < 0.3:
        return [0, 0, 0]
    if kind < 0.6:
        return [r.choice([0, 0, 1, -1, 1024, -1024]) for _ in range(3)]
    return [r.getrandbits(32) - (1 << 31) for _ in range(3)]


def case(r):
    log = log_region()
    regions = {LOG: log}
    me = bytearray(r.bytes(0x98))
    for off, value in ((-0x20, recorder(BOX, 3, cdecl=True)), (-0x10, recorder(RECORDS, 1, cdecl=True)),
                       (-0xc, recorder(GIVE, 1, cdecl=True)), (-8, recorder(COUNT, 1, cdecl=True))):
        struct.pack_into('<I', me, 0x98 + off, value)
    regions[SELF - 0x98] = bytes(me)
    regions[0x879a20] = r.bytes(4)
    regions[0x87b088] = r.bytes(4)
    regions[0x8c4b44] = r.bytes(4)
    items = [ITEMS + 4 * i for i in range(8)]
    regions[ITEMS] = r.bytes(0x20)
    recs = bytearray(r.bytes(0x100))
    for i in range(8):
        struct.pack_into('<I', recs, 0x10 + 0x1c * i, r.choice(items + [0xffffffff, 0xffffffff]))
    regions[RECS] = bytes(recs)
    density = r.choice([0, 2, 8, 30])
    counts = [r.choice([1, 1, 2, 3, 8, 0x80000000, 0xffffffff]) if r.randrange(40) < density else 0 for _ in range(40)]
    regions[TABLES] = struct.pack('<40I', *counts)
    answers(log, COUNT, TABLES)
    # (a negative count touches nothing, as the walk is under `count > 0`; its records stay wanted)
    regions[TABLES + 0x100] = struct.pack('<40I', *[r.choice([0, RECS, RECS, RECS]) for _ in range(40)])
    answers(log, RECORDS, TABLES + 0x100)
    regions[TABLES + 0x200] = struct.pack('<40I', *[r.choice([0, 0x40, 0x7c, 0x1234567f, r.getrandbits(32)]) for _ in range(40)])
    answers(log, CLIP, TABLES + 0x200)
    first, second = [], []
    for _ in range(8):
        first += box(r)
        second += box(r) if r.random() < 0.6 or any(first[-3:]) else [0, 0, 0]
    fill(log, BOX, 1, struct.pack('<24i', *first), records=8)
    fill(log, BOX, 2, struct.pack('<24i', *second), which=1, records=8)
    strings(log, BOX, 0, out=1 << 1 | 1 << 2)
    blob(log, CLIP, 0, 0x3f)
    return Call(0x57c4d0, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
