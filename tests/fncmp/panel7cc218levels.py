# Panel7cc218_FillLevels (src/game/ui/panel7cc218: 0x4ae140) against the original: every call a recorder numbered as Panel7cc218LevelsEnv from 10; the definitions, control records and words answered in turn.
# usage: panel7cc218levels.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, blob_bytes, log_region, put, set_return  # noqa: E402

B = Bench([(0x4ae140, 0x4ae42d)],
          hooks={0x6649d5: _recorder(10, 0), 0x412e60: _recorder(11, 0), 0x412e70: _recorder(12, 0), 0x550ca0: _recorder(13, 0), 0x67a7d0: _recorder(14, 1, cdecl=True), 0x4d3d10: _recorder(15, 2), 0x412fb0: _recorder(16, 3), 0x66403b: _recorder(17, 4), 0x4dd4e0: _recorder(18, 0),
                 0x4e03e0: _recorder(19, 0), 0x6acd60: _recorder(20, 5, cdecl=True), 0x485b00: _recorder(21, 1, cdecl=True), 0x67c430: _recorder(22, 2, cdecl=True), 0x663324: _recorder(23, 0)})
LIST, NEXTS, RECORDS, WORDS, OBJECT, OWN, DEFS, RECS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x8000


def case(r):
    log = log_region()
    regions = {LOG: log}
    ident = r.getrandbits(32)
    defs = []
    for i in range(r.choice([0, 1, 2, 3, 5])):
        d = bytearray(r.bytes(0x510))
        put(d, 0x508, 'I', r.choice([0, 0, 0, 1]))
        put(d, 0x284, 'I', r.choice([ident, ident, r.getrandbits(32)]))
        regions[DEFS + 0x800 * i] = d
        defs.append(DEFS + 0x800 * i)
    set_return(log, 11, defs[0] if defs else 0)
    regions[NEXTS] = struct.pack('<40I', *(defs[1:] + [0] * (41 - len(defs)))[:40])
    answers(log, 12, NEXTS)
    recs = []
    for i in range(6):
        rec = bytearray(r.bytes(0x20))
        rec[0x10] = r.choice([0, 1, 1, 0xff])
        put(rec, 0x14, 'I', r.choice([0, 1, 2, 3, 0xffffffff]))
        regions[RECS + 0x40 * i] = rec
        recs.append(RECS + 0x40 * i)
    regions[RECORDS] = struct.pack('<40I', *(recs + [recs[-1]] * 34))
    answers(log, 15, RECORDS)
    regions[WORDS] = struct.pack('<40I', *([r.getrandbits(32) | 1 for _ in range(r.randrange(4))] + [0] * 40)[:40])
    answers(log, 22, WORDS)
    obj = bytearray(r.bytes(0x60))
    obj[0x50] = r.choice([0, 0, 1, 1, 2, 2, 3, 0xff])
    regions[OBJECT] = obj
    regions[OWN] = r.bytes(0x300)
    set_return(log, 18, OWN)
    set_return(log, 19, r.choice([0, 0, 0, 1, r.getrandbits(32) & 0xffff0000]))
    for k in (17, 20, 21):
        set_return(log, k, r.getrandbits(32))
    blob_bytes(log, 17, 0, 0xff)
    blob_bytes(log, 20, 4, 0xfffff)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 15, 1 << 16)   # (the ids are words over stale upper halves)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 16, 1 << 16)
    regions[0xfb34c5] = bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([2, 0, 0, 0, 1, r.getrandbits(32)]))
    regions[0x7cf83c] = struct.pack('<I', r.getrandbits(32))
    regions[0x78ba94] = r.bytes(4)
    return Call(0x4ae140, [ident, LIST, r.choice([0, 1, r.getrandbits(32)]), OBJECT], regions)


if __name__ == '__main__':
    B.main(case)
