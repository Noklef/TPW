# The C library's category init routines (src/game/core/crtlocaleinit: __init_monetary 0x68d5c0, _get_lc_lconv 0x68d6b0, __init_ctype 0x68d8b0) against the originals:
# the Win32 calls COM recorders in their import slots (GetLocaleInfoW and A kept as found, GetStringTypeA and W too), _nh_malloc and calloc cdecl recorders, free the heap's hash;
# the C library's state (the categories' handles and lcids, the lconvs, the tables, the code page) varied.
# Bench limit: __init_ctype's __getlocaleinfo for a code page of 0 always succeeds here: when it fails, the original frees two uninitialised frame slots (sic).
# usage: crt_localeinit.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, fill, heap, import_table, log_region, set_return  # noqa: E402

CALLOC, NHMALLOC = 24, 28
INFOW, INFOA, LASTERROR, CPINFO, GSA, GSW = 14, 15, 18, 19, 20, 21
B = Bench([(0x68d5c0, 0x68d6af), (0x68d6b0, 0x68d7fb), (0x68d8b0, 0x68db6b), (0x68d800, 0x68d833), (0x68d840, 0x68d8a6),
           (0x691380, 0x69152c), (0x691680, 0x6917b0), (0x6917b0, 0x6918ef), (0x67dcd0, 0x67dcf0), (0x67ae00, 0x67aefe), (0x680430, 0x6804c5),
           (0x68a5d0, 0x68a708), (0x68a440, 0x68a5c4), (0x689180, 0x6891af), (0x688f10, 0x688f45)],
          hooks={0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x67fcb0: _recorder(CALLOC, 2, cdecl=True), 0x67ad90: _free})
A_MALLOC, A_CALLOC, A_INFOW, A_INFOA, PCTYPE, PCTYPE2, LCONV, CURRENT, OWNED, CBLOCK = (RAM + 0x200 * i for i in range(10))
BLOCKS = [HEAP + 0x100 + 0x300 * i for i in range(10)]
OPS = [n for n in ('lconv', 'lconv', 'monetary', 'monetary', 'monetary', 'ctype', 'ctype', 'ctype', 'ctype') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
LEADS = [b'', b'\x81\x9f\xe0\xfc', b'\x81\xfe', b'\xa1\xa1', b'\x90\x85', b'\xff\xff', b'\x00\x81', b'\x81\x00\xe0\xfc', b'\x81\x82\x84\x85\x87\x88\x8a\x8b\x8d\x8e']


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x3000),
               IMPORT_TABLE: import_table(log, {'infow': (INFOW, 0x6fd204, 4, 0, 0), 'infoa': (INFOA, 0x6fd28c, 4, 0, 0b100), 'lasterror': (LASTERROR, 0x6fd138, 0, 0, 0),
                                                'cpinfo': (CPINFO, 0x6fd180, 2, 0, 0b10), 'gsa': (GSA, 0x6fd1f0, 5, 0, 0b10000), 'gsw': (GSW, 0x6fd1ec, 4, 0, 0b1000)})}
    blocks = [BLOCKS[i % 10] if name != 'ctype' or r.random() < 0.93 else 0 for i in range(40)]
    for k, at, values in ((NHMALLOC, A_MALLOC, blocks), (CALLOC, A_CALLOC, [CBLOCK if r.random() < 0.9 else 0 for _ in range(40)]),
                          (INFOW, A_INFOW, [r.choice([2, 2, 2, 0]) for _ in range(40)]), (INFOA, A_INFOA, [r.choice([4] * 8 + [2, 2, 0]) for _ in range(40)])):   # (a failed grouping leaves the pointer as it was: null in calloc's block, a crash in both)
        answers(log, k, at)
        regions[at] = struct.pack('<40I', *values)
    set_return(log, LASTERROR, 0)
    fill(log, INFOW, 2, bytes(b for c in r.choice([b'2', b'12', b'0', b'x', b'']) + b'\0\0\0' for b in (c, 0))[:8])
    fill(log, INFOA, 2, r.choice([b'3;2\0', b'3;0\0', b'\0\0\0\0', b'$\0\0\0', b'12;x']) + bytes(4), which=1)
    regions[CBLOCK] = bytes(0x40)
    # The C library's state: the handles (LC_CTYPE +8, LC_MONETARY +0xc) and the code page (+0x18); the lconvs, the tables and their lcids.
    regions[0xfb1438] = struct.pack('<7I', 0x11, 0x22, 0 if r.random() < 0.25 else 0x409, 0 if r.random() < 0.35 else 0x40c, 0x55, 0x66,
                                    0 if r.random() < 0.25 else r.choice([932, 1252, 0x4e4]))
    regions[0xfb1938] = bytes(0xc) + struct.pack('<2I', 1, 2)   # (GetLocaleInfoW found to work, GetLocaleInfoA the A one)
    regions[0xfb188c] = struct.pack('<2I', 1, 2)                 # (GetStringTypeW, GetStringTypeA likewise)
    held = bytearray(r.bytes(0x24))
    struct.pack_into('<3I', held, 0, r.choice([0, OWNED, OWNED]), *[0 if r.random() < 0.3 else r.choice([HEAP + 0x40, r.getrandbits(32)]) for _ in range(2)])   # (the lconv owned is read: a real one)
    regions[0xfb18ac] = held
    owned = bytearray(r.bytes(0x30))
    if r.random() < 0.3:
        owned[0xc:0x10] = struct.pack('<I', 0xfb1940)   # (the "C" locale's own texts: not freed)
    regions[OWNED] = owned
    regions[CURRENT] = r.bytes(0x30)
    regions[0x781f98] = r.bytes(0x30) + struct.pack('<I', r.choice([CURRENT, CURRENT, 0x781f98]))
    table = bytearray(0x200)
    for c in range(0x100):
        table[c * 2] = (4 if 0x30 <= c <= 0x39 else 0) | r.choice([0, 1, 2, 8])
    regions[PCTYPE] = table
    regions[0x77e7dc] = struct.pack('<I', 1)
    regions[0x77ea90] = struct.pack('<2I', PCTYPE, PCTYPE2)
    regions[PCTYPE2] = r.bytes(0x10)
    if name == 'lconv':
        regions[LCONV] = r.bytes(0x1c) + struct.pack('<I', OWNED + 0x30) + r.bytes(0x20)   # (the grouping's pointer as it was: a text)
        regions[OWNED] = regions[OWNED][:0x30] + b'3;1\0'
        return Call(0x68d6b0, [r.choice([LCONV, LCONV, LCONV, 0])], regions, ret='a')
    if name == 'monetary':
        return Call(0x68d5c0, [], regions, ret='a')
    # The code page's info: its size (above 2 fails) and lead byte ranges, a zero pair within the 12 bytes.
    lead = r.choice(LEADS)
    fill(log, CPINFO, 1, struct.pack('<I', r.choice([1, 2, 2, 2, 3, 0])) + b'?\0' + lead + bytes(12 - len(lead)), which=2)
    set_return(log, CPINFO, 0 if r.random() < 0.1 else 1)
    fill(log, GSA, 4, r.bytes(0x70), which=3)
    fill(log, GSW, 3, r.bytes(0x70), which=4)
    set_return(log, GSA, 0 if r.random() < 0.1 else 1)
    set_return(log, GSW, 0 if r.random() < 0.1 else 1)
    regions[A_INFOW] = struct.pack('<40I', *[2] * 40)   # (see the bench limit)
    return Call(0x68d8b0, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
