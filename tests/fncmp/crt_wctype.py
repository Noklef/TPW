# The C library's wide case and class routines (src/game/core/crt: towlower 0x67e5d0, _towlower_lk 0x67e670, towupper 0x67ed00, _towupper_lk 0x67ed90, iswctype 0x67f330, __crtLCMapStringW
# 0x688d00, __crtGetStringTypeW 0x68a440) against the originals: the Win32 calls COM recorders in their import slots, answering from a table (the pointers to the callers' stack units logged as 0,
# their out units filled); malloc's _nh_malloc, calloc and the locks cdecl recorders; free the heap's hash of freed pointers; the table _pwctype made here.
# usage: crt_wctype.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, fill, heap, import_table, log_region, set_return  # noqa: E402

LOCK, UNLOCK, CALLOC, NHMALLOC = 7, 8, 24, 28
INC, DEC, M2W, W2M, LCW, LCA, GSW, GSA = 14, 15, 16, 17, 18, 19, 20, 29
B = Bench([(0x67e5d0, 0x67e666), (0x67e670, 0x67e6eb), (0x67ed00, 0x67ed88), (0x67ed90, 0x67ee0d), (0x67f330, 0x67f394), (0x688d00, 0x688f07), (0x68a440, 0x68a5c4), (0x688f10, 0x688f45),
           (0x67dcd0, 0x67dcf0), (0x67ae00, 0x67af10)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True),
                 0x67fcb0: _recorder(CALLOC, 2, cdecl=True), 0x67ad90: _free})
TEXT, TO, OUT, ANSWERS, BLOCKS, BLOCK1, BLOCK2, PWCTYPE = (RAM + 0x100 * i for i in range(8))
CASE = ('towlower', 'towupper', 'towlower_lk', 'towupper_lk', 'iswctype')
OPS = [n for n in CASE + CASE + ('lcmapw', 'lcmapw', 'typesw', 'typesw') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
UNITS = [0x41, 0x5a, 0x61, 0x7a, 0x40, 0x5b, 0x60, 0x7b, 0xc9, 0xe9, 0xff, 0x100, 0x130, 0x3a3, 0x3c3, 0xfffe, 0xffff]


def pwctype(r):
    words = [r.getrandbits(16) if r.random() < 0.1 else 0 for _ in range(0x100)]
    for c in range(0x100):
        if 0x41 <= c <= 0x5a or 0xc0 <= c <= 0xde:
            words[c] |= 0x101
        if 0x61 <= c <= 0x7a or 0xdf <= c <= 0xff:
            words[c] |= 0x102
    return struct.pack('<256H', *words)


def unit(r):
    c = r.choice(UNITS + [r.getrandbits(16)])
    return c | r.choice([0, 0, 0, r.getrandbits(16) << 16])   # (only the low 16 bits count)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x40),
               IMPORT_TABLE: import_table(log, {'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0), 'm2w': (M2W, 0x6fd174, 6, 0b100, 0b10000), 'w2m': (W2M, 0x6fd1dc, 8, 0, 0b10100),
                                                'lcw': (LCW, 0x6fd198, 6, 0, 0b10100), 'lca': (LCA, 0x6fd19c, 6, 0b100, 0b10000), 'gsw': (GSW, 0x6fd1ec, 4, 0, 0b1010), 'gsa': (GSA, 0x6fd1f0, 5, 0b100, 0b10000)})}
    regions[0xfb3ca4] = struct.pack('<2I', r.choice([0, 0, 1]), r.randrange(4))
    regions[0xfb1440] = struct.pack('<5I', r.choice([0, 0, 0x409, 0x411]), 0, 0, 0, r.choice([932, 1252]))
    regions[0xfb1548] = struct.pack('<I', r.randrange(2))
    # (Mode 3 is only given to the routines themselves: their callers would read their own unset locals.)
    regions[0xfb1550] = struct.pack('<I', r.choice([0, 0, 1, 1, 2, 3] if name == 'lcmapw' else [0, 0, 1, 1, 2, 2]))
    regions[0xfb188c] = struct.pack('<I', r.choice([0, 0, 1, 1, 2, 3] if name == 'typesw' else [0, 0, 1, 1, 2, 2]))
    regions[0x77ea94] = struct.pack('<I', PWCTYPE)
    regions[PWCTYPE] = pwctype(r)
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([0, 1, 1, 1, 2, 2, 3, 9]) for _ in range(40)])
    regions[ANSWERS + 0xa0] = struct.pack('<40I', *[r.choice([1, 1, 1, 1, 2, 0]) for _ in range(40)])
    regions[BLOCKS] = struct.pack('<4I', *[r.choice([BLOCK1, BLOCK1, BLOCK1, 0]), r.choice([BLOCK2, BLOCK2, BLOCK2, 0]), 0, 0]) + bytes(0x90)
    regions[BLOCK1] = r.bytes(0x20)
    regions[BLOCK2] = r.bytes(0x20)
    answers(log, M2W, ANSWERS + 0xa0)
    answers(log, W2M, ANSWERS)
    answers(log, LCW, ANSWERS + 0xa0)
    answers(log, LCA, ANSWERS)
    answers(log, GSW, ANSWERS + 0xa0)
    answers(log, GSA, ANSWERS)
    answers(log, NHMALLOC, BLOCKS)
    set_return(log, CALLOC, r.choice([BLOCK1, BLOCK1, BLOCK1, 0]))
    fill(log, LCA, 4, r.bytes(4), which=0)
    fill(log, W2M, 4, r.bytes(4), which=1)
    fill(log, GSW, 3, r.bytes(2), which=2)
    fill(log, GSA, 4, r.bytes(r.choice([2, 2, 4])), which=3)   # (over one or both 0xffff marks; no more: the probe's out is a 4-byte local)
    fill(log, LCW, 4, r.bytes(2), which=4)
    fill(log, M2W, 4, r.bytes(2), which=5)
    regions[TO] = r.bytes(0x10)
    regions[OUT] = r.bytes(0x10)
    regions[TEXT] = b''.join(struct.pack('<H', r.choice(UNITS[:-1] + [r.randrange(1, 0x10000)])) for _ in range(r.randrange(0, 6))) + bytes(2) + r.bytes(4)
    if name == 'lcmapw':
        flags = r.choice([0x100, 0x200, 0x400, 0x400])
        return Call(0x688d00, [r.choice([0, 0x409]), flags, TEXT, r.choice([-1, 0, 1, 2, 5]), r.choice([TO, TO, TO, 0]), r.choice([0, 0, 1, 2, 3, 8]), r.choice([0, 932])], regions, conv='c', ret='a')
    if name == 'typesw':
        return Call(0x68a440, [r.choice([1, 2, 4]), TEXT, r.choice([0, 1, 1, 2, 3]), OUT, r.choice([0, 932]), r.choice([0, 0x409])], regions, conv='c', ret='a')
    if name == 'iswctype':
        return Call(0x67f330, [unit(r), r.choice([1, 2, 3, 0x100, 0xffff, r.getrandbits(32)])], regions, conv='c', ret='a')
    address = {'towlower': 0x67e5d0, 'towupper': 0x67ed00, 'towlower_lk': 0x67e670, 'towupper_lk': 0x67ed90}[name]
    return Call(address, [unit(r)], regions, conv='c', ret='w')


if __name__ == '__main__':
    B.main(case)
