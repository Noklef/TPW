# src/game/core/regkey: Registry_GameInstalled (0x45f6b0) against the original, the registry imports recorders in the import table:
# RegCreateKeyExA giving a key (filled through its out argument) or failing, RegQueryValueExA giving a type (filled) or failing,
# RegCloseKey and RegOpenKeyA succeeding or not; the subkey and value names at [0x7a2b08] / [0x7a2358] (hashed as strings).
# usage: regkey.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, fill, import_table, log_region, set_return  # noqa: E402

B = Bench([(0x45f6b0, 0x45f7b0), (0x581da0, 0x581dc0), (0x5f8ae0, 0x5f8d00), (0x5f8d40, 0x5f8dc0)])
CLOSE, OPEN, QUERY, CREATE = 1, 2, 3, 4
# {name: (slot, import address, stack arguments, string arguments, out arguments)}: the out pointers are the callers' stack slots
IMPORTS = {'RegCloseKey': (CLOSE, 0x6fd000, 1, 0, 0), 'RegOpenKeyA': (OPEN, 0x6fd004, 3, 2, 4),
           'RegQueryValueExA': (QUERY, 0x6fd008, 6, 2, 0x38), 'RegCreateKeyExA': (CREATE, 0x6fd00c, 9, 2, 0x180)}


def installed(r):
    log = log_region()
    iat = import_table(log, IMPORTS)
    for k, ok in ((CREATE, 0.8), (QUERY, 0.7), (CLOSE, 0.85), (OPEN, 0.5)):
        set_return(log, k, 0 if r.random() < ok else r.choice([2, 5, r.getrandbits(32)]))
    fill(log, CREATE, 7, struct.pack('<I', r.choice([0, 0x1234, r.getrandbits(32)])))
    fill(log, QUERY, 3, struct.pack('<I', r.choice([1, 3, 4, 5, 6, 7, r.getrandbits(32)])), 1)
    names = bytearray(0x40)
    names[0:0x10] = r.choice([b'Theme Park World', b'Settings', b'x']).ljust(0x10, b'\0')
    names[0x20:0x30] = r.choice([b'Installed', b'Version', b'']).ljust(0x10, b'\0')
    regions = {LOG: log, IMPORT_TABLE: iat, RAM: names, 0x7a2b08: struct.pack('<I', r.choice([RAM, RAM, RAM, 0])),
               0x7a2358: struct.pack('<I', r.choice([RAM + 0x20, RAM + 0x20, RAM + 0x20, 0]))}
    return Call(0x45f6b0, [], regions, conv='c', ret='b')


def case(r):
    return installed(r)


if __name__ == '__main__':
    B.main(case)
