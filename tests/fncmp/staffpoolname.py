# A staff pool entry's name text (src/game/world/staff: 0x5074a0) against the original. Name: the entry's kind (+0) 0-4 mostly (or past
# them: the debug print only), its text id (+4) any; the five text tables 0x78baa8 - 0x78bab8 any; the buffer and its size (9 or more,
# so that the text formatter's log stub (0x6acd60, driver: TextFormatEnv) writes its hash there) passed on.
# usage: staffpoolname.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, _cdecl_log4, log_region, put  # noqa: E402

B = Bench([(0x5074a0, 0x507560), (0x5da3c0, 0x5da3c1)], hooks={0x6acd60: _cdecl_log4})
CALLS = {'Name': 0x5074a0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
ENTRY, BUF = RAM, RAM + 0x100


def case(r):
    r.deal(OPS)
    entry = bytearray(r.bytes(0x14))
    put(entry, 0, 'I', r.choice([0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 0xffffffff, r.getrandbits(32)]))
    regions = {LOG: log_region(), ENTRY: entry, BUF: r.bytes(0x100), 0x78baa8: struct.pack('<5I', *[r.getrandbits(32) for _ in range(5)])}
    return Call(CALLS['Name'], [ENTRY, BUF, r.choice([9, 0x20, 0x40, 0x7f])], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
