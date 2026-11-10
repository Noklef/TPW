# The C++ library's num_get of wchar_t (src/game/core/numget) against the originals: do_get of a bool, an unsigned short, an unsigned int, a long, an unsigned long, a pointer, a float, a double and a long double, and
# _Getifld, over numget.py's world:
# the stream buffer's underflow and uflow and the facet's slots thiscall recorders, the C library's strtol, strtoul and _Stod. The literals are widened by btowc over _Mbrtowc, a cdecl recorder whose
# answers come by the call (so '0', '+' or 'x' widen to what the case deals); the stream's units are narrowed by wctob over _Wcrtomb in the C locale, its InterlockedIncrement and
# InterlockedDecrement COM recorders.
# Bench limit: the recorders' answers stop moving once the log holds 40 calls (each narrowed unit logs two): a long input reads its buffer, and underflow answers the end only.
# usage: wnumget.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _cdecl_log, _free, _recorder, answers, blob, fill, heap, import_table, log_fold, log_region, recorder, set_return, strings  # noqa: E402
import numget  # noqa: E402

LOCK, UNLOCK, GETPTD, DELETE, UNDERFLOW, UFLOW, SEPARATOR, GROUPING, THROW, POINT, INC, DEC, MBRTOWC, NUMPUNCT, FALSE_NAME, TRUE_NAME = 7, 8, 2, 0, 10, 11, 12, 13, 14, 17, 20, 21, 23, 28, 15, 16

B = Bench([(0x69d1c0, 0x69d693), (0x69d6a0, 0x69db6f), (0x69db70, 0x69e02b), (0x69e030, 0x69e4eb), (0x69e4f0, 0x69e9de), (0x69e9e0, 0x69eec8), (0x69eed0, 0x69f3b8), (0x69f3c0, 0x69f863),
           (0x69d040, 0x69d098), (0x69d0a0, 0x69d15f), (0x69d160, 0x69d1b9), (0x69fea0, 0x69feef), (0x69f870, 0x69f8a2), (0x693dc0, 0x693e15), (0x69a5d0, 0x69a5fb), (0x69a460, 0x69a4d3),
           (0x69a4e0, 0x69a55e), (0x6a4790, 0x6a49da), (0x696090, 0x696450), (0x698600, 0x698640), (0x698bf0, 0x698c40), (0x6991f0, 0x699408),
           (0x4ef790, 0x4ef7e8), (0x4ef7f0, 0x4ef82a), (0x699ce0, 0x699d47), (0x699d50, 0x699d9b), (0x4f21a0, 0x4f21cf), (0x4f2180, 0x4f2199), (0x6996f0, 0x699770), (0x6a5b80, 0x6a5c00),
           (0x45b340, 0x45b9a0), (0x47b1c0, 0x47b27c), (0x47b280, 0x47b2f8), (0x5c8c30, 0x5c8d12), (0x69c660, 0x69c67b), (0x6a1880, 0x6a189b), (0x6a87f0, 0x6a880b),
           (0x67d520, 0x67d7f0), (0x680430, 0x6804c5), (0x67f1a0, 0x67f222), (0x686a00, 0x686a10), (0x67f570, 0x67f615), (0x5d0050, 0x5d0071), (0x67dcd0, 0x67dcf0), (0x67ae00, 0x67aefe),
           (0x67d250, 0x67d340), (0x67c250, 0x67c288), (0x67d7f0, 0x67d82a), (0x67df40, 0x67e010), (0x67a480, 0x67a519), (0x688730, 0x68876d), (0x688770, 0x6887d1),
           (0x6ac940, 0x6ac9a0), (0x6d7710, 0x6d782e), (0x67a560, 0x67a592), (0x691530, 0x69153e), (0x67bb90, 0x67bd82), (0x67faf0, 0x67fbf6), (0x67fcb0, 0x67fd63),
           (0x6816a0, 0x68173e), (0x6828a0, 0x682a0a), (0x682c00, 0x682e3b), (0x682e40, 0x682fba), (0x688710, 0x688738), (0x688f50, 0x68917d), (0x689180, 0x6891b7), (0x68a5d0, 0x68a710),
           (0x68bc00, 0x68bc6e), (0x68bc70, 0x68bce1), (0x68bce0, 0x68bd85), (0x68bd80, 0x68bda5), (0x68bda0, 0x68bdb7), (0x68bdb0, 0x68bdd5), (0x68bdd0, 0x68be89), (0x68be90, 0x68c067),
           (0x68c060, 0x68c080), (0x68c440, 0x68caa9), (0x68f9d0, 0x68f9fb), (0x68fa00, 0x68fa70), (0x68fa70, 0x68faa6), (0x68faa0, 0x68fad5), (0x68fad0, 0x68fbc9), (0x68ff60, 0x690220),
           (0x690220, 0x6902b2), (0x6918f0, 0x691a1a), (0x691ba0, 0x691cca),
           (0x69cb60, 0x69cf6c), (0x69f8b0, 0x69fe9c), (0x69cf70, 0x69d03b), (0x47b750, 0x47bcf0), (0x47cc10, 0x47cd60), (0x4f23f0, 0x4f2440),
           (0x405280, 0x405390), (0x479150, 0x4791b0)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True),
                 0x691de0: numget.lock, 0x691ea0: lambda e: None, 0x67ad90: _free, 0x67a7d0: _cdecl_log, 0x67d3a0: _recorder(THROW, 2, com=True), 0x69c680: _recorder(NUMPUNCT, 1)})
OUT, BUFFER, NEXT, LEFT, VTABLE, IOS, AREA, A_UNDER, A_UFLOW = RAM, RAM + 0x20, RAM + 0x80, RAM + 0x90, RAM + 0xa0, RAM + 0xc0, RAM + 0x100, RAM + 0x200, RAM + 0x300
FACET, FACET_VTABLE, IMP, IMP_VTABLE, FACETS, PTD, STATE, VALUE, PCTYPE, LCONV, FIRST, LAST, TEXT, LOCALE = (RAM + 0xa00 + 0x40 * i for i in range(14))
GROUP_BLOCK, FALSE_BLOCK, TRUE_BLOCK = HEAP + 0x800, HEAP + 0x840, HEAP + 0x880
GLOBALS = 0xfb1da8
GETS = {'bool': 0x69cb60, 'ushort': 0x69d1c0, 'uint': 0x69d6a0, 'long': 0x69db70, 'ulong': 0x69e030, 'pointer': 0x69f3c0, 'float': 0x69e4f0, 'double': 0x69e9e0, 'ldouble': 0x69eed0}
OPS = [n for n in list(GETS) + ['bool', 'getifld', 'getifld'] if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
# btowc's calls in the order a scan makes them: '0' first, then the sign's (no '-' after a '+'), then the base's or the exponent's
LITERALS = {'int': ['0+-xX', '0+xX', '0+-x', '0+-xX'], 'float': ['0+-eE+-', '0+eE+-', '0+-e+-', '0+-eE+', '0+-eE-']}
ODD = [0xff10, 0x3000, 0x100, 0xffff, 0x2b00, 0x30]   # (fullwidth '0', past a byte, WEOF, a high byte with '+' low, a narrow '0')


def name_string(r, regions, log, k, at, text, which, high):
    """numget.py's name_string with 16-bit units (the count still the byte before them), each with the high bits."""
    held = regions[HEAP]
    held[at - HEAP - 1] = r.choice([0, 0, 1, 2, 0xfe, 0xff])
    data = struct.pack('<%dH' % (len(text) + 1), *[c | high for c in text], 0)
    held[at - HEAP:at - HEAP + len(data)] = data
    strings(log, k, 0, out=1)
    fill(log, k, 0, struct.pack('<4I', 0, at if text or r.random() < 0.5 else 0, len(text), max(len(text), 0x1f)), which=which)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    log_fold(log)
    regions = {LOG: log, HEAP: heap(0x1000), PTD: bytes(8) + struct.pack('<I', r.choice([0, 0, 0, 0x22])) + bytes(0x10), OUT: r.bytes(8),
               IMPORT_TABLE: import_table(log, {'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0)})}
    set_return(log, GETPTD, PTD)
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[PCTYPE] = numget.CTYPE
    floats = name in ('float', 'double', 'ldouble')
    regions[0x77e7dc] = struct.pack('<I', 1 if floats else r.choice([1, 1, 1, 1, 2]))
    regions[0xfb1438] = bytes(0x1c)   # (_lc_handle, LC_CTYPE's 0: _Wcrtomb's C locale; _lc_codepage)
    regions[0x709b70] = b'0123456789abcdefABCDEF\0\0'
    regions[0x6fe5ac] = struct.pack('<I', 0xffffffff) + r.choice([b'\0', b'\0', b'\0', b'\x02', b'\x7f']) + bytes(0xf)
    regions[0x75c734] = b'missing locale facet\0\0\0\0'
    # The literals btowc widens, a call at a time.
    order = r.choice(LITERALS['float' if floats else 'int'])
    widened = [ord(c) if r.random() < 0.9 else r.choice(ODD) for c in order]
    widened += [r.choice(ODD + [0x2b, 0x30]) for _ in range(8)]
    fill(log, MBRTOWC, 0, struct.pack('<%dI' % len(widened), *widened), records=len(widened))
    strings(log, MBRTOWC, 0, out=9)
    blob(log, MBRTOWC, 1, 1)
    set_return(log, MBRTOWC, r.choice([1, 1, 1, 1, 1, 1, 0, 0xffffffff, 0xfffffffe]))
    # The stream: its units waiting in the buffer, or nothing there and every unit from underflow.
    text = r.choice(numget.WORDS if name == 'bool' and r.random() < 0.7 else numget.FLOATS if floats and r.random() < 0.8 else numget.NUMBERS)
    if floats and r.random() < 0.15:
        text = bytes(r.choice(b'0123456789') for _ in range(r.randrange(34, 42))) + r.choice([b'', b'.5', b'e-3'])
    high = r.choice([0, 0, 0x100, 0x3000]) if name == 'bool' else 0   # (the names and the input past a byte alike: they match as units, not as chars)
    units = [c | high for c in text]
    for i in range(len(units)):
        if r.random() < 0.06:
            units[i] = r.choice(ODD + [widened[0]])
    buffered = r.random() < 0.8
    if not buffered:
        units = units[:5]   # (four logged calls a unit or more: the input must end before the log's 40 are full)
    regions[AREA] = struct.pack('<%dH' % len(units), *units) + b'.\0' + r.bytes(0xc0 - 2 * len(units) - 2)
    regions[NEXT] = struct.pack('<I', AREA if buffered else r.choice([0, AREA]))
    regions[LEFT] = struct.pack('<I', len(units) if buffered else 0)
    eof = r.choice([0xffff, 0xffff, 0xffffffff])
    if not buffered:
        under, uflow = units, units
    elif len(units) < 5:
        under, uflow = [r.choice([eof, eof, 0x31, 0x2c]) for _ in range(3)], [r.choice([eof, 0x31]) for _ in range(3)]
    else:
        under, uflow = [], []
    long_read = buffered and not under
    regions[0xfb3ca4] = struct.pack('<2I', r.choice([0, 1]) if long_read else 0, r.randrange(4))   # (_setlc_active, the readers: locked, a unit logs four calls)
    regions[A_UNDER] = struct.pack('<40I', *(list(under) + [eof] * 40)[:40])
    regions[A_UFLOW] = struct.pack('<40I', *(list(uflow) + [eof] * 40)[:40])
    answers(log, UNDERFLOW, A_UNDER)
    answers(log, UFLOW, A_UFLOW)
    regions[VTABLE] = struct.pack('<6I', 0, 0, 0, 0, recorder(UNDERFLOW, 0), recorder(UFLOW, 0))
    regions[BUFFER] = struct.pack('<I', VTABLE) + bytes(0x18) + struct.pack('<I', NEXT) + bytes(0xc) + struct.pack('<I', LEFT) + bytes(0x10)
    # The locale and its facet.
    groups = r.choice(numget.GROUPS if text.count(b',') != 2 else [b'\x7f', b'\x03\x7f', b'\x02\x01\x7f'])
    held = regions[HEAP]
    held[GROUP_BLOCK - HEAP] = r.choice([0, 0, 0, 1, 2, 0xfe, 0xff])
    held[GROUP_BLOCK - HEAP + 1:GROUP_BLOCK - HEAP + 1 + len(groups)] = groups
    held[GROUP_BLOCK - HEAP + 1 + len(groups)] = 0
    fields = (GROUP_BLOCK + 1, len(groups), max(len(groups), r.choice([1, 0x1f]))) if r.random() < 0.92 else (0, 0, 0)
    strings(log, GROUPING, 0, out=1)
    fill(log, GROUPING, 0, struct.pack('<4I', 0, *fields), which=1)
    false_name, true_name = r.choice(numget.NAMES)
    name_string(r, regions, log, FALSE_NAME, FALSE_BLOCK, false_name, 2, high)
    name_string(r, regions, log, TRUE_NAME, TRUE_BLOCK, true_name, 3, high)
    regions[0x6fd6c4] = bytes(4)   # (the wide empty string, and npos)
    regions[0x6fef00] = struct.pack('<I', 0xffffffff)
    regions[FACET_VTABLE] = struct.pack('<6I', recorder(DELETE, 1), recorder(POINT, 0), recorder(SEPARATOR, 0), recorder(GROUPING, 1), recorder(FALSE_NAME, 1), recorder(TRUE_NAME, 1))
    set_return(log, POINT, r.getrandbits(16) << 16 | r.choice([0x2e, 0x2e, 0x2c, 0x65, 0, 0xff0e, 0x12e]))
    regions[LCONV] = struct.pack('<I', LCONV + 4) + r.choice([b'.', b',', b'\0']) + bytes(3)
    regions[0x781fc8] = struct.pack('<I', LCONV)
    regions[FACET] = struct.pack('<2I', FACET_VTABLE, r.choice([0, 1, 0xffffffff])) + bytes(0x10)
    set_return(log, SEPARATOR, r.getrandbits(16) << 16 | r.choice([0x2c, 0x2c, 0x2c, 0, 0x2e, 0x31, 0x12c, 0xff0c]))
    regions[IMP_VTABLE] = struct.pack('<I', recorder(DELETE, 1))
    regions[FACETS] = struct.pack('<16I', *[r.choice([FACET, FACET, FACET, 0]) for _ in range(16)])
    regions[IMP] = struct.pack('<5I', IMP_VTABLE, r.choice([1, 1, 2, 0, 0xffffffff]), FACETS, r.choice([0, 1, 3, 8, 16, 16]), 0) + bytes([r.choice([0, 0, 1])]) + bytes(0xb)
    ids = bytearray(0x90)
    struct.pack_into('<I', ids, 0x34, r.choice([0, r.randrange(1, 16), r.randrange(1, 16), 0x40]))   # (numpunct<wchar_t>'s id 0xfb1ddc, its classic cache 0xfb1e1c and holder 0xfb1e30)
    struct.pack_into('<I', ids, 0x74, FACET)   # (the classic facet's making: usefacet.py's, its atexit by the env natively)
    struct.pack_into('<I', ids, 0x88, r.getrandbits(32))
    struct.pack_into('<3I', ids, 0x24, r.choice([IMP, IMP, IMP, IMP, 0x1234]), IMP, r.randrange(0, 15))
    regions[GLOBALS] = ids
    set_return(log, NUMPUNCT, FACET)
    blob(log, THROW, 0, 0x7)
    flags = r.choice(numget.FLAGS)
    regions[IOS] = bytes(0xc) + struct.pack('<I', flags) + bytes(0x10) + struct.pack('<I', IMP) + bytes(0x1c)
    first = [BUFFER if r.random() < 0.95 else 0, r.getrandbits(32) & 0xffffff00 | r.choice([0, 0, 0, 0, 0, 1])]
    last = [0 if r.random() < 0.93 or (floats and not first[0]) else BUFFER, r.getrandbits(32) & 0xffffff00 | r.choice([0, 1])]   # (numget.py's: a stale unit with last on a streambuf can loop for ever)
    if name == 'getifld':
        regions[TEXT] = r.bytes(0x40)
        regions[FIRST] = struct.pack('<2I', *first)
        regions[LAST] = struct.pack('<2I', *last)
        regions[LOCALE] = struct.pack('<I', IMP)
        return Call(0x69f8b0, [TEXT, FIRST, LAST, flags, LOCALE], regions, conv='c', ret='a')
    regions[STATE] = struct.pack('<I', r.choice([0, 0, 0, 1, 2, 4, r.getrandbits(32)]))
    regions[VALUE] = r.bytes(8)
    return Call(GETS[name], [r.getrandbits(32), OUT] + first + last + [IOS, STATE, VALUE], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
