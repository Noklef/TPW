# The GUI library's list window drawn (src/game/ui/window705950draw: 0x47de30, its vtable 0x705950's slot 4) against the original: the
# window's rectangles +0x2c and +0x138 any words (small mostly), the first row shown, selection, row height and rows shown small (now and then
# negative or 0), a forest of 0-12 nodes as window705950.py's (first child +0x48, next +0x44, parent +0x4c; at most 6 deep: deeper the
# indent runs into the line, sic) from +0x144 with the count +0x14c theirs or fewer, their names wide strings at +0 (ASCII mostly, now and
# then up to 0xff; past it the CRT's wctomb sets errno through its thread data); the GUI's scale words [0xfaa5c0] / [0xfaa5c4] small or any; the palette (0xfa71f8) holding the five colours asked for
# early mostly (else the whole of it is searched); the indent's format text (0x786de4) short; the font table (0x7b059c) with font 1 or not,
# the text driver before none; display.py's draw context (0xfa76b8, its window and clip in order) with a driver of thiscall recorders for
# the fill (slot 3, its rectangle logged by its four dwords) and the glyphs' sprite shapes (slot 15; textdriver.py's font of random glyph sizes), the colours' index logged as its byte
# (the rest of its dword is what the stack held). wctomb's InterlockedIncrement / Decrement (on 0xfb3ca8, its lock not taken) are the
# emulator's own stand-ins (INTERLOCKED), not recorders: the driver's sprintf is crt.py's stand-in (the host's), which never calls them.
# usage: window705950draw.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import IMPORT_TABLE, LOG, LOG_BYTES, RAM, STUBS, Bench, Call, blob, log_region, put, vtable  # noqa: E402
import display  # noqa: E402
from gamestring import LOCALE  # noqa: E402

INTERLOCKED = {STUBS + 0x3c0: (0x6fd0c0, 1), STUBS + 0x3d0: (0x6fd12c, -1)}   # (InterlockedIncrement / Decrement: the IAT slot, the step)


def interlocked(step):
    def hook(e):   # stdcall (address): the dword stepped, the new value returned
        sp = e.r['ESP'] + (4 if e.tail else 0)
        p = e.u32(sp)
        v = (e.u32(p) + step) & 0xffffffff
        e.write(p, struct.pack('<I', v))
        e.r['EAX'] = v
        if e.tail:
            ret = e.u32(e.r['ESP'])
            e.r['ESP'] += 4
            e.write(e.r['ESP'], struct.pack('<I', ret))
        else:
            e.r['ESP'] += 4
    return hook


B = Bench([(0x47de30, 0x47e246), (0x659de5, 0x659e3e), (0x65ada4, 0x65ae3c), (0x5f7360, 0x5f76b7), (0x46c010, 0x46c084), (0x5f8db0, 0x5f8f71),
           (0x61b030, 0x61b090), (0x5f9780, 0x5f9eb0), (0x61af60, 0x61b030), (0x61b090, 0x61b1f0), (0x62f890, 0x62f8e0), (0x62f940, 0x62f990),
           (0x67a400, 0x690000)], hooks={at: interlocked(step) for at, (_, step) in INTERLOCKED.items()})
WINDOW, NODES, FONTS, FONTPTR, TABLE, INFO = RAM, RAM + 0x1000, RAM + 0x400, RAM + 0x480, RAM + 0x500, RAM + 0x600
FONT_AT, FONT = RAM + 0x2000, RAM + 0x2100   # (the font's glyphs from ' ' at FONT, the 32 before)
STATE, PALETTE, FORMAT = 0xfa76b8, 0xfa71f8, 0x786de4
FILL, SHAPE = 3, 15
COLOURS = [(0, 0, 0), (0xff, 0xff, 0xff), (0x80, 0x80, 0x80), (200, 0, 0)]


def word(r):
    return r.choice([r.randrange(0, 200), r.randrange(0, 200), r.randrange(-10, 10), r.getrandbits(16)]) & 0xffff


def forest(r, regions):
    """0-12 nodes of 0x50 bytes in depth-first order, at most 6 deep, each named; returns their addresses."""
    n = r.choice([0, 1, 2, 4, 7, 12])
    addr = [NODES + 0x50 * i for i in range(n)]
    bufs = []
    for _ in range(n):
        b = bytearray(r.bytes(0x50))
        name = [r.choice([r.randrange(0x20, 0x7f)] * 6 + [r.randrange(1, 0x100)]) for _ in range(r.choice([0, 1, 3, 8, 16]))]
        put(b, 0, '%dH' % (len(name) + 1), *name, 0)
        put(b, 0x44, 'III', 0, 0, 0)
        bufs.append(b)
    path, last = [], [None]
    for i in range(n):
        depth = r.randrange(min(len(path), 6) + 1)
        while len(path) > depth:
            path.pop()
            last.pop()
        if path:
            put(bufs[i], 0x4c, 'I', addr[path[-1]])
            if last[-1] is None:
                put(bufs[path[-1]], 0x48, 'I', addr[i])
        if last[-1] is not None:
            put(bufs[last[-1]], 0x44, 'I', addr[i])
        last[-1] = i
        path.append(i)
        last.append(None)
    for i, b in enumerate(bufs):
        regions[addr[i]] = b
    return addr


def palette(r):
    p = bytearray(r.bytes(0x400))
    if r.random() < 0.9:   # (the colours asked for early: the search stops at an exact one)
        for i, c in enumerate(COLOURS):
            at = 4 * r.choice([i, r.randrange(8)])
            p[at:at + 3] = bytes(c)
    return p


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * FILL, 1 << 2)
    struct.pack_into('<I', log, LOG_BYTES + 4 * SHAPE, 1 << 4)
    blob(log, FILL, 0, 0xf)   # (the selection's rectangle: the window's clip, on the stack)
    imports = bytearray(0x400)
    for at, (slot, _) in INTERLOCKED.items():
        put(imports, slot - IMPORT_TABLE, 'I', at)
    regions = {LOG: log, LOCALE: bytes(4), IMPORT_TABLE: imports, 0xfb3ca4: bytes(8)}   # (no CRT lock: [0xfb3ca4] 0)
    nodes = forest(r, regions)
    w = bytearray(r.bytes(0x158))
    put(w, 0x30, '4H', *[word(r) for _ in range(4)])
    put(w, 0x13c, '4H', *[word(r) for _ in range(4)])
    put(w, 0x144, 'I', nodes[0] if nodes else r.choice([0, 0]))
    put(w, 0x148, 'i', r.choice([-1, r.randrange(-1, 14)]))
    put(w, 0x14c, 'i', r.choice([len(nodes)] * 3 + [r.randrange(len(nodes) + 1), -1]))
    put(w, 0x150, 'i', r.choice([0, 0, r.randrange(-2, 13)]))
    put(w, 0x154, 'hh', r.choice([8, 12, 16, 0, -4, r.randrange(1, 40)]), r.choice([0, 1, 3, 6, 13, -1, r.randrange(1, 20)]))
    regions[WINDOW] = w
    regions[0xfaa5c0] = struct.pack('<hhhh', r.choice([0x600, 0x600, 0x480, r.getrandbits(15)]), 0,
                                    r.choice([0x800, 0x800, 0x400, r.getrandbits(15)]), 0)
    regions[PALETTE] = palette(r)
    regions[FORMAT] = r.choice([b'', b'', b'>', b'ab ', b'  ']) + b'\0'
    regions[0x7b059c] = struct.pack('<I', r.choice([FONTS] * 9 + [0]))
    regions[FONTS] = put(bytearray(r.bytes(0x30)), 0x24, 'I', r.choice([FONTPTR] * 9 + [0]))
    regions[FONTPTR] = struct.pack('<I', FONT)
    regions[0xfa76a0] = struct.pack('<2I', 0, r.getrandbits(32))
    glyphs = bytearray(r.bytes(0x900))
    for i in range(0, 0x900, 8):
        put(glyphs, i + 4, 'HH', r.choice([0, 1, 5, 8, r.getrandbits(16)]), r.choice([0, 8, 12, r.getrandbits(16)]))
    regions[FONT_AT] = glyphs
    state = bytearray(r.bytes(0x74))
    xs, ys = sorted([r.randrange(-20, 100), r.randrange(-20, 100)]), sorted([r.randrange(-20, 100), r.randrange(-20, 100)])
    put(state, 8, '4i', xs[0], ys[0], xs[1], ys[1])
    put(state, 0x18, 'I', STATE + r.choice([0x38, 0x40, 0x44, 0x48]))
    put(state, 0x30, 'I', INFO)
    for at in (0x38, 0x40, 0x44, 0x48):
        put(state, at, 'I', TABLE)
    regions[STATE] = state
    regions[TABLE] = vtable({FILL: 3, SHAPE: 5}, 16)
    regions[INFO] = display.info(r)
    return Call(0x47de30, [WINDOW], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
