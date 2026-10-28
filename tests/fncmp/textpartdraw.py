# The text parts' text drawing and query (src/game/render/textparts: TextPart_DrawText 0x48f830, TextPart_DrawShadowed 0x491ab0,
# TextPart_Query 0x491300) against the originals, in guitext.py's
# world (the draw context and its recorders, the language, the font table and fonts, the global style 0x78ba2c, no narrowing table: the
# text the global buffer's bytes as the case writes them, at most one record) with gauge.py's InWindow surface set-up (the current format
# 0x8bd554 a record whose bit count is 8-32 or odd, the game palette random; the display's surface left the stack's SurfaceInfo: STATE
# +0x30 ignored). The part: random bytes but for its sizes and offsets (s16s about the screen's), the alignments (+0xd8 0-2: past them the
# original's top is the stack's; +0xd9 any), +0xda / +0xdd 0 or set, the font index (0-12 mostly, else a fault in both), its pixels (a
# block of up to 64 x 64 at 4 bytes, or none), the callbacks +0x4c (k 22, answering 0 or 1) and +0x50 (k 23) cdecl recorders or none
# (the surface they get logged as 0), the image's textures none; its layout (+0xe4) as textlayout.py's Draw makes one (its records
# vector with room: the one record's garbage bytes masked). The screen's scales (0xfaa5c0 / 0xfaa5c4) usual, odd or negative; the
# palette now and then with white and a near white planted. Query2 (kind 2): the same world measured (no surface), the rectangle 0x7ca0a0
# random; Query1 (kind 1): the layout's positions (its +0x88 vector, up to 9) and an index about them, else 2 or any kind.
# usage: textpartdraw.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Draw,Shadow,Query1,Query2)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gauge  # noqa: E402
import guitext  # noqa: E402
import textlayout as tl  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, Mask, heap, log_region, put, recorder, set_return, strings  # noqa: E402

B = Bench(gauge.merged(gauge.RANGES + [(0x48f830, 0x48fc33), (0x4925a0, 0x4925cb), (0x659dac, 0x659de5), (0x588c20, 0x588c9c),
                                       (0x491ab0, 0x491f07), (0x491300, 0x49170e)]),
          hooks={0x67af00: gauge.crt.swprintf_hook})
CALLS = {'Draw': 0x48f830, 'Shadow': 0x491ab0, 'Query1': 0x491300, 'Query2': 0x491300}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
PART, PIXELS = RAM + 0x5000, RAM + 0x8000


def s16(r, usual):
    return r.choice([r.randrange(0, usual)] * 4 + [0, usual, -5, 0x4000, r.randrange(-0x8000, 0x8000)])


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x800)}
    lang, table, codes, lead = guitext.world(r, log, regions)
    font = r.choice([r.randrange(0, 13)] * 30 + [13, 0xff])
    entry_font = struct.unpack_from('<I', table, 24 * (font + 13 * lang) + 0x14)[0] if font < 13 else None
    text = bytearray(0x10)   # (guitext.py's: none, or one record)
    if r.randrange(0, 4):
        text[0] = r.choice([r.randrange(1, 0x100), 0x20, 9, 10, min(lead + 1, 0xff), max(lead, 1)])
        if text[0] - 1 >= lead:
            text[1] = r.choice([r.randrange(1, 0x100), 0])
            if not text[1]:
                text[2] = 0
    regions[guitext.BUFFER] = text
    n = tl.text_records(text, lead)
    regions[guitext.WIDE] = r.bytes(0x10)
    # the part
    part = bytearray(r.bytes(0x1a4))
    put(part, 0x3c, 'hh', s16(r, 640), s16(r, 480))
    put(part, 0x4c, 'II', r.choice([0, 0, recorder(22, 1, cdecl=True)]), r.choice([0, recorder(23, 1, cdecl=True)]))
    put(part, 0x9c, 'I', r.choice([0, r.getrandbits(32)]))
    put(part, 0xb2, 'H', 0)   # (the image's texture count: none to free)
    w, h = r.randrange(0, 65), r.randrange(0, 65)
    put(part, 0xa0, 'I', r.choice([PIXELS] * 4 + [0]))
    put(part, 0xa8, 'HH', w, h)
    put(part, 0xd8, 'BBBxB', r.randrange(0, 3), r.choice([0, 1, 2, 0, 1, 2, 3, 0xff]), r.choice([0, 0, 1, 0xff]), font)
    part[0xdd] = r.choice([0, 0, 1])
    put(part, 0xde, 'hh', s16(r, 200), s16(r, 200))
    strings(log, 22, 0, out=1)
    strings(log, 23, 0, out=1)
    set_return(log, 22, r.choice([0, 0, 1, r.getrandbits(32)]))
    # its layout (textlayout.py's Draw)
    body = bytearray(r.bytes(0xc0))
    put(body, 4, 'I', r.choice([tl.FONTS, tl.FONTS + 0x40, 0]))
    put(body, 8, 'I', r.getrandbits(32) & r.choice([~2, 0xffffffff, 2]))
    regions[tl.VEC8] = r.bytes(0x50)
    tl.layout_fields(r, body)
    put(body, 0x60, 'III', tl.VEC16, tl.VEC16 + 16 * r.randrange(0, 13), tl.VEC16 + 16 * 12)
    regions[tl.VEC16] = r.bytes(0xc0)
    cap = r.randrange(0, 10)
    put(body, 0x74, 'III', tl.LINES, tl.LINES + 12 * r.randrange(0, cap + 1), tl.LINES + 12 * cap)
    regions[tl.LINES] = r.bytes(12 * 9)
    put(body, 0x80, 'I', r.choice([0, r.randrange(0, 0x40)]))
    put(body, 0x94, 'B', r.choice([0, 1, r.getrandbits(8)]))
    part[0xe4:0x1a4] = body
    regions[PART] = part
    regions[PIXELS] = r.bytes(0x4000)
    # the format, palette and screen
    index = r.randrange(4)
    records = bytearray(r.bytes(32 * 4))
    put(records, 32 * index + 12, 'I', r.choice([8, 8, 16, 24, 32, 0, 1, 15]))
    regions[gauge.FORMAT] = struct.pack('<I', index) + records
    palette = bytearray(r.bytes(0x400))
    if r.random() < 0.5:   # (white and a near white, each red, green, blue)
        a, b = r.sample(range(0x100), 2)
        put(palette, 4 * a, 'BBB', 0xff, 0xff, 0xff)
        put(palette, 4 * b, 'BBB', *r.choice([(0xff, 0xff, 0xfe), (0xfe, 0xff, 0xff), (0xff, 0xfe, 0xff)]))
    regions[0xfa71f8] = palette
    regions[0xfaa5c0] = struct.pack('<hxxh', r.choice([768, 600, 480, 1536, r.randrange(1, 2000), -768]),
                                    r.choice([1024, 800, 640, 2048, r.randrange(1, 3000), -1024]))
    masks = []
    if entry_font and codes and n == 1:
        masks.append(Mask(tl.VEC16, list(tl.GARBAGE), tl.GARBAGE_WHY))
    text = r.choice([guitext.WIDE] * 9 + [0])
    if op in ('Draw', 'Shadow'):
        return Call(CALLS[op], [PART, text], regions, conv='t', ret='v', masks=masks, ignore=[(guitext.STATE + 0x30, 4)])
    regions[0x7ca0a0] = r.bytes(0x10)
    if op == 'Query2':
        return Call(0x491300, [PART, 2, text], regions, conv='t', ret='a', masks=masks)
    m = (struct.unpack_from('<I', body, 0x8c)[0] - tl.VEC8) // 8 if struct.unpack_from('<I', body, 0x88)[0] else 0
    n = r.choice([0, 1, m - 1, m, m + 1, -1, r.randrange(-3, 10), r.getrandbits(32)]) & 0xffffffff
    kind = 1 if r.random() < 0.9 else r.choice([0, 3, r.getrandbits(32)])
    return Call(0x491300, [PART, kind, n], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
