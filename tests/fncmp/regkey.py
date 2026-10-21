# src/game/core/regkey: Registry_GameInstalled (0x45f6b0) against the original, the registry imports recorders in the import table:
# RegCreateKeyExA giving a key (filled through its out argument) or failing, RegQueryValueExA giving a type (filled) or failing,
# RegCloseKey and RegOpenKeyA succeeding or not; the subkey and value names at [0x7a2b08] / [0x7a2358] (hashed as strings). And each RegKey
# method (0x5f8ae0-0x5f8da0) called directly on a key held or not: Open with a subkey or none, in either root, over a key it closes; Close;
# Reg_CreateKey volatile or not; Query, GetU32 and GetU32Or with a name or none, the read filling 0-4 bytes and the type any (the size
# passed hashed); TypeOf a value, a subkey or neither. Language: Registry_LoadLanguage (0x5c0d00) with the imports as for Installed, the
# value read mostly one of the ids it maps (three fills: the keys' handles, the value's type and its data), the name table at 0xf7ec30
# pointing to short strings.
# usage: regkey.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Installed,... to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, blob, fill, import_table, log_region, put, set_return  # noqa: E402

B = Bench([(0x45f6b0, 0x45f7b0), (0x581da0, 0x581dc0), (0x5f8ae0, 0x5f8dc0), (0x5c0d00, 0x5c0f07), (0x41a840, 0x41a850)])
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


IDS = [6, 7, 10, 0x10, 0x11, 0x13, 0x15, 0x1d, 0x404, 0x409, 0x40c, 0x40d, 0x412, 0x41e, 0x804, 0x809]


def language(r):
    log = log_region()
    iat = import_table(log, IMPORTS)
    for k, ok in ((CREATE, 0.93), (QUERY, 0.93), (CLOSE, 0.85), (OPEN, 0.5)):
        set_return(log, k, 0 if r.random() < ok else r.choice([2, 5, r.getrandbits(32)]))
    fill(log, CREATE, 7, struct.pack('<I', r.choice([0x1234, 0x1234, 0x1234, r.getrandbits(32), 0])))   # (the keys' handles)
    kind = r.choice([4, 4, 4, 4, 5, 6, 1, r.getrandbits(32)])
    fill(log, QUERY, 3, struct.pack('<I', kind), 1)   # (the type, for TypeOf's query)
    value = r.choice(IDS + [0, 0x41, r.getrandbits(32)])
    fill(log, QUERY, 4, struct.pack('<I' if kind != 6 else '>I', value), 2)   # (the data: big-endian for type 6, which GetU32 swaps)
    mem = bytearray(r.bytes(0x400))
    mem[0:0x10] = r.choice([b'Theme Park World', b'Settings', b'x']).ljust(0x10, b'\0')
    mem[0x20:0x30] = r.choice([b'Language', b'Version', b'']).ljust(0x10, b'\0')
    table = bytearray(r.bytes(0x140))
    for i in range(16):
        n = r.randrange(0, 12)
        mem[0x100 + 0x20 * i:0x100 + 0x20 * i + n + 1] = bytes(r.randrange(1, 256) for _ in range(n)) + b'\0'
        put(table, 0x14 * i, 'I', RAM + 0x100 + 0x20 * i)
    regions = {LOG: log, IMPORT_TABLE: iat, RAM: mem, 0xf7f6b8: struct.pack('<I', r.choice([RAM, RAM, RAM, 0])),
               0xf7ee18: struct.pack('<I', r.choice([RAM + 0x20, RAM + 0x20, RAM + 0x20, 0])), 0x79d868: r.bytes(4), 0xf7ec30: bytes(table),
               0xf7f230: r.bytes(0x20), 0x78b530: r.bytes(0x10)}
    return Call(0x5c0d00, [RAM + 0x80], regions, conv='c', ret='a')


CALLS = {'Installed': None, 'Language': None, 'Ctor': 0x5f8da0, 'Dtor': 0x5f8b70, 'Open': 0x5f8ae0, 'Close': 0x5f8b80, 'CreateKey': 0x5f8bc0, 'Query': 0x5f8c00,
         'GetU32': 0x5f8c50, 'GetU32Or': 0x5f8cf0, 'TypeOf': 0x5f8d40}
KEY, OUT, VALUE, DATA, NAME = RAM + 0x100, RAM + 0x110, RAM + 0x118, RAM + 0x120, RAM + 0x180


def method(r, op):
    log = log_region()
    # (the size Query passes, on its stack, hashed: arg 5 not masked)
    iat = import_table(log, dict(IMPORTS, RegQueryValueExA=(QUERY, 0x6fd008, 6, 2, 0x18)))
    blob(log, QUERY, 5, 1)
    for k in (CREATE, QUERY, CLOSE, OPEN):
        set_return(log, k, 0 if r.random() < 0.6 else r.choice([2, 5, r.getrandbits(32)]))
    handle = lambda: struct.pack('<I', r.choice([0, 0x1234, 0x80000001, r.getrandbits(32)]))  # noqa: E731
    fill(log, CREATE, 7, handle())
    fill(log, OPEN, 2, handle())
    fill(log, QUERY, 3, struct.pack('<I', r.choice([0, 1, 3, 4, 4, 5, 6, 7, r.getrandbits(32)])))
    fill(log, QUERY, 4, r.bytes(r.choice([0, 1, 2, 3, 4, 4, 4] + [8] * (op == 'Query'))), 1)   # (GetU32 reads into a dword)
    mem = bytearray(r.bytes(0x200))
    put(mem, 0x100, 'I', r.choice([0, 0, 0x4321, r.getrandbits(32)]))
    mem[0x180:0x190] = r.choice([b'Installed', b'Version', b'', b'Sub\\Key']).ljust(0x10, b'\0')
    regions = {LOG: log, IMPORT_TABLE: iat, RAM: mem}
    name = r.choice([NAME, NAME, NAME, 0])
    if op == 'CreateKey':
        return Call(CALLS[op], [r.choice([NAME, NAME, 0]), r.choice([0x80000001, 0x80000002, 0x4321, r.getrandbits(32)]),
                                r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='c', ret='a')
    args = {'Ctor': [], 'Dtor': [], 'Close': [OUT], 'Open': [OUT, name, r.choice([0, 1, 0x100, r.getrandbits(32)])],
            'Query': [OUT, name, DATA, r.choice([0, 1, 4, 0x40, r.getrandbits(32)])], 'GetU32': [OUT, name, VALUE],
            'GetU32Or': [OUT, name, VALUE, r.getrandbits(32)], 'TypeOf': [name]}[op]
    return Call(CALLS[op], [KEY] + args, regions, conv='t', ret='a' if op != 'Dtor' else 'v')


def case(r):
    op = r.deal(OPS)
    return installed(r) if op == 'Installed' else language(r) if op == 'Language' else method(r, op)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
