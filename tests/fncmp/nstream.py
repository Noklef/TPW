# The narrow streams' front ends (src/game/core/nstream: 0x5cfb60, 0x5d0080) against the originals, in wstream.py's world (its locale, stream and buffer; _Locinfo's constructor and destructor, _Getctype, the
# throw recorders; free and atexit logged): the facet's table +0x10 flags 0x48 at about a third of its units, the kept facet at 0xf827e0, the table kept at 0xfb1dd8 there or not.
# usage: nstream.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ctor,ipfx)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, LOG, _cdecl_log, _free, _recorder, answers, log_region, put, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import wstream as w  # noqa: E402

B = Bench([(0x5cfb60, 0x5cfc74), (0x5d0050, 0x5d0080), (0x5d0080, 0x5d032b), (0x699ce0, 0x699d47), (0x699d50, 0x699d9b), (0x4ef7f0, 0x4ef82a), (0x4ef7b0, 0x4ef7e8), (0x4f21a0, 0x4f21cf), (0x4f0110, 0x4f0134),
           (0x4f2180, 0x4f2199), (0x5d1460, 0x5d14e0), (0x693340, 0x6934c0), (0x67d250, 0x67d340), (0x67acd0, 0x67acee)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67d3a0: _recorder(w.THROW, 2, com=True), 0x698fc0: _recorder(w.LOCINFO, 1), 0x699100: _recorder(w.UNINFO, 0),
                 0x699bd0: _recorder(w.GETCTYPE, 1, cdecl=True), 0x67a7d0: _cdecl_log, 0x67ad90: _free})
OPS = [n for n in ('ctor', 'ipfx') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
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
    if name == 'ctor':
        regions[OUT] = r.bytes(0x20)
        return Call(0x5cfb60, [OUT, r.choice([0, 0, TABLE_B, r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    w.stream_regions(r, regions)
    w.wide_tables(r, regions, log)
    regions[w.GETN] = struct.pack('<I', r.choice([0, 1, 3, 16]))
    regions[w.BUF_VT] = vtable({w.OVERFLOW: 1, w.UNDERFLOW: 0, w.UFLOW: 0, w.XSPUTN: 2, w.SYNC: 0})
    return Call(0x5d0080, [w.STREAM, r.choice([0, 0, 0, 1, 0x100])], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
