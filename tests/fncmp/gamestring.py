# The game's String and WString classes (src/game/core/string: 0x5f5fc0-0x5f6c50, 0x5f7120-0x5f7e60, 0x5c7490) against the originals: strings of up to 24 units
# (a small alphabet, so equal, prefix and ordered cases come up), views of them, C strings and chars as arguments; the blocks come from the heap region.
# Format is left out (the game's vsprintf). StripSuffix (0x424d30) cuts a case-blind suffix off a string; SkipDot (0x41cea0) steps over a leading "." and the
# "\" / "." chars after it. EnsureSlash (0x5c5480) appends the global "\" unless s ends in it, IsWad (0x5c53c0) tests a directory entry's name for the
# global ".wad" at its end and JoinPath (0x5c5580) builds +0x3c of five strings from +0x28, "\" and a name.
# usage: gamestring.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, wide, extra arguments, returns). Extra arguments: t text pointer (or 0), c char pointer, v view of the other string, w the other
# class's string, n a count / start, f flags, o an output view, l a length.
OPS_TABLE = {
    'Alloc': (0x5f5fc0, 0, 'l', 'v'), 'SetLen': (0x5f6000, 0, 'l', 'v'), 'CtorStr': (0x5f60b0, 0, 't', 'a'), 'CtorCopy': (0x5f6040, 0, 'v', 'a'),
    'CtorChar': (0x5f6150, 0, 'c', 'a'), 'Ctor': (0x5f61a0, 0, '', 'a'), 'Dtor': (0x5f61e0, 0, '', 'v'), 'AssignChar': (0x5f6200, 0, 'c', 'a'),
    'AssignStr': (0x5f6240, 0, 't', 'a'), 'AssignWide': (0x5f6290, 0, 'w', 'a'), 'Assign': (0x5f62d0, 0, 'v', 'a'), 'AppendStr': (0x5f6320, 0, 't', 'a'),
    'AppendChar': (0x5f6360, 0, 'c', 'a'), 'Append': (0x5f6390, 0, 'v', 'a'), 'DelDtor': (0x5f7120, 0, 'f', 'a'), 'GetBuf': (0x5f78e0, 0, 'l', 'a'),
    'RelBuf': (0x5f7900, 0, 'L', 'v'), 'Equals': (0x5f7950, 0, 'v', 'b'), 'EqStr': (0x5f7980, 0, 't', 'b'), 'NeStr': (0x5f79e0, 0, 't', 'b'),
    'NeChar': (0x5f7a50, 0, 'c', 'b'), 'Less': (0x5f7a90, 0, 'v', 'b'), 'FindChar': (0x5f7ae0, 0, 'cn', 'a'), 'Find': (0x5f7b40, 0, 'tn', 'a'),
    'RFindChar': (0x5f7b90, 0, 'cn', 'a'), 'RFind': (0x5f7be0, 0, 'tn', 'a'), 'FindOneOf': (0x5f7c70, 0, 'tn', 'a'), 'RFindOneOf': (0x5f7cf0, 0, 'tn', 'a'),
    'Left': (0x5f7d60, 0, 'on', 'a'), 'Right': (0x5f7d90, 0, 'on', 'a'), 'From': (0x5f7dd0, 0, 'on', 'a'), 'Mid': (0x5f7e10, 0, 'onn', 'a'),
    'WAlloc': (0x5f6870, 1, 'l', 'v'), 'WSetLen': (0x5f68b0, 1, 'l', 'v'), 'WCtorStr': (0x5f6980, 1, 't', 'a'), 'WCtorCopy': (0x5f6900, 1, 'v', 'a'),
    'WCtor': (0x5f6a20, 1, '', 'a'), 'WDtor': (0x5f6a60, 1, '', 'v'), 'WAssignChar': (0x5f6a80, 1, 'c', 'a'), 'WAssignStr': (0x5f6ac0, 1, 't', 'a'),
    'WAssignNarrow': (0x5f6b10, 1, 'w', 'a'), 'WAssign': (0x5f6b50, 1, 'v', 'a'), 'WAppendStr': (0x5f6ba0, 1, 't', 'a'), 'WAppendChar': (0x5f6be0, 1, 'c', 'a'),
    'WAppend': (0x5f6c10, 1, 'v', 'a'), 'WDelDtor': (0x5f7150, 1, 'f', 'a'), 'WNeStr': (0x5f7e60, 1, 't', 'b'),
    'WCtorReturning': (0x5c7490, 1, '', 'a'),
    'StripSuffix': (0x424d30, 0, '', 'a'), 'SkipDot': (0x41cea0, 0, '', 'a'), 'EnsureSlash': (0x5c5480, 0, '', 'v'), 'IsWad': (0x5c53c0, 0, '', 'a'),
    'JoinPath': (0x5c5580, 0, '', 'v'),
}
B = Bench([(0x41cea0, 0x41cfe4), (0x424cf0, 0x424dc0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x5c53c0, 0x5c5510), (0x5c5580, 0x5c55a9), (0x5c7490, 0x5c749c)])
DOT, BACKSLASH = 0x78c648, 0x78bee0   # the game's global strings "." and "\" (String objects in .bss)
SLASH2, WAD = 0xf80be0, 0xf80808      # and "\" and ".wad" of the folder code
LOCALE = 0xfb1440   # the CRT's LC_CTYPE handle: 0, the C locale (_stricmp's)
S1, S2, S3, TXT, CHR, OUT = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200
D1, D2, D3 = RAM + 0x2000, RAM + 0x3000, RAM + 0x4000
VT = {0: (0x703090, 0x7030c0), 1: (0x7030c8, 0x7030f8)}  # string and view vtables by width
ALPHA = [b'a', b'b', b'A', b'a', b'z', b'\x80', b'\xff', b'b']


def units(r, wide, n):
    chars = [r.choice(ALPHA)[0] for _ in range(n)]
    return struct.pack('<%d%s' % (n, 'H' if wide else 'B'), *chars)


def string(r, wide, base, data, regions, text=None):
    """A string object at base with its block at data; returns its length."""
    n = r.choice([0, 1, 2, 3, 5, 7, 8, 9, 15, 16, 17, 24]) if text is None else len(text) // (2 if wide else 1)
    raw = text if text is not None else units(r, wide, n)
    cap = (n & ~7) + 8
    w = 2 if wide else 1
    block = bytearray(r.bytes(w * (cap + 1)))
    block[:len(raw)] = raw
    block[w * n:w * n + w] = bytes(w)
    regions[data] = block
    regions[base] = struct.pack('<5I', VT[wide][0], n, data, cap, 8) + bytes(0x20)
    return n


def strip_suffix(r):
    """0x424d30 (cdecl; suffix, s): the suffix the tail of s (case varied) or other text, of any length up to past s's; now and then s itself."""
    regions = {HEAP: heap(), LOCALE: bytes(4)}
    n = string(r, 0, S1, D1, regions)
    text = bytes(regions[D1][:n])
    if r.random() < 0.1:
        return Call(0x424d30, [S1, S1], regions, ret='a')
    k = r.choice([0, 1, 2, 3, n, n, n + 1, n + 3]) if r.random() < 0.6 else 0
    tail = bytearray(text[max(n - k, 0):])
    for i in range(len(tail)):
        if r.random() < 0.3 and (0x61 <= tail[i] <= 0x7a or 0x41 <= tail[i] <= 0x5a):
            tail[i] ^= 0x20
    suffix = bytes(tail) if r.random() < 0.6 else units(r, 0, r.choice([0, 1, 2, 3, 5, 25]))
    string(r, 0, S2, D2, regions, suffix)
    return Call(0x424d30, [S2, S1], regions, ret='a')


def skip_dot(r):
    """0x41cea0 (cdecl; s): s of dots, backslashes and letters (mostly starting with a dot); the globals "." and "\\", now and then
    of other texts or lengths (two chars, empty)."""
    regions = {HEAP: heap(), LOCALE: bytes(4)}
    n = r.choice([0, 1, 2, 3, 4, 6, 9, 17])
    text = bytes(r.choice(b'..\\\\aA/') for _ in range(n))
    if n and r.random() < 0.8:
        text = b'.' + text[1:]
    string(r, 0, S1, D1, regions, text)
    for base, data, usual in ((DOT, D2, b'.'), (BACKSLASH, D3, b'\\')):
        own = usual if r.random() < 0.85 else r.choice([b'', b'.\\', b'..', b'\\\\', b'a'])
        string(r, 0, base, data, regions, own)
    return Call(0x41cea0, [S1], regions, ret='a')


def ensure_slash(r):
    """0x5c5480 (cdecl; s): s of letters, "\\" and "/", mostly ending in "\\" or not; the global "\\", now and then another text or length."""
    regions = {HEAP: heap(), LOCALE: bytes(4)}
    n = r.choice([0, 1, 2, 3, 5, 8, 9, 16])
    text = bytes(r.choice(b'aAb\\/') for _ in range(n))
    if n and r.random() < 0.4:
        text = text[:-1] + b'\\'
    string(r, 0, S1, D1, regions, text)
    string(r, 0, SLASH2, D2, regions, b'\\' if r.random() < 0.8 else r.choice([b'', b'/', b'\\\\', b'a\\', b'A']))
    return Call(0x5c5480, [S1], regions, ret='v')


def is_wad(r):
    """0x5c53c0 (cdecl; entry): names ending in ".wad" (case varied), near misses, short ones, a null name now and then; the global ".wad",
    now and then another text."""
    regions = {HEAP: heap(), LOCALE: bytes(4)}
    stem = r.choice([b'', b'a', b'data', b'x.wad', b'.'])
    end = r.choice([b'.wad', b'.WAD', b'.Wad', b'.wa', b'wad', b'.wadx', b'', b'.bad', b'd'])
    name = stem + end
    regions[TXT] = name + bytes(1) + r.bytes(8)
    regions[S3] = r.bytes(0x20) + struct.pack('<I', 0 if r.random() < 0.05 else TXT) + r.bytes(0x10)
    string(r, 0, WAD, D2, regions, b'.wad' if r.random() < 0.8 else r.choice([b'', b'.WAD', b'd', b'.wadx']))
    return Call(0x5c53c0, [S3], regions, ret='a')


def join_path(r):
    """0x5c5580 (thiscall; name): five strings, of which +0x28 (read as the other class's string, as the game's does: a wide one or a
    narrow one with room past its block) and +0x3c are filled; a name string, now and then +0x28 or +0x3c itself; the global "\\"."""
    regions = {HEAP: heap(), LOCALE: bytes(4)}
    obj = bytearray(r.bytes(0x64))
    for i, (base, data) in enumerate(((S1 + 0x28, D1), (S1 + 0x3c, D2))):
        wide = i == 0 and r.random() < 0.6
        part = {}
        string(r, wide, base, data, part, units(r, wide, r.choice([0, 1, 2, 4, 7, 9])) if r.random() < 0.8 else None)
        obj[base - S1:base - S1 + 0x14] = part.pop(base)[:0x14]
        block = part[data]
        regions[data] = block + (r.bytes(len(block) + 2) if i == 0 and not wide else b'')
    regions[S1] = bytes(obj)
    k = r.random()
    if k < 0.1:
        name = S1 + 0x28
    elif k < 0.2:
        name = S1 + 0x3c
    else:
        string(r, 0, S2 + 0x40, D3, regions)
        name = S2 + 0x40
    string(r, 0, SLASH2, RAM + 0x5000, regions, b'\\' if r.random() < 0.85 else r.choice([b'', b'/', b'a\\']))
    return Call(0x5c5580, [S1, name], regions, conv='t', ret='v')


SPECIAL = {'StripSuffix': strip_suffix, 'SkipDot': skip_dot, 'EnsureSlash': ensure_slash, 'IsWad': is_wad, 'JoinPath': join_path}


def case(r):
    name = OPS[r.deal(range(len(OPS)))]
    if name in SPECIAL:
        return SPECIAL[name](r)
    address, wide, kinds, ret = OPS_TABLE[name]
    w = 2 if wide else 1
    regions = {HEAP: heap(), OUT: r.bytes(0x20)}
    s_text = None
    construct = name.lstrip('W') in ('CtorStr', 'CtorCopy', 'CtorChar', 'Ctor', 'CtorReturning')
    if construct:
        regions[S1] = r.bytes(0x20)  # not constructed yet
    else:
        string(r, wide, S1, D1, regions)
    args = []
    for k in kinds:
        if k == 't':  # a C string of the class's width, sometimes null, sometimes a piece of s itself
            n = r.choice([0, 1, 2, 3, 6, 9, 17])
            t = units(r, wide, n) + bytes(w)
            regions[TXT] = t + r.bytes(0x20)
            args.append(0 if r.random() < 0.1 and name not in ('FindOneOf',) and kinds != 'tn' else TXT)
        elif k == 'c':
            regions[CHR] = struct.pack('<H' if wide else '<B', r.choice([0, 0x61, 0x62, 0x41, 0x80, 0xff, 0x100 if wide else 0x7a])) + bytes(2)
            args.append(CHR)
        elif k == 'v':  # a view or string of the same class: another string, or s itself
            if r.random() < 0.15:
                args.append(S1) if not construct else args.append(S2)
                if construct:
                    string(r, wide, S2, D2, regions)
            else:
                string(r, wide, S2, D2, regions)
                args.append(S2)
        elif k == 'w':  # the other class's string
            string(r, 1 - wide, S3, D3, regions)
            args.append(S3)
        elif k == 'n':
            args.append(r.choice([0, 1, 2, 3, 5, 8, 16, 30, 0xffffffff]))
        elif k == 'f':
            args.append(r.choice([0, 1, 1, 2, 3]))
        elif k == 'l':
            args.append(r.choice([0, 1, 5, 7, 8, 9, 16, 17, 40]))
        elif k == 'L':
            args.append(r.choice([0, 1, 5, 7, 8, 9, 16, 0x7fffffff]))
        elif k == 'o':
            args.append(OUT)
    return Call(address, [S1] + args, regions, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(OPS_TABLE)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
