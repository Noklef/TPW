# src/game/core/startcheck: Startup_CheckCanRun (0x45f640) against the original: regkey.py's registry (installed or not), the game error
# report 0x785de8 any bytes, CreateMutexA (0x6fd134) making a handle or failing (0), GetLastError (0x6fd138) ERROR_ALREADY_EXISTS (0xb7) or
# another, CloseHandle (0x6fd13c) any, all recorders in the import table; the mutex name at [0x7a2508] (hashed as a string) or none.
# usage: startcheck.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, import_table, set_return  # noqa: E402
import regkey  # noqa: E402

B = Bench([(0x45f640, 0x45f6a5), (0x405710, 0x405742), (0x45f6b0, 0x45f7b0), (0x581da0, 0x581dc0), (0x5f8ae0, 0x5f8d00), (0x5f8d40, 0x5f8dc0)])   # (and regkey.py's)
MUTEX, LAST, CLOSE_HANDLE = 5, 6, 7
IMPORTS = dict(regkey.IMPORTS, CreateMutexA=(MUTEX, 0x6fd134, 3, 4, 0), GetLastError=(LAST, 0x6fd138, 0, 0, 0),
               CloseHandle=(CLOSE_HANDLE, 0x6fd13c, 1, 0, 0))


def case(r):
    c = regkey.installed(r)
    regions = dict(c.regions)
    log = regions[LOG]
    regions[IMPORT_TABLE] = import_table(log, IMPORTS)
    set_return(log, MUTEX, r.choice([0x1234, 0x88, r.getrandbits(32), 0]))
    set_return(log, LAST, r.choice([0xb7, 0xb7, 0, 5, 0xb6, 0xb8, r.getrandbits(32)]))
    set_return(log, CLOSE_HANDLE, r.choice([0, 1]))
    names = bytearray(regions[RAM])
    names[0x30:0x40] = r.choice([b'ThemeParkWorld', b'TPW', b'']).ljust(0x10, b'\0')
    regions[RAM] = names
    regions.update({0x7a2508: struct.pack('<I', r.choice([RAM + 0x30, RAM + 0x30, 0])), 0x7a267c: r.bytes(4), 0x785de8: r.bytes(0x60)})
    return Call(0x45f640, [], regions, conv='c', ret='b')


if __name__ == '__main__':
    B.main(case)
