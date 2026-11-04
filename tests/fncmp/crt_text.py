# The C library's text routines (src/game/core/crt: strrchr 0x67b5f0, strpbrk 0x67d7f0, _CxxThrowException 0x67d3a0, strncnt 0x689180, __crtGetStringTypeA 0x68a5d0, __crtLCMapStringA 0x688f50, _isctype 0x680430,
# toupper 0x67f1a0 and 0x67f230, strtoxl 0x67d540, strtol 0x67d520, strtoul 0x67d7d0) against the originals: the Win32 calls COM recorders in their import slots, answering from a table; malloc's _nh_malloc, calloc,
# the lock and _getptd cdecl recorders; free the heap's hash of freed pointers; the table _pctype made here (with some noise), words before it too.
# usage: crt_text.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, blob, fill, heap, import_table, log_region, set_return  # noqa: E402

GETPTD, LOCK, UNLOCK, CALLOC, NHMALLOC = 2, 7, 8, 24, 28
INC, DEC, M2W, W2M, LCW, LCA, GSW, GSA, RAISE = 14, 15, 16, 17, 18, 19, 20, 29, 30
B = Bench([(0x67b5f0, 0x67b617), (0x67d3a0, 0x67d3e7), (0x67d520, 0x67d53a), (0x67d540, 0x67d7c1), (0x67d7d0, 0x67d7ea), (0x67d7f0, 0x67d82a), (0x67dcd0, 0x67dcf0), (0x67f1a0, 0x67f222), (0x67f230, 0x67f32e),
           (0x680430, 0x6804c5), (0x686a00, 0x686a10), (0x688f50, 0x689175), (0x689180, 0x6891af), (0x68a5d0, 0x68a708)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True),
                 0x67fcb0: _recorder(CALLOC, 2, cdecl=True), 0x67ad90: _free})
PTD, TEXT, SET, END, TO, OUT, ANSWERS, BLOCKS, BLOCK1, BLOCK2 = (RAM + 0x100 * i for i in range(10))
PCTYPE = RAM + 0x1200
OPS = [n for n in ('strrchr', 'strpbrk', 'throw', 'strncnt', 'types', 'map', 'isctype', 'toupper', 'toupper_lk', 'strtoxl', 'strtol', 'strtoul') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
NUMBERS = [b'0', b'7', b'123', b'777', b'0x1aF', b'0X1g', b'0x', b'0', b'zZ', b'', b'-', b'+', b'4294967295', b'4294967296', b'2147483647', b'2147483648', b'2147483649', b'99999999999', b'FFFFFFFF', b'100000000',
           b'\xe9', b'12\xe9', b'9a', b'g', b'Z', b'08', b'1z2']


def pctype(r):
    words = [r.getrandbits(16) if r.random() < 0.05 else 0 for _ in range(0x200)]   # (from 0x100 words before it)
    for c in range(0x100):
        if 0x30 <= c <= 0x39:
            words[0x100 + c] |= 4
        if 0x41 <= c <= 0x5a:
            words[0x100 + c] |= 0x101
        if 0x61 <= c <= 0x7a:
            words[0x100 + c] |= 0x102
        if c in (0x20, 9):
            words[0x100 + c] |= 8
        if c in (0x81, 0x82, 0xe9):
            words[0x100 + c] |= 0x8000
    return struct.pack('<512H', *words)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, PTD: bytearray(0x20), HEAP: heap(0x40),
               IMPORT_TABLE: import_table(log, {'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0), 'm2w': (M2W, 0x6fd174, 6, 0b100, 0), 'w2m': (W2M, 0x6fd1dc, 8, 0, 0b10000),
                                                'lcw': (LCW, 0x6fd198, 6, 0, 0), 'lca': (LCA, 0x6fd19c, 6, 0b100, 0b10000), 'gsw': (GSW, 0x6fd1ec, 4, 0, 0b1000), 'gsa': (GSA, 0x6fd1f0, 5, 0b100, 0b10000), 'raise': (RAISE, 0x6fd218, 4, 0, 0)})}
    set_return(log, GETPTD, PTD)
    regions[0xfb3ca4] = struct.pack('<2I', r.choice([0, 0, 1]), r.randrange(4))
    regions[0xfb1440] = struct.pack('<5I', r.choice([0, 0, 0x409, 0x411]), 0, 0, 0, r.choice([932, 1252]))
    regions[0xfb1548] = struct.pack('<I', r.randrange(2))
    # (Mode 3 can never be stored: only the routines themselves take it, the callers would read their own unset locals.)
    regions[0xfb1890] = struct.pack('<I', r.choice([0, 0, 1, 1, 2, 3] if name == 'types' else [0, 0, 1, 1, 2]))
    regions[0xfb1554] = struct.pack('<I', r.choice([0, 0, 1, 1, 2, 3] if name == 'map' else [0, 0, 1, 1, 2]))
    regions[0x77e7dc] = struct.pack('<i', r.choice([1, 1, 2, 3, 0]))
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[PCTYPE - 0x200] = pctype(r)
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([0, 1, 1, 1, 2, 2, 3, 9]) for _ in range(40)])
    regions[ANSWERS + 0xa0] = struct.pack('<40I', *[r.choice([1, 1, 1, 1, 2, 0]) for _ in range(40)])
    regions[BLOCKS] = struct.pack('<4I', *[r.choice([BLOCK1, BLOCK1, BLOCK1, 0]), r.choice([BLOCK2, BLOCK2, BLOCK2, 0]), 0, 0]) + bytes(0x90)
    # (The calls that find out which import works answer as all do: the steps past them are reached anyway through the kept modes.)
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
    fill(log, GSA, 4, r.bytes(2), which=3)
    blob(log, RAISE, 3, 0b111)
    regions[TO] = r.bytes(0x10)
    regions[OUT] = r.bytes(0x10)
    regions[END] = r.bytes(8)
    if name == 'strrchr':
        regions[TEXT] = bytes(r.choice([0x41, 0x42, 0x5c, 0xe9, r.randrange(1, 256)]) for _ in range(r.randrange(0, 12))) + b'\0' + r.bytes(4)
        return Call(0x67b5f0, [TEXT, r.choice([0x41, 0x5c, 0xe9, 0, 0x100, 0x141, r.getrandbits(32)])], regions, conv='c', ret='a')
    if name == 'strpbrk':
        regions[TEXT] = bytes(r.choice([0x31, 0x41, 0x7f, 0xe9, 0x80, 0xff, r.randrange(1, 256)]) for _ in range(r.randrange(0, 12))) + b'\0' + r.bytes(4)
        regions[SET] = bytes(r.choice([0x31, 0x7f, 0x80, 0xe9, 0xff, 0x1f, 0x20, r.randrange(1, 256)]) for _ in range(r.randrange(0, 5))) + b'\0'
        return Call(0x67d7f0, [TEXT, SET], regions, conv='c', ret='a')
    if name == 'throw':
        if r.random() < 0.5:
            regions[0x7089e8] = r.bytes(0x20)
        return Call(0x67d3a0, [r.getrandbits(32), r.getrandbits(32)], regions, conv='s', ret='v')
    if name == 'strncnt':
        regions[TEXT] = bytes(r.randrange(1, 256) for _ in range(r.randrange(0, 8))) + b'\0' + r.bytes(4)
        return Call(0x689180, [TEXT, r.choice([0, 1, 2, 3, 5, 8, 16])], regions, conv='c', ret='a')
    text = bytes(r.randrange(1, 256) for _ in range(r.randrange(0, 6))) + b'\0'
    if name == 'types':
        regions[TEXT] = text
        return Call(0x68a5d0, [r.choice([1, 2, 4]), TEXT, r.choice([-1, 1, 2, 5]), OUT, r.choice([0, 932]), r.choice([0, 0x409]), r.randrange(2)], regions, conv='c', ret='a')
    if name == 'map':
        regions[TEXT] = text
        return Call(0x688f50, [r.choice([0, 0x409]), r.choice([0x100, 0x200, 0x400, 0x400]), TEXT, r.choice([-1, 0, 1, 2, 5]), r.choice([TO, TO, TO, 0]), r.choice([0, 0, 1, 2, 3, 8]), r.choice([0, 932]), r.randrange(2)], regions, conv='c', ret='a')
    if name in ('isctype', 'toupper', 'toupper_lk'):
        c = r.choice([0x41, 0x61, 0x7a, 0x7b, 0x60, 0x30, 0xe9, 0xff, 0x100, 0x3042, 0x8141, 0xe941, 0x10041, -1, -5, -0x100, r.randrange(0x100), r.getrandbits(16)])
        if name == 'isctype':
            return Call(0x680430, [c & 0xffffffff, r.choice([1, 2, 4, 8, 0x103, 0x8000, 0x10000, 0xffffffff])], regions, conv='c', ret='a')
        return Call(0x67f1a0 if name == 'toupper' else 0x67f230, [c & 0xffffffff], regions, conv='c', ret='a')
    regions[TEXT] = r.choice([b'', b' ', b'  \t', b'']) + r.choice([b'', b'', b'-', b'+']) + r.choice(NUMBERS) + r.choice([b'', b'', b' ', b'.5']) + b'\0'
    base = r.choice([0, 0, 2, 8, 10, 10, 10, 16, 16, 36, 35, 37, 1, -1, 3])
    end = r.choice([END, END, END, 0])
    if name == 'strtoxl':
        return Call(0x67d540, [TEXT, end, base & 0xffffffff, r.choice([0, 1, 2, 3])], regions, conv='c', ret='a')
    return Call(0x67d520 if name == 'strtol' else 0x67d7d0, [TEXT, end, base & 0xffffffff], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
