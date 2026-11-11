# The C++ library's money_get<wchar_t> (src/game/core/moneyget): _Getmfld, do_get of a long double and of a string, against the originals, over wmoneyput.py's world (the wide facet, its strings, btowc)
# with moneyput.py's input fields as units: ctype<wchar_t>'s is() wmoneyput.py's recorder, answering in turn by the field's units; the digits to _Stod narrowed by wctob over _Wcrtomb in the C locale.
# Bench limit: is() answers by turn, not by its mask and unit (the driver cannot run the exe's 0x4f18d0 from a vtable).
# usage: wmoneyget.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import moneyput as M  # noqa: E402
import wmoneyput as W  # noqa: E402
from localefacet import OTHER_FACETS, PCTYPE  # noqa: E402
from numget import CTYPE as REAL_CTYPE  # noqa: E402
from fncmp import Bench, Call, answers, recorder, set_return  # noqa: E402

B = Bench(W.merged(W.RANGES + [(0x6a0510, 0x6a072a), (0x6a0730, 0x6a08f4), (0x6a0900, 0x6a174a), (0x6a1750, 0x6a1880), (0x69cf70, 0x69d03b), (0x69d040, 0x69d098), (0x69d0a0, 0x69d15f),
                               (0x69d160, 0x69d1b9), (0x69fea0, 0x69feef), (0x69f870, 0x69f8a2), (0x4f1eb0, 0x4f1ecf), (0x69a5d0, 0x69a5fb), (0x69a460, 0x69a4d3),
                               (0x69a4e0, 0x69a55e), (0x67acd0, 0x67acee)]), hooks=W.HOOKS)
OPS = [n for n in ('getmfld', 'getmfld', 'getmfld', 'getmfld', 'getmfld', 'getmfld', 'getld', 'getld', 'getld', 'getld', 'getstr', 'getstr', 'getstr', 'getstr')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def stream(r, regions, log):
    """moneyput.py's stream() over units: the field, or its start, in the buffer; underflow and uflow answer the end (0xffff in the low word).
    (Only the end: the log fills with is() and btowc calls, its answers then stop moving, and a unit answered there would come forever.)"""
    text = W.units(r, M.money_text(r))
    held = r.choice([len(text), len(text), len(text), r.randrange(len(text) + 1), 0])
    regions[M.IN_TEXT] = struct.pack('<%dH' % len(text), *text) + bytes(8)
    regions[M.NEXT] = struct.pack('<I', M.IN_TEXT)
    regions[M.LEFT] = struct.pack('<I', held)
    regions[M.SOURCE_VTABLE] = struct.pack('<6I', 0, 0, 0, 0, recorder(M.UNDERFLOW, 0), recorder(M.UFLOW, 0))
    regions[M.IN_BUFFER] = struct.pack('<I', M.SOURCE_VTABLE) + bytes(0x18) + struct.pack('<I', M.NEXT) + bytes(0xc) + struct.pack('<I', M.LEFT) + bytes(0x10)
    end = r.choice([0xffff, 0xffff, 0xffffffff])
    for k, table in ((M.UNDERFLOW, M.ANSWER_UNDER), (M.UFLOW, M.ANSWER_U)):
        answers(log, k, table)
        regions[table] = struct.pack('<40I', *[end] * 40)
    # (last always the end: on a streambuf, as moneyput.py now and then has it, it stays unequal once first is at the end, and is() stopped on true loops forever)
    first = r.choice([M.IN_BUFFER] * 12 + [0])
    regions[M.FIRST] = struct.pack('<IBBH', first, r.choice([0, 0, 0, 1]), r.getrandbits(8), r.getrandbits(16)) + bytes(8)
    regions[M.LAST] = struct.pack('<IBBH', 0, r.choice([1, 1, 0]), r.getrandbits(8), r.getrandbits(16)) + bytes(8)
    return text


def case(r):
    name = r.deal(OPS)
    log, regions, at = W.world(r)
    # (the locale's facet count all of them and ctype<wchar_t> there: a facet not found, a throw the recorder returns from, wmoneyput.py benches; here the first use of the null facet crashes in both)
    source = bytearray(regions[M.SOURCE])
    struct.pack_into('<I', source, 0xc, 0x20)
    regions[M.SOURCE] = bytes(source)
    facets = bytearray(regions[OTHER_FACETS])
    struct.pack_into('<I', facets, 4 * struct.unpack_from('<I', regions[0xfb1de0], 0x18)[0], W.WCTYPE)
    regions[OTHER_FACETS] = bytes(facets)
    # The facet's point and separator mostly those the field was made with; ctype<wchar_t> the exe's is() over a table.
    set_return(log, M.DECIMAL, r.getrandbits(16) << 16 | (M.CHOSEN[M.DECIMAL] if r.random() < 0.8 else r.choice([0x66b, 0xff0e, 0x12e])))
    set_return(log, M.SEPARATOR, r.getrandbits(16) << 16 | (M.CHOSEN[M.SEPARATOR] if r.random() < 0.8 else r.choice([0x66c, 0xff0c, 0x12c])))
    W.widened(log, r, '0000000-')
    text = stream(r, regions, log)
    # is(mask, unit) in turn (wmoneyput.py's recorder): about one call a unit, so the field's digits and spaces in order, now and then the other answer; AL alone is the answer.
    said = [(1 if 0x30 <= u <= 0x39 or u == 0x20 else 0) ^ (r.random() < 0.06) for u in text] + [0]
    answers(log, W.IS, W.A_IS)
    regions[W.A_IS] = struct.pack('<40I', *[(s | r.choice([0, 0, 0x100, 0xffffff00])) for s in (said + [0] * 40)[:40]])
    intl = r.choice([0, 1, 1, 0x100, 0x101])
    if name == 'getmfld':
        # (out's first byte is the allocator byte of a string on the original's stack, which nothing set)
        return Call(0x6a0900, [r.getrandbits(32), M.OUT, M.FIRST, M.LAST, intl, M.IOS], regions, conv='t', ret='a', ignore=[(M.OUT, 1)])
    regions[M.STATE] = struct.pack('<I', r.choice([0, 0, 0, 1, 2, 4, r.getrandbits(32)]))
    if name == 'getld':
        regions[PCTYPE] = REAL_CTYPE   # (_Stod's strtod over a real ctype table, the C locale's MB_CUR_MAX, errno; wctob's _Wcrtomb in the C locale)
        regions[0x77e7dc] = struct.pack('<I', 1)
        regions[M.PTD] = bytes(8) + struct.pack('<I', r.choice([0, 0, 0, 0x22])) + bytes(0x10)
        set_return(log, M.GETPTD, M.PTD)
        regions[0xfb1438] = bytes(0x1c)
        regions[0xfb3ca4] = bytes(8)   # (_setlc_active, the readers: none, or a facet made here waits in setlocale on Sleep)
        regions[M.VALUE] = r.bytes(8)
    else:
        fields, at = W.wide_block(r, regions, at, W.units(r, r.choice(M.TEXTS)))
        if r.random() < 0.05:
            fields = (0, 0, 0)
        regions[M.VALUE] = struct.pack('<4I', r.getrandbits(32), *fields)
    first, last = struct.unpack_from('<2I', regions[M.FIRST]), struct.unpack_from('<2I', regions[M.LAST])
    return Call(0x6a0510 if name == 'getld' else 0x6a0730, [r.getrandbits(32), M.OUT, *first, *last, intl, M.IOS, M.STATE, M.VALUE], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
