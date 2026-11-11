# The C++ library's money_put<wchar_t> (src/game/core/moneyput): do_put of a number and of a string, _Putmfld, and moneypunct<wchar_t>'s constructors on "C", against the originals, over moneyput.py's world
# with wide strings: the facet's symbol and signs, the value, the text. btowc's literals come from _Mbrtowc, a cdecl recorder answering by the call; ctype<wchar_t>'s is() is a thiscall recorder answering
# in turn. Overflow and is() take a wint_t (unsigned short): their unit is logged as its low word (the original pushes it in registers whose high half is stale: a call's answer, the iterator's address).
# Bench limits: ctype<wchar_t>'s classic cache is always set (making one runs its constructor's env, other recorders natively); GetVersion is the import recorder 40 (23 is _Mbrtowc's).
# usage: wmoneyput.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import moneyput as M  # noqa: E402
from localefacet import OTHER_FACETS, state  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, blob, fill, heap, import_table, log_region, recorder, set_return, strings  # noqa: E402

MBRTOWC, IS, VERSION = 23, 21, 40
WIDE = [(0x6a1bf0, 0x6a1ed8), (0x6a2150, 0x6a3388), (0x69b7f0, 0x69b889), (0x693dc0, 0x693e15), (0x47b750, 0x47bcd6), (0x405280, 0x405380), (0x479150, 0x47919d), (0x478cf0, 0x478d1d),
        (0x479230, 0x479256), (0x4f23f0, 0x4f2437), (0x6a5dc0, 0x6a5e40), (0x6a5ee0, 0x6a5f60), (0x4f6080, 0x4f6086)]


def merged(ranges):
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


RANGES = merged(M.RANGES + WIDE)
HOOKS = {**M.HOOKS, 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True)}
B = Bench(RANGES, hooks=HOOKS)
WCTYPE, WCTYPE_VTABLE, A_IS, WTEXT = RAM + 0x7000, RAM + 0x7020, RAM + 0x7040, RAM + 0x7100
OPS = [n for n in ('intl', 'local', 'putmfld', 'putmfld', 'putmfld', 'putmfld', 'putmfld', 'putmfld', 'number', 'number', 'number', 'number', 'string', 'string', 'string', 'string')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
ODD = [0x20ac, 0xff04, 0x3000, 0xffff, 0x100]
FINITE = [v for v in M.VALUES if v >> 52 & 0x7ff != 0x7ff]


def units(r, text):
    """A narrow text's bytes as units, now and then one past a byte."""
    return [c if r.random() < 0.9 else c | r.choice([0x100, 0xff00]) for c in text]


def wide_block(r, regions, at, text):
    """A wide string's block in the heap at TEXT_BLOCKS + at: two bytes, the owners' byte the second, then the units and their end. The string fields, and the next offset."""
    held = regions[HEAP]
    start = M.TEXT_BLOCKS + at + 2
    held[start - HEAP - 1] = r.choice([0, 0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    data = struct.pack('<%dH' % (len(text) + 1), *text, 0)
    held[start - HEAP:start - HEAP + len(data)] = data
    return (start, len(text), max(len(text), r.choice([1, 0x1f, 0x40]))), at + len(data) + 0x10


def wide_out(r, regions, log, slot, which, at):
    """Slot `slot` writes a wide string: moneyput.py's narrow choice as units. Returns the next offset."""
    fields, at = wide_block(r, regions, at, units(r, M.CHOSEN[slot]))
    if r.random() < 0.08:
        fields = (0, 0, 0)
    fill(log, slot, 0, struct.pack('<4I', 0, *fields), which=which)
    return at


def widened(log, r, literals):
    """btowc's answers in the order of its calls: each literal itself mostly. _Mbrtowc's count answer: mostly 1."""
    got = [ord(c) if r.random() < 0.9 else r.choice(ODD + [0x30]) for c in literals][:28]
    fill(log, MBRTOWC, 0, struct.pack('<%dI' % len(got), *got), records=len(got))
    strings(log, MBRTOWC, 0, out=9)
    blob(log, MBRTOWC, 1, 1)
    set_return(log, MBRTOWC, r.choice([1, 1, 1, 1, 1, 0, 0xffffffff]))


def world(r):
    """moneyput.py's world with the wide facet, its strings, ids and caches, ctype<wchar_t> (is() a recorder). The log, the regions and the next block offset."""
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x20000),   # (wide blocks are twice the size; an infinity's 5000 zeros, grouped, still run out of it: rare, a crash in both)
               IMPORT_TABLE: import_table(log, {'lcid': (M.LCID, 0x6fd260, 0, 0, 0), 'version': (VERSION, 0x6fd264, 1, 0, 1), 'validcp': (M.VALIDCP, 0x6fd288, 1, 0, 0), 'validlocale': (M.VALIDLOCALE, 0x6fd284, 2, 0, 0),
                                                'enum': (M.ENUM, 0x6fd1f8, 2, 0, 0), 'info': (M.INFO, 0x6fd28c, 4, 0, 0b100), 'inc': (M.INC, 0x6fd0c0, 1, 0, 0), 'dec': (M.DEC, 0x6fd12c, 1, 0, 0)})}
    state(r, regions, log)
    struct.pack_into('<I', regions[HEAP], 0, M.FIRST_BLOCK + r.choice([0] * 40 + [1, 2, 3, 5]))
    at = M.facet_state(r, regions, log)
    M.lconv(r, regions)
    blob(log, M.THROW, 0, 0x7)
    regions[M.OUT] = r.bytes(0x20)
    regions[M.MADE] = r.bytes(0x2c)
    # The wide facet's answers and strings, the final vtables of the facets made here, a put area of units, overflow's unit a word.
    set_return(log, M.DECIMAL, r.getrandbits(16) << 16 | r.choice([0x2e, 0x2e, 0x2c, 0x66b, 0xff0e]))
    set_return(log, M.SEPARATOR, r.getrandbits(16) << 16 | r.choice([0x2c, 0x2c, 0x2e, 0x20, 0x66c, 0xff0c]))
    for slot, which in ((M.SYMBOL, 2), (M.PLUS, 3), (M.MINUS, 4)):
        at = wide_out(r, regions, log, slot, which, at)
    regions[0x709da4] = regions[M.FACET_VTABLE]
    regions[0x709bbc] = regions[M.FACET_VTABLE]
    regions[M.COUNT] = struct.pack('<I', r.choice([0, 0, 1, 2, 5, 30, 0x400, 0xc00]))
    struct.pack_into('<I', log, LOG_BYTES + 4 * M.OVERFLOW, 1 << 16)
    regions[0x6fd6c4] = bytes(4)   # (the wide empty string, and npos)
    regions[0x6fef00] = struct.pack('<I', r.choice([0xffffffff] * 10 + [0x7000]))
    # The ids of moneypunct<wchar_t, true> 0xfb1de8, ctype<wchar_t> 0xfb1df8, moneypunct<wchar_t, false> 0xfb1e0c; their classic caches 0xfb1e2c, 0x7ced3c, 0xfb1e28; the holders 0xfb1e34, 0xfb1e38.
    money_ids = r.sample(range(1, 0x18), 2)
    ctype_id = r.choice([i for i in range(1, 0x18) if i not in money_ids])
    ids = bytearray(0x60)
    struct.pack_into('<I', ids, 0x08, r.choice([0, money_ids[0], money_ids[0], money_ids[0]]))   # (a new id finds no facet: then "missing locale facet", whose throw the recorder returns from)
    struct.pack_into('<I', ids, 0x18, ctype_id)
    struct.pack_into('<I', ids, 0x2c, r.choice([0, money_ids[1], money_ids[1], money_ids[1]]))
    struct.pack_into('<4I', ids, 0x48, r.choice([0, M.OBJECT]), r.choice([0, M.OBJECT]), r.getrandbits(32), r.getrandbits(32))
    struct.pack_into('<I', ids, 0x58, r.getrandbits(32))
    regions[0xfb1de0] = ids
    regions[0x7ced3c] = struct.pack('<4I', WCTYPE, 0, r.getrandbits(32), 0)
    facets = bytearray(regions[OTHER_FACETS])
    struct.pack_into('<I', facets, 4 * ctype_id, r.choice([WCTYPE] * 5 + [0]))
    regions[OTHER_FACETS] = bytes(facets)
    source = bytearray(regions[M.SOURCE])   # (the locale's facet count: mostly all of them, a facet not found being a throw the recorder returns from)
    struct.pack_into('<I', source, 0xc, r.choice([0x20] * 6 + [0, 3, 8]))
    regions[M.SOURCE] = bytes(source)
    regions[WCTYPE_VTABLE] = struct.pack('<3I', recorder(M.DELETE, 1), 0, recorder(IS, 2))
    regions[WCTYPE] = struct.pack('<2I', WCTYPE_VTABLE, r.choice([0, 1, 0xffffffff])) + bytes(0x10)
    struct.pack_into('<I', log, LOG_BYTES + 4 * IS, 1 << 17)
    return log, regions, at


def case(r):
    name = r.deal(OPS)
    log, regions, at = world(r)
    c = r.choice([0, 0x30, 0x41, 0xff, 0x20ac, 0x1ff, r.getrandbits(32)])
    if name in ('intl', 'local'):
        widened(log, r, '.,' + 'x' * 26)
        return Call(0x6a1bf0 if name == 'intl' else 0x6a2150, [M.MADE, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
    common = [M.OUT, r.getrandbits(32), r.choice([M.BUFFER, M.BUFFER, M.BUFFER, M.BUFFER, 0]), r.choice([0, 1, 1, 0x100, 0x101]), M.IOS, c]
    if name == 'putmfld':
        widened(log, r, '00')
        fields, at = wide_block(r, regions, at, units(r, r.choice(M.NUMBERS)))
        if r.random() < 0.05:
            fields = (0, 0, 0)
        return Call(0x6a2940, [r.getrandbits(32)] + common + [r.choice([0, 1, 0x100, 0xff]), r.getrandbits(32)] + list(fields), regions, conv='t', ret='a')
    if name == 'number':
        regions[0x77eca0] = struct.pack('<8I', recorder(M.CFLT, 5, cdecl=True), 0, 0, 0, 0, 0, 0, 0)
        blob(log, M.CFLT, 0, 0b11)
        strings(log, M.CFLT, 0, 0b10)
        text = r.choice(M.CONVERTED)
        fill(log, M.CFLT, 1, text + bytes(0x28 - len(text)), which=7)
        widened(log, r, text.decode('latin-1') + '000')
        value = r.choice(FINITE if r.random() < 0.93 else M.VALUES) if r.random() < 0.85 else r.getrandbits(64)   # (an infinity or NaN: 5000 zeros, slow to emulate)
        return Call(0x6a2450, [r.getrandbits(32)] + common + [value & 0xffffffff, value >> 32], regions, conv='t', ret='a')
    text = units(r, r.choice(M.TEXTS))
    widened(log, r, '-000')
    fields, at = wide_block(r, regions, at, text)
    if r.random() < 0.05:
        fields = (0, 0, 0)
    regions[WTEXT] = struct.pack('<4I', r.getrandbits(32), *fields)
    # is(digit, unit) in turn: the digits as they are, now and then the other answer; AL alone is the answer.
    rest = text[1:] if text[:1] == [0x2d] else text
    said = [(1 if 0x30 <= u <= 0x39 else 0) ^ (r.random() < 0.06) for u in rest] + [0]
    answers(log, IS, A_IS)
    regions[A_IS] = struct.pack('<40I', *[(s | r.choice([0, 0, 0x100, 0xffffff00])) for s in (said + [0] * 40)[:40]])
    return Call(0x6a2690, [r.getrandbits(32)] + common + [WTEXT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
