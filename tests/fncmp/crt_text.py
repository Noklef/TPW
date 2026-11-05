# The C library's text routines (src/game/core/crt: strrchr 0x67b5f0, strpbrk 0x67d7f0, _CxxThrowException 0x67d3a0, strncnt 0x689180, __crtGetStringTypeA 0x68a5d0, __crtLCMapStringA 0x688f50, _isctype 0x680430,
# toupper 0x67f1a0 and 0x67f230, strtoxl 0x67d540, strtol 0x67d520, strtoul 0x67d7d0) against the originals: the Win32 calls COM recorders in their import slots, answering from a table; malloc's _nh_malloc, calloc,
# the lock and _getptd cdecl recorders; free the heap's hash of freed pointers; the table _pctype made here (with some noise), words before it too.
# usage: crt_text.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, blob, fill, heap, import_table, log_region, set_return, vtable  # noqa: E402

GETPTD, LOCK, UNLOCK, CALLOC, NHMALLOC = 2, 7, 8, 24, 28
INC, DEC, M2W, W2M, LCW, LCA, GSW, GSA, RAISE = 14, 15, 16, 17, 18, 19, 20, 29, 30
B = Bench([(0x67ff00, 0x67ff62), (0x689400, 0x689490), (0x67fa60, 0x67fae2), (0x6887e0, 0x6887fe), (0x67b5f0, 0x67b617), (0x67d3a0, 0x67d3e7), (0x67d520, 0x67d53a), (0x67d540, 0x67d7c1), (0x67d7d0, 0x67d7ea), (0x67d7f0, 0x67d82a), (0x67dcd0, 0x67dcf0), (0x67f1a0, 0x67f222), (0x67f230, 0x67f32e),
           (0x680430, 0x6804c5), (0x686a00, 0x686a10), (0x688f50, 0x689175), (0x689180, 0x6891af), (0x68a5d0, 0x68a708),
           (0x67b8c0, 0x67b9e3), (0x67bb90, 0x67bed0),(0x67c4e0, 0x67c5c8), (0x67df40, 0x67e010), (0x67f670, 0x67f771), (0x67faf0, 0x67fbee), (0x688770, 0x6887d1), (0x699ab0, 0x699bc8), (0x69a340, 0x69a458),
           (0x67ff70, 0x67ffeb), (0x67fff0, 0x680048), (0x680060, 0x6800e4), (0x67f570, 0x67f615), (0x67bfb0, 0x67c07c), (0x67c080, 0x67c100), (0x67a480, 0x67a519), (0x688730, 0x68876d),
           (0x67b0d0, 0x67b10f), (0x67bed0, 0x67bf0f), (0x67bf10, 0x67bf3d), (0x67c290, 0x67c2e2), (0x67c2f0, 0x67c31e), (0x67c3d0, 0x67c428), (0x67c430, 0x67c4de), (0x67f410, 0x67f44c), (0x67f450, 0x67f4a0),
           (0x699a20, 0x699ab0), (0x69a2b0, 0x69a340), (0x4f19d0, 0x4f1a05), (0x4f1a10, 0x4f1a43), (0x4f1a50, 0x4f1a85), (0x4f1a90, 0x4f1ac3)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True),
                 0x67fcb0: _recorder(CALLOC, 2, cdecl=True), 0x67ad90: _free})
del B.emu.hooks[0x67bb90]   # (memmove's stand-in in the emulator: here the original itself is the subject)
PTD, TEXT, SET, END, TO, OUT, ANSWERS, BLOCKS, BLOCK1, BLOCK2 =(RAM + 0x100 * i for i in range(10))
PCTYPE = RAM + 0x1200
CTYPE, CTYPEVT = RAM + 0x2000, RAM + 0x2100
WIDE = ('wcsncpy', 'wcscpy', 'wcscat', 'wcsncat', 'wcscmp', 'wcsncmp', 'wcsstr', 'wcschr', 'wcstok')
OPS = [n for n in ('genfname', 'genfname', 'ultoa', 'strrchr', 'strpbrk', 'throw', 'strncnt', 'types', 'map', 'isctype', 'toupper', 'tolower', 'toupper_lk', 'strtoxl', 'strtol', 'strtoul', 'stricmp', 'strnicmp', 'strnicmp', 'memmove', 'xtoa', 'strncat', 'strtok', 'tolower_lk', 'tolower_ctype', 'toupper_ctype', 'tolower_lock', 'toupper_lock', 'ctype_lower', 'ctype_upper', 'ctype_lower_range', 'ctype_upper_range',
                   'strlen', 'memset', 'strcmp', 'strstr', 'memchr', 'strchr', 'atol', 'itoa') + WIDE if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
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
    if name == 'stricmp':
        words = [b'', b'a', b'A', b'am/pm', b'AM/PM', b'a/p', b'Zz', b'zZ', b'[', b'_', b'`', b'\xe9', b'\xc9', b'abc', b'abd', b'ABE']
        regions[TEXT] = r.choice(words) + b'\0'
        regions[SET] = r.choice(words) + b'\0'
        if r.random() < 0.3:   # (Two capitals that map to one same pair of bytes: x keeps the map's high byte, and stricmp returns it at the end.)
            same = r.choice([b'', b'A', b'a/p'])
            regions[TEXT] = r.choice([b'B', b'XA']) + same + b'\0'
            regions[SET] = r.choice([b'C', b'YB']) + same + b'\0'
            regions[0xfb1440] = struct.pack('<5I', r.choice([0x409, 0x411]), 0, 0, 0, r.choice([932, 1252]))
            regions[0xfb1554] = struct.pack('<I', 2)
            regions[ANSWERS] = struct.pack('<40I', *[2] * 40)
        return Call(0x67df40, [TEXT, SET], regions, conv='c', ret='a')
    if name == 'strnicmp':
        words = [b'', b'a', b'A', b'abc', b'ABC', b'abd', b'ABE', b'Zz', b'zZ', b'[', b'_', b'`', b'\xe9', b'\xc9', b'abcdef', b'ABCDEG']
        regions[TEXT] = r.choice(words) + b'\0'
        regions[SET] = r.choice(words) + b'\0'
        most = r.choice([0, 1, 2, 3, 5, 8, 0x7fffffff])
        if r.random() < 0.3:   # (Two capitals that map to one same pair of bytes: their words keep the map's high byte, over the ends too, so only n stops the walk.)
            same = r.choice([b'', b'A', b'a/p'])
            regions[TEXT] = r.choice([b'B', b'XA']) + same + b'\0'
            regions[SET] = r.choice([b'C', b'YB']) + same + b'\0'
            regions[0xfb1440] = struct.pack('<5I', r.choice([0x409, 0x411]), 0, 0, 0, r.choice([932, 1252]))
            regions[0xfb1554] = struct.pack('<I', 2)
            regions[ANSWERS] = struct.pack('<40I', *[2] * 40)
            most = r.choice([1, 2, 3, 5, 8])
        return Call(0x67f670, [TEXT, SET, most], regions, conv='c', ret='a')
    if name == 'memmove':
        regions[TEXT] = r.bytes(0x100)
        return Call(0x67bb90, [TEXT + r.randrange(0x60), TEXT + r.randrange(0x60), r.choice([0, 1, 2, 3, 4, 7, 8, 0x1f, 0x20, 0x21, 0x40, r.randrange(0x60)])], regions, conv='c', ret='a')
    if name == 'xtoa':
        regions[TEXT] = r.bytes(0x30)
        return Call(0x688770, [r.choice([0, 1, 9, 10, 35, 36, 255, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)]), TEXT + 2, r.choice([2, 8, 10, 16, 36]), r.choice([0, 0, 1])], regions, conv='c')
    if name == 'itoa':
        regions[TEXT] = r.bytes(0x30)
        return Call(0x688730, [r.choice([0, 1, 9, 10, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)]), TEXT + 2, r.choice([2, 8, 10, 10, 16, 36])], regions, conv='c', ret='a')
    if name == 'genfname':
        regions[TEXT] = r.choice([b'a.0', b's3.1', b'x.y.vv', b'b.v', b'.1', b'c.vvt', b'c.vvu', b'c.vvv', b'c.zz', b'c.', b'nodot']) + b'\0' + r.bytes(8)
        regions[0xfb1870] = struct.pack('<I', r.choice([0, 0, 932]))
        regions[0x77ea9a] = bytes(r.choice([0, 0, 4]) for _ in range(0x100))
        return Call(0x67ff00, [TEXT], regions, conv='c', ret='a')
    if name == 'ultoa':
        regions[TEXT] = r.bytes(0x30)
        return Call(0x6887e0, [r.choice([0, 1, 9, 10, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)]), TEXT + 2, r.choice([2, 8, 10, 10, 16, 36])], regions, conv='c', ret='a')
    if name == 'strncat':
        regions[TEXT] = r.choice([b'', b'a', b'ab', b'abc', b'abcd', b'abcdefgh']) + b'\0' + r.bytes(0x60)
        regions[SET] = b'\xff' * r.randrange(4) + r.choice([b'', b'x', b'xy', b'xyz', b'wxyz', b'12345678', b'12345\x80\x8112345', b'0123456789abcdef']) + b'\0' + r.bytes(0x20)
        return Call(0x67b8c0, [TEXT + r.randrange(4), SET + r.randrange(4), r.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, 0x10, 0x40])], regions, conv='c', ret='a')
    if name == 'strtok':
        regions[TEXT] = r.choice([b'', b' ', b'a', b' a', b'a b', b'  a  b ', b'abc,def', b',,,', b'\xe9 x']) + b'\0' + r.bytes(8)
        regions[SET] = r.choice([b'', b' ', b', ', b'a', b'\xe9']) + b'\0'
        regions[PTD] = bytes(0x18) + struct.pack('<I', TEXT + 1) + bytes(4)
        return Call(0x67c4e0, [r.choice([TEXT, TEXT, TEXT, 0]), SET], regions, conv='c', ret='a')
    if name in ('tolower_lock', 'toupper_lock'):
        regions[TEXT] = struct.pack('<2I', r.choice([0, 0x409, 0x411]), r.choice([0, 932, 1252]))
        c = r.choice([0x41, 0x5a, 0x40, 0x5b, 0x61, 0x7a, 0x60, 0x7b, 0x30, 0xe9, 0xff, 0x100, 0x3042, r.randrange(0x100), r.getrandbits(16)])
        return Call(0x699a20 if name == 'tolower_lock' else 0x69a2b0, [c, r.choice([TEXT, TEXT, 0])], regions, conv='c', ret='a')
    if name.startswith('ctype_'):   # (ctype<wchar_t>'s: the slots 0x30 and 0x28 recorders, the locale in the object)
        object_ = bytearray(r.bytes(0x10))
        struct.pack_into('<I', object_, 0, CTYPEVT)
        struct.pack_into('<2I', object_, 8, r.choice([0, 0x409, 0x411]), r.choice([0, 932, 1252]))
        regions[CTYPE] = object_
        lower = 'lower' in name
        if name.endswith('_range'):
            count = r.choice([0, 1, 2, 3, 6])
            regions[TEXT] = struct.pack('<8H', *[r.choice([0x41, 0x5a, 0x61, 0x7a, 0xe9, 0x100, 0x3042, r.getrandbits(16)]) for _ in range(8)])
            return Call(0x4f1a10 if lower else 0x4f1a90, [CTYPE, TEXT, TEXT + 2 * count], regions, conv='t', ret='a')
        regions[CTYPEVT] = vtable({12: 2, 10: 1})
        set_return(log, 12, r.choice([0x41, 0x5a, 0x61, 0x7a, 0x80, 0xe9, r.getrandbits(32)]))
        set_return(log, 10, r.getrandbits(32))
        return Call(0x4f19d0 if lower else 0x4f1a50, [CTYPE, r.choice([0x41, 0x61, 0x3042, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name in ('tolower_lk', 'tolower_ctype', 'toupper_ctype'):
        c = r.choice([0x41, 0x5a, 0x40, 0x5b, 0x61, 0x7a, 0x60, 0x7b, 0x30, 0xe9, 0xff, 0x100, 0x3042, 0x8141, 0xe941, r.randrange(0x100), r.getrandbits(16)])
        if name == 'tolower_lk':
            return Call(0x67faf0, [c], regions, conv='c', ret='a')
        regions[TEXT] = struct.pack('<2I', r.choice([0, 0x409, 0x411]), r.choice([0, 932, 1252]))
        # (EDI: the caller's, which they pass on by mistake as __crtLCMapStringA's last argument.)
        return Call(0x699ab0 if name == 'tolower_ctype' else 0x69a340, [r.choice([0, 1, r.getrandbits(32)]), c, r.choice([TEXT, TEXT, 0])], regions, conv='edi', ret='a')
    if name == 'strlen':
        regions[TEXT] = r.bytes(4) + bytes(r.randrange(1, 256) for _ in range(r.randrange(0, 12))) + b'\0' + r.bytes(8)
        return Call(0x67ff70, [TEXT + r.randrange(4)], regions, conv='c', ret='a')
    if name == 'memset':
        regions[TEXT] = r.bytes(0x60)
        return Call(0x67fff0, [TEXT + r.randrange(8), r.choice([0, 0x41, 0xff, 0x1ff, r.getrandbits(32)]), r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x1f, 0x20, 0x21, r.randrange(0x40)])], regions, conv='c', ret='a')
    if name in ('strcmp', 'strstr'):
        words = [b'', b'a', b'A', b'ab', b'abc', b'abd', b'abcd', b'abcde', b'abcdefgh', b'abcdefgi', b'\xe9', b'\xe9a', b'a\xe9', b'aab', b'aaab', b'abab', b'xabcdabcde', b'bc', b'cd', b'de']
        regions[TEXT] = bytes(r.randrange(1, 256) for _ in range(3)) + r.choice(words) + b'\0' + r.bytes(8)
        regions[SET] = bytes(r.randrange(1, 256) for _ in range(3)) + r.choice(words) + b'\0' + r.bytes(8)
        if name == 'strstr' and r.random() < 0.1:     # (an empty one wanted in an empty text)
            regions[TEXT] = regions[SET] = bytes(3) + b'\0' + r.bytes(8)
        return Call(0x680060 if name == 'strcmp' else 0x67c080, [TEXT + 3, SET + 3], regions, conv='c', ret='a')
    if name in ('memchr', 'strchr'):
        wanted = r.choice([0x41, 0x80, 0xff, 0xe9, 0x141, 0, r.getrandbits(32)])
        regions[TEXT] = bytes(r.choice([0x41, 0x42, 0x80, 0xff, 0xc1, 0xe9, 0x7f, 0x81, 1, r.randrange(1, 256)]) for _ in range(r.randrange(0, 14))) + b'\0' + bytes(r.randrange(1, 256) for _ in range(12))
        at = TEXT + r.randrange(4) if len(regions[TEXT]) > 8 else TEXT
        if name == 'strchr':
            return Call(0x67bfc0, [at, wanted], regions, conv='c', ret='a')
        return Call(0x67f570, [at, wanted, r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xc, r.randrange(0x18)])], regions, conv='c', ret='a')
    if name == 'atol':
        regions[TEXT] = r.choice([b'', b' ', b'  \t', b'\t ']) + r.choice([b'', b'', b'-', b'+', b'--', b'+-']) + r.choice([b'', b'0', b'7', b'123', b'2147483647', b'2147483648', b'4294967296', b'99999999999', b'12a3', b'a', b' 1', b'\xe9', b'1\xe9']) + b'\0'
        return Call(0x67a480, [TEXT], regions, conv='c', ret='a')
    if name in WIDE:
        units = lambda most: b''.join(struct.pack('<H', r.choice([0x41, 0x42, 0x43, 0x100, 0x141, 0xff41, 0x8000, r.randrange(1, 0x10000)])) for _ in range(r.randrange(most)))   # noqa: E731
        regions[TEXT] = units(8) + bytes(2) + r.bytes(0x20)
        regions[SET] = units(5) + bytes(2) + r.bytes(0x10)
        count = r.choice([0, 1, 2, 3, 4, 5, 8, 0x10])
        if name in ('wcsncpy', 'wcscpy'):
            regions[TO] = r.bytes(0x40)
            return Call(*((0x67b0d0, [TO, TEXT, count]) if name == 'wcsncpy' else (0x67bf10, [TO, TEXT])), regions, conv='c', ret='a')
        if name in ('wcscat', 'wcsncat'):
            return Call(*((0x67f450, [TEXT, SET, count]) if name == 'wcsncat' else (0x67bed0, [TEXT, SET])), regions, conv='c', ret='a')
        if name in ('wcscmp', 'wcsncmp', 'wcsstr'):
            if r.random() < 0.6:    # (a shared start, or the second inside the first)
                start = r.randrange(0, 6)
                shared = regions[TEXT][2 * start:]
                regions[SET] = shared[:2 * r.randrange(0, 5)] + r.choice([b'', b'\x41\0']) + bytes(2)
                if r.random() < 0.5:
                    regions[SET] = regions[TEXT][:0x10]
            if name == 'wcscmp':
                return Call(0x67c290, [TEXT, SET], regions, conv='c', ret='a')
            return Call(*((0x67f410, [TEXT, SET, count]) if name == 'wcsncmp' else (0x67c3d0, [TEXT, SET])), regions, conv='c', ret='a')
        if name == 'wcschr':
            return Call(0x67c2f0, [TEXT, r.choice([0x41, 0x100, 0xff41, 0, 0x10041, r.getrandbits(32)])], regions, conv='c', ret='a')
        regions[TEXT] = b''.join(struct.pack('<H', r.choice([0x20, 0x2c, 0x41, 0x100, 0x120])) for _ in range(r.randrange(8))) + bytes(2) + r.bytes(8)
        regions[SET] = b''.join(struct.pack('<H', r.choice([0x20, 0x2c, 0x120])) for _ in range(r.randrange(3))) + bytes(2)
        regions[PTD] = bytes(0x1c) + struct.pack('<I', TEXT + 2)
        return Call(0x67c430, [r.choice([TEXT, TEXT, TEXT, 0]), SET], regions, conv='c', ret='a')
    if name in ('isctype', 'toupper', 'tolower', 'toupper_lk'):
        c = r.choice([0x41, 0x61, 0x7a, 0x7b, 0x60, 0x30, 0xe9, 0xff, 0x100, 0x3042, 0x8141, 0xe941, 0x10041, -1, -5, -0x100, r.randrange(0x100), r.getrandbits(16)])
        if name == 'isctype':
            return Call(0x680430, [c & 0xffffffff, r.choice([1, 2, 4, 8, 0x103, 0x8000, 0x10000, 0xffffffff])], regions, conv='c', ret='a')
        return Call({'toupper': 0x67f1a0, 'tolower': 0x67fa60, 'toupper_lk': 0x67f230}[name], [c & 0xffffffff], regions, conv='c', ret='a')
    regions[TEXT] = r.choice([b'', b' ', b'  \t', b'']) + r.choice([b'', b'', b'-', b'+']) + r.choice(NUMBERS) + r.choice([b'', b'', b' ', b'.5']) + b'\0'
    base = r.choice([0, 0, 2, 8, 10, 10, 10, 16, 16, 36, 35, 37, 1, -1, 3])
    end = r.choice([END, END, END, 0])
    if name == 'strtoxl':
        return Call(0x67d540, [TEXT, end, base & 0xffffffff, r.choice([0, 1, 2, 3])], regions, conv='c', ret='a')
    return Call(0x67d520 if name == 'strtol' else 0x67d7d0, [TEXT, end, base & 0xffffffff], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
