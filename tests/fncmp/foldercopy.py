# The folders' maker that also copies a file (src/game/files/gamepaths: 0x5c8190) against the original: fileopen.py's imports and folder list, gamestring.py's Strings for the second
# path of 0xf816c0, the data folder 0xf81980 and the extension 0xf825e0; the copy flag any, GetFileAttributesA found or not, the heap failing now and then.
# usage: foldercopy.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, fill, global_heap, heap, import_table, log_region, put, set_return  # noqa: E402
import fileopen  # noqa: E402
import gamestring  # noqa: E402

B = Bench(fileopen.RANGES + gamestring.B_RANGES + [(0x5c8190, 0x5c83b0), (0x619240, 0x619250)])
IMPORTS = dict(fileopen.IMPORTS, CopyFileA=(39, 0x6fd158, 3, 3, 0))
K = {name: v[0] for name, v in IMPORTS.items()}
ROOT, FOLDERS, SEP = RAM + 0x900, RAM + 0xa00, 0xf81b48
TEXTS = RAM + 0x4000
STRING_VTABLE, WSTRING_VTABLE = 0x703090, 0x7030c8


def named(r, at, data, regions, choices):
    text = r.choice(choices)
    gamestring.string(r, 0, at, data, regions, text)


def case(r):
    log = log_region()
    table = bytearray(import_table(log, IMPORTS).ljust(0x600, b'\0'))
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    root = r.choice([b'C:\\tp\\save', b'D:', b'', b'C:\\tp\\'])
    set_return(log, K['WideCharToMultiByte'], len(root) + 1 if r.random() < 0.9 else r.choice([0, 1]))
    fill(log, K['WideCharToMultiByte'], 4, root + bytes(1))
    set_return(log, K['GetFileAttributesA'], r.choice([0xffffffff, 0x20, 0x20, 0x11]))
    set_return(log, K['CreateDirectoryA'], r.choice([0, 1, 1]))
    set_return(log, K['CopyFileA'], r.choice([0, 1, 1]))
    globals_ = bytearray(r.bytes(0x318))
    globals_[0:8] = b'tp.exe\0\0'
    base = r.choice([b'C:\\tp\\', b'D:\\'])
    globals_[0x108:0x109 + len(base)] = base + b'\0'
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000, r.choice(range(1, 12)) if r.random() < 0.05 else 0), fileopen.GLOBALS: globals_, fileopen.PATHS: r.bytes(0x210), fileopen.LOCALE: bytes(4),
               ROOT: struct.pack('<5I', WSTRING_VTABLE, len(root), ROOT + 0x20, len(root) + 8, 8) + bytes(0xc) + root.decode().encode('utf-16-le') + bytes(2)}
    named(r, SEP, TEXTS, regions, [b'\\', b'\\', b'\\', b'', b'/x\\'])
    named(r, 0xf816e8, TEXTS + 0x100, regions, [b'C:\\tp', b'D:', b'', b'E:\\tp\\data'])
    named(r, 0xf81980, TEXTS + 0x200, regions, [b'data', b'', b'a\\b'])
    named(r, 0xf825e0, TEXTS + 0x300, regions, [b'.cfg', b'', b'.'])
    n = r.choice([0, 1, 1, 2, 3])
    folders = bytearray(r.bytes(0x100))
    for k in range(n):   # node k at +0x10 * k {name, next}; name k's String at +0x40 + 0x14 * k, its text at +0xc0 + 0x10 * k
        struct.pack_into('<2I', folders, 0x10 * k, FOLDERS + 0x40 + 0x14 * k, FOLDERS + 0x10 * (k + 1) if k + 1 < n else 0)
        name = r.choice([b'users', b'online', b'', b'a\\b'])
        put(folders, 0x40 + 0x14 * k, '5I', STRING_VTABLE, len(name), FOLDERS + 0xc0 + 0x10 * k, len(name) + 8, 8)
        folders[0xc0 + 0x10 * k:0xc0 + 0x10 * k + len(name) + 1] = name + bytes(1)
    regions[FOLDERS] = bytes(folders)
    regions[0x786b90] = struct.pack('<I', FOLDERS if n else 0)
    return Call(0x5c8190, [ROOT, r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='s', ret='v')


if __name__ == '__main__':
    B.main(case)
