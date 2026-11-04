# A sign's text drawn onto a model's textures (src/game/model/signapply: 0x467a80) against the original. Records of each kind (flag 8, flag 0x10, named sign1.tga or sign2.tga in any case, others), one or two lines of text, the parts
# naming the entries or not, the saved pairs failing, the language's file there or not. The callees and GetDC / ReleaseDC are recorders; SignStyle_Render's first copies its two wide lines off the stack to a region, so they compare.
# usage: signapply.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, fail_mask, fill, import_table, log_region, mask_this, put, set_return, strings  # noqa: E402

CTOR, TEARDOWN, LOAD, FONTS, RENDER, PAIRS, LANGUAGE, TIDY, EXISTS, ALLOC, FREE, GETDC, RELEASEDC = range(1, 14)
OUT = RAM + 0x7000
_render_record = _recorder(RENDER, 2)


def _render(e):
    """SignStyle_Render: the two wide lines copied off the stack (tools/fncmp/driver.cpp's signRenderStandIn, the same), then its recorder."""
    params = e.u32(e.r['ESP'] + (4 if e.tail else 0) + 4)
    for i in range(2):
        text = e.u32(params + 4 * i)
        for n in range(0x100):
            unit = e.read(text + 2 * n, 2)
            e.write(OUT + 0x200 * i + 2 * n, unit)
            if unit == b'\0\0':
                break
    _render_record(e)


B = Bench([(0x467a80, 0x4681f4), (0x5da3c0, 0x5da3c1), (0x67a400, 0x6902c0)],
          hooks={0x5ebfc0: _recorder(CTOR, 0), 0x5ec220: _recorder(TEARDOWN, 0), 0x5ec3a0: _recorder(LOAD, 1), 0x5ec680: _recorder(FONTS, 1), 0x5ecb40: _render, 0x57b120: _recorder(PAIRS, 2), 0x41a820: _recorder(LANGUAGE, 0),
                 0x467310: _recorder(TIDY, 6, cdecl=True), 0x46f120: _recorder(EXISTS, 1, cdecl=True), 0x45b9d0: _recorder(ALLOC, 2, cdecl=True), 0x45ba00: _recorder(FREE, 1, cdecl=True)})
OBJECT, HOLDER, MODEL, RECORDS, ENTRIES, TEXTURES, PARTS, MORE, FACES, BLOCK, OWNER, TEXTS = (RAM + a for a in (0, 0x100, 0x200, 0x400, 0x600, 0x800, 0x1000, 0x1800, 0x2000, 0x3000, 0x3100, 0x4000))
NAMES = (b'sign1.tga', b'SIGN1.TGA', b'Sign2.Tga', b'sign2.tga', b'tree.tga', b'sign3.tga', b'sign1.tg', b'')
LINES = ('Big Wheel', 'Ghost Train Ride', 'Nospace', 'A', ' a', 'a ', 'two  spaces', 'long name with many short parts', ' ', '   ', '')


class Texts:
    def __init__(self):
        self.buf = bytearray()

    def add(self, text):
        at = TEXTS + len(self.buf)
        self.buf += text + b'\0'
        return at

    def wide(self, text):
        at = TEXTS + len(self.buf)
        self.buf += text.encode('utf-16-le') + b'\0\0'
        return at


def case(r):
    log = log_region()
    texts = Texts()
    count = r.choice([1, 2, 2, 2, 3, 3, 4, 6])
    records = bytearray(r.bytes(0x10 * count))
    kinds = [0, 0]
    for i in range(count):
        kind = r.choice(['eight', 'ten', 'named', 'named', 'named', 'skip', 'word'])
        flags = r.getrandbits(32) & ~0x1a
        if kind in ('eight', 'ten'):
            put(records, 0x10 * i, 'I', flags | (8 if kind == 'eight' else 0x10) | r.choice([0, 2]) | (r.choice([0, 0x10]) if kind == 'eight' else 0))
            put(records, 0x10 * i + 0xc, 'I', TEXTURES + 0x20 * i)
            kinds[kind == 'ten'] += 1
        else:
            word = 3 if kind == 'word' else r.choice([1, 1, 1, 0, 2])
            name = r.choice(NAMES)
            put(records, 0x10 * i, 'I', flags | (2 if kind == 'skip' else 0))
            put(records, 0x10 * i + 0xa, 'H', word)
            put(records, 0x10 * i + 0xc, 'I', texts.add(name))
            if kind == 'named' and word == 1 and name.lower() in (b'sign1.tga', b'sign2.tga'):
                kinds[name.lower() == b'sign2.tga'] += 1

    words = r.choice(LINES)
    second = r.choice([None, None, None, 'Line two', ''])
    if not all(kinds) and (second is not None or not words):   # (two of one kind: the other texture is what the stack holds, and only the split's scan sets it)
        second = None
        words = words or 'Big Wheel'
    text = 0 if r.randrange(12) == 0 else texts.wide(words)
    text2 = texts.wide(second) if second is not None else 0

    parts_count = r.choice([0, 1, 2, 3])
    parts = bytearray(r.bytes(0xa0 * parts_count))
    more = bytearray(r.bytes(0x58 * parts_count))
    faces = bytearray(r.bytes(0x40 * parts_count))
    for i in range(parts_count):
        n = r.choice([0, 1, 2, 4])
        put(parts, 0xa0 * i, 'I', r.getrandbits(32) & ~0x200 | r.choice([0, 0x200]))
        put(parts, 0xa0 * i + 0x5a, 'H', n)
        put(parts, 0xa0 * i + 0x6c, 'I', FACES + 0x40 * i)
        put(more, 0x58 * i, 'I', r.getrandbits(32) & ~0x200 | r.choice([0, 0x200]))
        for j in range(n):
            put(faces, 0x40 * i + 0x10 * j, 'I', ENTRIES + 8 * r.randrange(count) if r.randrange(3) else r.getrandbits(32))

    model = bytearray(r.bytes(0x100))
    put(model, 0x36, 'H', count)
    put(model, 0x42, 'H', parts_count)
    put(model, 0x44, 'H', r.choice([parts_count, parts_count, 0, min(1, parts_count)]))
    put(model, 0x50, 'I', ENTRIES)
    put(model, 0x54, 'I', RECORDS)
    put(model, 0x70, 'I', PARTS)
    put(model, 0x74, 'I', MORE)
    holder = bytearray(r.bytes(0x40))
    put(holder, 4, 'I', MODEL)
    put(holder, 0x1c, 'I', r.choice([0, OWNER]))
    put(holder, 0x20, 'I', OWNER + 0x10)
    obj = bytearray(r.bytes(0x20))
    put(obj, 4, 'I', r.getrandbits(32) & ~1 | r.choice([0, 1]))
    put(obj, 8, 'I', HOLDER)

    folder = texts.add(r.choice([b'Data\\Theme\\Rides', b'Data\\Theme\\Rides\\', b'Rides', b'Rides\\', b'\\']))
    style = r.choice([0, 0, texts.add(b'Fancy'), texts.add(b'')])
    fonts = texts.add(r.choice([b'Data\\Fonts\\', b'C:\\Fonts']))
    set_return(log, LANGUAGE, texts.add(r.choice([b'english', b'fr'])))
    set_return(log, ALLOC, BLOCK)
    set_return(log, GETDC, 0xdc000000 | r.getrandbits(16))
    fail_mask(log, EXISTS, r.choice([0, 1]))
    strings(log, EXISTS, 1)
    strings(log, LOAD, 1)
    strings(log, FONTS, 1)
    strings(log, TIDY, 0, 1 << 5)   # (warned: a stack address)
    blob(log, PAIRS, 0, 3)
    blob(log, RENDER, 1, 0x7c)   # (the lines' addresses out: _render compares what they point to)
    for k in (CTOR, TEARDOWN, LOAD, FONTS, RENDER):
        mask_this(log, k)
    set_return(log, PAIRS, r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    if r.randrange(4) == 0:
        fill(log, PAIRS, 0, struct.pack('<ii', r.randrange(100), r.randrange(100)))
    table = import_table(log, {'GetDC': (GETDC, 0x6fd34c, 1, 0, 0), 'ReleaseDC': (RELEASEDC, 0x6fd348, 2, 0, 0)})
    regions = {LOG: log, IMPORT_TABLE: table, OBJECT: obj, HOLDER: holder, MODEL: model, RECORDS: records, ENTRIES: r.bytes(8 * count), TEXTURES: r.bytes(0x20 * count), BLOCK: r.bytes(0x40), OWNER: bytes(0x20), OUT: bytes(0x400),
               0x8bd554: struct.pack('<I', r.choice([0, 1, 2, 3, 4])), 0x7a130c: r.bytes(4), TEXTS: texts.buf + bytes(0x100)}
    if parts_count:
        regions.update({PARTS: parts, MORE: more, FACES: faces})
    return Call(0x467a80, [OBJECT, folder, style, fonts, text, text2], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
