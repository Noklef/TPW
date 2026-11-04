# An answer from the online city (src/game/online/session: Online_ListAnswered, 0x5b6da0) against the original: every code of -11..9, a request of one of the nine lists or none, its old native list set or
# not, the data null or with a count of 1 or more. The imports are recorders: OutputDebugStringA (stdcall) and the lists' Release and destructors (thiscall).
# usage: onlinelistanswered.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, heap, log_region, put, recorder  # noqa: E402

B = Bench([(0x5b6da0, 0x5b726b)])
SESSION, LISTS, DATA = RAM, RAM + 0x200, RAM + 0x600
SLOTS = [0x6fd538, 0x6fd4ec, 0x6fd4e8, 0x6fd4f0, 0x6fd528, 0x6fd524, 0x6fd520, 0x6fd51c, 0x6fd518, 0x6fd514]
CODES = list(range(-11, 10)) + [2, 3, 3, 3, 3, 4, 5, 6]   # (a list kind's codes more often: parks has five lists)
ASKS = {2: [0], 3: [1, 2, 3, 4, 5], 4: [7], 5: [6], 6: [8]}   # a code's lists, by their place from +0xe0


def case(r):
    table = bytearray(0x600)
    struct.pack_into('<I', table, 0x6fd1e0 - IMPORT_TABLE, recorder(1, 1, True))
    for k, slot in enumerate(SLOTS):
        struct.pack_into('<I', table, slot - IMPORT_TABLE, recorder(2 + k, 0))
    session = bytearray(r.bytes(0x104))
    lists = bytearray(r.bytes(0x40 * 9))
    ids = list(range(100, 109)) if r.random() < 0.8 else [100] * 9
    for k in range(9):
        put(session, 0xe0 + 4 * k, 'I', LISTS + 0x40 * k)
        put(lists, 0x40 * k + 4, 'I', ids[k])
        put(lists, 0x40 * k + 0x30, 'I', r.choice([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    regions = {SESSION: session, LISTS: lists, IMPORT_TABLE: table, LOG: log_region(), HEAP: heap()}
    # The data freed needs a block of the heap region; the free only counts it.
    data = 0 if r.random() < 0.1 else DATA
    regions[DATA] = put(bytearray(r.bytes(0x10)), 0xc, 'I', r.choice([1, 1, 2, 0, r.getrandbits(32)]))
    code = r.deal(CODES)
    request = ids[r.deal(ASKS[code])] if code in ASKS and r.random() < 0.7 else r.choice(ids + [99])
    return Call(0x5b6da0, [SESSION, code & 0xffffffff, request, r.getrandbits(32), data], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
