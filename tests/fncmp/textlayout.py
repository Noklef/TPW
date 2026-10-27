# The text layout engine (src/game/gfx/textlayout: 0x6ad5d0-0x6ad840, 0x6ade00-0x6adeb0, 0x6b0230-0x6b0270) against the originals: an
# object of random bytes at RAM (top byte 0x30) or in the game's data (top byte 0: the layout's constructor stores the top byte of `this`),
# its font (+4 of the style) 0 or one of two objects with recorder vtables (slot 9, the flags, answering a value with bit 1 set or clear),
# the vectors' blocks heap blocks, random or 0 (Mem_Free logs them), the deleting destructors' flags 0, 1 or random.
# usage: textlayout.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, Mask, echo, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x5fa680, 0x5fa6b0), (0x5fb770, 0x5fb7a0), (0x62f8d0, 0x62f950), (0x6add90, 0x6ade00), (0x6ad5d0, 0x6ad740), (0x6ad7c0, 0x6ad860),
           (0x6ade00, 0x6aeab0), (0x6aeab0, 0x6af770), (0x6b70d0, 0x6b70f0),
           (0x6b0230, 0x6b0340)])
SOURCE = RAM + 0x400   # Assign's and SetStyle's source style
TABLE = 0x782d18       # SetStyle's renderers (0x782d38) and the dwords before them
RENDERER, VTABLE2 = RAM + 0x500, RAM + 0x480   # Render's renderer: slot 1 takes 5 arguments
ELEMS = RAM + 0x600    # Erase12's elements (12 bytes each)
VEC, VALUE = RAM + 0x700, RAM + 0x800   # Insert8/12/16's and Append's storage (up to 8 elements) and value
RECS = RAM + 0x900     # Append's 16-byte character records
DATA = 0x7d0000   # an object in the game's data (top byte 0)
FONTS, VTABLE, COLOUR, STATUS = RAM + 0x200, RAM + 0x300, RAM + 0x380, RAM + 0x3c0
# op: (address, the style's offset in the object, extra arguments)
CALLS = {'StyleCtor': (0x6ad5d0, 0), 'StyleDtor': (0x6ad640, 0), 'StyleDel': (0x6ad620, 0), 'SetFont': (0x6ad7c0, 0),
         'SetColour': (0x6ad6e0, 0), 'HolderCtor': (0x6b0230, 4), 'HolderDtor': (0x6b0270, 4), 'HolderDel': (0x6b0250, 4),
         'LayoutCtor': (0x6ade00, 8), 'LayoutDtor': (0x6adeb0, 8), 'LayoutDel': (0x6ade90, 8),
         'Assign': (0x6ad660, 0), 'GetFont': (0x6ad820, 0), 'FontKind': (0x6ad840, 0),
         'SetStyle': (0x6b0280, 4), 'Render': (0x6b0300, 4), 'CharClass': (0x6aeab0, 0), 'Erase12': (0x6aeda0, 0),
         'Insert8': (0x6aedf0, 0), 'Insert16': (0x6af0a0, 0), 'Insert12': (0x6af3b0, 0), 'Append': (0x6add90, 0),
         'BreakLines': (0x6aec80, 0), 'Arrange': (0x6aeb10, 0), 'SetOrigin': (0x6adfb0, 0),
         'SetText': (0x6ae930, 0), 'Place': (0x6ae550, 0), 'Draw': (0x6ae0f0, 0), 'Measure': (0x6ae1c0, 0)}
RET = {'StyleCtor': 'a', 'StyleDel': 'a', 'SetFont': 'a', 'SetColour': 'a', 'HolderCtor': 'a', 'HolderDel': 'a', 'LayoutCtor': 'a',
       'LayoutDel': 'a', 'Assign': 'a', 'GetFont': 'a', 'FontKind': 'a',
       'SetStyle': 'a', 'Render': 'a', 'CharClass': 'b', 'Erase12': 'a', 'BreakLines': 'a', 'Arrange': 'a', 'SetOrigin': 'a',
       'SetText': 'a', 'Place': 'a', 'Draw': 'a', 'Measure': 'a'}

# SetText, Place, Draw and Measure: the font (+4 of the style SOURCE) answers the line height (slot 8), its code table (slot 11: CODES, or
# 0) and each character's width (slot 7 (out, code): the width written through out, which it answers; out logged as 0); the holder's
# renderer (slot 1 (status, &pen, code, &glyph, style): the three pointers logged as 0) writes glyph rects through its fourth argument.
TEXT, CODES, GLYPHS = RAM + 0xa00, RAM + 0xa40, RAM + 0xc00   # the text, the code table (+8 lead, +0xa count, +0xc glyphs), the glyphs
LINES = RAM + 0xa80     # the lines (12 bytes each, up to 9)
VEC16, VEC8 = RAM + 0x1000, RAM + 0x1100   # the records (room for 12) and the positions (up to 9)
BOUNDS, RECT = RAM + 0x1180, RAM + 0x11a0
GARBAGE = (5, 6, 7, 0xe, 0xf)   # a SetText record's bytes it never sets: the original's stack garbage
GARBAGE_WHY = ('SetText (0x6ae930) never sets a record\'s bytes +5-+7 and +0xe-+0xf (stack garbage in the original); only +0, +2, +4, +8 '
               'and +0xc are read (Arrange / BreakLines +4 / +8, Place +2 / +4 / +8 / +0xc, Records16_AnyInRange5b0 +0; no right to left '
               'glyph here, so no record is moved), and the vector has room, so none is copied')


def text_records(text, lead):
    """How many records SetText makes of text (the region's bytes) with the table's lead byte: a byte whose code (c - 1) is lead or more
    takes the next along (the terminator too)."""
    n = i = 0
    while text[i]:
        c = text[i]
        i += 2 if c - 1 >= lead else 1
        n += 1
    return n


def font_setup(r, regions, log, rtl):
    """The font's slots, its code table and glyphs (right to left glyphs only with rtl), and a text; the number of records it makes."""
    regions[VTABLE] = vtable({7: 2})
    set_return(log, 8, r.choice([r.randrange(0, 0x20)] * 3 + [r.getrandbits(32)]))
    table = r.choice([CODES] * 9 + [0])
    set_return(log, 11, table)
    echo(log, 7, 0)
    strings(log, 7, 0, out=1)
    k = r.randrange(1, 8)
    fill(log, 7, 0, b''.join(struct.pack('<I', r.choice([r.randrange(0, 20)] * 4 + [0, r.getrandbits(32)])) for _ in range(k)), records=k)
    lead = r.choice([0xff, 0x80, 0x80, 0, r.randrange(0, 0x100)])
    regions[CODES] = struct.pack('<8xBxHI', lead, r.choice([0, 0x41, 0x100, 0x200, r.randrange(0, 0x200)]), GLYPHS)
    glyphs = []
    for _ in range(0x200):
        g = r.choice([9, 10, 13, 0x20, 0x20, 0x41, 0x28, 0x29, r.getrandbits(16)] + ([0x5d0, r.randrange(0x5b0, 0x5ec)] if rtl else []))
        if not rtl and 0x5b0 < g < 0x5eb:
            g = 0x41
        glyphs.append(g)
    regions[GLYPHS] = struct.pack('<512H', *glyphs)
    length = r.randrange(0, 10)
    text = bytearray(0x40)
    for i in range(length):
        text[i] = r.choice([r.randrange(1, 0x80), r.randrange(0x80, 0x100), 9, 10, 13, 0x20, 0x21, 0x42, min(lead + 1, 0xff), max(lead, 1)])
    text[length + 1] = r.choice([0, 0, r.randrange(1, 0x100)])   # past the terminator (a lead byte there takes it)
    regions[TEXT] = text
    style = bytearray(r.bytes(0x50))
    put(style, 4, 'I', r.choice([FONTS] * 8 + [FONTS + 0x40, 0]))
    put(style, 0xc, 'i', r.choice([0, 1, 2, 0, 1, 2, 3]))
    put(style, 0x10, 'I', r.choice([0, 1]))
    regions[SOURCE] = style
    font = struct.unpack_from('<I', style, 4)[0]
    return text_records(text, lead) if font and table else 0


def renderer_setup(r, regions, log):
    """The renderers' table (SetStyle) and the renderer's slot 1: it writes glyph rects (some empty), and now and then the pen."""
    set_return(log, 10, r.choice([0, 1, 2, 3, 4, 0, 1, 2, 5, -1]))
    regions[TABLE] = b''.join(struct.pack('<I', r.choice([RENDERER] * 6 + [0])) for _ in range(0x40))
    regions[VTABLE2] = vtable({1: 5})
    regions[RENDERER] = struct.pack('<I', VTABLE2)
    strings(log, 1, 0, out=0b1011)
    k = r.randrange(1, 8)
    rects = []
    for _ in range(k):
        if r.random() < 0.3:
            x, y = r.randrange(-50, 50), r.randrange(-50, 50)
            rects.append(struct.pack('<4i', x, y, x, y))
        else:
            rects.append(struct.pack('<4i', *[r.randrange(-100, 100) for _ in range(4)]))
    fill(log, 1, 3, b''.join(rects), which=1, records=k)
    if r.random() < 0.2:
        fill(log, 1, 1, struct.pack('<2i', r.randrange(-100, 100), r.randrange(-100, 100)), which=2)


def layout_fields(r, body):
    """The positions (+0x84), the kinds (+0xa0, +0xa4), the text box (+0xac-+0xb8) and the line height (+0xbc)."""
    cap = r.randrange(0, 10)
    m = r.randrange(0, cap + 1)
    put(body, 0x88, 'III', *([0, 0, 0] if r.random() < 0.2 else [VEC8, VEC8 + 8 * m, VEC8 + 8 * cap]))
    put(body, 0xa0, 'I', r.choice([0, 2, 5, 2, 5, r.getrandbits(32)]))
    put(body, 0xa4, 'I', r.choice([0, 1, 2, 1, 2, r.getrandbits(32)]))
    for off in range(0xac, 0xbc, 4):
        put(body, off, 'I', r.choice([r.randrange(-0x100, 0x400) & 0xffffffff] * 4 + [r.getrandbits(32)]))
    put(body, 0xbc, 'I', r.choice([r.randrange(0, 0x20)] * 4 + [r.getrandbits(32)]))


def layout_case(r, op, address, obj, body, log, regions):
    regions[HEAP] = heap(0x800)
    regions[VEC8] = r.bytes(0x50)
    masks = []
    if op == 'SetText':
        n = font_setup(r, regions, log, True)
        if n <= 1 and r.random() < 0.3:   # an empty vector: one record goes to the heap's first block
            put(body, 0x60, 'III', 0, 0, 0)
            if n:
                masks.append(Mask(HEAP, [0x10 + g for g in GARBAGE], GARBAGE_WHY))
        else:
            put(body, 0x60, 'III', VEC16, VEC16 + 16 * r.randrange(0, 13), VEC16 + 16 * 12)
            if n:
                masks.append(Mask(VEC16, [16 * i + g for i in range(n) for g in GARBAGE], GARBAGE_WHY))
        regions[VEC16] = r.bytes(0xc0)
        args = [obj, STATUS, TEXT, SOURCE]
    elif op == 'Place':
        font_setup(r, regions, log, True)
        renderer_setup(r, regions, log)
        layout_fields(r, body)
        n = r.randrange(0, 9)
        index = list(range(n))
        if r.random() < 0.4:
            r.shuffle(index)
        recs = bytearray()
        for i in range(8):
            glyph = r.choice([0x41, 0x20, 0x28, 0x29, 0x5d0, r.randrange(0x5b1, 0x5eb), r.randrange(0x5a0, 0x600), r.getrandbits(16)])
            flags = r.choice([0, 0, 2, 4, 0xc, 0x40, 0x10, 0x12, 0x42, r.getrandbits(8)])
            width = r.choice([r.randrange(0, 20)] * 5 + [r.getrandbits(32)])
            recs += struct.pack('<HHB3xIHH', glyph, r.getrandbits(16), flags, width, index[i] if i < n else r.randrange(0, n + 1),
                                r.getrandbits(16))
        regions[RECS] = recs
        put(body, 0x60, 'II', RECS, RECS + 16 * n)
        k = r.randrange(0, 5)
        cuts = sorted([0] + [r.randrange(0, n + 1) for _ in range(k - 1)] + [n]) if k else []
        lines = b''.join(struct.pack('<III', RECS + 16 * cuts[i], RECS + 16 * cuts[i + 1], r.choice([r.randrange(0, 0x40), r.getrandbits(32)]))
                         for i in range(k))
        regions[LINES] = lines or b'\0'
        put(body, 0x74, 'III', LINES, LINES + 12 * k, LINES + 12 * k)
        args = [obj, STATUS, SOURCE, r.choice([BOUNDS, BOUNDS, 0]), r.choice([0, 1, 1, r.getrandbits(32)])]
        regions[BOUNDS] = r.bytes(0x10)
    else:   # Draw, Measure
        n = font_setup(r, regions, log, False)
        renderer_setup(r, regions, log)
        layout_fields(r, body)
        put(body, 0x60, 'III', VEC16, VEC16 + 16 * r.randrange(0, 13), VEC16 + 16 * 12)
        regions[VEC16] = r.bytes(0xc0)
        if n:
            masks.append(Mask(VEC16, [16 * i + g for i in range(n) for g in GARBAGE], GARBAGE_WHY))
        cap = r.randrange(0, 10)
        put(body, 0x74, 'III', LINES, LINES + 12 * r.randrange(0, cap + 1), LINES + 12 * cap)
        regions[LINES] = r.bytes(12 * 9)
        put(body, 0x80, 'I', r.choice([0, r.randrange(0, 0x40)]))
        put(body, 0x94, 'B', r.choice([0, 1, r.getrandbits(8)]))
        left, top = r.randrange(-0x40, 0x100), r.randrange(-0x40, 0x100)
        regions[RECT] = struct.pack('<4i', left, top, left + r.choice([r.randrange(0, 0x60), r.randrange(-0x10, 0x200)]),
                                    top + r.randrange(-0x10, 0x100))
        regions[BOUNDS] = r.bytes(0x10)
        args = [obj, STATUS, RECT, TEXT, SOURCE, r.choice([BOUNDS, BOUNDS, BOUNDS, 0])]
    regions[obj] = body
    return Call(address, args, regions, conv='t', ret='a', masks=masks)


def case(r):
    op = r.deal(OPS)
    address, style = CALLS[op]
    obj = r.choice([RAM, DATA])
    log = log_region()
    set_return(log, 9, r.getrandbits(32) & r.choice([~2, 0xffffffff, 2]))
    set_return(log, 10, r.choice([0, 1, 2, 3, 4, 5, 6, -1, r.getrandbits(32)]))
    body = bytearray(r.bytes(0xc0))
    fonts = [FONTS, FONTS + 0x40]
    put(body, style + 4, 'I', r.choice(fonts + [0]))
    put(body, style + 8, 'I', r.getrandbits(32) & r.choice([~2, 0xffffffff, 2]))
    for off in (0x60, 0x74, 0x88):
        put(body, off, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
    regions = {obj: body, LOG: log, HEAP: heap(0x200), VTABLE: vtable({}),
               FONTS: b''.join(struct.pack('<I', VTABLE) + r.bytes(0x3c) for _ in fonts),
               COLOUR: r.bytes(8), STATUS: r.bytes(4)}
    args = [obj]
    if op in ('SetText', 'Place', 'Draw', 'Measure'):
        return layout_case(r, op, address, obj, body, log, regions)
    if op == 'SetFont':
        args += [STATUS, r.choice(fonts + [0])]
    elif op == 'SetColour':
        args += [STATUS, COLOUR]
    elif op.endswith('Del'):
        args.append(r.choice([0, 1, r.getrandbits(32)]))
    elif op == 'Assign':
        source = bytearray(r.bytes(0x50))
        put(source, 4, 'I', r.choice(fonts * 6 + [0]))   # a source without a font crashes in both
        regions[SOURCE] = source
        args.append(r.choice([SOURCE] * 5 + [obj]))
    elif op == 'SetStyle':
        source = bytearray(r.bytes(0x50))
        put(source, 4, 'I', r.choice(fonts * 6 + [0]))
        put(source, 0xc, 'i', r.choice([0, 1, 2, 0, 1, 2, -1, 3, r.getrandbits(31)]))
        put(source, 0x10, 'I', r.choice([0, 1, 2]))
        set_return(log, 10, r.choice([0, 1, 2, 3, 4, 5, 6, -1, -2]))   # the index stays in TABLE (a wild read faults only in the emulator)
        regions[SOURCE] = source
        regions[TABLE] = b''.join(struct.pack('<I', r.choice([0, r.getrandbits(32)])) for _ in range(0x40))
        args += [STATUS, SOURCE]
    elif op == 'Render':
        put(body, 0x54, 'I', r.choice([0, RENDERER, RENDERER]))
        regions[VTABLE2] = vtable({1: 5})
        regions[RENDERER] = struct.pack('<I', VTABLE2)
        args += [STATUS, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]
    elif op == 'CharClass':
        ch = r.choice([9, 10, 13, 0x20, r.randrange(0, 0x30), r.randrange(0, 0x30), r.getrandbits(16)]) | r.choice([0, r.getrandbits(16) << 16])
        return Call(address, [ch], regions, conv='s', ret='b')
    elif op == 'Erase12':
        n = r.randrange(0, 9)
        j = r.randrange(0, n + 1)
        i = r.randrange(0, j + 1)
        put(body, 4, 'II', ELEMS, ELEMS + 12 * n)
        regions[ELEMS] = r.bytes(12 * 8)
        args += [ELEMS + 12 * i, ELEMS + 12 * j]
    elif op in ('Insert8', 'Insert12', 'Insert16', 'Append'):
        size = {'Insert8': 8, 'Insert16': 16}.get(op, 12)
        cap = r.randrange(0, 9)
        n = r.randrange(0, cap + 1)
        i = r.randrange(0, n + 1)
        count = r.choice([0, 1, 1, 2, 3, r.randrange(0, 7)])
        if r.random() < 0.15:
            put(body, 4, 'III', 0, 0, 0)   # an empty vector
            position = 0
        else:
            put(body, 4, 'III', VEC, VEC + size * n, VEC + size * cap)
            position = VEC + size * i
        regions[VEC] = r.bytes(size * 8)
        regions[VALUE] = r.bytes(16)
        regions[HEAP] = heap(0x400, fail=r.choice([0] * 7 + [1]))
        value = VALUE if r.random() < 0.8 or n == 0 else VEC + size * r.randrange(0, n)   # a value inside the vector too
        if op == 'Append':
            put(body, 0x10, 'I', r.choice([0, r.randrange(0, 0x40), r.getrandbits(32)]))
            j = r.randrange(0, 9)
            i = r.randrange(0, j + 1)
            regions[RECS] = r.bytes(0x80)
            args += [r.getrandbits(32), RECS + 16 * i, RECS + 16 * j, r.choice([r.randrange(0, 0x40), r.getrandbits(32)])]
        else:
            args += [position, count, value]
    elif op in ('BreakLines', 'Arrange'):
        # the layout's 16-byte character records (+4 flags: 2 space, 4 line break, 8 with 4: CR LF, 0x10 break allowed; +8 width) and
        # its lines (a 12-byte vector at +0x74, the widest at +0x80)
        n = r.randrange(0, 9)
        recs = bytearray(r.bytes(0x80))
        for k in range(8):
            put(recs, 16 * k + 4, 'B', r.choice([0, 0, 0x10, 0x12, 2, 2, 4, 0xc, 0x14, r.getrandbits(8)]))
            put(recs, 16 * k + 8, 'I', r.choice([r.randrange(0, 20)] * 6 + [0, r.getrandbits(32)]))
        regions[RECS] = recs
        cap = r.randrange(0, 9)
        lines = r.randrange(0, cap + 1)
        put(body, 0x74, 'III', VEC, VEC + 12 * lines, VEC + 12 * cap)
        put(body, 0x80, 'I', r.choice([0, r.randrange(0, 0x40)]))
        put(body, 0xa8, 'I', r.choice([r.randrange(0, 0x50)] * 5 + [0, r.getrandbits(32)]))
        regions[VEC] = r.bytes(12 * 8)
        regions[HEAP] = heap(0x800)
        if op == 'BreakLines':
            j = r.randrange(0, n + 1)
            i = r.randrange(0, j + 1)
            args += [RECS + 16 * i, RECS + 16 * j]
        else:
            put(body, 0x60, 'II', RECS, RECS + 16 * n)
            put(body, 0xa0, 'I', r.choice([0, 2, 5, 2, 5, r.getrandbits(32)]))
            put(body, 0xa4, 'I', r.choice([0, 1, 2, 1, 2, r.getrandbits(32)]))
            put(body, 0xbc, 'I', r.choice([r.randrange(0, 0x20), r.getrandbits(32)]))
            args += [STATUS] + [r.choice([r.randrange(-0x100, 0x400) & 0xffffffff, r.getrandbits(32)]) for _ in range(4)]
    elif op == 'SetOrigin':
        # the positions: an 8-byte vector at +0x84 (first +0x88), empty (all 0) or in VEC
        cap = r.randrange(0, 9)
        n = r.randrange(0, cap + 1)
        put(body, 0x88, 'III', *([0, 0, 0] if r.random() < 0.2 else [VEC, VEC + 8 * n, VEC + 8 * cap]))
        regions[VEC] = r.bytes(8 * 8)
        regions[HEAP] = heap(0x200)
        put(body, 0xa0, 'I', r.choice([0, 2, 5, 2, 5, r.getrandbits(32)]))
        put(body, 0xa4, 'I', r.choice([0, 1, 2, 1, 2, r.getrandbits(32)]))
        put(body, 0xbc, 'I', r.choice([r.randrange(0, 0x20), r.getrandbits(32)]))
        regions[VALUE] = b''.join(struct.pack('<I', r.choice([r.randrange(-0x100, 0x400) & 0xffffffff, r.getrandbits(32)])) for _ in range(4))
        args += [STATUS, VALUE]
    return Call(address, args, regions, conv='t', ret=RET.get(op, 'v'))


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
