# The error report shown (src/game/core/gameerror: GameError_Show, 0x405750) against the original: each index 0-0x2a and past it, set or not, the flag [0x78b530] or not; the string table's call a recorder writing the text;
# the C library's swprintf real for a text at +0xc; index 0xc pops the engine's errors (0x879ad0) to the last of kind 3. MessageBoxW a recorder hashing the text's bytes (none for the unset buffer: its address only masked).
# usage: gameerrorshow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob_bytes, fill, import_table, log_region, strings  # noqa: E402

FORMAT, BOX = 1, 44
RET4 = 0x6702c6
B = Bench([(0x405750, 0x405cb0), (0x56afa0, 0x56afc0), (RET4, RET4 + 3), (0x67a400, 0x690000)], hooks={0x6acd60: _recorder(FORMAT, 4, cdecl=True)})
REPORT = RAM
INDEXES = list(range(0x2b)) + [0x2b, 0x40]


def case(r):
    index = r.deal(INDEXES)
    log = log_region()
    table = bytearray(import_table(log, {'MessageBoxW': (BOX, 0x6fd378, 4, 0, 0)}))
    for slot in (0x6fd0c0, 0x6fd12c):   # (the locale's lock, for %hs in the original's C library: a RET 4, not logged; the driver's C library takes no such lock)
        struct.pack_into('<I', table, slot - IMPORT_TABLE, RET4)
    text = bytes(r.choice(b'abcd ') for _ in range(r.randrange(1, 6)))
    extra = bytes(r.choice(b'XY:') for _ in range(r.choice([0, 0, 1, 2, 5])))
    entries = bytearray(0x204)
    count = r.choice([0, 1, 2, 3])
    for i in range(count):
        struct.pack_into('<II', entries, 8 * i, r.choice([3, 3, 1, 0]), r.choice([0xb0754000 + r.randrange(0x19), 0xb0754100 + r.randrange(3), r.getrandbits(32)]))
    struct.pack_into('<i', entries, 0x200, r.choice([count, count, count, -1]))
    unset = index == 0xd or index > 0x2a
    if unset:
        extra = b''
    words = list(text) + ([] if not extra else [0x20, 0x28] + list(extra) + [0x29])
    wide = struct.pack('<%dH' % (len(words) + 1), *words, 0)
    fill(log, FORMAT, 1, text.decode('latin1').encode('utf-16-le') + bytes(2))
    strings(log, FORMAT, 0, out=1 << 1)
    if unset:
        strings(log, BOX, 0, out=1 << 1)
    else:
        blob_bytes(log, BOX, 1, (1 << min(len(wide), 32)) - 1)
    report = struct.pack('<III', r.choice([1, 1, 1, 1, 1, 1, 1, 0]), index, r.getrandbits(32)) + extra + b'\0' + r.bytes(0x4f - len(extra))
    regions = {LOG: log, IMPORT_TABLE: table, REPORT: report, 0x78b530: struct.pack('<I', r.choice([1, 1, 1, 1, 1, 1, 0, 0x100])), 0x78ba88: r.bytes(4), 0x879ad0: bytes(entries), 0xfb1440: bytes(4), 0xfb3ca4: bytes(8)}
    return Call(0x405750, [REPORT], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
