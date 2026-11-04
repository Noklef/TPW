# The built objects' numbers (src/game/world/objectdefs: 0x51ac20) against the original, with the walk and the C library's strrchr, strpbrk and strtol it calls: definitions and the bitset 0x802080 made here.
# usage: objectdefsbuilt.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

GETPTD = 2
B = Bench([(0x51ac20, 0x51ad2b), (0x412e60, 0x412e90), (0x67b5f0, 0x67b617), (0x67d7f0, 0x67d82a), (0x67d520, 0x67d53a), (0x67d540, 0x67d7c1), (0x67f1a0, 0x67f222), (0x686a00, 0x686a10)],
          hooks={0x5da3c0: lambda e: None, 0x682460: _recorder(GETPTD, 0, cdecl=True)})
OUT, PCTYPE, PTD = RAM, RAM + 0x400, RAM + 0x800
DEF = RAM + 0x1000


def case(r):
    log = log_region()
    set_return(log, GETPTD, PTD)
    regions = {LOG: log, OUT: r.bytes(0x210), PTD: bytes(0x20)}
    table = [(0x105 if 0x41 <= c <= 0x5a or 0x61 <= c <= 0x7a else 0) | (4 if 0x30 <= c <= 0x39 else 0) | (8 if c == 0x20 else 0) for c in range(0x100)]
    regions[PCTYPE] = struct.pack('<256H', *table)
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[0x77e7dc] = struct.pack('<i', 1)
    regions[0xfb1440] = bytes(4)
    defs = bytearray(r.bytes(0x740))
    n = r.choice([0, 1, 3, 8, 16, 40])
    ids = [r.choice([1, 2, 8, 9, 0x20, 0x100, 0x101, 0, r.randrange(1, 0x101), r.randrange(1, 0x101), r.randrange(1, 0x101)]) for _ in range(n)]
    for i, id_ in enumerate(ids):
        at = DEF + 0x300 * i
        put(defs, 4 * i, 'I', at)
        body = bytearray(r.bytes(0x300))
        put(body, 0x94, 'I', id_)
        kind = r.random()
        if kind < 0.03:
            name = b'plain7.wad'           # (no backslash: the original faults on the null)
        elif kind < 0.06:
            name = b'c:\\7\\none.wad'      # (no digit after the last: it faults too)
        else:
            name = b'c:\\7\\' + r.choice([b'ride', b'a b', b'']) + r.choice([b'12', b'0', b'7000', b'65535', b'65536', b'70000', b'99999999999', b'007a', b'010', b'9 1']) + r.choice([b'.wad', b'', b'z'])
        body[0x28c:0x28c + len(name) + 1] = name + b'\0'
        regions[at] = body
    if not n:
        put(defs, 0, 'I', 0)
    put(defs, 0x528, 'i', n)
    regions[0x7890a0] = defs
    bits = bytearray(0x20)
    for id_ in ids:
        if 0 < id_ <= 0x100 and r.random() < 0.8:
            bits[(id_ - 1) >> 3] |= 1 << ((id_ - 1) & 7)
    for _ in range(r.choice([0, 0, 2, 8])):
        i = r.randrange(0x100)
        bits[i >> 3] |= 1 << (i & 7)
    regions[0x802080] = bits
    return Call(0x51ac20, [OUT], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)
