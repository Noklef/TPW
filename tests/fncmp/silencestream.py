# The silence stream's read (src/game/core/silencestream: 0x6c67f0) against the original: a stream at STREAM (+0x2c 0, 1 or any; +0x14
# about +0x18: below, at, past or any), its next stream a recorder (slot 7 logged, answering 0, 1 or any, writing a count through done
# now and then); a buffer of 0x400 bytes, the bytes up to 0x400 and *done up to them (past them now and then: a fault in both).
# The format set (0x6c6f40: a format's +0x1a bits 0x10 or not, +0x1b bit 0 set or not, its word +0x18 the rates slot 15 knows or any,
# the format in one of two 64 KB chunks: the pushed dword's high half) on a recorder (slot 15 logged), and the constructor (0x6c6730)
# through the real vtable 0x70ab58 (slot 15 Obj_SetRateBits20; the rate Obj_SampleRate20), the ms small, large or any.
# usage: silencestream.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, fill, log_region, set_return, vtable  # noqa: E402

B = Bench([(0x6c67f0, 0x6c688c), (0x6c6730, 0x6c67c5), (0x6c6f40, 0x6c6f87), (0x6c6f90, 0x6c6fee)])
STREAM, SVT, NEXT, NVT, DONE, BUFFER = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x1000
FORMATS = [RAM + 0x500, RAM + 0x20500]   # (two high halves)
CALLS = {'read': 0x6c67f0, 'setformat': 0x6c6f40, 'ctor': 0x6c6730}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def format_(r, regions):
    fmt = bytearray(r.bytes(0x20))
    struct.pack_into('<H', fmt, 0x18, r.choice([0x5622, 0xac44, 0x2b11, r.getrandbits(16)]))
    fmt[0x1a] = r.choice([0x10, 0x10, 8, r.getrandbits(8)])
    fmt[0x1b] = r.choice([0, 1, r.getrandbits(8)])
    at = r.choice(FORMATS)
    regions[at] = fmt
    return at


def case(r):
    name = r.deal(OPS)
    if name == 'setformat':
        regions = {LOG: log_region(), STREAM: struct.pack('<I', SVT) + r.bytes(0x2c), SVT: vtable({15: 1})}
        return Call(CALLS[name], [STREAM, format_(r, regions)], regions, conv='t', ret='v')
    if name == 'ctor':
        regions = {STREAM: r.bytes(0x30)}
        ms = r.choice([0, 1, 1000, r.randrange(100000), r.getrandbits(32)])
        return Call(CALLS[name], [STREAM, ms, format_(r, regions)], regions, conv='t', ret='a')
    log = log_region()
    bytes_ = r.choice([0, 1, 2, r.randrange(0x400), 0x400])
    done = r.choice([0, 0, bytes_, r.randrange(bytes_ + 1), bytes_ - 1 if bytes_ else 0, bytes_ + 1 if r.random() < 0.05 else 0])
    total = r.choice([0, r.randrange(0x400), r.randrange(0x400, 0x10000), r.getrandbits(32)])
    played = r.choice([0, total, total - 1, total + 1, total - r.randrange(0x400), r.getrandbits(32)]) & 0xffffffff
    stream = bytearray(r.bytes(0x30))
    struct.pack_into('<II', stream, 0, SVT, NEXT)
    struct.pack_into('<II', stream, 0x14, played, total)
    stream[0x2c] = r.choice([0, 0, 1, r.getrandbits(8)])
    set_return(log, 7, r.choice([0, 1, 1, 0x100, r.getrandbits(32)]))
    if r.random() < 0.5:
        fill(log, 7, 2, struct.pack('<I', r.randrange(bytes_ + 1)))
    regions = {STREAM: stream, SVT: vtable({}), NEXT: struct.pack('<I', NVT) + r.bytes(0xc), NVT: vtable({7: 4}), DONE: struct.pack('<I', done),
               BUFFER: r.bytes(0x400), LOG: log}
    return Call(0x6c67f0, [STREAM, BUFFER, bytes_, DONE, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
