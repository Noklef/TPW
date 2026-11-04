# The advisor's choice among its criteria (src/game/core/advisorscoring: 0x59c680) against the original: the criteria table 0x76e300 replaced by a dealt one (entries skipped, ids their own, another's, one not in the table or the two out of range; kinds 0, 1, 2, others; the game state 0xfb3b7c any and its static guard either way), the criteria recorders answering scores, the game table's and object's calls recorders; the cursor anywhere, near the wrap too; 0-8 steps.
# usage: advisorpick.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_fold, log_region, put, recorder, set_return  # noqa: E402

ENTRY, READY, INIT, ATEXIT, CRITERION = 2, 3, 4, 5, 6
TABLE, COUNT = 0x76e300, 0x15f
B = Bench([(0x59c680, 0x59c855), (0x5da3c0, 0x5da3c1)], hooks={0x5194d0: _recorder(ENTRY, 2), 0x59abc0: _recorder(READY, 0), 0x550ca0: _recorder(INIT, 0), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True)})
SELF, POINTER, GAME, SCORES, READIES = RAM, RAM + 0x400, RAM + 0x440, RAM + 0x800, RAM + 0x900
ID_NONE = 0x500   # (no entry has it)


def table(r, start):
    data = bytearray(0x38 * (COUNT + 1))
    for k in range(COUNT + 1):
        put(data, 0x38 * k + 4, 'i', k)
    for j in range(0x30):
        k = (start + j) % COUNT
        entry = bytearray(r.bytes(0x38))
        other = r.randrange(COUNT)
        ident = r.choice([k, k, k, k, other, other, 0x160, 0x15f, ID_NONE])
        put(entry, 0, 'I', r.choice([0, 0, 0, 0, 0, 1]))
        put(entry, 4, 'i', ident)
        put(entry, 0xc, 'i', r.choice([2, 2, 2, 0, 0, 1, 1, 3]))
        put(entry, 0x10, '2I', recorder(CRITERION, 0), r.randrange(0, 0x40) * 4)
        data[0x38 * k:0x38 * k + 0x38] = entry
        if ident < COUNT and ident != k and r.randrange(2):   # (the other's entry not its own: the search finds k, the address is by the id, sic)
            put(data, 0x38 * ident + 4, 'i', r.randrange(COUNT))
            put(data, 0x38 * ident + 0xc, 'i', r.choice([2, 2, 0, 1]))
    return data


def case(r):
    log = log_region()
    log_fold(log)
    start = r.choice([0, r.randrange(COUNT), COUNT - 1, COUNT - 3, COUNT, COUNT + 5])
    self = bytearray(r.bytes(0x100))
    put(self, 0, 'i', start)
    set_return(log, ENTRY, r.getrandbits(32))
    answers(log, CRITERION, SCORES)
    answers(log, READY, READIES)
    scores = [r.choice([0, -1, 1, 2, 3, 5, 100, r.randrange(1 << 31), r.getrandbits(32)]) for _ in range(0x40)]
    readies = [r.choice([1, 1, 1, 0, 0x100, 0xff00]) for _ in range(0x40)]
    regions = {LOG: log, SELF: self, TABLE: table(r, 0 if start >= COUNT else start), 0x7cf6ec: struct.pack('<I', POINTER), POINTER: struct.pack('<I', GAME), GAME: r.bytes(0x10),
               SCORES: struct.pack('<64I', *[s & 0xffffffff for s in scores]), READIES: struct.pack('<64I', *readies),
               0xfb34bc: struct.pack('<B', r.choice([1, 1, 0, 0xff, 0xfe])), 0xfb3b7c: struct.pack('<I', r.choice([0, 0, 2, 2, 1, r.getrandbits(32)]))}
    return Call(0x59c680, [SELF, r.choice([0, 1, 2, 3, 4, 8, -1])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
