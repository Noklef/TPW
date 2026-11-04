# A narrow file buffer's constructor, destructor, pbackfail, uflow, close, _Init and deleting destructor, and the destructors of the narrow streams holding one (src/game/core/narrowfilebuf: 0x5d06c0, 0x5cec80, 0x5cefe0, 0x5cf200, 0x5d09b0, 0x5d0a90, 0x5cf890, 0x5ce920, 0x5cf630) against the originals: fclose, ungetc and fgetc cdecl recorders 18, 23 and 24, the conversion's slot 4 a recorder of
# 7 arguments whose stack ones are logged as 0 (it writes its local byte); the locale references' deletes slot 0 of a vtable of recorders. The string +0x44 real, in the heap.
# usage: narrowfilebuf.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ctor,dtor,putback,uflow,close,init,delete,ifstream,ofstream)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

FCLOSE, UNGETC, FGETC = 18, 23, 24
IN = 4   # (the conversion's slot)
B = Bench([(0x5ce920, 0x5cea27), (0x5cf630, 0x5cf737), (0x5cf890, 0x5cf8ae), (0x5d09b0, 0x5d0a4c),(0x5d0a90, 0x5d0b72), (0x4f5a50, 0x4f5ad0),(0x5d06c0, 0x5d08e0),(0x5d0e00, 0x5d0e70), (0x5cec80, 0x5cedd0), (0x5cefe0, 0x5cf1c0), (0x5cf200, 0x5cf410), (0x45b340, 0x45b9a0), (0x47b280, 0x47b300), (0x4f6020, 0x4f6080), (0x4ef7b0, 0x4ef800), (0x67acb0, 0x67acb1), (0x67acc0, 0x67acc1)],
          hooks={0x699da0: _recorder(15, 0, cdecl=True), 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67b620: _recorder(FCLOSE, 1, cdecl=True), 0x67cbe0: _recorder(UNGETC, 2, cdecl=True), 0x67cb90: _recorder(FGETC, 1, cdecl=True)})
BUF, GET, GETN, GETB, CONVERT, CONVERT_VT, STRING, TABLE, TABLE2, LOCALE, LOCALE2, LOCALE_VT = (RAM + 0x100 * i for i in range(12))
FILE = 0x12345678
VBTABLE, STREAM = RAM + 0x1f00, RAM + 0x2000
OPS = [n for n in ('ctor', 'dtor', 'putback', 'uflow', 'close', 'init', 'delete', 'ifstream', 'ofstream') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def string_regions(r, regions, length=None):
    """The string +0x44: no text, or a text in the heap with its use count 0, shared or not shareable."""
    block = heap(0x400, fail=r.choice([0] * 39 + [1]))   # (a failed block: a fault in both in StdString_Grow, which ends the case)
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
    c = r.choice([-1, -1, 0x41, r.randrange(256), r.getrandbits(32) - 0x80000000])
    put(buf, 0x50, 'I', r.choice([FILE, FILE, FILE, FILE, 0]))
    put(buf, 0x38, 'I', r.choice([CONVERT, CONVERT, CONVERT, 0]))
    put(buf, 0x44, 'I', STRING)
    regions[CONVERT] = struct.pack('<I', CONVERT_VT)
    regions[CONVERT_VT] = vtable({IN: 7})
    string_regions(r, regions)
    if name == 'ctor':
        set_return(log, 15, LOCALE)
        regions[LOCALE] = struct.pack('<II', LOCALE_VT, r.choice([0, 1, 1, 2, 5, 0xfffffffe, 0xffffffff]))
        regions[LOCALE_VT] = vtable({0: 1})
        regions[0xfb1dd0] = struct.pack('<I', LOCALE)
        regions[0xfb2760] = r.bytes(4)
        return Call(0x5d06c0, [BUF, r.choice([0, r.getrandbits(32) | 1, RAM + 0x1000])], regions, conv='t', ret='a')
    if name in ('dtor', 'delete', 'ifstream', 'ofstream'):
        buf[0x48] = r.choice([0, 1, 1, 1])
        put(buf, 0x44, 'I', r.choice([STRING, STRING, 0]))
        put(buf, 0x4c, 'I', r.choice([LOCALE, LOCALE, 0]))
        put(buf, 0x34, 'I', r.choice([LOCALE2, LOCALE2, 0]))
        for at in (LOCALE, LOCALE2):
            regions[at] = struct.pack('<II', LOCALE_VT, r.choice([0, 1, 1, 2, 5, 0xffffffff]))
        regions[LOCALE_VT] = vtable({0: 1})
        regions[0xfb2760] = r.bytes(4)
        set_return(log, FCLOSE, r.choice([0, 0, 1, 0xffffffff]))
        if name in ('ifstream', 'ofstream'):   # (a stream holding the buffer, called on its ios: its table gives where its ios is)
            ios, head = (0x5c, 8) if name == 'ifstream' else (0x58, 4)
            stream = bytearray(r.bytes(ios + 0x10))
            stream[head:head + 0x54] = buf[:0x54]
            put(stream, 0, 'I', VBTABLE)
            del regions[BUF]
            regions[STREAM] = stream
            regions[VBTABLE] = struct.pack('<2I', r.getrandbits(32), r.choice([ios, ios, ios + 8]))
            return Call(0x5ce920 if name == 'ifstream' else 0x5cf630, [STREAM + ios], regions, conv='t', ret='v')
        if name == 'delete':
            return Call(0x5cf890, [BUF, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
        return Call(0x5cec80, [BUF], regions, conv='t', ret='v')
    if name == 'close':   # (0x5d09b0: no file, fclose failing or not)
        regions[0xfb2760] = r.bytes(4)
        set_return(log, FCLOSE, r.choice([0, 0, 0, 1, 0xffffffff]))
        return Call(0x5d09b0, [BUF], regions, conv='t', ret='a')
    if name == 'init':    # (0x5d0a90: the file set or not, the mode 0, 1 or other; mode 0 loses the locale reference +0x4c)
        put(buf, 0x4c, 'I', r.choice([LOCALE, LOCALE, 0]))
        regions[LOCALE] = struct.pack('<II', LOCALE_VT, r.choice([0, 1, 1, 2, 5, 0xffffffff]))
        regions[LOCALE_VT] = vtable({0: 1})
        regions[0xfb1dd0] = struct.pack('<I', LOCALE)
        regions[0xfb2760] = r.bytes(4)
        set_return(log, 15, LOCALE)
        return Call(0x5d0a90, [BUF, r.choice([0, RAM + 0x1000, RAM + 0x1000]), r.choice([0, 0, 1, 1, 2, r.getrandbits(32)])], regions, conv='t', ret='v')
    if name == 'putback':
        put(buf, 0xc, 'I', GETB)
        put(buf, 0x2c, 'I', GETN)
        regions[GET] = bytearray(0x20)
        put(regions[GET], 0, 'I', r.choice([0, GET + 0x11, GET + 0x11, GET + 0x11]))
        regions[GET][0x10] = r.choice([c & 0xff, r.randrange(256)])
        regions[GETB] = struct.pack('<I', r.choice([GET + 0x10, GET + 0x11, GET + 0x12]))
        regions[GETN] = struct.pack('<I', r.randrange(100))
        put(buf, 0x1c, 'I', GET)
        regions[TABLE] = struct.pack('<40I', *[r.choice([0, 0x41, 0x41, 0x41, 0x41, 0xffffffff]) for _ in range(40)])
        answers(log, UNGETC, TABLE)
        return Call(0x5cefe0, [BUF, c & 0xffffffff], regions, conv='t', ret='a')
    put(buf, 0x1c, 'I', GET)
    put(buf, 0x2c, 'I', GETN)
    regions[GET] = bytearray(0x20)
    position = r.choice([0, GET + 0x10, GET + 0x10, 0xfffffff0])
    put(regions[GET], 0, 'I', position)
    regions[GET][0x10] = r.randrange(256)
    # (the position near the top wraps with the big count, so no read; with a small count it reads and faults in both)
    regions[GETN] = struct.pack('<I', r.choice([0, 0x80000000] if position == 0xfffffff0 else [0, 0, 0, 1, 3, 0x80000000]))
    regions[TABLE] = struct.pack('<40I', *[r.choice([0, 1, 1, 3, 3, 2]) for _ in range(40)])
    answers(log, IN, TABLE)
    fgetc = [r.choice([0x41, r.randrange(256), r.randrange(256), r.getrandbits(32) & ~1]) for _ in range(r.randrange(1, 8))] + [0xffffffff]
    regions[TABLE2] = struct.pack('<40I', *(fgetc + [0xffffffff] * 40)[:40])
    answers(log, FGETC, TABLE2)
    strings(log, IN, 0, out=0b1111000)
    fill(log, IN, 4, bytes([r.randrange(256)]), which=0)
    return Call(0x5cf200, [BUF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
