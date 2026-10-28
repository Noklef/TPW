# The GUI's text drawing and measuring (src/game/gfx/guitext: 0x492750, 0x4929f0, cdecl) against the originals, with the text layout's
# world from textlayout.py: the global style (0x78ba2c) and the language's font table (0x78b54c; the language 0x78b530: +4 its index, +8
# its id, 0x40d Hebrew) holding fonts with recorder vtables (slot 7 the widths, slot 8 the line height, slot 9 the flags, slot 10 the kind,
# slot 11 the code table), the renderers' table (0x782d38) a recorder whose slot 1 writes glyph rects; the draw context (0xfa76b8) and the
# GUI's SurfaceInfo (0x7ca0e8) as display.py makes them; the narrowing table (0x78ba80) none, so the text is the global buffer's bytes
# (0x7b5378) as the case writes them. The layout lives on the stack, so a text makes at most one record: SetText leaves a record's bytes
# +5-+7 and +0xe-+0xf as the stack lies (masked in the heap's first block, which a second record would copy). The alignments are 0-2:
# past them the layout's kinds (+0xa0, +0xa4, which TextLayout_Ctor leaves) are the stack's garbage. Rgb (0x492c40): the same world and
# text, the rectangle as four arguments, the surface's bits / width / height (a, c, d) any or usual, the rgb any, the palette random.
# usage: guitext.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import display  # noqa: E402
import textlayout as tl  # noqa: E402
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, Mask, echo, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

RANGES = ([(0x41a850, 0x41a870), (0x486780, 0x4867c0), (0x492650, 0x492740), (0x492750, 0x492d20), (0x5f73a0, 0x5f76c0), (0x5f9ac0, 0x5f9da0),
           (0x5fa680, 0x5fa6b0), (0x5fb770, 0x5fb7a0),
           (0x61af60, 0x61b040), (0x61b090, 0x61b140), (0x62f8d0, 0x62f950), (0x6ad5d0, 0x6ad740), (0x6ad740, 0x6ad7c0), (0x6ad7c0, 0x6ad860), (0x6ad8f0, 0x6ad946),
           (0x6ada00, 0x6adaa0), (0x6add90, 0x6af770), (0x6b0230, 0x6b0340), (0x6b70d0, 0x6b70f0)])
B = Bench(RANGES)
CALLS = {'Draw': 0x492750, 'Measure': 0x4929f0, 'Rgb': 0x492c40}
PALETTE = 0xfa71f8
SURFACE, INFO, LANG, FONT_TABLE, STYLE, NARROW, BUFFER = 0x7ca120, 0x7ca0e8, 0x78b530, 0x78b54c, 0x78ba2c, 0x78ba80, 0x7b5378
STATE = display.STATE
WIDE, RECT, COLOUR, OUT = RAM + 0x2000, RAM + 0x2040, RAM + 0x2060, RAM + 0x2080
DTABLE, DINFO = display.TABLE + 0x3000, display.INFO + 0x3000   # the draw context's driver table and SurfaceInfo (display.py's, moved)


def byte_arg(r, v):
    """A char argument: its byte v, the rest of the dword whatever the caller had."""
    return (v & 0xff) | r.choice([0, r.getrandbits(24)]) << 8


def world(r, log, regions):
    """The text's world (shared: gauge.py's grid draws labels in it): the draw context and its driver table (recorders; slots 8 and 9,
    the lines, take 5 arguments, the colour's index logged as its byte), the surfaces, the language, the fonts and the font table, the
    global style, no narrowing table, the font's and the renderer's recorders. Returns (lang, table, codes, lead): the language's index,
    the font table's bytes, the code table (slot 11's answer: 0 or tl.CODES) and its lead byte."""
    # the draw context
    state = bytearray(r.bytes(0x74))
    xs, ys = sorted([r.randrange(-20, 300), r.randrange(-20, 300)]), sorted([r.randrange(-20, 300), r.randrange(-20, 300)])
    put(state, 8, '4i', xs[0], ys[0], xs[1], ys[1])
    put(state, 0x18, 'I', STATE + r.choice([0x38, 0x40, 0x44, 0x48]))
    put(state, 0x30, 'I', DINFO)
    for at in (0x38, 0x40, 0x44, 0x48):
        put(state, at, 'I', DTABLE)
    regions.update({STATE: state, DTABLE: vtable({8: 5, 9: 5}, 16), DINFO: display.info(r), INFO: display.info(r)})
    for slot in (8, 9):
        struct.pack_into('<I', log, LOG_BYTES + 4 * slot, 1 << 4)
    regions[SURFACE] = struct.pack('<I', r.choice([0, r.getrandbits(32) | 1, 1]))
    lang = r.randrange(0, 3)
    regions[LANG] = struct.pack('<4I', r.getrandbits(32), lang, r.choice([0x40d, 0x409, 0x40d, r.getrandbits(32)]), r.getrandbits(32))
    # the fonts (textlayout.py's) and the font table
    fonts = [tl.FONTS, tl.FONTS + 0x40]
    regions[tl.FONTS] = b''.join(struct.pack('<I', tl.VTABLE) + r.bytes(0x3c) for _ in fonts)
    regions[tl.VTABLE] = vtable({7: 2})
    table = bytearray(r.bytes(39 * 24))
    for i in range(39):
        put(table, 24 * i + 0x14, 'I', r.choice(fonts * 4 + [0]))
    regions[FONT_TABLE] = table
    style = bytearray(r.bytes(0x50))
    put(style, 4, 'I', r.choice(fonts + [0]))
    regions[STYLE] = style
    regions[NARROW] = struct.pack('<I', 0)
    set_return(log, 9, r.getrandbits(32) & r.choice([~2, 0xffffffff, 2]))
    # the font's slots: the line height, the code table, the widths
    set_return(log, 8, r.choice([r.randrange(0, 0x20)] * 3 + [r.getrandbits(32)]))
    codes = r.choice([tl.CODES] * 9 + [0])
    set_return(log, 11, codes)
    echo(log, 7, 0)
    strings(log, 7, 0, out=1)
    fill(log, 7, 0, struct.pack('<I', r.choice([r.randrange(0, 20)] * 4 + [0, r.getrandbits(32)])))
    lead = r.choice([0xff, 0x80, 0x80, 0, r.randrange(0, 0x100)])
    regions[tl.CODES] = struct.pack('<8xBxHI', lead, r.choice([0, 0x41, 0x100, 0x200, r.randrange(0, 0x200)]), tl.GLYPHS)
    glyphs = [r.choice([9, 10, 13, 0x20, 0x41, 0x28, 0x29, r.getrandbits(16)]) for _ in range(0x200)]
    regions[tl.GLYPHS] = struct.pack('<512H', *[0x41 if 0x5b0 < g < 0x5eb else g for g in glyphs])   # (no right to left glyph)
    tl.renderer_setup(r, regions, log)
    return lang, table, codes, lead


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x400)}
    lang, table, codes, lead = world(r, log, regions)
    font = 0xc if op == 'Rgb' else r.choice([r.randrange(0, 13)] * 24 + [13, 0xff])
    entry_font = struct.unpack_from('<I', table, 24 * (font + 13 * lang) + 0x14)[0] if font < 13 else None
    n = buffer_text(r, regions, lead)
    regions[WIDE] = r.bytes(0x10)
    left, top = r.randrange(-0x40, 0x200), r.randrange(-0x40, 0x200)
    regions[RECT] = struct.pack('<4i', left, top, left + r.choice([r.randrange(0, 0x100), r.randrange(-0x10, 0x8000)]),
                                top + r.randrange(-0x10, 0x100))
    regions[COLOUR] = r.bytes(8)
    regions[OUT] = r.bytes(0x10)
    wide = r.choice([WIDE] * 7 + [0])
    out = r.choice([OUT] * 4 + [0])
    align = [byte_arg(r, r.randrange(0, 3)) for _ in range(2)]   # (past 2 the layout's kind is left as the stack lies: not comparable)
    fit = r.choice([0, 1, r.getrandbits(32)])
    if op == 'Rgb':   # (font 0xc, aligned left / top, fit 1, no bounds, its own surface: the bits, width and height a, c, d)
        regions[PALETTE] = r.bytes(0x400)
        strings(log, 1, 0, out=0b11011)   # (the renderer's style, the layout's holder + 4, is on the stack here: a surface of its own)
        style = bytearray(regions[STYLE])   # (the renderer's index from +0xc and +0x10, which the SetOn10 sets to 1: kept near the table, as
        put(style, 0xc, 'iI', r.choice([0, 1, 2, 3, -1]), r.choice([0, 1]))   # random ones can land in memory only the emulator maps)
        regions[STYLE] = style
        runs = wide
        size = [r.choice([640, 800, 64, 0, r.getrandbits(32)]) for _ in range(2)]
        args = [wide, r.getrandbits(32)] + size + list(struct.unpack('<4i', regions[RECT])) + [r.getrandbits(32)]
    elif op == 'Draw':
        keep = r.choice([0, 0, 1])
        runs = wide and (struct.unpack('<I', regions[SURFACE])[0] or keep)
        args = [wide, RECT, COLOUR, byte_arg(r, font)] + align + [fit, out, keep]
    else:
        runs = wide
        args = [wide, RECT, byte_arg(r, font)] + align + [fit, out]
    return Call(CALLS[op], args, regions, masks=heap_masks(runs and entry_font and codes and n == 1))


def buffer_text(r, regions, lead):
    """The global buffer's text (0x7b5378; no narrowing table): none, or one record (a byte, or a lead byte and the next). Returns its count of
    records (tl.text_records())."""
    text = bytearray(0x10)
    kind = r.randrange(0, 4)
    if kind:
        text[0] = r.choice([r.randrange(1, 0x100), 0x20, 9, 10, min(lead + 1, 0xff), max(lead, 1)])
        if text[0] - 1 >= lead:
            text[1] = r.choice([r.randrange(1, 0x100), 0])
            if not text[1]:
                text[2] = 0   # (the terminator taken along: then the next byte ends it)
    regions[BUFFER] = text
    return tl.text_records(text, lead)


def heap_masks(one):
    """The masks for a measure or draw that made one record (one true), else none."""
    return [Mask(HEAP, [0x10 + g for g in tl.GARBAGE], 'SetText (0x6ae930) never sets a record\'s bytes +5-+7 and +0xe-+0xf (the '
                 'original\'s stack garbage); the one record (in the heap\'s first block) is read only at +0, +2, +4, +8 and +0xc, and '
                 'no glyph is right to left, so it is never moved')] if one else []


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
