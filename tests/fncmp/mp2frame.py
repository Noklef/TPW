# The MP2 decoder's frame headers (src/game/core/mp2frame) against the originals: the parser (0x6cc7b0) over headers with the sync
# mostly (any other bits: every layer, ID, MPEG 2.5, mode, bitrate index 0 now and then, the rate bits 3 that make MPEG 2.5 divide by 0),
# random bits now and then, half rate (0xfb2708) now and then; the stream decoder's slots 4 and 5 (0x6cca00, 0x6ccac0) reading the header
# and the frame through a stream whose read is a sequenced read (the header's buffer is the caller's local: logged as 0), the data now and
# then shorter than the frame; the buffer decoder's (0x6cca60, 0x6ccaf0) over a buffer with the header at the bit position (now and then
# below the buffer's start, or random bytes). The decoder's other bytes random.
# usage: mp2frame.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, seq_read, strings, vtable  # noqa: E402

CALLS = {'parse': 0x6cc7b0, 'stream_frame': 0x6cca00, 'stream_header': 0x6ccac0, 'buffer_frame': 0x6cca60, 'buffer_header': 0x6ccaf0}
B = Bench([(0x6cc7b0, 0x6cc9f8), (0x6cca00, 0x6ccb0e)])
DECODER, STREAM, VT, DATA, BUF = RAM, RAM + 0x4000, RAM + 0x4100, RAM + 0x5000, RAM + 0x6000
HALF = 0xfb2708
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def header(r):
    if r.random() < 0.08:
        return r.getrandbits(32)
    h = 0xffe00000 | r.getrandbits(21)
    if r.random() < 0.7:   # a known standard and layer, a nonzero bitrate index, the rate bits 0-2
        h = h & ~0x1efc00 | r.choice([0x100000, 0x180000, 0]) | r.randrange(1, 4) << 17 | r.randrange(1, 15) << 12 | r.randrange(3) << 10
    return h


def case(r):
    name = r.deal(OPS)
    decoder = bytearray(r.bytes(0x2c80))
    regions = {DECODER: decoder, HALF: struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))}
    h = header(r)
    if name == 'parse':
        return Call(CALLS[name], [DECODER, h], regions, conv='t', ret='a')
    if name.startswith('stream'):
        log = log_region()
        frame = r.bytes(r.choice([r.randrange(0, 0x40), r.randrange(0x40, 0x800)]))
        data = struct.pack('>I', h) + frame
        seq_read(log, 1, DATA, len(data), buffer=0, count=1)
        strings(log, 1, 0, out=1)
        struct.pack_into('<I', decoder, 0x2c7c, STREAM)
        regions.update({LOG: log, STREAM: struct.pack('<I', VT), VT: vtable({1: 2}), DATA: data})
        return Call(CALLS[name], [DECODER], regions, conv='t', ret='a')
    buf = bytearray(r.bytes(0x500))
    position = r.randrange(-0x400, 0x1e00) if r.random() < 0.2 else r.randrange(0, 0x1e00)
    at = 0x100 + (position >> 3)
    if r.random() < 0.9:
        struct.pack_into('>I', buf, at if name == 'buffer_frame' else 0x100, h)
    struct.pack_into('<2I', decoder, 0x2428, BUF + 0x100, r.getrandbits(32))
    struct.pack_into('<i', decoder, 0x2430, position)
    regions[BUF] = buf
    return Call(CALLS[name], [DECODER], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
