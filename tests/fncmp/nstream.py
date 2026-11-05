# The narrow streams' front ends (src/game/core/nstream: 0x5cfb60, 0x5d0080, 0x5cfd50, 0x5cfd30, 0x5cfc80, 0x5cf900, 0x5cfd90, 0x5d1190, 0x5d1120, 0x5cf9c0, 0x5cfe40, 0x5d3530, 0x5d0e70) against the originals, in wstream.py's world (its locale, stream and buffer; _Locinfo's constructor and destructor, _Getctype, the
# throw recorders; free and atexit logged): the facet's table +0x10 flags 0x48 at about a third of its units, the kept facet at 0xf827e0, the table kept at 0xfb1dd8 there or not.
# usage: nstream.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ctor,ipfx)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, LOG, _cdecl_log, _free, _recorder, answers, log_region, put, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import wstream as w  # noqa: E402

IOS_INIT, IOS_ADDSTD = 20, 21   # (ios_base's own, each with a bench of its own)
XSGETN = 6   # (the buffer's slot)
B = Bench([(0x5cfd30, 0x5cfd4e), (0x5cf9c0, 0x5cfb5b), (0x4f5d10, 0x4f5d40), (0x45b340, 0x45b9a0), (0x4f1890, 0x4f18a2),(0x4f7770, 0x4f77f0), (0x4f1eb0, 0x4f1ecf), (0x5cfd50, 0x5cfd74), (0x5cfc80, 0x5cfc8f), (0x5cf900, 0x5cf9b4), (0x5cfd90, 0x5cfe3f), (0x5d1190, 0x5d1217), (0x5d1120, 0x5d118b), (0x6a65e0, 0x6a65e3), (0x5cfb60, 0x5cfc74),(0x5d0050, 0x5d0080), (0x5d0080, 0x5d032b), (0x699ce0, 0x699d47), (0x699d50, 0x699d9b), (0x4ef7f0, 0x4ef82a), (0x4ef7b0, 0x4ef7e8), (0x4f21a0, 0x4f21cf), (0x4f0110, 0x4f0134),
           (0x4f2180, 0x4f2199), (0x5d1460, 0x5d14e0), (0x693340, 0x6934c0), (0x67d250, 0x67d340), (0x67acd0, 0x67acee), (0x5cfe40, 0x5d0049), (0x5d3530, 0x5d35ff), (0x5d0e70, 0x5d1120)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67d3a0: _recorder(w.THROW, 2, com=True), 0x698fc0: _recorder(w.LOCINFO, 1), 0x699100: _recorder(w.UNINFO, 0),
                 0x699bd0: _recorder(w.GETCTYPE, 1, cdecl=True), 0x67a7d0: _cdecl_log, 0x67ad90: _free, 0x693960: _recorder(IOS_INIT, 0), 0x693910: _recorder(IOS_ADDSTD, 0)})
OPS = [n for n in ('ctor', 'ipfx', 'ctype_dtor', 'ctype_delete', 'table_free', 'ostream_ctor', 'istream_ctor', 'opfx', 'sentry', 'widen', 'getline', 'read', 'puttext') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
ID, KEPT, KEPT_TABLE = 0xfb1da4, 0xf827dc, 0xfb1dd8
OUT, TABLE, TABLE_B = RAM + 0x7000, RAM + 0x7100, RAM + 0x7400


def flags(r):
    return struct.pack('<256H', *[r.choice([0x48, 8, 0x40, 0, 1, 0x20, 0xffb7, r.getrandbits(16)]) for _ in range(256)])


def world(r):
    log = log_region()
    regions = {LOG: log}
    w.locale_regions(r, regions, log)
    del regions[w.KEPT]
    del regions[0x709cac]
    regions[w.LOCALE] = struct.pack('<I', w.IMP)
    regions[ID] = struct.pack('<I', r.choice([0, 0, 1, 2, 3]))
    regions[KEPT] = struct.pack('<3I', r.getrandbits(32), r.choice([0, 0, w.FACET]), r.getrandbits(32))
    regions[KEPT_TABLE] = struct.pack('<I', r.choice([0, TABLE_B]))
    regions[TABLE], regions[TABLE_B] = flags(r), flags(r)
    regions[w.FACET] = put(bytearray(r.bytes(0x18)), 0, '2I', w.FACET_VT, 1)
    put(regions[w.FACET], 0x10, 'I', TABLE)
    regions[w.CTYPE] = struct.pack('<4I', r.getrandbits(32), r.getrandbits(32), TABLE, r.choice([0, 0, 1, r.getrandbits(32)]))
    return regions, log


def case(r):
    name = r.deal(OPS)
    regions, log = world(r)
    if name == 'ctype_dtor':
        regions[OUT] = put(bytearray(r.bytes(0x20)), 0x10, '2I', TABLE, r.choice([0, 0, 1, r.getrandbits(32)]))
        return Call(0x5cfd50, [OUT], regions, conv='t', ret='v')
    if name == 'ctype_delete':
        regions[OUT] = put(bytearray(r.bytes(0x20)), 0x10, '2I', TABLE, r.choice([0, 0, 1, r.getrandbits(32)]))
        return Call(0x5cfd30, [OUT, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'widen':
        ios = bytearray(r.bytes(0x40))
        put(ios, 0x20, 'I', w.IMP)
        regions[OUT] = ios
        return Call(0x5cf9c0, [OUT, r.getrandbits(32)], regions, conv='t', ret='b')
    if name == 'table_free':
        return Call(0x5cfc80, [], regions, ret='v')
    if name in ('ostream_ctor', 'istream_ctor'):   # (a vtable table of the game's, the ios at its base; the buffer null or not)
        vbtable = 0x702980 if name == 'ostream_ctor' else 0x702988
        self_ = bytearray(r.bytes(0x60))
        put(self_, 0, 'I', vbtable)
        ios = 4 if name == 'ostream_ctor' else 8   # (the state any, the exception mask 0: a throw is ios_base's own, in its bench)
        put(self_, ios + 4, 'I', r.choice([0, 0, 1, 2, 4]))
        put(self_, ios + 0x14, 'I', 0)
        regions[OUT] = self_
        args = [OUT, r.choice([0, w.BUF, w.BUF]), r.choice([0, 0, 1, 0x100])]
        if name == 'ostream_ctor':
            args.append(r.choice([0, 1, 1, 0x100]))
        args.append(r.choice([0, 1, r.getrandbits(32)]))
        return Call(0x5cf900 if name == 'ostream_ctor' else 0x5cfd90, args, regions, conv='t', ret='a')
    if name == 'ctor':
        regions[OUT] = r.bytes(0x20)
        return Call(0x5cfb60, [OUT, r.choice([0, 0, TABLE_B, r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    w.stream_regions(r, regions)
    w.wide_tables(r, regions, log)
    regions[w.GETN] = struct.pack('<I', r.choice([0, 1, 3, 16]))
    regions[w.BUF_VT] = vtable({w.OVERFLOW: 1, w.UNDERFLOW: 0, w.UFLOW: 0, XSGETN: 2, w.XSPUTN: 2, w.SYNC: 0})
    if name == 'getline':
        delimiter = r.randrange(256)
        units = [r.choice([delimiter, 0xff, 0x41, r.randrange(256), r.randrange(256), r.randrange(256)]) for _ in range(16)]
        if r.random() < 0.3:
            units[0] = delimiter
        regions[w.GET] = struct.pack('<I', r.choice([0, w.GET + 8, w.GET + 8, w.GET + 8])) + b'\0' * 4 + bytes(units)
        regions[w.TABLE] = struct.pack('<40I', *[r.choice([0xffffffff, delimiter, 0x41, r.randrange(256), r.getrandbits(32)]) for _ in range(40)])
        regions[w.OUT] = r.bytes(0x40)
        return Call(0x5cfe40, [w.STREAM, w.OUT, r.choice([0, 1, 2, 4, 16, 0xffffffff]), r.choice([delimiter, delimiter | 0x100, delimiter | 0xffffff00, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'read':
        count = r.choice([0, 1, 2, 2, 8, 0x80000000])
        regions[w.TABLE3] = struct.pack('<40I', *[r.choice([count, count, count, 0, count + 1, count - 1 & 0xffffffff]) for _ in range(40)])
        answers(log, XSGETN, w.TABLE3)
        regions[w.OUT] = r.bytes(0x10)
        return Call(0x5d3530, [w.STREAM, w.OUT, count], regions, conv='t', ret='a')
    if name == 'puttext':
        length = r.choice([0, 1, 3, 8])
        regions[w.TEXT] = bytes(r.randrange(1, 256) for _ in range(length)) + b'\0'
        regions[w.TABLE3] = struct.pack('<40I', *[r.choice([length, length, length, 0, length + 1]) for _ in range(40)])
        answers(log, w.XSPUTN, w.TABLE3)
        regions[w.PUTN] = struct.pack('<I', r.choice([0, 1, 3, 8, 0x80000000, 0xffffffff]))   # (the last: the position plus the room wraps)
        return Call(0x5d0e70, [w.STREAM, w.TEXT], regions, conv='c', ret='a')
    if name == 'opfx':
        return Call(0x5d1190, [w.STREAM], regions, conv='t', ret='b')
    if name == 'sentry':
        regions[OUT] = struct.pack('<II', r.getrandbits(32), w.STREAM)
        return Call(0x5d1120, [OUT], regions, conv='t', ret='v')
    return Call(0x5d0080, [w.STREAM, r.choice([0, 0, 0, 1, 0x100])], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
