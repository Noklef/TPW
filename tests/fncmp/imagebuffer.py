# An image file kept in the screen's pixel format (src/game/render/imagebuffer: 0x586e40, 0x586f40) against the originals, the callees recorders (driver.cpp's kImageBufferCalls): the format's answer 0, positive or negative, the driver kind, no pixels, no file, a file in the rows and over the top or past them, a file as wide as the image or narrower.
# usage: imagebuffer.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, set_return, strings, vtable  # noqa: E402

MESSAGE, LOGGED, ERROR, TABLE, LOAD, FREE, FORMAT = 1, 2, 3, 4, 5, 6, 21
B = Bench([(0x586e40, 0x586f3e), (0x586f40, 0x58710e), (0x67a400, 0x6902c0)], hooks={0x56a770: _recorder(MESSAGE, 1, cdecl=True), 0x564650: _recorder(LOGGED, 1, cdecl=True), 0x56af20: _recorder(ERROR, 2), 0x591270: _recorder(TABLE, 9, cdecl=True), 0x574bd0: _recorder(LOAD, 6, cdecl=True), 0x579b80: _recorder(FREE, 1, cdecl=True)})
SELF, NAME, DIRECT, SURFACE, VT, TEXT, BLOCK, PIXELS = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xa0, RAM + 0xc0, RAM + 0xf0, RAM + 0x1000, RAM + 0x10000
OPS = ['Load', 'Load', 'Load fails', 'Load file none', 'Stamp', 'Stamp', 'Stamp', 'Stamp fails', 'Stamp no pixels', 'Stamp file none', 'Stamp high', 'Stamp past']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    answer = r.choice([r.randrange(1, 100), -r.randrange(1, 1 << 30)]) if 'fails' in op else 0
    kind = r.choice([1, 1, 1, 4, 0, 2])
    bits = r.choice([16, 24, 32])
    bytes_ = bits >> 3 if kind == 1 else 4
    fmt = bytearray(r.bytes(0x20))
    put(fmt, 0xc, 'I', bits)
    set_return(log, FORMAT, answer)
    fill(log, FORMAT, 1, bytes(fmt[4:]), offset=4)
    blob(log, FORMAT, 1, 1)   # (the format's size set before the call)
    set_return(log, MESSAGE, TEXT)
    strings(log, LOGGED, 1)
    table_mask = 1 << 4 if kind == 1 else 0
    w, h = r.randrange(1, 12), r.randrange(1, 12)
    fw, fh = r.randrange(1, w + 1), r.randrange(1, h + 3)
    y = r.randrange(0, h + 1)
    if op == 'Stamp high':
        y, fh = r.randrange(0, h), h + r.randrange(1, 4)
    if op == 'Stamp past':
        y = h + r.randrange(0, 3)
    set_return(log, LOAD, 0 if 'none' in op else BLOCK)
    strings(log, TABLE, 0, 1)
    pixels = r.bytes(0x1000)
    self = bytearray(r.bytes(0x10))
    if op.startswith('Load'):
        strings(log, LOAD, 1, table_mask)
        fill(log, LOAD, 1, struct.pack('<I', r.randrange(0, 1 << 16)), which=1)
        fill(log, LOAD, 2, struct.pack('<I', r.randrange(0, 1 << 16)), which=2)
    else:
        strings(log, LOAD, 1, 6 | table_mask)
        fill(log, LOAD, 1, struct.pack('<I', fw), which=1)
        fill(log, LOAD, 2, struct.pack('<I', fh), which=2)
        put(self, 0, 'IIII', 0 if op == 'Stamp no pixels' else PIXELS, w, h, bytes_)
    regions = {LOG: log, SELF: self, NAME: b'title.tga\0', DIRECT: put(bytearray(r.bytes(0x18)), 0x10, 'I', SURFACE), SURFACE: struct.pack('<I', VT), VT: vtable({}, 0x60, com={FORMAT: 2}), TEXT: b'bad\0', BLOCK: pixels, PIXELS: r.bytes(0x1000),
               0x879a24: struct.pack('<I', DIRECT), 0x879cd4: r.bytes(4), 0x8bd524: struct.pack('<I', kind)}
    if op.startswith('Load'):
        return Call(0x586e40, [SELF, NAME], regions, conv='t', ret='a')
    return Call(0x586f40, [SELF, NAME, r.getrandbits(32), y], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
