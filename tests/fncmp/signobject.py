# An object's sign text applied (src/game/model/signapply: 0x4682b0) against the original: no definition, no index, no folder, the fonts' path with or without its backslash (the C library's sprintf real), signs off, the name's conversion failing, the holder there or not, its flag set or not. The callees are recorders.
# usage: signobject.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill_this, log_region, mask_this, put, set_return, strings  # noqa: E402

DEF, WIDE, NAME, ASSIGN, APPLY, NAME_END, WIDE_END = 14, 15, 16, 17, 18, 19, 20
B = Bench([(0x4682b0, 0x46841c), (0x5c5520, 0x5c5530), (0x67a400, 0x6902c0)],
          hooks={0x4dd4e0: _recorder(DEF, 0), 0x5f6980: _recorder(WIDE, 1), 0x5f61a0: _recorder(NAME, 0), 0x424db0: _recorder(ASSIGN, 2, cdecl=True), 0x467a80: _recorder(APPLY, 6, cdecl=True), 0x5f61e0: _recorder(NAME_END, 0), 0x5f6a60: _recorder(WIDE_END, 0)})
OBJECT, DEFINITION, HOLDER, STYLE, TEXT, TEXT2, PATHS = (RAM + a for a in (0, 0x100, 0x800, 0x900, 0xa00, 0xa80, 0xb00))
OPS = ['applied', 'applied', 'applied', 'applied', 'no def', 'no index', 'no folder', 'signs off', 'no conversion', 'no holder', 'no holder off', 'flag set', 'text too']
RESULTS = [0, 1, 0x100, 0xffffffff]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    obj = bytearray(r.bytes(0x40))
    index = 0 if op == 'no index' else r.randrange(1, 8)
    put(obj, 0x20, 'I', index)
    set_return(log, DEF, 0 if op == 'no def' else DEFINITION)
    holder = bytearray(r.bytes(0x10))
    put(holder, 4, 'I', r.getrandbits(32) | 1 if op == 'flag set' else r.getrandbits(32) & ~1)
    table = bytearray(r.bytes(0x20))
    put(table, 4 * index, 'I', 0 if op in ('no holder', 'no holder off') else HOLDER)
    path = r.choice([b'C:\\Games\\Theme Park', b'Data\\', b'\\', b'x', b'Long\\Path\\Name\\'])
    paths = r.bytes(1) + path + b'\0'
    folder = bytearray(r.bytes(0x10))
    put(folder, 8, 'I', 0 if op == 'no folder' else PATHS + 1)
    fill_this(log, NAME, struct.pack('<I', STYLE), 8)
    for k in (WIDE, NAME, NAME_END, WIDE_END):
        mask_this(log, k)
    strings(log, ASSIGN, 0, 3)
    strings(log, APPLY, 1 << 3)
    set_return(log, ASSIGN, 0 if op == 'no conversion' else r.choice([1, 2, 0x100]))
    set_return(log, APPLY, r.choice(RESULTS))
    regions = {LOG: log, OBJECT: obj, DEFINITION: bytes(4), HOLDER: holder, STYLE: b'style\0', TEXT: r.bytes(0x10), TEXT2: r.bytes(0x10), PATHS: paths, 0xf816c0: folder, 0x7a4610: table,
               0x78d8d8: struct.pack('<I', 0 if op in ('signs off', 'no holder off') else r.choice([1, 0x100, r.getrandbits(32) | 1]))}
    # (a failed conversion returns a stack slot the original never set: its return does not compare)
    return Call(0x4682b0, [OBJECT, TEXT, r.choice([0, TEXT2])], regions, conv='c', ret='v' if op == 'no conversion' else 'a')


if __name__ == '__main__':
    B.main(case)
