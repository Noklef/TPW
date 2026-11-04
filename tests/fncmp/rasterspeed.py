# The software rasteriser's speed measure (src/game/render/rasteriser: 0x56a450) against the original: any width and height (small, zero, large enough to overflow), the time-stamp counter's two answers near or equal or wrapped.
# The triangle routine at 0x6ef7f0 is a stand-in copying its three corners to a region (tools/fncmp/driver.cpp's rasterSpanStandIn, the same), then its recorder.
# usage: rasterspeed.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_fold, log_region, strings  # noqa: E402

TICKS, SPAN = 1, 2
OUT = RAM + 0x1000
_span_record = _recorder(SPAN, 3, cdecl=True)


def _span(e):
    """The corners off the stack, 36 bytes a call after a count."""
    sp = e.r['ESP'] + (4 if e.tail else 0)
    n = e.u32(OUT)
    e.write(OUT + 4 + 36 * n, b''.join(e.read(e.u32(sp + 4 * i), 12) for i in range(3)))
    e.write(OUT, struct.pack('<I', n + 1))
    _span_record(e)


B = Bench([(0x56a450, 0x56a50d)], hooks={0x57cb90: _recorder(TICKS, 0, cdecl=True), 0x6ef7f0: _span})


def case(r):
    log = log_region()
    log_fold(log)
    strings(log, SPAN, 0, 7)
    t0 = r.getrandbits(32)
    t1 = (t0 + r.choice([0, 1, 0x100, r.getrandbits(20), r.getrandbits(32), 0x10000000, -1])) & 0xffffffff
    table = struct.pack('<40I', t0, t1, *[0] * 38)
    answers(log, TICKS, RAM)
    size = [r.choice([0, 1, 16, 320, 640, 480, 1024, r.randrange(0x1000), r.getrandbits(32)]) for _ in range(2)]
    regions = {LOG: log, RAM: table, OUT: bytes(4 + 36 * 100), 0x8bcbd0: struct.pack('<2I', *size), 0x6f1a10: r.bytes(4), 0xfbe044: r.bytes(4)}
    return Call(0x56a450, [], regions, conv='c', ret='d')


if __name__ == '__main__':
    B.main(case)
