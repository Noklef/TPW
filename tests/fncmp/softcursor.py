# The software mouse cursor (src/game/gfx/cursor: 0x5fad40-0x5fb8dd) against the originals: the singleton at 0xfa7a80 (its guard 0xfa7a78) with its
# state random within reason (the buffer indices next / shown / parked 0-2, small sizes, rectangles and positions, a position now and then the last
# one), its three SurfaceBuffers created or not, borrowed now and then. Every IDirectDrawSurface (the buffers', the target's, the image's, a new
# one) shares one vtable of COM recorders, told apart in the log by `this`: BltFast (its source rectangle hashed; succeeding, failing or finding
# the surface lost), GetSurfaceDesc (filling the description, the stack, logged as 0), SetColorKey (its DDCOLORKEY hashed), IsLost, Restore,
# Release. The IDirectDraw at [0xfa76b0] makes surfaces (CreateSurface: its description hashed, the new surface filled in, failing now and then).
# atExit fncmp's cdecl log on both sides (driver.cpp's Cursor env); the busy byte 0x775874.
# usage: softcursor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=GetInfo,... to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, blob, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x5f9000, 0x5f92f0), (0x5fa680, 0x5fa6b0), (0x5fad40, 0x5fb8f0), (0x61b090, 0x61b0f0), (0x61b130, 0x61b1f0), (0x62f810, 0x62f8e0),
           (0x6306c0, 0x6307d0), (0x67a740, 0x67a800), (0x67acf0, 0x67ada0), (0x67afa0, 0x67b060), (0x67fa20, 0x67fa40)],
          hooks={0x67a7d0: _cdecl_log})
CALLS = {'GetInfo': 0x5fad40, 'Get': 0x5fad80, 'AtExit': 0x5fadc0, 'Ctor': 0x5fadd0, 'DeletingDtor': 0x5faeb0, 'Dtor': 0x5faed0, 'Draw': 0x5faef0,
         'Discard': 0x5faf70, 'CreateBuffers': 0x5faf80, 'Flip': 0x5fb020, 'ReleaseBuffers': 0x5fb060, 'SetImage': 0x5fb090, 'SaveUnder': 0x5fb290,
         'EraseShown': 0x5fb400, 'Paint': 0x5fb4d0, 'Move': 0x5fb520, 'ClearImage': 0x5fb790, 'SetKey': 0x5fb7b0, 'Update': 0x5fb7f0,
         'Restore': 0x5fb820}
GUARD, C = 0xfa7a78, 0xfa7a80
OBJS, VT, DD, VTD, TARGET, IMAGE, OUT, POS, RECT, HOT, KEY, BITS = (RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x380, RAM + 0x400, RAM + 0x460,
                                                                    RAM + 0x4c0, RAM + 0x500, RAM + 0x510, RAM + 0x520, RAM + 0x530, RAM + 0x540)
B0, B1, B2, TOBJ, IOBJ, NEW = (OBJS + 0x10 * i for i in range(6))   # the IDirectDrawSurfaces: the buffers', the target's, the image's, a new one
RELEASE, CREATE, BLTFAST, DESC, ISLOST, RESTORE, SETKEY = 2, 6, 7, 22, 24, 27, 29
LOST = 0x887601c2
FORMATS = [(8, 0, 0, 0, 0), (16, 0xf800, 0x7e0, 0x1f, 0), (16, 0x7c00, 0x3e0, 0x1f, 0x8000), (24, 0xff0000, 0xff00, 0xff, 0),
           (32, 0xff0000, 0xff00, 0xff, 0xff000000)]


def desc(r):
    d = bytearray(r.bytes(0x6c))
    put(d, 8, '3i', r.randrange(0, 64), r.randrange(0, 64), r.choice([64, 128, r.getrandbits(31)]))
    put(d, 0x24, 'I', BITS)
    put(d, 0x54, '5I', *(r.choice(FORMATS) if r.random() < 0.9 else tuple(r.getrandbits(32) for _ in range(5))))
    return bytes(d)


def rect(r):
    left, top = r.randrange(-12, 60), r.randrange(-12, 60)
    if r.random() < 0.1:
        return struct.pack('<4i', left, top, left - r.randrange(0, 8), top + r.randrange(0, 40))
    return struct.pack('<4i', left, top, left + r.randrange(0, 40), top + r.randrange(0, 40))


def surface(r, obj):
    """A Surface wrapper's 0x50 bytes: +0x24 its IDirectDrawSurface."""
    s = bytearray(r.bytes(0x50))
    put(s, 0x24, 'I', obj)
    return s


def cursor(r):
    c = bytearray(r.bytes(0x230))
    put(c, 5, 'B', r.choice([0, 1, 1, r.getrandbits(8)]))
    size = lambda: r.choice([0, 1, 8, 16, 32, r.randrange(0, 48), r.getrandbits(32)])  # noqa: E731
    put(c, 0xc, '4I', size(), size(), r.randrange(0, 40), r.randrange(0, 40))
    put(c, 0x1c, '5i', r.choice([0, 1, 2]), r.choice([0, 1, 1, 2, 2]), r.choice([0, 0, 1, 2]), r.choice([0, 1]), r.choice([0, 1]))
    c[0x30:0x40] = r.choice([struct.pack('<4i', 0, 0, 640, 480), struct.pack('<4i', 0, 0, 40, 30), rect(r)])
    put(c, 0x40, '4i', -r.randrange(0, 16), -r.randrange(0, 16), r.randrange(-8, 80), r.randrange(-8, 80))
    for i, obj in enumerate((B0, B1, B2)):
        at = 0x50 + 0x5c * i
        c[at + 4:at + 0x54] = surface(r, obj if r.random() < 0.97 else 0)
        put(c, at + 0x54, '2I', r.choice([0, 1, 1, r.getrandbits(32)]), r.choice([0, 0, 0, 1]))
    for i in range(12):
        c[0x164 + 0x10 * i:0x174 + 0x10 * i] = rect(r)
    put(c, 0x224, '3I', *[TARGET if r.random() < 0.85 else 0 for _ in range(3)])
    return c


def case(r):
    op = r.deal(OPS)
    log = log_region()
    strings(log, DESC, 0, 1 << 1)        # (the description: the stack)
    fill(log, DESC, 1, desc(r), 0)
    set_return(log, DESC, r.choice([0, 0, 0, 0, 0x80004005]))
    blob(log, CREATE, 1, 0xf | 1 << 26)  # (the size, flags, height, width and caps: the original leaves the rest stack garbage)
    fill(log, CREATE, 2, struct.pack('<I', NEW), 1)
    set_return(log, CREATE, r.choice([0, 0, 0, 0, 1, 0x88760064]))
    blob(log, BLTFAST, 4, 0xf)           # (the source rectangle)
    set_return(log, BLTFAST, r.choice([0, 0, 0, 0, LOST, 1]))
    blob(log, SETKEY, 2, 3)              # (the DDCOLORKEY: the stack)
    set_return(log, ISLOST, r.choice([0, LOST]))
    set_return(log, RESTORE, r.choice([0, 1]))
    c = cursor(r)
    pos = struct.unpack_from('<2i', c, 0x48) if r.random() < 0.3 else (r.randrange(-8, 80), r.randrange(-8, 80))
    regions = {LOG: log, GUARD: struct.pack('<I', r.choice([0, 1, r.getrandbits(32)])) + r.bytes(4) + c,
               OBJS: struct.pack('<4I', VT, 0, 0, 0) * 6, VT: vtable({}, com={RELEASE: 1, BLTFAST: 6, DESC: 2, ISLOST: 1, RESTORE: 1, SETKEY: 3}),
               DD: struct.pack('<I', VTD) + r.bytes(0xc), VTD: vtable({}, com={CREATE: 4}), 0xfa76b0: struct.pack('<I', DD) + r.bytes(4),
               TARGET: surface(r, TOBJ), IMAGE: surface(r, IOBJ), OUT: r.bytes(0x28), POS: struct.pack('<2i', *pos), RECT: rect(r),
               HOT: struct.pack('<2i', r.randrange(-4, 20), r.randrange(-4, 20)), KEY: r.bytes(8), BITS: r.bytes(4), 0x775874: r.bytes(4)}
    address = CALLS[op]
    target = r.choice([TARGET, TARGET, 0])
    if op == 'GetInfo':
        return Call(address, [OUT, r.choice([IMAGE, TARGET, C + 0x54])], regions, conv='c', ret='a')
    if op == 'Get':
        return Call(address, [], regions, conv='c', ret='a')
    if op == 'AtExit':
        return Call(address, [], regions, conv='c', ret='v')
    if op == 'DeletingDtor':
        regions[HEAP] = heap()
        return Call(address, [C, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op in ('Ctor', 'Dtor', 'Discard', 'Flip', 'ReleaseBuffers', 'SaveUnder', 'EraseShown', 'Paint'):
        return Call(address, [C], regions, conv='t', ret='b' if op in ('SaveUnder', 'EraseShown', 'Paint') else 'a' if op == 'Ctor' else 'v')
    if op == 'Draw':
        return Call(address, [C, POS, target], regions, conv='t', ret='v')
    if op in ('CreateBuffers', 'ClearImage'):
        return Call(address, [C, OUT], regions, conv='t', ret='a')
    if op == 'SetImage':
        return Call(address, [C, OUT, r.choice([IMAGE, IMAGE, IMAGE, 0]), RECT, HOT], regions, conv='t', ret='a')
    if op == 'Move':
        return Call(address, [C, POS], regions, conv='t', ret='v')
    if op == 'SetKey':
        return Call(address, [C, KEY], regions, conv='t', ret='v')
    byte = lambda: r.choice([0, 1, 0x100, r.getrandbits(32)])  # noqa: E731
    if op == 'Update':
        return Call(address, [C, POS, r.getrandbits(32), target, byte(), byte()], regions, conv='t', ret='v')
    return Call(address, [C, target, r.choice([TARGET, IMAGE]), byte(), byte()], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
