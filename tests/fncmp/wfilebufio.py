# A wide file buffer's overflow, pbackfail and uflow (src/game/core/wfilebuf: 0x4f0a10, 0x4f0c50, 0x4f0e60) against the originals: the C library's file calls cdecl recorders 20-25, the conversion's slots recorders of
# 7 arguments whose stack ones are logged as 0, the "from" of overflow by its bytes; they write their outs (fill_arg: from_next is from or from_end, to_next is to or to_end; uflow's "to" gets a word). The string +0x44 real, in the heap.
# usage: wfilebufio.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=overflow,putback,uflow)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, fill_arg, heap, log_region, put, strings, vtable  # noqa: E402

FWRITE, FPUTWC, UNGETWC, UNGETC, FGETC, FGETWC = range(20, 26)
IN, OUT = 4, 5   # (the conversion's slots)
hooks = {0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None}
for at, k, n in ((0x67b6d0, FWRITE, 4), (0x67c9f0, FPUTWC, 2), (0x67cca0, UNGETWC, 2), (0x67cbe0, UNGETC, 2), (0x67cb90, FGETC, 1), (0x67cdf0, FGETWC, 1)):
    hooks[at] = _recorder(k, n, cdecl=True)
B = Bench([(0x4f0a10, 0x4f0c45), (0x4f0c50, 0x4f0e2d), (0x4f0e60, 0x4f1064), (0x45b590, 0x45b5e0), (0x45b5e0, 0x45b6a0), (0x45b6a0, 0x45b7c0), (0x45b7d0, 0x45b8c5), (0x45b8d0, 0x45b9a0), (0x47b280, 0x47b300)], hooks=hooks)
BUF, GET, PUT, GETN, PUTN, GETB, CONVERT, CONVERT_VT, STRING, TABLE, TABLE2 = (RAM + 0x100 * i for i in range(11))
FILE = 0x12345678
OPS = [n for n in ('overflow', 'putback', 'uflow') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
EOF = 0xffff


def string_regions(r, regions, length=None):
    """The string +0x44: no text, or a text in the heap with its use count 0, shared or not shareable."""
    block = heap(0x400, fail=r.choice([0, 0, 0, 0, 0, 0, 1]))
    string = bytearray(0x10)
    length = r.choice([0, 1, 2, 5, 16]) if length is None else length
    if length or r.random() < 0.8:
        text = HEAP + 0x200
        put(string, 4, 'I', text)
        put(string, 8, 'I', length)
        put(string, 0xc, 'I', max(length, r.choice([length, 31])))
        block[text - HEAP - 1] = r.choice([0, 0, 1, 2, 0xff])
        data = bytes(r.choice([0x41, 0x42, 0, 0xc3]) if r.random() < 0.1 else r.randrange(1, 255) for _ in range(length))
        block[text - HEAP:text - HEAP + length + 1] = data + b'\0'
    regions[HEAP] = block
    regions[STRING] = string


def case(r):
    name = r.deal(OPS)
    log = log_region()
    buf = bytearray(r.bytes(0x60))
    regions = {LOG: log, BUF: buf}
    c = r.choice([EOF, 0x41, r.randrange(0x10000), r.getrandbits(32) | 0xffff, r.getrandbits(32)])
    put(buf, 0x50, 'I', r.choice([FILE, FILE, FILE, FILE, 0]))
    put(buf, 0x38, 'I', r.choice([CONVERT, CONVERT, CONVERT, 0]))
    put(buf, 0x44, 'I', STRING)
    regions[CONVERT] = struct.pack('<I', CONVERT_VT)
    regions[CONVERT_VT] = vtable({IN: 7, OUT: 7})
    string_regions(r, regions)
    table = [r.choice([0, 0, 1, 2, 3, 3]) for _ in range(40)]
    if name == 'overflow':
        put(buf, 0x20, 'I', PUT)
        put(buf, 0x30, 'I', PUTN)
        regions[PUT] = struct.pack('<I', r.choice([0, PUT + 0x10, PUT + 0x10, 0xfffffff0]))
        regions[PUTN] = struct.pack('<I', r.choice([0, 0, 0, 1, 3, 0x80000000]))
        strings(log, OUT, 0, out=0b1001100)
        blob(log, OUT, 1, 1)
        fill_arg(log, OUT, 3, r.choice([1, 2]), which=0)
        fill_arg(log, OUT, 6, r.choice([4, 5]), which=1)
        fill(log, OUT, 4, r.bytes(4), which=2)
        regions[TABLE] = struct.pack('<40I', *table)
        answers(log, OUT, TABLE)
        regions[TABLE2] = struct.pack('<40I', *[r.choice([1, 1, 0, 8, 9]) for _ in range(40)])
        answers(log, FWRITE, TABLE2)
        blob(log, FWRITE, 0, 3)
        return Call(0x4f0a10, [BUF, c], regions, conv='t', ret='w')
    if name == 'putback':
        put(buf, 0xc, 'I', GETB)
        put(buf, 0x2c, 'I', GETN)
        regions[GET] = bytearray(0x20)
        put(regions[GET], 0, 'I', r.choice([0, GET + 0x12, GET + 0x12, GET + 0x12]))
        put(regions[GET], 0x10, 'H', r.choice([c & 0xffff, r.randrange(0x10000)]))
        regions[GETB] = struct.pack('<I', r.choice([GET + 0x10, GET + 0x12, GET + 0x14]))
        regions[GETN] = struct.pack('<I', r.randrange(100))
        put(buf, 0x1c, 'I', GET)
        regions[TABLE] = struct.pack('<40I', *[r.choice([0, 0x41, 0x41, 0x41, 0xffffffff]) for _ in range(40)])
        answers(log, UNGETC, TABLE)
        return Call(0x4f0c50, [BUF, c], regions, conv='t', ret='w')
    put(buf, 0x1c, 'I', GET)
    put(buf, 0x2c, 'I', GETN)
    regions[GET] = bytearray(0x20)
    put(regions[GET], 0, 'I', r.choice([0, GET + 0x10, GET + 0x10, 0xfffffff0]))
    put(regions[GET], 0x10, 'H', r.randrange(0x10000))
    regions[GETN] = struct.pack('<I', r.choice([0, 0, 0, 1, 3, 0x80000000]))
    regions[TABLE] = struct.pack('<40I', *[r.choice([0, 1, 1, 3, 3, 2]) for _ in range(40)])
    answers(log, IN, TABLE)
    fgetc = [r.choice([0x41, r.randrange(256), r.randrange(256), r.getrandbits(32) & ~1]) for _ in range(r.randrange(1, 8))] + [0xffffffff]
    regions[TABLE2] = struct.pack('<40I', *(fgetc + [0xffffffff] * 40)[:40])
    answers(log, FGETC, TABLE2)
    strings(log, IN, 0, out=0b1111000)
    fill(log, IN, 4, struct.pack('<H', r.randrange(0x10000)), which=0)
    return Call(0x4f0e60, [BUF], regions, conv='t', ret='w')


if __name__ == '__main__':
    B.main(case)
