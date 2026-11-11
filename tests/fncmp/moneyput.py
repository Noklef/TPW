# The C++ library's money_put<char>, moneypunct's constructors and small methods (src/game/core/moneyput), money_get<char>'s _Getmfld and do_get (src/game/core/moneyget) against the originals, over the strings' ports, the locale's, sprintf and _Stod: the locks do nothing, _CxxThrowException a COM recorder, atexit the shared globals' log, _cfltcvt a cdecl recorder, ctype's constructor and the facets' slots thiscall recorders.
# Bench limit: after a bad_cast the original goes on with a null facet and faults, as the port does: the throw's arguments are not compared there. _Xlen and _Xran end a case, so they are not reached (npos stays large).
# usage: moneyput.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from localefacet import OTHER_FACETS, PCTYPE, state  # noqa: E402
from numget import CTYPE as REAL_CTYPE  # noqa: E402
from locimp import SOURCE  # noqa: E402
from locinfo import NHMALLOC, REALLOC, LOCK, UNLOCK  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _cdecl_log, _free, _recorder, answers, blob, fill, heap, import_table, log_region, recorder, set_return, strings  # noqa: E402

DELETE, THROW, CTYPE = 0, 14, 18
DECIMAL, SEPARATOR, GROUPING, SYMBOL, PLUS, MINUS, DIGITS, PLUS_PATTERN, MINUS_PATTERN, OVERFLOW, CFLT = 10, 11, 12, 13, 15, 16, 17, 19, 29, 30, 31
LCID, VERSION, VALIDCP, VALIDLOCALE, ENUM, INFO, INC, DEC = 22, 23, 24, 25, 26, 27, 32, 3
GETPTD = 2   # (the driver's CrtInnerEnv getptd is the cdecl recorder 2: InterlockedIncrement moved to the COM slot 32)


def lock(e):
    e.r['EAX'] = e.r['ECX']


RANGES = [(0x694ac0, 0x694b09), (0x694b10, 0x694b71), (0x69c660, 0x69c67b), (0x6a1880, 0x6a189b), (0x6a87f0, 0x6a880b), (0x6a8810, 0x6a882b), (0x6a8cf0, 0x6a8ef2), (0x6a8f00, 0x6a8f68), (0x6a8f70, 0x6a9172), (0x6a9260, 0x6a9475), (0x6a9480, 0x6a96f4), (0x6a9700, 0x6aa127),
           (0x4ef7b0, 0x4ef7f0), (0x4ef7f0, 0x4ef82a), (0x699ce0, 0x699d47), (0x699d50, 0x699d9b), (0x4f21a0, 0x4f21cf), (0x4f1890, 0x4f18a2), (0x4f2180, 0x4f2199), (0x691530, 0x691536), (0x5d1460, 0x5d14e0), (0x6ac150, 0x6ac270), (0x6ac270, 0x6ac390),
           (0x47b280, 0x47b2f6), (0x67bb90, 0x67bd98), (0x67d250, 0x67d340), (0x45b340, 0x45b9a0), (0x4f1400, 0x4f1511),
           (0x67a650, 0x67a6b8), (0x680c00, 0x681480), (0x681590, 0x6815d2), (0x6815e0, 0x681619), (0x681620, 0x68165d), (0x681660, 0x681671), (0x681680, 0x681692), (0x683aa0, 0x683ab2), (0x68c350, 0x68c3b8), (0x68c3c0, 0x68c435),
           (0x698fc0, 0x6990fa), (0x699100, 0x6991e2), (0x683ac0, 0x683d88), (0x683d90, 0x683ee5), (0x683ef0, 0x68400b), (0x684010, 0x684202), (0x684210, 0x684254), (0x684260, 0x684367), (0x684370, 0x6843da), (0x68db70, 0x68dbae),
           (0x67c250, 0x67c288), (0x68dbb0, 0x68dd41), (0x68dd50, 0x68ddad), (0x68ddb0, 0x68de3a), (0x68e120, 0x68e17e), (0x68e260, 0x68e29e), (0x68e340, 0x68e35b), (0x68e360, 0x68e443), (0x68e510, 0x68e549), (0x68e6c0, 0x68e6e1),
           (0x67d7f0, 0x67d82a), (0x67dcd0, 0x67dcf0), (0x67df40, 0x67e010), (0x67a480, 0x67a519), (0x688730, 0x68876d), (0x688770, 0x6887d1), (0x67ae00, 0x67aefe),
           (0x6a7920, 0x6a86fc), (0x696330, 0x6963eb), (0x6963f0, 0x69644b), (0x698bf0, 0x698c3c), (0x696160, 0x696226), (0x6a8700, 0x6a87e9), (0x47b1c0, 0x47b280),
           (0x6a7550, 0x6a7762), (0x6a7760, 0x6a7927), (0x5d0050, 0x5d0079), (0x67acb0, 0x67acc6), (0x67acc0, 0x67acd8), (0x686a00, 0x686a11), (0x6ac940, 0x6ac9a0), (0x6d7710, 0x6d782e),
           (0x67fcb0, 0x67fd63), (0x680430, 0x6804cd), (0x6816a0, 0x68173e), (0x6828a0, 0x682a0a), (0x682c00, 0x682e3b), (0x682e40, 0x682fba), (0x688710, 0x688738), (0x68a5d0, 0x68a710),
           (0x68bc00, 0x68bc6e), (0x68bc70, 0x68bce1), (0x68bce0, 0x68bd85), (0x68bd80, 0x68bda5), (0x68bda0, 0x68bdb7), (0x68bdb0, 0x68bdd5), (0x68bdd0, 0x68be89), (0x68be90, 0x68c067),
           (0x68c060, 0x68c080), (0x68c440, 0x68caa9), (0x68f9d0, 0x68f9fb), (0x68fa00, 0x68fa70), (0x68fa70, 0x68faa6), (0x68faa0, 0x68fad5), (0x68fad0, 0x68fbc9), (0x68ff60, 0x690220),
           (0x690220, 0x6902b2), (0x6918f0, 0x691a1a), (0x691ba0, 0x691cca)]
HOOKS = {0x682460: _recorder(GETPTD, 0, cdecl=True),0x67ad90: _free, 0x691de0: lock, 0x691ea0: lambda e: None, 0x67d3a0: _recorder(THROW, 2, com=True), 0x67a7d0: _cdecl_log, 0x5cfb60: _recorder(CTYPE, 3),
         0x681740: _recorder(REALLOC, 2, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True)}
B = Bench(RANGES, hooks=HOOKS)
BASE = RAM + 0x3000
MONEY_LCONV, MONEY_TEXTS, OBJECT, FACET_VTABLE, IOS, BUFFER, POINTER, COUNT, BUFFER_VTABLE, ITERATOR, IMP_VTABLE, A_OVERFLOW, TABLE, OUT, TEXT, MADE = (
    BASE + i for i in (0, 0x100, 0x300, 0x400, 0x440, 0x480, 0x4c0, 0x4d0, 0x4e0, 0x4f0, 0x500, 0x600, 0x800, 0xa00, 0xa20, 0xa40))
VALUE, STATE, PTD = BASE + 0xa80, BASE + 0xa90, BASE + 0xaa0
AREA = RAM + 0x4000
TEXT_BLOCKS, FIRST_BLOCK = HEAP + 0x8000, HEAP + 0x9000
OPS = [n for n in ('sputc', 'iteratorput', 'grouping', 'plus', 'symbol', 'minus', 'pattern', 'pattern', 'intl', 'local', 'putmfld', 'putmfld', 'putmfld', 'putmfld', 'putmfld', 'putmfld', 'number', 'number', 'number', 'number', 'string', 'string', 'string', 'string',
                   'getmfld', 'getmfld', 'getmfld', 'getmfld', 'getmfld', 'getmfld', 'getld', 'getld', 'getld', 'getld', 'getstr', 'getstr', 'getstr', 'getstr')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
UNDERFLOW, UFLOW = 4, 5   # (the input stream's slots)
INPUT = RAM + 0x6000
SOURCE_VTABLE, NEXT, LEFT, ANSWER_UNDER, ANSWER_U, FIRST, LAST, IN_BUFFER, IN_TEXT = (INPUT + i for i in (0, 0x20, 0x30, 0x40, 0x100, 0x1c0, 0x1d0, 0x1e0, 0x240))
CHOSEN = {}   # (what facet_state dealt: the input is built from it)
GROUPS = [b'', b'\x03', b'\x03\x02', b'\x01', b'\x7f', b'\x03\x7f', b'\xff', b'\x03\xff', b'\x02\x00\x03', b'\x03\x03\x7f', b'\x03\x03\x7f']
SIGNS = [b'', b'-', b'+', b'()', b'-x', b'CR ', b'a longer sign text']
SYMBOLS = [b'', b'$', b'USD ', b'\xa3', b'a currency symbol longer than the small size']
NUMBERS = [b'', b'0', b'5', b'12', b'123', b'1234', b'1234567', b'12345678901234567890', b'x' * 0x28]
PATTERNS = [b'$+ v', b'+$ v', b'+ $v', b'v $+', b'$+vx', b' $+v', b'+v$ ', b'vvvv', b'    ', b'++++', b'$$$$', b'\x80$+v', b'z v+']
TEXTS = [b'', b'-', b'0', b'-0', b'12', b'-1234', b'0012x', b'x12', b'--5', b'1234567890123456789012345678901234567890']
CONVERTED = [b'', b'0', b'12', b'-5', b'1234567', b'1.#INF', b'-1.#QNAN', b'12345678901234567890123456789012345678', b'123456789012345678901234567890123456789']
VALUES = [0, 0x8000000000000000, 0x3ff8000000000000, 0xbff8000000000000, 0x4733426172c74d81, 0x4733426172c74d82, 0x4733426172c74d83, 0xc733426172c74d82, 0x54b249ad2594c37d, 0xfe3749cc8fb1d5e3, 0x7fefffffffffffff,
          0x7ff0000000000000, 0xfff0000000000000, 0x7ff8000000000000, 0xfff8000000000001, 0x7ff0000000000001]


def block(r, regions, at, text):
    """A string's block in the heap at TEXT_BLOCKS + at: its owners' byte, text and end. The string fields, and the next free offset."""
    held = regions[HEAP]
    start = TEXT_BLOCKS + at + 1
    held[start - HEAP - 1] = r.choice([0, 0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    held[start - HEAP:start - HEAP + len(text) + 1] = text + b'\0'
    return (start, len(text), max(len(text), r.choice([1, 0x1f, 0x40]))), at + len(text) + 0x10


def out_string(r, regions, log, slot, which, at, choices):
    """Slot `slot` writes a string, whose block starts at offset `at`, through its out argument. Returns the next offset."""
    text = r.choice(choices)
    CHOSEN[slot] = text
    fields, at = block(r, regions, at, text)
    if r.random() < 0.08:
        fields = (0, 0, 0)
    strings(log, slot, 0, out=1)
    fill(log, slot, 0, struct.pack('<4I', 0, *fields), which=which)
    return at


def facet_state(r, regions, log):
    """The facet's slots, its answers and the streams' state."""
    slots = struct.pack('<10I', recorder(DELETE, 1), recorder(DECIMAL, 0), recorder(SEPARATOR, 0), recorder(GROUPING, 1), recorder(SYMBOL, 1), recorder(PLUS, 1), recorder(MINUS, 1), recorder(DIGITS, 0), recorder(PLUS_PATTERN, 1), recorder(MINUS_PATTERN, 1))
    regions[FACET_VTABLE] = slots
    regions[0x709f2c] = slots
    regions[0x709f5c] = slots
    regions[OBJECT] = struct.pack('<5I', FACET_VTABLE, r.choice([0, 1, 0xffffffff]), 0, 0, TABLE) + bytes(0x14) + bytes([r.choice([0, 1])]) + bytes(0x17)
    table = bytearray(0x200)
    for c in range(0x100):
        table[c * 2] = (4 if 0x30 <= c <= 0x39 else 0) | r.choice([0, 1, 2, 8, 0x80])
        table[c * 2 + 1] = r.choice([0, 4, 0xff])
    regions[TABLE] = table
    CHOSEN[DIGITS] = r.choice([0, 0, 0, 1, 2, 3, 5, 0x30])
    CHOSEN[DECIMAL] = r.choice(b'.,;')
    CHOSEN[SEPARATOR] = r.choice(b",. '")
    set_return(log, DIGITS, CHOSEN[DIGITS])
    set_return(log, DECIMAL, r.getrandbits(24) << 8 | CHOSEN[DECIMAL])
    set_return(log, SEPARATOR, r.getrandbits(24) << 8 | CHOSEN[SEPARATOR])
    set_return(log, CTYPE, OBJECT)
    at = 0
    at = out_string(r, regions, log, GROUPING, 1, at, GROUPS)
    at = out_string(r, regions, log, SYMBOL, 2, at, SYMBOLS)
    at = out_string(r, regions, log, PLUS, 3, at, SIGNS)
    at = out_string(r, regions, log, MINUS, 4, at, SIGNS)
    strings(log, PLUS_PATTERN, 0, out=1)
    strings(log, MINUS_PATTERN, 0, out=1)
    CHOSEN[MINUS_PATTERN] = r.choice(PATTERNS)
    fill(log, PLUS_PATTERN, 0, r.choice(PATTERNS), which=5)
    fill(log, MINUS_PATTERN, 0, CHOSEN[MINUS_PATTERN], which=6)
    # The stream's buffer.
    room = r.choice([0, 0, 1, 2, 5, 30, 0x400, 0x1800])
    place = r.choice([AREA, AREA, AREA, 0])
    regions[POINTER] = struct.pack('<I', place)
    regions[COUNT] = struct.pack('<I', room)
    regions[AREA] = r.bytes(0x20) + bytes(0x1800)
    regions[BUFFER_VTABLE] = struct.pack('<2I', 0, recorder(OVERFLOW, 1))
    regions[BUFFER] = struct.pack('<I', BUFFER_VTABLE) + bytes(0x1c) + struct.pack('<I', POINTER) + bytes(0xc) + struct.pack('<I', COUNT)
    answers(log, OVERFLOW, A_OVERFLOW)
    regions[A_OVERFLOW] = struct.pack('<40I', *[r.choice([0, 0x41, 0x41, 0xffffffff, 0xffff, 0xffff0041]) for _ in range(40)])
    regions[IOS] = bytes(0xc) + struct.pack('<3I', r.choice([0, 8, 0x40, 0x48, 0x100, 0x108, 0x140, 0x1c0, 0x88, 0xfffffe3f, 0xffffffff]), r.getrandbits(32), r.choice([0, 0xffffffff, 1, 5, 12, 30, 100, 0xffffffec])) + bytes(8) + struct.pack('<I', SOURCE) + bytes(0x1c)
    # The stream's locale.
    regions[IMP_VTABLE] = struct.pack('<I', recorder(DELETE, 1))
    regions[OTHER_FACETS] = struct.pack('<%dI' % 0x20, *[r.choice([OBJECT, OBJECT, OBJECT, 0]) for _ in range(0x20)])
    regions[SOURCE] = struct.pack('<5I', IMP_VTABLE, r.choice([1, 1, 2, 0, 0xffffffff]), OTHER_FACETS, r.choice([0, 1, 3, 8, 0x20, 0x20, 0x20]), 0) + bytes([r.choice([0, 0, 1])]) + bytes(3) + bytes(0x10)
    ids = bytearray(0x40)
    struct.pack_into('<I', ids, 0, r.choice([0, r.randrange(1, 0x18)]))        # (0xfb1e80)
    struct.pack_into('<I', ids, 0x10, r.choice([0, r.randrange(1, 0x18)]))     # (0xfb1e90)
    struct.pack_into('<4I', ids, 0x2c, r.choice([0, OBJECT]), r.choice([0, OBJECT]), r.getrandbits(32), r.getrandbits(32))    # (0xfb1eac, 0xfb1eb0 the caches; 0xfb1eb4, 0xfb1eb8)
    regions[0xfb1e80] = ids
    regions[0xf827dc] = struct.pack('<4I', r.getrandbits(32), r.choice([0, OBJECT, OBJECT]), r.getrandbits(32), r.getrandbits(32))
    regions[0x6fe5ac] = struct.pack('<I', r.choice([0xffffffff] * 10 + [0x7000])) + bytes(0x10)
    regions[0x709dcc] = b'%.0Lf\0\0\0'
    regions[0x709d10] = bytes(r.randrange(0x20, 0x7f) for _ in range(0x60))
    return at


def lconv(r, regions):
    pointers, texts = [], bytearray()
    for _ in range(10):
        pointers.append(MONEY_TEXTS + len(texts))
        texts += r.choice([b'', b'$', b'.', b',', b'\x03\x03', b'USD ', b'-', b'+']) + b'\0'
    regions[MONEY_TEXTS] = bytes(texts) + bytes(8)
    data = struct.pack('<10I', *pointers) + bytes(r.choice([0, 0, 0, 1, 1, 2, 0x7f, 0x7e, 0x80, 0xff, 0x10]) for _ in range(2))
    data += bytes(r.choice([0, 0, 1, 1, 1, 2, 0x81, 0xfe, 0xff]) for _ in range(4)) + bytes(r.choice([0, 1, 2, 3, 4, 4, 5, 0x7f, 0x80, 0xff]) for _ in range(2))
    regions[MONEY_LCONV] = data
    regions[0x781fc8] = struct.pack('<I', MONEY_LCONV)


def digit_groups(r):
    """Digits with separators: by the grouping dealt, or off it."""
    sizes = [c if c < 0x80 else 0 for c in CHOSEN[GROUPING]] or [3]
    sep = bytes([CHOSEN[SEPARATOR]])
    if r.random() < 0.4:   # (every group as the grouping wants it but the leftmost: the original's test reaches that one only past a 0x7f)
        groups = [b'5' * (sizes[min(i, len(sizes) - 1)] or sizes[0] or 3) for i in range(r.choice([1, 1, 2, 2, 3]))] + [b'7' * r.choice([1, 2, 3, 4])]
        return sep.join(reversed(groups))
    groups = [b'7' * r.choice([1, 2, 3, 4, sizes[0] or 1, 0x81])]
    for i in range(r.choice([0, 0, 1, 2, 3])):
        size = sizes[min(i, len(sizes) - 1)]
        groups.append(b'5' * (size if r.random() < 0.85 else r.choice([0, 1, 2, 4])))
    groups.reverse()
    return sep.join(groups) if r.random() < 0.9 else b''.join(groups)


def money_text(r):
    """A field as the pattern wants it, sometimes spoiled."""
    plus, minus, symbol = CHOSEN[PLUS], CHOSEN[MINUS], CHOSEN[SYMBOL]
    sign = r.choice([plus, minus, minus, b''])
    out = b''
    for c in CHOSEN[MINUS_PATTERN]:
        if c == 0x24:
            out += r.choice([symbol, symbol, symbol, symbol[:1], b''])
        elif c == 0x2b:
            out += sign[:1]
        elif c == 0x76:
            out += digit_groups(r)
            if r.random() < 0.7:
                out += bytes([CHOSEN[DECIMAL]]) + b'9' * r.choice([0, 1, 2, 3, 6])
        else:
            out += b' ' * r.choice([0, 1, 1, 3])
    out += sign[1:]
    if out and r.random() < 0.2:
        out = bytearray(out)
        out[r.randrange(len(out))] = r.getrandbits(8)
    if r.random() < 0.1:
        out = out[:r.randrange(len(out) + 1)]
    return bytes(out) + r.choice([b'', b'', b'tail', b'  '])


def stream(r, regions, log):
    """The iterators: most of the text in the buffer, or all by the slots' answers; the end, or the same streambuf."""
    text = money_text(r)
    held = r.choice([len(text), len(text), len(text), r.randrange(len(text) + 1), 0])
    place = r.choice([IN_TEXT, IN_TEXT, IN_TEXT, 0])
    regions[IN_TEXT] = text + bytes(8)
    regions[NEXT] = struct.pack('<I', place)
    regions[LEFT] = struct.pack('<I', held)
    regions[SOURCE_VTABLE] = struct.pack('<6I', 0, 0, 0, 0, recorder(UNDERFLOW, 0), recorder(UFLOW, 0))
    regions[IN_BUFFER] = struct.pack('<I', SOURCE_VTABLE) + bytes(0x18) + struct.pack('<I', NEXT) + bytes(0xc) + struct.pack('<I', LEFT) + bytes(0x10)
    rest = list(text[held:] if place else text)
    for k, table in ((UNDERFLOW, ANSWER_UNDER), (UFLOW, ANSWER_U)):
        given = (rest[:2] + [0xffffffff] * 40)[:40]   # (the answers stop moving when the log's 40 are full, with the locale's calls in it: an end soon, or a digit for ever)
        if r.random() < 0.3:
            given = [g if r.random() < 0.85 else r.choice([0xffffffff, 0xffff, r.getrandbits(32)]) for g in given[:2]] + [0xffffffff] * 38
        answers(log, k, table)
        regions[table] = struct.pack('<40I', *given)
    # (first at its end with last still on a streambuf loops for ever on a digit or space, in the original too: "not equal" and nothing moves. So last has a streambuf only with both already fetched, and then nothing is read.)
    both = r.random() < 0.06
    first, last = r.choice([IN_BUFFER] * 12 + [0]), 0
    regions[FIRST] = struct.pack('<IBB', IN_BUFFER if both else first, 1 if both else r.choice([0, 0, 0, 1]), r.getrandbits(8)) + bytes(10)
    regions[LAST] = struct.pack('<IBB', r.choice([IN_BUFFER, IN_TEXT]) if both else last, 1 if both else r.choice([1, 1, 0]), r.getrandbits(8)) + bytes(10)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x10000),
               IMPORT_TABLE: import_table(log, {'lcid': (LCID, 0x6fd260, 0, 0, 0), 'version': (VERSION, 0x6fd264, 1, 0, 1), 'validcp': (VALIDCP, 0x6fd288, 1, 0, 0), 'validlocale': (VALIDLOCALE, 0x6fd284, 2, 0, 0), 'enum': (ENUM, 0x6fd1f8, 2, 0, 0), 'info': (INFO, 0x6fd28c, 4, 0, 0b100),
                                                'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0)})}
    state(r, regions, log)
    struct.pack_into('<I', regions[HEAP], 0, FIRST_BLOCK + r.choice([0] * 40 + [1, 2, 3, 5]))   # (a value's 5000 zeros stay off the blocks the locale state keeps low in the heap)
    at = facet_state(r, regions, log)
    lconv(r, regions)
    blob(log, THROW, 0, 0x7)   # (vtable, text, owned)
    regions[OUT] = r.bytes(0x20)
    regions[MADE] = r.bytes(0x2c)
    c = r.choice([0, 0x30, 0x41, 0xff, 0x1ff, r.getrandbits(32)])
    if name == 'sputc':
        return Call(0x694ac0, [BUFFER, c], regions, conv='t', ret='a')
    if name == 'iteratorput':
        regions[ITERATOR] = struct.pack('<2I', r.getrandbits(32), r.choice([BUFFER, BUFFER, BUFFER, 0]))
        return Call(0x694b10, [ITERATOR, c], regions, conv='t', ret='a')
    if name in ('grouping', 'plus', 'symbol', 'minus'):
        return Call({'grouping': 0x69c660, 'plus': 0x6a1880, 'symbol': 0x6a87f0, 'minus': 0x6a8810}[name], [OBJECT, OUT], regions, conv='t', ret='a')
    if name == 'pattern':
        return Call(0x6a8f00, [OBJECT, OUT, r.choice([0, 1, 1, 2, 0x81, 0x101, 0x100]), r.choice([0, 1, 1, 2, 0x81, 0x101, 0x100]), r.choice([0, 1, 2, 3, 4, 4, 5, 0x7f, 0x80, 0xff, 0x103])], regions, conv='t', ret='a')
    if name in ('intl', 'local'):
        return Call(0x6a8cf0 if name == 'intl' else 0x6a8f70, [MADE, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'getmfld':
        stream(r, regions, log)
        # (out's first byte is the allocator byte of a string on the original's stack, which nothing set)
        return Call(0x6a7920, [r.getrandbits(32), OUT, FIRST, LAST, r.choice([0, 1, 1, 0x100, 0x101]), IOS], regions, conv='t', ret='a', ignore=[(OUT, 1)])
    if name in ('getld', 'getstr'):
        stream(r, regions, log)
        regions[PCTYPE] = REAL_CTYPE   # (_Stod's strtod over a real ctype table, the C locale's MB_CUR_MAX and errno)
        regions[0x77e7dc] = struct.pack('<I', 1)
        regions[PTD] = bytes(8) + struct.pack('<I', r.choice([0, 0, 0, 0x22])) + bytes(0x10)
        set_return(log, GETPTD, PTD)
        regions[STATE] = struct.pack('<I', r.choice([0, 0, 0, 1, 2, 4, r.getrandbits(32)]))
        if name == 'getld':
            regions[VALUE] = r.bytes(8)
        else:
            fields, at = block(r, regions, at, r.choice(TEXTS))
            if r.random() < 0.05:
                fields = (0, 0, 0)
            regions[VALUE] = struct.pack('<4I', r.getrandbits(32), *fields)
        first, last = struct.unpack_from('<2I', regions[FIRST]), struct.unpack_from('<2I', regions[LAST])
        return Call(0x6a7550 if name == 'getld' else 0x6a7760, [r.getrandbits(32), OUT, *first, *last, r.choice([0, 1, 1, 0x100, 0x101]), IOS, STATE, VALUE], regions, conv='t', ret='a')
    common = [OUT, r.getrandbits(32), r.choice([BUFFER, BUFFER, BUFFER, BUFFER, 0]), r.choice([0, 1, 1, 0x100, 0x101]), IOS, r.getrandbits(24) << 8 | r.choice(b' *0_')]
    if name == 'putmfld':
        fields, at = block(r, regions, at, r.choice(NUMBERS))
        if r.random() < 0.05:
            fields = (0, 0, 0)
        return Call(0x6a9700, [r.getrandbits(32)] + common + [r.choice([0, 1, 0x100, 0xff]), r.getrandbits(32)] + list(fields), regions, conv='t', ret='a')
    if name == 'number':
        regions[0x77eca0] = struct.pack('<8I', recorder(CFLT, 5, cdecl=True), 0, 0, 0, 0, 0, 0, 0)
        blob(log, CFLT, 0, 0b11)
        strings(log, CFLT, 0, 0b10)
        text = r.choice(CONVERTED)
        fill(log, CFLT, 1, text + bytes(0x28 - len(text)), which=7)
        value = r.choice(VALUES) if r.random() < 0.85 else r.getrandbits(64)
        return Call(0x6a9260, [r.getrandbits(32)] + common + [value & 0xffffffff, value >> 32], regions, conv='t', ret='a')
    fields, at = block(r, regions, at, r.choice(TEXTS))
    if r.random() < 0.05:
        fields = (0, 0, 0)
    regions[TEXT] = struct.pack('<4I', r.getrandbits(32), *fields)
    return Call(0x6a9480, [r.getrandbits(32)] + common + [TEXT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
