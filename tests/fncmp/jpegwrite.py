# The game's JPEG writer (src/game/gfx/jpegwrite: 0x64bf90, 0x64c202, 0x64c258, 0x64c552, 0x64c5c4, 0x64dc00, 0x64dc5a, 0x64dc9c, 0x64dd00) against the originals. libjpeg 6a's calls (their own benches are
# jpeg6a_*.py), the guarded heap and the guarded file are recorders: the compress struct logged by its fields (err, mem, dest, the image's four), each row by what it holds. jpeg_CreateCompress's recorder
# writes the memory manager's table and the destination's slot; jpeg_set_defaults' sometimes another height (the loop reads it live). The stand-in for ERREXIT returns, so the code after it runs.
# usage: jpegwrite.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, blob_bytes, fill, log_region, mask_this, put, recorder, set_return, strings  # noqa: E402

ALLOC, FREE, WRITE = 1, 2, 3
STDERR, CREATE, DEFAULTS, QUALITY, START, MARKER, SCANLINES, FINISH, DESTROY, SMALL, EXIT = 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
RANGES = [(0x64bf90, 0x64c202), (0x64c202, 0x64c258), (0x64c258, 0x64c552), (0x64c552, 0x64c5c4), (0x64c5c4, 0x64c5ea), (0x64dc00, 0x64dc5a), (0x64dc5a, 0x64dc9c), (0x64dc9c, 0x64dd00), (0x64dd00, 0x64dd5d)]
HOOKS = {0x5d4ba0: _recorder(ALLOC, 1), 0x5d4c20: _recorder(FREE, 1), 0x5d5350: _recorder(WRITE, 3)}
HOOKS.update({a: _recorder(k, n, cdecl=True) for a, k, n in ((0x64dd60, STDERR, 1), (0x64c5f0, CREATE, 3), (0x64cf20, DEFAULTS, 1), (0x64cef8, QUALITY, 3), (0x64ca90, START, 2), (0x64c982, MARKER, 4),
                                                             (0x64cb3c, SCANLINES, 3), (0x64c7fd, FINISH, 1), (0x64c740, DESTROY, 1))})
B = Bench(RANGES, hooks=HOOKS)
INFO, PIXELS, ROW, CINFO, MEM, DEST, ERR, OUT, ANSWERS = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x2400, RAM + 0x2600, RAM + 0x2700, RAM + 0x2800, RAM + 0x2900, RAM + 0x2a00
FIELDS = 0x3e3   # (the compress struct's dwords: err, mem, dest, width, height, components, colour space)
OPS_TABLE = {'Row': 0x64bf90, 'Open': 0x64c202, 'Image': 0x64c258, 'Grey': 0x64c552, 'Close': 0x64c5c4, 'SetDest': 0x64dc00, 'Init': 0x64dc5a, 'Empty': 0x64dc9c, 'Term': 0x64dd00}
OPS = [n for n in OPS_TABLE if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def entries(r, count):
    """count 4-byte entries, grey or not by one byte of one entry, or of any bytes."""
    data = bytearray(r.bytes(count * 4))
    k = r.random()
    if k < 0.7:
        for i in range(count):
            v = r.getrandbits(8)
            put(data, i * 4, '3B', v, v, v)
        if k < 0.35 and count:
            data[r.randrange(count) * 4 + r.randrange(3)] ^= 1 << r.randrange(8)
    return data


def info_block(r, width, height, bits):
    data = bytearray(r.bytes(0x440))
    colours = r.choice([0, 1, 16, 256])
    data[0x28:0x428] = entries(r, 256)
    if r.random() < 0.5:
        data[0x28:0x28 + 4 * colours] = entries(r, colours)
    put(data, 0x10, '3I', width & 0xffffffff, height & 0xffffffff, bits)
    put(data, 0x24, 'I', colours)
    put(data, 0x428, 'I', r.getrandbits(32) & ~1 | (r.random() < 0.25))
    put(data, 0x430, 'I', r.choice([0, 1, 50, 75, 100, r.getrandbits(32)]))
    return data


def cinfo_block(r, dest=DEST):
    data = bytearray(r.bytes(0x168))
    put(data, 0, 'II', ERR, MEM)
    put(data, 0x14, 'I', dest)
    return data


def methods(log, r):
    """The memory manager's table and the error manager: alloc_small and error_exit recorders; the destination."""
    strings(log, SMALL, 0)
    set_return(log, SMALL, OUT)
    return {MEM: struct.pack('<4I', recorder(SMALL, 3, cdecl=True), *[r.getrandbits(32) | 0x80000000 for _ in range(3)]), ERR: put(bytearray(r.bytes(0x20)), 0, 'I', recorder(EXIT, 1, cdecl=True)),
            DEST: r.bytes(0x1c), OUT: r.bytes(0x20)}


def case(r):
    op = r.deal(OPS)
    address = OPS_TABLE[op]
    log = log_region()
    for k in (ALLOC, FREE, WRITE):   # (stdcall: ECX is whatever was there)
        mask_this(log, k)
    regions = {LOG: log}
    if op == 'Row':
        count = r.choice([0, 1, 2, 3, 8, -1])
        bits = r.choice([0x20, 8, 8, 16, 24])
        regions.update({PIXELS: r.bytes(0x40), ROW: r.bytes(0x40), INFO: info_block(r, count, 1, bits)})
        return Call(address, [PIXELS, ROW, count & 0xffffffff, r.choice([0, 1, 2, 0, 1, 2, 3, -1]) & 0xffffffff, INFO], regions, ret='v')
    if op == 'Grey':
        count = r.choice([0, 1, 2, 3, 8, -1])
        regions[PIXELS] = entries(r, 8)
        return Call(address, [PIXELS, count & 0xffffffff], regions, ret='a')
    if op == 'Open':
        set_return(log, ALLOC, 0 if r.random() < 0.2 else OUT)
        regions.update({OUT: r.bytes(0x20), PIXELS: r.bytes(4)})
        return Call(address, [PIXELS, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='s', ret='a')
    if op == 'Close':
        set_return(log, FREE, r.getrandbits(32))
        return Call(address, [r.choice([OUT, OUT, 0])], regions, conv='s', ret='a')
    if op == 'SetDest':
        regions.update(methods(log, r))
        regions[CINFO] = cinfo_block(r, r.choice([DEST, 0]))
        set_return(log, SMALL, DEST)
        return Call(address, [CINFO, r.getrandbits(32)], regions, ret='v')
    if op in ('Init', 'Empty', 'Term'):
        regions.update(methods(log, r))
        regions[CINFO] = cinfo_block(r)
        dest = bytearray(regions[DEST])
        put(dest, 4, 'I', r.choice([0, 1, 0x800, 0xfff, 0x1000, 0x1001, r.getrandbits(32)]))
        regions[DEST] = dest
        wanted = 0x1000 if op == 'Empty' else 0x1000 - struct.unpack_from('<I', dest, 4)[0] & 0xffffffff
        set_return(log, WRITE, wanted if r.random() < 0.7 else r.choice([0, 1, wanted + 1 & 0xffffffff]))
        ret = {'Init': 'v', 'Empty': 'a', 'Term': 'v'}[op]
        return Call(address, [CINFO], regions, ret=ret)
    # The whole image.
    width, height = r.randrange(0, 7), r.randrange(0, 5)
    bits = r.choice([0x20, 8, 8, 0x18, 0x10, 4, 0])
    info = info_block(r, width, height, bits)
    pitch = r.choice([width * 4, width, width * 4 + 4, -(width * 4)])
    pixels = bytearray(0x400)
    for y in range(height):
        pixels[0x200 + y * pitch:0x200 + y * pitch + width * 4] = entries(r, width)
    regions.update({INFO: info, PIXELS: pixels, ROW: r.bytes(0x40), ERR: r.bytes(0x20), OUT: r.bytes(0x20), MEM: struct.pack('<I', recorder(SMALL, 3, cdecl=True)) + r.bytes(0xc),
                    DEST: r.bytes(0x1c), ANSWERS: r.bytes(0x10)})
    set_return(log, SMALL, DEST)
    strings(log, SMALL, 0, out=1)
    set_return(log, STDERR, ERR)
    strings(log, STDERR, 0, out=1)
    fill(log, CREATE, 0, struct.pack('<5I', MEM, 0, 0, 0, r.choice([0, 0, DEST])), which=0, offset=4)
    set_return(log, ALLOC, 0 if r.random() < 0.15 else ROW)
    set_return(log, FREE, r.getrandbits(32))
    if r.random() < 0.15:
        fill(log, DEFAULTS, 0, struct.pack('<i', r.choice([0, 1, height + 1, -1])), which=1, offset=0x1c)
    blob(log, CREATE, 0, 0x23)   # (the image's fields not set yet)
    for k in (DEFAULTS, QUALITY, START, MARKER, FINISH, DESTROY):
        blob(log, k, 0, FIELDS)
    blob_bytes(log, SCANLINES, 1, 0, inner=(0, (1 << 32) - 1))
    strings(log, SCANLINES, 0, out=1)
    return Call(address, [OUT, INFO, PIXELS + 0x200, pitch & 0xffffffff], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)
