# The C++ library's num_put (src/game/core/numput) against the originals: do_put of a pointer, a long, an unsigned long, a double and a long double, _Fput and _Iput, of char and of wchar_t, over the C library's sprintf and the locale's ports: _Mbrtowc a cdecl recorder,
# the buffer's overflow and the facet's slots thiscall recorders, _cfltcvt and its neighbours in sprintf's table cdecl recorders, numpunct's constructors (0x695910, 0x69c680: A's) thiscall recorders, _CxxThrowException a COM recorder, the locks do nothing.
# Bench limit: after a bad_cast the original goes on with a null facet and faults, as the port does. A float's text comes from the _cfltcvt recorder and fits the frame: %f of a large value overruns the original's frame, and a stack overrun cannot be compared.
# usage: numput.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _free, _recorder, answers, blob, fill, heap, log_fold, log_region, recorder, set_return, strings  # noqa: E402

LOCK, UNLOCK, GETPTD, FLSBUF, MBRTOWC, OVERFLOW = 7, 8, 2, 22, 23, 12
DELETE, THROW, DECIMAL, SEPARATOR, GROUPING, CFLT, CROP, FORCE, POSITIVE, NUMPUNCT, WNUMPUNCT = 0, 14, 10, 11, 13, 31, 24, 25, 26, 28, 29


def lock(e):
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x694ac0, 0x694b09), (0x694b10, 0x694b71), (0x694b80, 0x694e5f), (0x694e60, 0x695030), (0x695030, 0x695448), (0x695450, 0x695883), (0x695890, 0x69590d),
           (0x69b7f0, 0x69b88f), (0x69b890, 0x69bb6f), (0x69bb70, 0x69bd2c), (0x69bd30, 0x69c18a), (0x69c190, 0x69c5d7), (0x69c5e0, 0x69c65d), (0x69c660, 0x69c67b),
           (0x693dc0, 0x693e20), (0x67a650, 0x67a6b8), (0x680c00, 0x681480), (0x681590, 0x6815d2), (0x6815e0, 0x681619), (0x681620, 0x68165d), (0x681660, 0x681671), (0x681680, 0x681692), (0x683aa0, 0x683ab2), (0x68c350, 0x68c3b8), (0x68c3c0, 0x68c435),
           (0x4ef7b0, 0x4ef7e8), (0x4ef7f0, 0x4ef82a), (0x699ce0, 0x699d47), (0x699d50, 0x699d9b), (0x4f21a0, 0x4f21cf), (0x4f2180, 0x4f2199), (0x691530, 0x691536), (0x6996f0, 0x699770), (0x6a5b80, 0x6a5c00),
           (0x45b340, 0x45b9a0), (0x67f570, 0x67f615), (0x68db70, 0x68dbae), (0x67d250, 0x67d340), (0x67c250, 0x67c288), (0x67d7f0, 0x67d82a), (0x67dcd0, 0x67dcf0), (0x67df40, 0x67e010), (0x67a480, 0x67a519), (0x688730, 0x68876d), (0x688770, 0x6887d1), (0x67ae00, 0x67aefe)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x680ad0: _recorder(FLSBUF, 2, cdecl=True), 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True),
                 0x691de0: lock, 0x691ea0: lambda e: None, 0x67ad90: _free, 0x67a7d0: _cdecl_log, 0x67d3a0: _recorder(THROW, 2, com=True), 0x695910: _recorder(NUMPUNCT, 1), 0x69c680: _recorder(WNUMPUNCT, 1)})
OUT, BUFFER, POINTER, COUNT, VTABLE, IOS, AREA, A_OVERFLOW, A_WIDE = RAM, RAM + 0x20, RAM + 0x80, RAM + 0x90, RAM + 0xa0, RAM + 0xc0, RAM + 0x100, RAM + 0x200, RAM + 0x300
FACET, FACET_VTABLE, IMP, IMP_VTABLE, FACETS, LCONV, TEXT, DECIMAL_TEXT = RAM + 0xa00, RAM + 0xa20, RAM + 0xa40, RAM + 0xa60, RAM + 0xa80, RAM + 0xb00, RAM + 0xb10, RAM + 0xb80
GROUP_BLOCK = HEAP + 0x800
GLOBALS = 0xfb1da8
POINTERS = {'narrow': 0x694e60, 'wide': 0x69bb70}
INTEGERS = {'long': 0x694b80, 'ulong': 0x694c20, 'wlong': 0x69b890, 'wulong': 0x69b930}
FLOATS = {'double': 0x694cc0, 'ldouble': 0x694d90, 'wdouble': 0x69b9d0, 'wldouble': 0x69baa0}
FPUTS = {'fput': 0x695030, 'wfput': 0x69bd30}
IPUTS = {'iput': 0x695450, 'wiput': 0x69c190}
OPS = [n for n in list(POINTERS) + list(INTEGERS) + list(FLOATS) + list(FPUTS) * 2 + list(IPUTS) * 2 if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
GROUPS = [b'', b'', b'\x03', b'\x03', b'\x03\x02', b'\x01', b'\x7f', b'\x03\x7f', b'\xff', b'\x03\xff', b'\x02\x00\x03', b'\x02\x01\x7f']
FLOAT_TEXTS = [b'', b'0', b'1.5', b'-1.5', b'+1.5', b'1e+10', b'-1.25e-10', b'1.', b'.5', b'12345.678', b'1.#INF', b'-1.#QNAN', b'e', b'1.5e', b'-', b'+', b'.', b'1,5', b'1.2.3', b'1e2e3']
INT_TEXTS = [b'', b'0', b'5', b'12', b'123', b'1234', b'1234567', b'-1234567', b'+1234567', b'-', b'+', b'0x', b'0X1234567', b'0x12345', b'0', b'-0', b'12,345', b',', b'12345678901']
FLAGS = [0, 0, 0x20, 8, 0x10, 0x28, 4, 0x24, 0x400, 0x800, 0x804, 0xc00, 0x808, 0x2000, 0x1000, 0x3000, 0x2030, 0x1014, 0x40, 0x100, 0x140, 0x1c0, 0x80, 0x120, 0x8c4, 0x2100, 0xffffffff, 0xfffffe3f]


def number_world(r, regions, log):
    """The stream's buffer is the case's; here its locale, facet, grouping and the C locale's decimal point."""
    regions[HEAP] = heap(0x1000)
    groups = r.choice(GROUPS)
    block = bytearray(0x20)
    block[0] = r.choice([0, 0, 0, 1, 2, 0xfe, 0xff])
    block[1:1 + len(groups)] = groups
    regions[HEAP][GROUP_BLOCK - HEAP:GROUP_BLOCK - HEAP + 0x20] = block
    fields = (GROUP_BLOCK + 1, len(groups), max(len(groups), r.choice([1, 0x1f]))) if r.random() < 0.92 else (0, 0, 0)
    strings(log, GROUPING, 0, out=1)
    fill(log, GROUPING, 0, struct.pack('<4I', 0, *fields), which=1)
    regions[FACET_VTABLE] = struct.pack('<4I', recorder(DELETE, 1), recorder(DECIMAL, 0), recorder(SEPARATOR, 0), recorder(GROUPING, 1))
    regions[FACET] = struct.pack('<2I', FACET_VTABLE, r.choice([0, 1, 0xffffffff])) + bytes(0x10)
    set_return(log, DECIMAL, r.getrandbits(24) << 8 | r.choice(b'.,;\0'))
    set_return(log, SEPARATOR, r.getrandbits(16) << 16 | r.choice([0, 0, 0x2c, 0x2e, 0x27, 0x100, 0xff00, 0x12c, 0xffff]))
    regions[IMP_VTABLE] = struct.pack('<I', recorder(DELETE, 1))
    regions[FACETS] = struct.pack('<16I', *[r.choice([FACET, FACET, FACET, 0]) for _ in range(16)])
    regions[IMP] = struct.pack('<5I', IMP_VTABLE, r.choice([1, 1, 2, 0, 0xffffffff]), FACETS, r.choice([0, 1, 3, 8, 16, 16]), 0) + bytes([r.choice([0, 0, 1])]) + bytes(0xb)
    ids = bytearray(0x90)
    identifier = r.choice([0, r.randrange(1, 16), r.randrange(1, 16), 0x40])
    struct.pack_into('<I', ids, 0, identifier)
    struct.pack_into('<I', ids, 0x34, identifier)
    cached = r.choice([0, FACET, FACET])
    struct.pack_into('<I', ids, 0x10, cached)
    struct.pack_into('<I', ids, 0x74, cached)
    struct.pack_into('<I', ids, 0x14, r.getrandbits(32))
    struct.pack_into('<I', ids, 0x88, r.getrandbits(32))
    struct.pack_into('<3I', ids, 0x24, r.choice([IMP, IMP, IMP, 0x1234]), r.choice([IMP, IMP, 0x1234]), r.randrange(0, 15))
    regions[GLOBALS] = ids
    set_return(log, NUMPUNCT, FACET)
    set_return(log, WNUMPUNCT, FACET)
    blob(log, THROW, 0, 0x7)
    regions[DECIMAL_TEXT] = r.choice([b'.', b'.', b',', b'\0', b'e']) + bytes(3)
    regions[LCONV] = struct.pack('<I', DECIMAL_TEXT)
    regions[0x781fc8] = struct.pack('<I', LCONV)
    regions[0x6fe5ac] = struct.pack('<I', 0xffffffff) + r.choice([b'\0', b'\0', b'\x02', b'\x7f']) + bytes(0xf)
    regions[0x75c734] = b'missing locale facet\0\0\0\0'
    regions[0x709b54] = b'e\0\0\0E\0\0\0,\0\0\0'


def case(r):
    name = r.deal(OPS)
    wide = name[0] == 'w'
    log = log_region()
    log_fold(log)
    regions = {LOG: log}
    regions[OUT] = r.bytes(8)
    regions[0x709b50] = b'%p\0\0'
    regions[0x77ea90] = struct.pack('<I', RAM + 0x800)
    regions[RAM + 0x800] = bytes(0x200)
    room = r.choice([0, 0, 1, 2, 5, 9, 12, 30, 0xffffffff, 0x80000000])
    place = r.choice([AREA, AREA, AREA, 0])
    regions[POINTER] = struct.pack('<I', place)
    regions[COUNT] = struct.pack('<I', room if room < 0x100 else (room if place else 0))
    regions[AREA] = r.bytes(0x60)
    regions[VTABLE] = struct.pack('<2I', 0, recorder(OVERFLOW, 1))
    regions[BUFFER] = struct.pack('<I', VTABLE) + bytes(0x1c) + struct.pack('<I', POINTER) + bytes(0xc) + struct.pack('<I', COUNT)
    answers(log, OVERFLOW, A_OVERFLOW)
    regions[A_OVERFLOW] = struct.pack('<40I', *[r.choice([0, 0x41, 0x41, 0xffffffff, 0xffff, 0xffff0041]) for _ in range(40)])
    width = r.choice([0, 0, -1, 1, 7, 8, 9, 12, 20, -20 & 0xffffffff])
    if wide:
        strings(log, MBRTOWC, 0, out=9)
        blob(log, MBRTOWC, 1, 1)
        fill(log, MBRTOWC, 0, r.bytes(2) + b'\0\0')
        set_return(log, MBRTOWC, r.choice([0, 1, 1, 1, 0xffffffff, 0xfffffffe]))
    common = [OUT, r.getrandbits(32), r.choice([BUFFER, BUFFER, BUFFER, BUFFER, 0])]
    fill_char = r.choice([0x20, 0x2a, 0x30, 0x12a, r.getrandbits(32)])
    if name in POINTERS:
        regions[IOS] = bytes(0xc) + struct.pack('<3I', r.choice([0, 0x40, 0x40, 0x80, 0x100, 0x140, 0x1c0, 0xffffffff, 0xfffffe3f]), r.getrandbits(32), width & 0xffffffff)
        return Call(POINTERS[name], [r.getrandbits(32), OUT, common[1], common[2], IOS, r.getrandbits(32), r.choice([0, 1, 0xdeadbeef, r.getrandbits(32)])], regions, conv='t', ret='a')
    number_world(r, regions, log)
    flags = r.choice(FLAGS)
    precision = r.choice([0, 0, -1 & 0xffffffff, 1, 3, 6, 0x24, 0x25, 0x40, 0x80, -0x80 & 0xffffffff])   # (a precision in the billions is that many zeros put, one call each)
    regions[IOS] = bytes(0xc) + struct.pack('<3I', flags, precision, width & 0xffffffff) + bytes(8) + struct.pack('<I', IMP) + bytes(0x1c)
    if name in INTEGERS:
        value = r.choice([0, 1, 5, 1234, 1234567, 0xffffffff, 0x80000000, 0x7fffffff, r.getrandbits(32)])
        return Call(INTEGERS[name], [r.getrandbits(32)] + common + [IOS, fill_char, value], regions, conv='t', ret='a')
    if name in FLOATS:
        regions[0x77eca0] = struct.pack('<5I', recorder(CFLT, 5, cdecl=True), recorder(CROP, 1, cdecl=True), 0, recorder(FORCE, 1, cdecl=True), recorder(POSITIVE, 1, cdecl=True))
        blob(log, CFLT, 0, 0b11)
        strings(log, CFLT, 0, 0b10)
        strings(log, CROP, 1)
        strings(log, FORCE, 1)
        blob(log, POSITIVE, 0, 0b11)
        set_return(log, POSITIVE, r.choice([0, 1]))
        text = r.choice(FLOAT_TEXTS)
        fill(log, CFLT, 1, text + bytes(0x28 - len(text)), which=7)
        value = r.getrandbits(64)
        return Call(FLOATS[name], [r.getrandbits(32)] + common + [IOS, fill_char, value & 0xffffffff, value >> 32], regions, conv='t', ret='a')
    if name in FPUTS:
        text = r.choice(FLOAT_TEXTS)
        regions[TEXT] = text + b'\0' + r.bytes(0x40 - len(text) - 1)
        count = len(text) if r.random() < 0.9 else r.randrange(0, len(text) + 1)
        if r.random() < 0.1:   # (internal, a sign in the text and a count of 0: the sign stays where it is)
            regions[TEXT] = r.choice([b'+', b'-']) + r.bytes(0x3f)
            regions[IOS] = bytes(0xc) + struct.pack('<3I', 0x100, precision, width & 0xffffffff) + bytes(8) + struct.pack('<I', IMP) + bytes(0x1c)
            count = 0
        return Call(FPUTS[name], common + [IOS, fill_char, TEXT, r.choice([0, 0, 1, 3, 8]), count], regions, conv='c', ret='a')
    text = r.choice(INT_TEXTS)
    regions[TEXT] = text + b'\0' + r.bytes(0x40 - len(text) - 1)
    return Call(IPUTS[name], common + [IOS, fill_char, TEXT, len(text)], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
