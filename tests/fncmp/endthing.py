# The "End" object made (src/game/world/endthing: EndThing_Create 0x516b00) against the original. The definitions (gObjectDefs at
# 0x7890a0, 1-6 of them, the count now and then short of them) with wide names at +0x390: "End" in another case, near misses
# ("En", "Ends", "Emd") or anything, the match anywhere or nowhere; the WString at 0x7cf9c0 with "End" (now and then something else) as
# its data. The heap (Mem_Alloc) failing now and then; agent C's unported object constructor 0x4db090 a thiscall recorder (k 11 of 4,
# driver.cpp's kEndThingEnv; the cell and the id logged as words, the constructor reading only those) answering an object (now and then 0:
# the handle then read from address 0 in both); agent C's 0x481b60 (thiscall on 0x7b51f0) recorder 12 of 1.
# usage: endthing.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, heap, log_region, put, set_return  # noqa: E402

B = Bench([(0x412e60, 0x412f00), (0x50b350, 0x50b370), (0x516b00, 0x516c10), (0x67b140, 0x67b25a), (0x67dcd0, 0x67dd00),
           (0x67e670, 0x67e700), (0x682720, 0x6827d0), (0x5da3c0, 0x5da3c1)],
          hooks={0x4db090: _recorder(11, 4), 0x481b60: _recorder(12, 1)})
DEFS, ITEMS, ITEM, OBJECT, NAME = 0x7890a0, RAM + 0x1000, 0x400, RAM + 0x3000, RAM + 0x3800


def wide(s):
    return s.encode('utf-16-le') + b'\0\0'


def name(r):
    return r.choice(['End', 'end', 'END', 'eNd', 'En', 'Ends', 'Emd', 'Fnd', '', 'Entrance', ''.join(chr(r.randrange(0x20, 0x7f)) for _ in range(r.randrange(1, 8)))])


def case(r):
    log = log_region()
    regions = {HEAP: heap(fail=r.choice([0] * 12 + [1])), LOG: log}
    n = r.randrange(1, 7)
    dtab = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(ITEM * n))
    match = r.choice([None, r.randrange(n), r.randrange(n)])
    for i in range(n):
        put(dtab, 4 * i, 'I', ITEMS + ITEM * i)
        s = r.choice(['End', 'end', 'END', 'eNd']) if i == match else name(r)
        items[ITEM * i + 0x390:ITEM * i + 0x390 + 2 * len(s) + 2] = wide(s)
    put(dtab, 0x528, 'i', r.choice([n, n, n, n - 1, 0, 1, -r.randrange(1, 1 << 31)]))
    put(dtab, 0x72c, 'I', r.getrandbits(32))
    regions.update({DEFS: dtab, ITEMS: items})
    wstr = bytearray(r.bytes(0x14))
    put(wstr, 8, 'I', NAME)
    regions[0x7cf9c0] = wstr
    regions[NAME] = wide(r.choice(['End'] * 6 + ['end', 'Ends', '']))
    regions[0x7cf4f4] = r.bytes(4)
    regions[OBJECT] = r.bytes(0x40)
    set_return(log, 11, r.choice([OBJECT] * 12 + [0]))
    struct.pack_into('<I', log, LOG_BYTES + 4 * 11, 3 << 16)   # (the cell and the id: words)
    return Call(0x516b00, [], regions)


if __name__ == '__main__':
    B.main(case)
