# The C++ library's num_get of char (src/game/core/numget) against the originals: do_get of a bool, an unsigned short, an unsigned int, a long, an unsigned long, a pointer, a float, a double and a long double, and _Getifld, over the C library's strtol, strtoul and _Stod: the stream buffer's underflow and uflow and the facet's slots thiscall recorders,
# numpunct's constructor (0x695910: A's) a thiscall recorder, _CxxThrowException a COM recorder, the locks do nothing.
# Bench limit: after a bad_cast the original goes on with a null facet and faults, as the port does.
# usage: numget.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _free, _recorder, answers, blob, fill, heap, log_fold, log_region, recorder, set_return, strings  # noqa: E402

LOCK, UNLOCK, GETPTD, DELETE, UNDERFLOW, UFLOW, SEPARATOR, GROUPING, THROW, FALSE_NAME, TRUE_NAME, NUMPUNCT, POINT = 7, 8, 2, 0, 10, 11, 12, 13, 14, 15, 16, 28, 17


def lock(e):
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x695ca0, 0x696090), (0x696090, 0x696450), (0x696450, 0x6968a0), (0x6968a0, 0x696ce0), (0x696ce0, 0x6970e0), (0x6970e0, 0x6974e0), (0x6981f0, 0x698600), (0x698600, 0x698640), (0x698640, 0x698bf0), (0x698bf0, 0x698c40), (0x6991f0, 0x699408),
           (0x4ef790, 0x4ef7e8), (0x4ef7f0, 0x4ef82a), (0x699ce0, 0x699d47), (0x699d50, 0x699d9b), (0x4f21a0, 0x4f21cf), (0x4f2180, 0x4f2199), (0x6996f0, 0x699770), (0x6a5b80, 0x6a5c00),
           (0x45b340, 0x45b9a0), (0x47b1c0, 0x47b27c), (0x47b280, 0x47b2f8), (0x5c8c30, 0x5c8d12), (0x69c660, 0x69c67b), (0x6a1880, 0x6a189b), (0x6a87f0, 0x6a880b),
           (0x67d520, 0x67d7f0), (0x680430, 0x6804c5), (0x67f1a0, 0x67f222), (0x686a00, 0x686a10), (0x67f570, 0x67f615), (0x5d0050, 0x5d0071), (0x67dcd0, 0x67dcf0), (0x67ae00, 0x67aefe),
           (0x67d250, 0x67d340), (0x67c250, 0x67c288), (0x67d7f0, 0x67d82a), (0x67df40, 0x67e010), (0x67a480, 0x67a519), (0x688730, 0x68876d), (0x688770, 0x6887d1),
           (0x6974e0, 0x6981f0), (0x6ac940, 0x6ac9a0), (0x6d7710, 0x6d782e), (0x67a560, 0x67a592), (0x691530, 0x69153e), (0x67bb90, 0x67bd82), (0x67faf0, 0x67fbf6), (0x67fcb0, 0x67fd63),
           (0x6816a0, 0x68173e), (0x6828a0, 0x682a0a), (0x682c00, 0x682e3b), (0x682e40, 0x682fba), (0x688710, 0x688738), (0x688f50, 0x68917d), (0x689180, 0x6891b7), (0x68a5d0, 0x68a710),
           (0x68bc00, 0x68bc6e), (0x68bc70, 0x68bce1), (0x68bce0, 0x68bd85), (0x68bd80, 0x68bda5), (0x68bda0, 0x68bdb7), (0x68bdb0, 0x68bdd5), (0x68bdd0, 0x68be89), (0x68be90, 0x68c067),
           (0x68c060, 0x68c080), (0x68c440, 0x68caa9), (0x68f9d0, 0x68f9fb), (0x68fa00, 0x68fa70), (0x68fa70, 0x68faa6), (0x68faa0, 0x68fad5), (0x68fad0, 0x68fbc9), (0x68ff60, 0x690220),
           (0x690220, 0x6902b2), (0x6918f0, 0x691a1a), (0x691ba0, 0x691cca)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True),
                 0x691de0: lock, 0x691ea0: lambda e: None, 0x67ad90: _free, 0x67a7d0: _cdecl_log, 0x67d3a0: _recorder(THROW, 2, com=True), 0x695910: _recorder(NUMPUNCT, 1)})
OUT, BUFFER, NEXT, LEFT, VTABLE, IOS, AREA, A_UNDER, A_UFLOW = RAM, RAM + 0x20, RAM + 0x80, RAM + 0x90, RAM + 0xa0, RAM + 0xc0, RAM + 0x100, RAM + 0x200, RAM + 0x300
FACET, FACET_VTABLE, IMP, IMP_VTABLE, FACETS, PTD, STATE, VALUE, FIRST, LAST, TEXT, LOCALE, PCTYPE, LCONV = (RAM + 0xa00 + 0x40 * i for i in range(14))
GROUP_BLOCK, FALSE_BLOCK, TRUE_BLOCK = HEAP + 0x800, HEAP + 0x840, HEAP + 0x880
GLOBALS = 0xfb1da8
GETS = {'bool': 0x695ca0, 'ushort': 0x696450, 'uint': 0x6968a0, 'long': 0x696ce0, 'ulong': 0x6970e0, 'pointer': 0x6981f0, 'float': 0x6974e0, 'double': 0x697930, 'ldouble': 0x697d90}
FLOATS = [b'', b'0', b'1.5', b'-0.25e3', b'+12.5E-2', b'1e', b'1e+', b'.5', b'.', b'-.', b'0.000123', b'000.000', b'1e400', b'1e-400', b'-1e+38', b'3.5e38', b'1e-45', b'0e5', b'1,5', b'1.2.3', b'e5', b'1E05',
          b'12345678901234567890123456789012345678901234.5e1', b'0.00000000000000000000000000000000000000001', b'123e123456789', b'00012.500', b'1.e3', b'7x', b'\xff', b'99999999999999999999999999999999999999', b'0.7', b'.7', b'0.9876543210123', b'.07e1']
OPS = [n for n in list(GETS) * 2 + ['getifld'] * 2 if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
GROUPS = [b'', b'', b'\x03', b'\x03', b'\x03\x02', b'\x01', b'\x7f', b'\x03\x7f', b'\xff', b'\x03\xff', b'\x02\x00\x03', b'\x02\x01\x7f']
NUMBERS = [b'', b'0', b'5', b'12', b'-12', b'+12', b'0x1f', b'0X1F', b'0x', b'077', b'08', b'1,234', b'12,345', b'1,234,567', b'12,34,567', b',1', b'1,', b'1,,2', b'1,2,', b'1234,567', b'00,012',
           b'123456789012345678901234567890123456789', b'000000000000000000000000000000000000012', b'abc', b'fFg', b'70000', b'65535', b'65536', b'-1', b'-65536', b'4294967295', b'4294967296', b'2147483648', b'-2147483649', b'-', b'+', b'-0', b'1 2', b'\xff1', b'1\xff', b'127777777777777777,7']
NAMES = [(b'false', b'true'), (b'false', b'true'), (b'no', b'yes'), (b'f', b'flse'), (b'ab', b'abc'), (b'abc', b'ab'), (b'', b''), (b'', b'x'), (b'same', b'same'), (b'a\xffb', b'a\x7fb')]
WORDS = [b'', b'true', b'false', b'tr', b'fals', b't', b'f', b'x', b'truex', b'falsey', b'no', b'yes', b'y', b'n', b'abc', b'ab', b'a', b'same', b'a\xffb', b'0', b'1', b'2']
FLAGS = [0, 0, 0x200, 0x200, 0x400, 0x800, 0xc00, 0xe00, 0x4000, 0x4200, 0x4800, 0xffffffff, 0xffffbfff]


def pctype():
    table = bytearray(0x400)
    for c in range(0x100):
        v = 0
        if 0x30 <= c <= 0x39:
            v = 4 | 0x80
        elif 0x61 <= c <= 0x7a:
            v = 2 | (0x80 if c <= 0x66 else 0)
        elif 0x41 <= c <= 0x5a:
            v = 1 | (0x80 if c <= 0x46 else 0)
        elif c in (0x20, 9, 0xa, 0xd):
            v = 8
        struct.pack_into('<H', table, c * 2, v)
    return table


CTYPE = bytes(pctype())


def name_string(r, regions, log, k, at, text, which):
    held = regions[HEAP]
    held[at - HEAP - 1] = r.choice([0, 0, 1, 2, 0xfe, 0xff])
    held[at - HEAP:at - HEAP + len(text) + 1] = text + b'\0'
    strings(log, k, 0, out=1)
    fill(log, k, 0, struct.pack('<4I', 0, at if text or r.random() < 0.5 else 0, len(text), max(len(text), 0x1f)), which=which)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    log_fold(log)
    regions = {LOG: log, HEAP: heap(0x1000), PTD: bytes(8) + struct.pack('<I', r.choice([0, 0, 0, 0x22])) + bytes(0x10), OUT: r.bytes(8)}
    set_return(log, GETPTD, PTD)
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[PCTYPE] = CTYPE
    floats = name in ('float', 'double', 'ldouble')
    regions[0x77e7dc] = struct.pack('<I', 1 if floats else r.choice([1, 1, 1, 1, 2]))   # (the float scan's isdigit takes a byte past 0x7f to GetStringTypeA when MB_CUR_MAX is 2: not imported here)
    regions[0xfb1440] = bytes(4)
    regions[0x709b70] = b'0123456789abcdefABCDEF\0\0'
    regions[0x6fe5ac] = struct.pack('<I', 0xffffffff) + r.choice([b'\0', b'\0', b'\0', b'\x02', b'\x7f']) + bytes(0xf)
    regions[0x75c734] = b'missing locale facet\0\0\0\0'
    # The stream: its text waiting in the buffer, or nothing there and every character from underflow.
    text = r.choice(WORDS if name == 'bool' and r.random() < 0.7 else FLOATS if floats and r.random() < 0.8 else NUMBERS)
    if floats and r.random() < 0.15:   # (0x24 digits are kept: the rest only count as a power of ten, which _Stod multiplies in)
        text = bytes(r.choice(b'0123456789') for _ in range(r.randrange(34, 42))) + r.choice([b'', b'.5', b'e-3'])
    buffered = r.random() < 0.8
    if not buffered:
        text = text[:8]   # (two logged calls a character, and the answers stop moving when the log's 40 entries are full: the input must end before that, the other calls' entries too)
    regions[AREA] = text + b'.' + r.bytes(0x60 - len(text) - 1)
    regions[NEXT] = struct.pack('<I', AREA if buffered else r.choice([0, AREA]))
    regions[LEFT] = struct.pack('<I', len(text) if buffered else 0)
    eof = 0xffffffff
    regions[A_UNDER] = struct.pack('<40I', *((list(text) if not buffered else [r.choice([eof, eof, 0x31, 0x2c]) for _ in range(6)]) + [eof] * 40)[:40])
    regions[A_UFLOW] = struct.pack('<40I', *((list(text) if not buffered else [r.choice([eof, 0x31]) for _ in range(6)]) + [eof] * 40)[:40])
    answers(log, UNDERFLOW, A_UNDER)
    answers(log, UFLOW, A_UFLOW)
    regions[VTABLE] = struct.pack('<6I', 0, 0, 0, 0, recorder(UNDERFLOW, 0), recorder(UFLOW, 0))
    regions[BUFFER] = struct.pack('<I', VTABLE) + bytes(0x18) + struct.pack('<I', NEXT) + bytes(0xc) + struct.pack('<I', LEFT) + bytes(0x10)
    # The locale and its facet.
    groups = r.choice(GROUPS if text.count(b',') != 2 else [b'\x7f', b'\x03\x7f', b'\x02\x01\x7f'])   # (three groups take _Getifld's check two at a time past 0 and round 2^32 times, in the original too, unless the grouping ends at 0x7f)
    held = regions[HEAP]
    held[GROUP_BLOCK - HEAP] = r.choice([0, 0, 0, 1, 2, 0xfe, 0xff])
    held[GROUP_BLOCK - HEAP + 1:GROUP_BLOCK - HEAP + 1 + len(groups)] = groups
    held[GROUP_BLOCK - HEAP + 1 + len(groups)] = 0
    fields = (GROUP_BLOCK + 1, len(groups), max(len(groups), r.choice([1, 0x1f]))) if r.random() < 0.92 else (0, 0, 0)
    strings(log, GROUPING, 0, out=1)
    fill(log, GROUPING, 0, struct.pack('<4I', 0, *fields), which=1)
    false_name, true_name = r.choice(NAMES)
    name_string(r, regions, log, FALSE_NAME, FALSE_BLOCK, false_name, 2)
    name_string(r, regions, log, TRUE_NAME, TRUE_BLOCK, true_name, 3)
    regions[FACET_VTABLE] = struct.pack('<6I', recorder(DELETE, 1), recorder(POINT, 0), recorder(SEPARATOR, 0), recorder(GROUPING, 1), recorder(FALSE_NAME, 1), recorder(TRUE_NAME, 1))
    set_return(log, POINT, r.getrandbits(24) << 8 | r.choice([0x2e, 0x2e, 0x2c, 0x65, 0]))
    regions[LCONV] = struct.pack('<I', LCONV + 4) + r.choice([b'.', b',', b'\0']) + bytes(3)
    regions[0x781fc8] = struct.pack('<I', LCONV)
    regions[FACET] = struct.pack('<2I', FACET_VTABLE, r.choice([0, 1, 0xffffffff])) + bytes(0x10)
    set_return(log, SEPARATOR, r.getrandbits(16) << 16 | r.choice([0x2c, 0x2c, 0x2c, 0, 0x2e, 0x31, 0x12c]))
    regions[IMP_VTABLE] = struct.pack('<I', recorder(DELETE, 1))
    regions[FACETS] = struct.pack('<16I', *[r.choice([FACET, FACET, FACET, 0]) for _ in range(16)])
    regions[IMP] = struct.pack('<5I', IMP_VTABLE, r.choice([1, 1, 2, 0, 0xffffffff]), FACETS, r.choice([0, 1, 3, 8, 16, 16]), 0) + bytes([r.choice([0, 0, 1])]) + bytes(0xb)
    ids = bytearray(0x90)
    identifier = r.choice([0, r.randrange(1, 16), r.randrange(1, 16), 0x40])
    struct.pack_into('<I', ids, 0, identifier)
    struct.pack_into('<I', ids, 0x10, r.choice([0, FACET, FACET]) if name in ('bool', 'getifld') else FACET)   # (use_facet's making of the classic facet is its own bench's)
    struct.pack_into('<I', ids, 0x14, r.getrandbits(32))
    struct.pack_into('<3I', ids, 0x24, r.choice([IMP, IMP, IMP, IMP, 0x1234]), IMP, r.randrange(0, 15))
    regions[GLOBALS] = ids
    set_return(log, NUMPUNCT, FACET)
    blob(log, THROW, 0, 0x7)
    flags = r.choice(FLAGS)
    regions[IOS] = bytes(0xc) + struct.pack('<I', flags) + bytes(0x10) + struct.pack('<I', IMP) + bytes(0x1c)
    first = [BUFFER if r.random() < 0.95 else 0, r.getrandbits(32) & 0xffffff00 | r.choice([0, 0, 0, 0, 0, 1])]
    last = [0 if r.random() < 0.93 or (floats and not first[0]) else BUFFER, r.getrandbits(32) & 0xffffff00 | r.choice([0, 1])]   # (first at its end, last on a streambuf: a stale '0' or digit loops for ever, in the original too)
    if name == 'getifld':
        regions[TEXT] = r.bytes(0x40)
        regions[FIRST] = struct.pack('<2I', *first)
        regions[LAST] = struct.pack('<2I', *last)
        regions[LOCALE] = struct.pack('<I', IMP)
        return Call(0x698640, [TEXT, FIRST, LAST, flags, LOCALE], regions, conv='c', ret='a')
    regions[STATE] = struct.pack('<I', r.choice([0, 0, 0, 1, 2, 4, r.getrandbits(32)]))
    regions[VALUE] = r.bytes(8)
    return Call(GETS[name], [r.getrandbits(32), OUT] + first + last + [IOS, STATE, VALUE], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
