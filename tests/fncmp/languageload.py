# The language object's resources (src/game/core/language: 0x419710) against the original: all that load, or one that fails at any of its steps (the folder, the file, the two tables, each charset, each font of the set), statuses at and about the sign, the setting and the screen that pick the set; every callee a recorder, the sprintf's text filled for OutputDebugStringA.
# usage: languageload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, fill, import_table, log_region, strings  # noqa: E402

OPEN, LOAD, RANGE, CODE, CHARSET, FONT, ERROR, STATUS = 1, 2, 3, 4, 5, 6, 7, 8
SPRINTF = 9
TH = {0x6ad1b0: (OPEN, 2), 0x6ad230: (LOAD, 2), 0x6ad4b0: (RANGE, 3), 0x6ad460: (CODE, 3), 0x6ad510: (CHARSET, 3), 0x6ad570: (FONT, 3), 0x405710: (ERROR, 3), 0x6ad790: (STATUS, 1)}
HOOKS = {a: _recorder(k, n) for a, (k, n) in TH.items()}
HOOKS[0x67a650] = _recorder(SPRINTF, 4, cdecl=True)
B = Bench([(0x419710, 0x41a594), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
SAY = 40
STATUSES = RAM + 0x100   # dwords below 0 and not, which the answers point at
GOOD = [0, 1, 0x7fffffff, 0x12345678]
BAD = [-1, -0x80000000, -2, -0x1000]
TABLES = RAM + 0x200   # 40 answers for each step
STEPS = [(OPEN, 1), (LOAD, 1), (RANGE, 1), (CODE, 1), (CHARSET, 21), (FONT, 13)]


def case(r):
    log = log_region()
    table = import_table(log, {'OutputDebugStringA': (SAY, 0x6fd1e0, 1, 1, 0)})
    fails = None
    if r.random() < 0.65:
        k, n = r.choice([(OPEN, 1), (LOAD, 1), (RANGE, 1), (CODE, 1), (CHARSET, 21), (CHARSET, 21), (CHARSET, 21), (FONT, 13), (FONT, 13), (FONT, 13)])
        fails = (k, r.randrange(n))
    tables = b''
    for index, (k, _) in enumerate(STEPS):
        address = TABLES + index * 160
        answers(log, k, address)
        strings(log, k, 0, out=1)
        rows = [STATUSES + 4 * r.randrange(4) for _ in range(40)]
        if fails and fails[0] == k:
            rows[fails[1]] = STATUSES + 16 + 4 * r.randrange(4)
        tables += struct.pack('<40I', *rows)
    strings(log, STATUS, 0, out=1)
    strings(log, SPRINTF, 0b1110, out=1)
    fill(log, SPRINTF, 0, r.bytes(8).hex().encode() + b'\0')
    regions = {LOG: log, IMPORT_TABLE: table, RAM: r.bytes(16), STATUSES: struct.pack('<4i4i', *GOOD, *BAD), TABLES: tables,
               0x7858ec: struct.pack('<i', r.choice([0, 0, 0, 1, 2, 3, 4, 5, 0x7fffffff, -1, -5])), 0x78d8dc: struct.pack('<i', r.choice([0, 1, 2, 3, 4, -1, 0x10000]))}
    return Call(0x419710, [RAM], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
