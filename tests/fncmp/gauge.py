# The framed gauge's plain members (src/game/gfx/gauge: 0x47faa0-0x47fbe0) against the
# original: init, marks, frame and colours on a gauge of random bytes; and its draws (0x47fc00, 0x47fc50) through the display front end
# and the 8-bit primitives on a 64-pixel-pitch surface: frames and marks in and around the clip rectangle (the first mark's y often
# negative), plain, remapped (0x20) or blended (8, 0x10) drawing. Setup (0x4806a0) on a window rectangle of random s16s with the screen
# scales 0xfaa5c0 / 0xfaa5c4 and a random game palette (0xfa71f8). The grid (0x47fcd0) in guitext.py's world (the lines to the draw
# context's recorders, the labels through GuiText_Draw; swprintf crt.py's hook): 12, 36 or 144 columns (another count leaves the
# original's step as the stack lies), none or negative; the labels' wide text narrowed through a table of the digits (each a lead byte,
# so a label of one or two digits is one record) or, for 144 columns (labels of three digits), the global buffer's bytes as the case
# writes them (no table); each label's record masked as guitext.py's one (its block: every label's layout allocates three). InWindow
# (0x480810): see in_window().
# usage: gauge.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import crt  # noqa: E402
import guitext  # noqa: E402
import textlayout as tl  # noqa: E402
from fncmp import HEAP, LOG, LOG_BYTES, Bench, Call, Mask, RAM, blob, fill, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


RANGES = merged([(0x47faa0, 0x47fbf3),  # 0x47fbe0's ret 8 is at 0x47fbf0
                  (0x47fc00, 0x47fc50), (0x47fc50, 0x47fcd0), (0x47fcd0, 0x47fe85), (0x5f9780, 0x5f97c0), (0x5f9860, 0x5f98d0),
                  (0x5f98d0, 0x5f9940), (0x5f9a80, 0x5f9ac0), (0x61af40, 0x61af50), (0x61af50, 0x61af60), (0x61c2e0, 0x61c490),
                  (0x61c490, 0x61c630), (0x61eed0, 0x61f0a0), (0x62f810, 0x62f890), (0x62f890, 0x62f8d0), (0x62f940, 0x62f980),
                  (0x4806a0, 0x480810), (0x5f73a0, 0x5f76b7),
                  (0x480810, 0x4808ec), (0x4925d0, 0x492660), (0x5f9a80, 0x5f9da0), (0x61af60, 0x61b040), (0x61b090, 0x61b140),
                  (0x62f890, 0x62f8e0), (0x62f940, 0x62f990), (0x65d6db, 0x65d727), (0x65d745, 0x65d783)] + guitext.RANGES)
B = Bench(RANGES, hooks={0x67af00: crt.swprintf_hook})
CALLS = {'Init': (0x47faa0, 0), 'MarksX': (0x47fb10, 3), 'MarksY': (0x47fb30, 3), 'Rect': (0x47fb50, 4),
         'Colour0': (0x47fb80, 2), 'Colour1': (0x47fba0, 2), 'Colour2': (0x47fbc0, 2), 'Colour3': (0x47fbe0, 2),
         'DrawBack': (0x47fc00, 0), 'DrawAxes': (0x47fc50, 0), 'Setup': (0x4806a0, 6), 'Grid': (0x47fcd0, 0), 'InWindow': (0x480810, 3)}
WINDOW, IMPL, IMPL_VT = RAM + 0x4200, RAM + 0x4300, RAM + 0x4340   # (InWindow's window, its surface object and that one's vtable)
FORMAT = 0x8bd554   # (the current format's index, then its records of 32 bytes)
GRID, NARROWER, SEGMENT = RAM + 0x4000, RAM + 0x4100, RAM + 0x4140   # the grid's gauge, the narrowing table and its one segment
LABEL_BLOCKS = 0x30   # (each label's layout: its records, positions and lines blocks)
STATE, TABLE8 = 0xfa76b8, 0x7034d8
SURFACE, BITS, REMAP, GHOST = RAM + 0x100, RAM + 0x1000, RAM + 0x200, RAM + 0x10000
PITCH, ROWS = 64, 56


def coord(r, top):
    return r.choice([r.randrange(-20, top + 20)] * 2 + [r.randrange(top)] * 6 + [-1, 0, top, r.getrandbits(32) - (1 << 31)])


def draw(r, at):
    """A gauge's draw: its frame and marks and colours random-ish, the draw context's clip inside the surface, its flags."""
    gauge = bytearray(r.bytes(0x50))
    put(gauge, 4, 'i', r.choice([0, 1, 7, 30, -5, r.getrandbits(32) - (1 << 31)]))
    xs, ys = [coord(r, 60), coord(r, 60)], [coord(r, 48), coord(r, 48)]
    if r.random() < 0.8:   # in order (left <= right, top <= bottom), as the game sets them
        xs.sort()
        ys.sort()
    put(gauge, 8, 'iiii', xs[0], ys[0], xs[1], ys[1])
    put(gauge, 0x1c, 'i', r.choice([-1, -5, -30, 0, 3, coord(r, 48)]))
    put(gauge, 0x24, 'i', r.choice([0, 5, 20, -40, -1, coord(r, 48)]))
    left, top = r.randrange(0, 40), r.randrange(0, 30)
    clip = r.choice([(0, 0, 60, 48), (left, top, r.randrange(left, 61), r.randrange(top, 49))])
    state = bytearray(r.bytes(0x74))
    put(state, 0, 'I', REMAP)
    put(state, 8, 'iiii', *clip)
    put(state, 0x18, 'I', STATE + 0x38)
    put(state, 0x30, 'I', SURFACE)
    put(state, 0x38, 'II', TABLE8, r.choice([0, 0, 0, 0x20, 8, 0x10, 4]))
    put(state, 0x4c, 'I', BITS)
    put(state, 0x70, 'I', GHOST)
    surface = put(bytearray(r.bytes(0x40)), 0x24, 'I', PITCH)
    put(surface, 0, 'II', PITCH, ROWS)
    regions = {RAM: gauge, STATE: state, SURFACE: surface, BITS: r.bytes(PITCH * ROWS), REMAP: r.bytes(0x100), GHOST: r.bytes(0x10000)}
    return Call(at, [RAM], regions, conv='t', ret='v')


def grid(r):
    """The grid (0x47fcd0) in guitext.py's world: a gauge of random bytes but for its frame, sizes and counts; the labels' font (entry
    [0x74f9d4] 10) often none, so that the lines' log entries come before the log's room runs out."""
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x800)}
    lang, table, codes, lead = guitext.world(r, log, regions)
    gauge = bytearray(r.bytes(0x50))
    xs, ys = sorted(r.randrange(-50, 700) for _ in range(2)), sorted(r.randrange(-50, 500) for _ in range(2))
    put(gauge, 8, '4i', xs[0], ys[0], xs[1], ys[1])
    put(gauge, 0, '2i', xs[1] - xs[0], ys[1] - ys[0])
    if r.random() < 0.2:
        put(gauge, r.choice([0, 4, 8, 0xc, 0x10, 0x14]), 'I', r.getrandbits(32))
    columns = r.choice([0xc, 0x24, 0x90] * 3 + [0, -3, -0x90])
    rows = r.choice([r.randrange(1, 9)] * 4 + [0, -2, r.randrange(9, 30)])
    put(gauge, 0x28, '2i', columns, rows)
    regions[GRID] = gauge
    font = r.choice([tl.FONTS, tl.FONTS + 0x40, 0, 0])
    put(table, 24 * (10 + 13 * lang) + 0x14, 'I', font)
    text = bytearray(r.bytes(0x10))
    if columns == 0x90 or r.random() < 0.2:   # no narrowing table: the buffer's bytes, at most one record
        text[0] = r.choice([0, r.randrange(1, 0x100), 0x20, min(lead + 1, 0xff), max(lead, 1)])
        if text[0] and text[0] - 1 >= lead:
            text[1] = r.choice([r.randrange(1, 0x100), 0])
            if text[1]:
                text[2] = 0
            else:
                text[2:4] = b'\0\0'   # (the terminator taken along: then the next byte ends it)
        else:
            text[1] = 0
        n = tl.text_records(text, lead)
    else:   # the digits narrowed to lead bytes: a label of one or two digits one record
        lead = r.choice([0x80, 0x40, 0, r.randrange(0, 0xf0)])
        codes_bytes = bytearray(regions[tl.CODES])
        codes_bytes[8] = lead
        regions[tl.CODES] = codes_bytes
        regions[guitext.NARROW] = struct.pack('<I', NARROWER)
        regions[NARROWER] = r.bytes(8) + struct.pack('<I', SEGMENT) + r.bytes(4)
        regions[SEGMENT] = struct.pack('<4H10H', 0x30, 0, 0x39, 0, *[r.randrange(lead + 1, 0x100) for _ in range(10)])
        text[2] = 0
        n = 1
    regions[guitext.BUFFER] = text
    masks = []
    if columns > 0 and font and codes and n == 1:
        masks.append(Mask(HEAP, [0x10 + LABEL_BLOCKS * k + g for k in range(12) for g in tl.GARBAGE], "SetText (0x6ae930) never sets a record's bytes "
                          "+5-+7 and +0xe-+0xf (the original's stack garbage); each label's one record (its layout's first block) is read "
                          "only at +0, +2, +4, +8 and +0xc, and no glyph is right to left, so it is never moved (as in guitext.py)"))
    return Call(0x47fcd0, [GRID], regions, conv='t', ret='v', masks=masks)


def in_window(r):
    """InWindow (0x480810): grid()'s world and gauge (the draw table's slot 3, the box fill, given its three arguments, the rectangle on the
    stack logged as a hash of its four dwords, the colour's index dword as its byte: DrawBack passes its caller's stack as the pad); the window's surface object +0x9c (none now and then) with slot 7 (k 21) answering the
    bits (0 now and then) and writing the width and height (0-64 mostly) through its two out arguments (logged as 0), slot 6 (k 20); the
    current format (index 0-3) a record whose bit count is 8, 16, 24, 32 or other; the game palette random; after a cdecl recorder (k 22)
    or none. The display's surface is left the stack's SurfaceInfo (STATE +0x30: ignored, the two stacks differ)."""
    c = grid(r)
    regions, log = dict(c.regions), bytearray(c.regions[LOG])
    regions[guitext.DTABLE] = vtable({3: 3, 8: 5, 9: 5}, 16)
    blob(log, 3, 0, 0xf)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 3, 1 << 2)   # (the colour's index logged as its byte: DrawBack's pad bytes are its caller's stack)
    vt = bytearray(0x20)
    put(vt, 0x18, 'II', recorder(20, 0), recorder(21, 2))
    impl = put(bytearray(r.bytes(0x10)), 0, 'I', IMPL_VT)
    window = put(bytearray(r.bytes(0xa0)), 0x9c, 'I', r.choice([IMPL] * 9 + [0]))
    set_return(log, 21, r.choice([RAM + 0x6000] * 5 + [r.getrandbits(32) | 0x80000000, 0]))
    set_return(log, 20, r.getrandbits(32))
    strings(log, 21, 0, out=0b11)
    fill(log, 21, 0, struct.pack('<i', r.choice([r.randrange(0, 65)] * 4 + [r.randrange(-5, 300)])), which=4)
    fill(log, 21, 1, struct.pack('<i', r.choice([r.randrange(0, 57)] * 4 + [r.randrange(-5, 300)])), which=5)
    index = r.randrange(4)
    records = bytearray(r.bytes(32 * 4))
    put(records, 32 * index + 12, 'I', r.choice([8, 8, 16, 24, 32, r.getrandbits(8)]))
    regions.update({LOG: log, WINDOW: window, IMPL: impl, IMPL_VT: vt, FORMAT: struct.pack('<I', index) + records,
                    0xfa71f8: r.bytes(0x400)})
    after = r.choice([recorder(22, 1, cdecl=True), 0])
    return Call(0x480810, [WINDOW, GRID, after], regions, ret='a', masks=c.masks, ignore=[(guitext.STATE + 0x30, 4)])


def case(r):
    op = r.deal(OPS)
    at, n = CALLS[op]
    if op == 'Grid':
        return grid(r)
    if op == 'InWindow':
        return in_window(r)
    if op.startswith('Draw'):
        return draw(r, at)
    if op == 'Setup':   # (the window's corners mostly in order and on screen; the scales the screen's size or wild)
        window = bytearray(r.bytes(0x10))
        xs, ys = sorted(r.randrange(-100, 800) for _ in range(2)), sorted(r.randrange(-100, 600) for _ in range(2))
        if r.random() < 0.2:
            xs, ys = [r.getrandbits(16) for _ in range(2)], [r.getrandbits(16) for _ in range(2)]
        put(window, 4, 'HHHH', xs[0] & 0xffff, ys[0] & 0xffff, xs[1] & 0xffff, ys[1] & 0xffff)
        scales = r.choice([(480, 640), (600, 800), (768, 1024), (r.getrandbits(16), r.getrandbits(16))])
        regions = {RAM: bytearray(r.bytes(0x50)), RAM + 0x100: window, 0xfaa5c0: struct.pack('<HHHH', scales[0], r.getrandbits(16), scales[1], r.getrandbits(16)),
                   0xfa71f8: r.bytes(0x400)}
        args = [r.choice([r.randrange(-50, 700), r.getrandbits(32)]) & 0xffffffff for _ in range(n)]
        return Call(at, [RAM, RAM + 0x100] + args, regions, conv='c', ret='v')
    args = [r.choice([r.randrange(-50, 700), r.getrandbits(32)]) & 0xffffffff for _ in range(n)]
    return Call(at, [RAM] + args, {RAM: bytearray(r.bytes(0x50))}, conv='t', ret='a' if op == 'Init' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
