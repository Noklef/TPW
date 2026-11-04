# The wide streams' front ends (src/game/core/wstream, stdlocale: 0x699ce0, 0x699d50, 0x4f7620, 0x4f6a30, 0x4f1640, 0x4f1ed0, 0x4f1ca0, 0x4f6be0) against the originals: a stream and its ios at a virtual base offset, the buffer's and
# the facet's slots recorders, a locale whose facet array has the id or not, transparent or classic or neither. _Locinfo's constructor and destructor, _Getctype, opfx and the throw are recorders. The state mask 0: failures thrown are iosbase.py's.
# usage: wstream.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, answers, blob, heap, log_region, mask_this, put, set_return, strings, vtable  # noqa: E402

THROW, LOCINFO, UNINFO, GETCTYPE, PREFIX = 14, 26, 27, 28, 29
OVERFLOW, UNDERFLOW, UFLOW, XSPUTN, SYNC = 1, 4, 5, 7, 11   # (the buffer's slots)
IS, WIDEN = 2, 10   # (the facet's)
B = Bench([(0x699ce0, 0x699d47), (0x699d50, 0x699d9b), (0x4f7620, 0x4f76a5), (0x4f6a30, 0x4f6bd5), (0x4f1640, 0x4f181c), (0x4f1ed0, 0x4f2177), (0x4f1ca0, 0x4f1ead), (0x4f6be0, 0x4f6e6a),
           (0x4ef7f0, 0x4ef82a), (0x4ef7b0, 0x4ef7e8), (0x4f21a0, 0x4f21cf), (0x4f7770, 0x4f77f0), (0x4f0110, 0x4f0134), (0x4f1eb0, 0x4f1ecf), (0x4f2180, 0x4f2199), (0x4f1890, 0x4f18a2), (0x693340, 0x6934c0),
           (0x67d250, 0x67d340), (0x67acd0, 0x67acee), (0x6a65e0, 0x6a65e3)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67d3a0: _recorder(THROW, 2, com=True), 0x698fc0: _recorder(LOCINFO, 1), 0x699100: _recorder(UNINFO, 0),
                 0x699bd0: _recorder(GETCTYPE, 1, cdecl=True), 0x5d1190: _recorder(PREFIX, 0), 0x67a7d0: _cdecl_log})
STREAM, STREAM_VT, BUF, BUF_VT, GET, GETN, PUT, PUTN, TIED, TIED_VT, TIED_BUF, LOCALE, IMP, IMP_VT, ARRAY, GLOBAL, GLOBAL_ARRAY, FACET, FACET_VT, CTYPE, TABLE, TABLE2, TABLE3, TABLE4, TEXT, OUT = (RAM + 0x100 * i for i in range(26))
OPS = [n for n in ('getfacet', 'iscloc', 'ctor', 'usefacet', 'widen', 'ipfx', 'getline', 'puttext') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
ID_GLOBAL, CLASSIC, KEPT = 0xfb1df8, 0xfb1dcc, 0x7ced3c   # (CLASSIC: then the global _Locimp 0xfb1dd0, the id counter 0xfb1dd4; KEPT: then 0x7ced44 two on)


def locale_regions(r, regions, log):
    """The _Locimp, the global one, the classic one, the facet id, the array."""
    imp = bytearray(r.bytes(0x20))
    id_now = r.choice([0, 0, 1, 2, 3])
    count = r.choice([0, 2, 4, 8])
    put(imp, 0, 'I', IMP_VT)
    put(imp, 4, 'I', r.choice([0, 1, 1, 2, 0xffffffff]))
    put(imp, 8, 'I', ARRAY)
    put(imp, 0xc, 'I', count)
    imp[0x14] = r.choice([0, 1, 0x80])
    regions[IMP] = imp
    regions[IMP_VT] = vtable({0: 1})
    array = [FACET if r.random() < 0.5 else 0 for _ in range(8)]
    regions[ARRAY] = struct.pack('<8I', *array)
    other = bytearray(0x20)
    put(other, 8, 'I', GLOBAL_ARRAY)
    put(other, 0xc, 'I', r.choice([0, 2, 8]))
    regions[GLOBAL] = other
    regions[GLOBAL_ARRAY] = struct.pack('<8I', *[FACET if r.random() < 0.5 else 0 for _ in range(8)])
    held = r.choice([IMP, IMP, GLOBAL, GLOBAL, RAM + 0x7f00])
    regions[CLASSIC] = struct.pack('<3I', held, r.choice([GLOBAL, GLOBAL, held]), r.randrange(4))
    regions[ID_GLOBAL] = struct.pack('<I', id_now)
    regions[KEPT] = struct.pack('<3I', r.choice([0, 0, FACET]), r.getrandbits(32), r.getrandbits(32))
    regions[FACET] = struct.pack('<2I', FACET_VT, 1)
    regions[FACET_VT] = vtable({IS: 2, WIDEN: 1})
    regions[0x709cac] = vtable({IS: 2, WIDEN: 1})   # (a facet made here calls recorders too: its real slots are other ports' benches)
    regions[CTYPE] = r.bytes(16)
    set_return(log, GETCTYPE, CTYPE)
    regions[HEAP] = heap(0x400, fail=r.choice([0, 0, 0, 0, 0, 1]))
    mask_this(log, LOCINFO)
    mask_this(log, UNINFO)
    strings(log, LOCINFO, 1)
    strings(log, GETCTYPE, 0, out=1)
    blob(log, THROW, 0, 0b111)   # (the bad_cast: vtable, text, owned)


def stream_regions(r, regions, name='s'):
    """A stream, its ios at a base offset, a buffer with its get and put areas."""
    base = r.choice([8, 8, 0x10, 0x10, 0x20, 0x10010])
    regions[STREAM_VT] = struct.pack('<2I', 0, base)
    stream = bytearray(r.bytes(0x40))
    put(stream, 0, 'I', STREAM_VT)
    ios_at = STREAM + base
    ios = bytearray(0x40)
    put(ios, 4, 'I', r.choice([0, 0, 0, 0, 1, 2, 4]))
    put(ios, 0xc, 'I', r.choice([0, 1, 2, 3, 0x40, 0x41, 0x42, 0x80, 0x100, 0x1c0]))
    put(ios, 0x14, 'I', r.choice([0, 0, 1, 3, 8, 0xffffffff]))
    put(ios, 0x20, 'I', IMP)
    put(ios, 0x28, 'I', r.choice([BUF, BUF, BUF, BUF, BUF, 0]))
    put(ios, 0x2c, 'I', r.choice([0, 0, 0, TIED]))
    put(ios, 0x30, 'H', r.choice([0x20, 0x20, 0xffff, r.randrange(0x10000)]))
    if base < 0x40:
        stream[base:] = ios[:0x40 - base]
        stream += ios[0x40 - base:]
    else:
        regions[ios_at] = ios
    regions[STREAM] = stream
    buf = bytearray(0x40)
    put(buf, 0, 'I', BUF_VT)
    put(buf, 0x1c, 'I', GET)
    put(buf, 0x20, 'I', PUT)
    put(buf, 0x2c, 'I', GETN)
    put(buf, 0x30, 'I', PUTN)
    regions[BUF] = buf
    regions[BUF_VT] = vtable({OVERFLOW: 1, UNDERFLOW: 0, UFLOW: 0, XSPUTN: 2, SYNC: 0})
    delimiter = r.randrange(0x10000)
    units = [r.choice([delimiter, 0xffff, 0x41, r.randrange(0x10000), r.randrange(0x10000), r.randrange(0x10000)]) for _ in range(16)]
    regions[GET] = struct.pack('<I', r.choice([0, GET + 8, GET + 8, GET + 8])) + b'\0' * 4 + struct.pack('<16H', *units)
    regions[GETN] = struct.pack('<I', r.choice([0, 1, 3, 16, 0x80000000]))
    regions[PUT] = struct.pack('<I', r.choice([0, PUT + 8, PUT + 8])) + bytes(0x40)
    regions[PUTN] = struct.pack('<I', r.choice([0, 1, 3, 8, 0x80000000]))
    tied = bytearray(0x60)
    put(tied, 0, 'I', TIED_VT)
    put(tied, 8 + 4, 'I', r.choice([0, 0, 0, 1, 2, 4, 8]))
    put(tied, 8 + 0x28, 'I', TIED_BUF)
    regions[TIED] = tied
    regions[TIED_VT] = struct.pack('<2I', 0, 8)
    regions[TIED_BUF] = struct.pack('<I', BUF_VT)
    return delimiter


def wide_tables(r, regions, log):
    """The buffer's and the facet's answers."""
    regions[TABLE] = struct.pack('<40I', *[r.choice([0xffff, 0xffffffff, 0x41, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]) for _ in range(40)])
    for slot in (UNDERFLOW, UFLOW, OVERFLOW):
        answers(log, slot, TABLE)
    regions[TABLE2] = struct.pack('<40I', *[r.choice([0, 1, 1, 1, 0x100, 0xffffffff]) for _ in range(40)])
    answers(log, IS, TABLE2)
    answers(log, SYNC, TABLE2)
    answers(log, WIDEN, TABLE)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    locale_regions(r, regions, log)
    regions[LOCALE] = struct.pack('<I', IMP)
    if name == 'getfacet':
        return Call(0x699ce0, [LOCALE, r.choice([0, 1, 2, 3, 7, 8, 9]), r.choice([0, 1, 0x100, 0xffffffff])], regions, conv='t', ret='a')
    if name == 'iscloc':
        return Call(0x699d50, [LOCALE], regions, conv='t', ret='b')
    if name == 'ctor':
        regions[OUT] = r.bytes(0x20)
        return Call(0x4f7620, [OUT, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'usefacet':
        return Call(0x4f6a30, [LOCALE, r.getrandbits(32), r.choice([0, 1, 1, 1, 0x100, 0xffffffff])], regions, conv='c', ret='a')
    delimiter = stream_regions(r, regions)
    wide_tables(r, regions, log)
    if name == 'widen':
        ios = bytearray(0x40)
        put(ios, 0x20, 'I', IMP)
        regions[OUT] = ios
        return Call(0x4f1640, [OUT, r.getrandbits(32)], regions, conv='t', ret='a')
    if name == 'ipfx':
        return Call(0x4f1ed0, [STREAM, r.choice([0, 0, 0, 1, 0x100])], regions, conv='t', ret='b')
    if name == 'getline':
        regions[OUT] = r.bytes(0x40)
        return Call(0x4f1ca0, [STREAM, OUT, r.choice([0, 1, 2, 4, 16, 0xffffffff]), r.choice([delimiter, delimiter | 0x10000, r.getrandbits(32)])], regions, conv='t', ret='a')
    length = r.choice([0, 1, 3, 8])
    regions[TEXT] = struct.pack('<%dH' % (length + 1), *([r.randrange(1, 0x10000) for _ in range(length)] + [0]))
    regions[TABLE3] = struct.pack('<40I', *[r.choice([length, length, length, 0, length + 1]) for _ in range(40)])
    answers(log, XSPUTN, TABLE3)
    regions[TABLE4] = struct.pack('<40I', *[r.choice([0, 1, 1, 1, 0x100, 0xffffffff]) for _ in range(40)])
    answers(log, PREFIX, TABLE4)
    return Call(0x4f6be0, [STREAM, TEXT], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
