# The Direct3D driver probes (src/game/render/d3dprobe) against the originals: a probe whose virtual base (at a random offset from its base table) has a
# vtable of recorders (slot 2 flags, 3 clear, 0x10 the read back: its pixel out logged as 0 and filled, 0x11 draw) and a device of COM recorders
# (SetRenderState 22, SetTextureStageState 40); SetFlags runs the Shadow_ setters (vregs) too; Stages a mode and two textures; the caps bits at [0x879da8] + 0xa4 / + 0xa8, the render caps 0x8bcbc8 and the pixel mask 0x8bd4dc random,
# the pixel 0, the mask, or random bits; the read back fails now and then. Draw (0x567130, on the probe + 0x2c): a size, colours and uvs of any
# bits (floats mostly, NaNs now and then), its vertices logged through DrawPrimitive's (COM slot 28) pointer as their 24 dwords.
# Gap: one fill answers both of the fog test's reads, so its second read never fails nor differs from the first.
# The renderer's drawing (on the probe + 0x2c; ops from Begin on): device results from a list (0 mostly; failures run the engine debug log's and error list's real code, the log's C library calls
# recorders as in enginedebug.py, the error list's debug flag 0x8bd508 off as engineerrors.py covers it), vertices and indices logged by their contents (blob_array), the clock (FillRate) a recorder.
# usage: d3dprobe.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, blob_array, fill, log_region, put, recorder, set_float_return, set_return, strings, vtable  # noqa: E402
import enginedebug  # noqa: E402

CALLS = {'AlphaTest': (0x567270, 0x5673c5), 'FogTest': (0x5673d0, 0x5675b4), 'SetFlags': (0x567620, 0x5679ac), 'Stages': (0x5686d0, 0x568aaf), 'Draw': (0x567130, 0x567269),
         'Begin': (0x566fa0, 0x567010), 'End': (0x567010, 0x567060), 'ReadPixel': (0x567060, 0x567130), 'SetTexture': (0x5675c0, 0x567620), 'SetFog': (0x567ad0, 0x567c10),
         'TestQuad': (0x568ac0, 0x568c50), 'Quad': (0x568c50, 0x568f50), 'Lines': (0x568f50, 0x569040), 'Frame': (0x569040, 0x569410), 'Box': (0x569410, 0x569720), 'FillRate': (0x569720, 0x569c70),
         'Tinted': (0x567c10, 0x5680a8)}
TICKS, TRANSFORM = 15, 6   # (driver.cpp's kD3dProbeEnv; the transform's slot)
B = Bench(list(CALLS.values()) + [(0x5679b0, 0x567aca), (0x5832e0, 0x583360), (0x56a770, 0x56ae60), (0x56af20, 0x56af9c), (0x5826f0, 0x582704), enginedebug.RANGE],   # (the Shadow_ setters SetFlags calls, what the drawing calls)
          hooks={**enginedebug.HOOKS, 0x57cb90: _recorder(TICKS, 0, cdecl=True)})
PROBE, VBT, VT, DEV, DVT, CAPS = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600
ARG, ERRORS, TAIL, TABLE = RAM + 0x800, 0x879ad0, 0x8bd470, RAM + 0xa00   # (ARG: a call's array, a pixel buffer ...)
OBJ, OBJVT, OUT, DRAWN = RAM + 0x900, RAM + 0x940, RAM + 0x9e0, RAM + 0xb00
BATCH, DBUF, INDICES, SOURCE, FREE = RAM + 0xc00, RAM + 0xc80, RAM + 0xd00, RAM + 0x1010, RAM + 0x1800
GONE = 0x887601c2
SELF = PROBE + 0x2c


def hresult(r):
    return r.choice([0, 0, 0, 0, 1, 0x88760000 + r.randrange(0x300), 0x80004005, 0x80070057, 0xb0754002, r.getrandbits(32), GONE])


def tinted(r, probe, off, log, regions, hr):
    """0x567c10, on the probe itself: a batch of 0-8 vertices, the draw buffer's free vertices elsewhere, the batch's own or overlapping them; the flags the three kinds, 0x22, 0x52 or any."""
    fvf = r.choice([0x1c4, 0x1c4, 0xc4, 0x2c4, 0x3c4, r.getrandbits(32) & 0xfff])
    stride = {0x1c4: 0x20, 0xc4: 0x18, 0x2c4: 0x28, 0x3c4: 0x30}.get(fvf, 0)
    count = r.choice([0, 1, 3, 4, 8, r.randrange(9)])
    source = SOURCE + r.choice([0, 0, 1, 2, 3])
    free = r.choice([FREE, FREE + r.randrange(4), source, source + r.choice([4, 8, stride]), source - r.choice([4, 8])])
    flags = r.getrandbits(32)
    kind = r.randrange(5)
    if kind == 0:
        flags |= 0x556
    elif kind == 1:
        flags = flags & ~0x400 | 0x2356
    elif kind == 2:
        flags = (flags & ~0xff) | r.choice([0x22, 0x52])
    if r.random() < 0.7:
        flags |= 0x100
    batch = bytearray(r.bytes(0x30))
    put(batch, 4, 'I', flags)
    put(batch, 0x18, 'II', r.randrange(8), fvf)
    put(batch, 0x20, 'II', count, r.randrange(0x40))
    put(batch, 0x28, 'II', source, INDICES)
    stream = r.randrange(1, 4)   # (0x5826f0's: +4 the current stream, of 0x18 bytes each from -0x10: its start and what is used)
    used = r.choice([0, 0x20, 0x100])
    buffer = bytearray(r.bytes(0x80))
    put(buffer, 4, 'I', stream)
    put(buffer, 0x18 * stream - 0x10, 'II', free - used & 0xffffffff, used)
    if r.random() < 0.4:
        put(probe, off + 0x14, 'B', 0x91)
    put(probe, 4, 'B', r.getrandbits(8))
    set_return(log, 29, hr)
    blob_array(log, 29, 3, 4, stride or 4)
    regions.update({BATCH: batch, DBUF: buffer, 0x8bcbc4: struct.pack('<I', DBUF), SOURCE - 0x10: r.bytes(0x400), FREE: r.bytes(0x400), INDICES: r.bytes(0x80)})
    return Call(CALLS['Tinted'][0], [PROBE, BATCH], regions, conv='t', ret='a')


def drawing(r, name, probe, off, log, regions):
    enginedebug.texts(log)
    if r.random() < 0.3:
        set_return(log, enginedebug.FOPEN, 0)
    list_ = bytearray(r.bytes(0x204))   # (the result 0x879cd4 next after it)
    put(list_, 0x200, 'I', r.choice([0, 1, 0x3f, 0x40, r.randrange(0x40)]))
    regions.update({0x879cd4: r.bytes(4), 0x879a2c: bytearray(struct.pack('<I', r.choice([0, 1]))), ERRORS: list_, 0x8bd508: bytearray(4), 0x8bcbc8: r.bytes(4)})
    shadow = r.getrandbits(32)
    if r.random() < 0.3:
        shadow = (shadow & ~0xff) | r.choice([0x56, 0x56, 0xd6])
    put(probe, off + 0x14, 'I', r.choice([shadow, shadow & 0xffff7bff, shadow | 0x108f00]))
    put(probe, 8, 'I', r.randrange(4))
    hr = hresult(r)
    blob_array(log, 29, 3, 4, 0x20)
    blob_array(log, 29, 5, 6, 2, which=1)
    wh = lambda: r.choice([640, 800, 1024, 480, 600, r.randrange(1, 3000), 0x1000001, 0x7fffffff, 0x80000000, r.getrandbits(32), 0])  # noqa: E731
    if name == 'Begin':
        set_return(log, 9, hr)
        regions[0x8bd4a8] = r.bytes(4)
        return Call(CALLS[name][0], [SELF], regions, conv='t', ret='a')
    if name == 'End':
        set_return(log, 10, hr)
        return Call(CALLS[name][0], [SELF], regions, conv='t', ret='a')
    if name == 'ReadPixel':
        pitch, pixels = r.choice([0x20, 0x40]), ARG
        set_return(log, 25, hr)
        fill(log, 25, 2, struct.pack('<5I', pitch, 0, 0, 0, 0) + struct.pack('<I', pixels), offset=0x10)
        blob(log, 25, 2, (1 << 31) - 1)
        regions.update({0x879a24: bytearray(struct.pack('<I', OBJ)), OBJ: bytearray(struct.pack('<9I', 0, 0, 0, 0, 0, OBJ + 0x20, 0, 0, OBJVT)), OBJVT: vtable({}, size=33, com={25: 5, 32: 2}),
                        ARG: r.bytes(0x100), OUT: r.bytes(4), 0x8bcbe4: bytearray(struct.pack('<I', r.choice([16, 16, 32, 32, 24, 8]))), 0x8bd4dc: r.bytes(4)})
        return Call(CALLS[name][0], [SELF, r.randrange(8), r.randrange(4), OUT], regions, conv='t', ret='a')
    if name == 'SetTexture':
        texture = r.choice([struct.unpack_from('<I', probe, 0x20)[0], r.getrandbits(32), 0])
        set_return(log, 38, hr)
        return Call(CALLS[name][0], [SELF, r.randrange(8), texture], regions, conv='t', ret='a')
    if name == 'SetFog':
        regions.update({TAIL: r.bytes(0x40), 0x8bcbc8: struct.pack('<I', r.getrandbits(32) & ~0xc000 | r.choice([0, 0, 0x4000, 0x8000, 0xc000]))})
        start, end = r.f(r.choice([1, 1000])), r.f(r.choice([1, 1000]))
        return Call(CALLS[name][0], [SELF, start, r.choice([end, start, r.f(1000)]), r.getrandbits(32)], regions, conv='t', ret='a')
    if name == 'TestQuad':
        return Call(CALLS[name][0], [SELF], regions, conv='t', ret='a')
    if name == 'Quad':
        set_return(log, 29, hr)
        regions.update({OBJ: bytearray(struct.pack('<I', OBJVT)), OBJVT: vtable({}, size=3)})
        colour = r.getrandbits(32)
        colour = r.choice([colour, colour | 0xff000000])
        return Call(CALLS[name][0], [SELF, r.f(1000), r.f(1000), r.f(1000), r.f(1000), colour, r.choice([0, OBJ])], regions, conv='t', ret='a')
    if name == 'Lines':
        count = r.choice([0, 1, 2, 3, 8, 15, 16, r.randrange(1, 17), 0xffffffff])
        points = bytearray(0x80)
        for i in range(0x20):
            put(points, 4 * i, 'I', r.f(r.choice([1, 1000])))
        set_return(log, 28, hr)
        blob_array(log, 28, 3, 4, 0x20, which=2)
        regions[ARG] = points
        return Call(CALLS[name][0], [SELF, ARG, count, r.getrandbits(32), r.f(r.choice([1, 1000]))], regions, conv='t', ret='a')
    if name == 'Tinted':
        return tinted(r, probe, off, log, regions, hr)
    regions[0x8bcbd0] = bytearray(struct.pack('<2I', wh(), wh()))
    if name == 'Frame':
        set_return(log, 29, hr)
        regions[0x8bcbc8] = struct.pack('<I', r.getrandbits(32))
        return Call(CALLS[name][0], [SELF], regions, conv='t', ret='a')
    if name == 'Box':
        set_return(log, 29, hr)
        corners = bytearray(0x80)
        for i in range(0x20):
            put(corners, 4 * i, 'I', r.f(1000))
        set_float_return(log, TRANSFORM, 1.5)
        blob_array(log, TRANSFORM, 1, 3, 0x20, which=2)
        regions.update({ARG: corners, 0x87b1d8: bytearray(struct.pack('<I', OBJ)), OBJ: bytearray(struct.pack('<I', OBJVT)),
                        OBJVT: bytearray(struct.pack('<4I', 0, 0, 0, recorder(TRANSFORM, 5, cdecl=True)))})
        return Call(CALLS[name][0], [SELF, ARG, r.getrandbits(32), r.getrandbits(32) >> r.choice([0, 26])], regions, conv='t', ret='a')
    # FillRate
    if r.random() < 0.2:
        set_return(log, 7, 0)
    elif r.random() < 0.2:
        set_return(log, 8, 0)
    regions[0x879cd4] = r.choice([bytearray(4), bytearray(4), bytearray(4), bytearray(r.bytes(4))])
    begun = r.getrandbits(32)
    regions[TABLE] = bytearray(struct.pack('<2I', begun, (begun + r.choice([0, 0x10000000, 1, r.randrange(1 << 20), r.getrandbits(32)])) & 0xffffffff) + bytes(38 * 4))
    answers(log, TICKS, TABLE)
    regions[DRAWN] = struct.pack('<3I', *r.choice([(0, 0, 0), (0, 0, 0), (0, 0, 0), (hr, 0, 0), (0, hr, 0), (0, 0, hr)])) + bytes(37 * 4)   # (drawing's answers: a failure leaves the loop)
    answers(log, 29, DRAWN)
    return Call(CALLS[name][0], [SELF], regions, conv='t', ret='d')


def case(r):
    name = r.deal(OPS)
    probe = bytearray(r.bytes(0x100))
    off = r.choice([0x30, 0x40, 0x80])
    put(probe, 0, 'I', VBT)
    put(probe, off, 'I', VT)
    put(probe, 0x24, 'I', DEV)
    log = log_region()
    strings(log, 16, 0, out=0b100)
    if name not in ('AlphaTest', 'FogTest', 'SetFlags', 'Stages', 'Draw'):
        regions = {PROBE: probe, VBT: bytearray(struct.pack('<II', r.getrandbits(32), off)), VT: vtable({2: 1, 3: 2, 16: 3, 17: 7}, size=24),
                   DEV: bytearray(struct.pack('<I', DVT) + r.bytes(0x1c)), DVT: vtable({}, size=48, com={9: 1, 10: 1, 22: 3, 28: 6, 29: 8, 38: 3, 40: 4}), LOG: log}
        return drawing(r, name, probe, off, log, regions)
    mask = r.choice([0xffffff, 0xffff, 0xf800, r.getrandbits(32)])
    fill(log, 16, 2, struct.pack('<I', r.choice([0, mask, r.getrandbits(32), r.getrandbits(32) & mask, mask | 0x1000000])))
    if r.random() < 0.25:
        set_return(log, 16, 0)
    caps = bytearray(r.bytes(0x100))
    if r.random() < 0.7:
        put(caps, 0xa4, 'B', caps[0xa4] | 0x40)
        put(caps, 0xa8, 'I', struct.unpack_from('<I', caps, 0xa8)[0] | 0x80000)
    regions = {PROBE: probe, VBT: bytearray(struct.pack('<II', r.getrandbits(32), off)), VT: vtable({2: 1, 3: 2, 16: 3, 17: 7}, size=24),
               DEV: bytearray(struct.pack('<I', DVT) + r.bytes(0x1c)), DVT: vtable({}, size=48, com={22: 3, 28: 6, 40: 4}), LOG: log,
               CAPS: caps, 0x879da8: bytearray(struct.pack('<I', CAPS)), 0x8bcbc8: r.bytes(4), 0x8bd4dc: bytearray(struct.pack('<I', mask))}
    if name == 'Draw':
        blob(log, 28, 3, 0xffffff)
        nan = lambda: r.getrandbits(1) << 31 | 0x7f800000 | r.randrange(1, 1 << 23)  # noqa: E731
        floats = [nan() if r.random() < 0.1 else r.f(r.choice([1, 20, 1000])) for _ in range(5)]
        args = [r.choice([0x41a00000, floats[0]]), r.getrandbits(32), r.getrandbits(32)] + floats[1:]
        return Call(CALLS[name][0], [PROBE + 0x2c] + args, regions, conv='t', ret='a')
    if name == 'SetFlags':   # on the probe + 0x2c; the flags set last differ in a few bits, or are random
        put(probe, 8, 'I', r.choice([0, 1, 2, 3]))
        flags = r.getrandbits(32)
        if r.random() < 0.3:
            flags = (flags & ~0xff) | r.choice([0x52, 0x22, 0xd2])
        last = r.choice([flags ^ (r.getrandbits(32) & r.getrandbits(32) & r.getrandbits(32)), flags ^ (r.getrandbits(32) & 0x75fce00), flags, flags ^ r.choice([1, 0x80, 0x100]),
                         r.getrandbits(32)])   # (only the bits it sets differing: the flags set last end equal)
        put(probe, off + 0x14, 'I', last)
        regions[0x8bcbc8] = struct.pack('<I', r.getrandbits(32))
        return Call(CALLS[name][0], [PROBE + 0x2c, flags], regions, conv='t', ret='a')
    if name == 'Stages':   # a mode (0-2, or another) and the textures (+8, +0xc); the probe's +0x18 is mostly 4
        put(probe, 0x18, 'I', r.choice([4, 4, 4, 4, 3, 0]))
        last = r.getrandbits(32)
        if r.random() < 0.4:
            last = (last & ~0xff) | 0x93
        put(probe, off + 0x14, 'I', last)
        regions[RAM + 0x700] = r.bytes(0x20)
        return Call(CALLS[name][0], [PROBE, r.choice([0, 1, 2, 0, 1, 2, 3, 0xffffffff]), RAM + 0x700], regions, conv='t')
    return Call(CALLS[name][0], [PROBE], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
