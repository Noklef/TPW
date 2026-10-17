# The conversions between ANSI and UTF-16 text of src/game/core/string against the originals. The imports are recorders (COM-style, fncmp
# import_table()) that answer a size for the case and write a few bytes through their output buffer (fncmp fill()): MultiByteToWideChar (logged by the
# ANSI text's hash) and WideCharToMultiByte; GlobalAlloc / GlobalFree are fncmp's heap stubs (their slots inside the same table), the
# allocation failing now and then. FromWide (0x424db0) / FromAnsi (0x424e10) assign the other kind's text to a string through its vtable's +0x14
# (a recorder logging the block's text); AnsiToWide (0x4bbb80) converts into a buffer or, with none, the static one at 0x7ba058.
# usage: wideconv.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FromWide,FromAnsi,AnsiToWide)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, fill, global_heap, heap, import_table, log_region, set_return, strings, vtable  # noqa: E402

B = Bench([(0x424db0, 0x424e6e), (0x4bbb80, 0x4bbbc4), (0x5f5660, 0x5f5690)])
STR, VT, OTHER, TEXT, BUF = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x600
STATIC = 0x7ba058
# The imports: slot, import table address, stack arguments, string arguments (mask), out arguments (mask).
IMPORTS = {'MultiByteToWideChar': (40, 0x6fd174, 6, 4, 0), 'WideCharToMultiByte': (41, 0x6fd1dc, 8, 0, 0)}
ASSIGN = 5   # (the vtable's +0x14: its recorder logs the text it is given)
SIZES = [0, 0, -1, 1, 2, 5, 0x20, 0x40, 0x7fffffff, -0x80000000, 0x100000]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    table = import_table(log, IMPORTS)
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    text = bytes(r.choice(b'abcXYZ09 \x80\xe9\xff') for _ in range(r.choice([0, 1, 3, 8, 30]))) + b'\0'
    size = r.choice(SIZES) if r.random() < 0.9 else r.getrandbits(32) - (1 << 31)
    regions = {LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 1])), TEXT: text + r.bytes(0x10), BUF: r.bytes(0x100), STATIC: r.bytes(0x100)}
    k = IMPORTS['WideCharToMultiByte' if op == 'FromWide' else 'MultiByteToWideChar'][0]
    set_return(log, k, size)
    out = r.bytes(r.randrange(0, max(1, min(size if size > 0 else 0, 0x70) * (1 if op == 'FromWide' else 2)) + 1))
    fill(log, k, 4, out[:0x70])
    if op == 'AnsiToWide':
        regions[IMPORT_TABLE] = table
        buffer = r.choice([0, 0, BUF])
        return Call(0x4bbb80, [TEXT, buffer, r.choice(SIZES) & 0xffffffff], regions, ret='a')
    strings(log, ASSIGN, 1 if op == 'FromWide' else 0)
    regions[IMPORT_TABLE] = table
    regions[STR] = struct.pack('<I', VT) + r.bytes(0x10)
    regions[VT] = vtable({ASSIGN: 1})
    regions[OTHER] = r.bytes(8) + struct.pack('<I', TEXT) + r.bytes(8)
    return Call(0x424db0 if op == 'FromWide' else 0x424e10, [STR, OTHER], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', 'FromWide,FromAnsi,AnsiToWide').split(',') if k]

if __name__ == '__main__':
    B.main(case)
