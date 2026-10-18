# The object definition lookups, walk and schema slots (src/game/world/objectdefs: 0x412e90,
# 0x412ef0, 0x412f50, 0x412e60, 0x412e70, 0x412c90, 0x4143f0, 0x413410) against the original: ids among the list
# (repeated, missing), the cache hit (its pointer anything, null included) or missed, counts zero,
# negative or up to the full 0x14a; the walk cursor about the end. The marked cells (0x413410): grids of up to 22 columns
# and 4 rows (or none, or negative counts) with cells of kind 9 and 0xa or not (IntPair_Copy 0x4d99c0 run).
# usage: objectdefs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Find,At390,At410,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, _cdecl_log4, global_heap, heap, log_region, memory_stream, put, recorder, set_return  # noqa: E402
from x87emu import exe_sections  # noqa: E402

B = Bench([(0x412e90, 0x412eea), (0x412ef0, 0x412f4c), (0x412f50, 0x412fac), (0x412e60, 0x412e6d), (0x412e70, 0x412e8f),
           (0x412c90, 0x412ca4), (0x4143f0, 0x414404), (0x413410, 0x4135ee), (0x4d99c0, 0x4d99d3),
           (0x413600, 0x4138b0), (0x4138b0, 0x413930), (0x413380, 0x413410), (0x5f5660, 0x5f5690), (0x5da3c0, 0x5da3c1),
           (0x4146b0, 0x4147ec), (0x4147f0, 0x41486a), (0x414870, 0x4148c7), (0x412fb0, 0x413018), (0x413020, 0x4130a9),
           (0x4130b0, 0x413137), (0x414670, 0x41467a), (0x67a400, 0x6902c0)],
          hooks={0x6acd60: _cdecl_log4})  # the text formatter: a log stub (driver: TextFormatEnv)
DEFS, ITEMS, DEF, GRID, REF = RAM, RAM + 0x1000, RAM + 0x3000, RAM + 0x3800, RAM + 0x3c00
OTXT, OUTW, TXTS, DEFN = RAM + 0x6000, RAM + 0x6200, RAM + 0x6400, RAM + 0x7000
CALLS = {'Find': 0x412e90, 'At390': 0x412ef0, 'At410': 0x412f50, 'First': 0x412e60, 'Next': 0x412e70, 'Schema': 0x412c90,
         'TrackSchema': 0x4143f0, 'Cells': 0x413410, 'LoadText': 0x413600,
         'FullName': 0x4146b0, 'Name': 0x4147f0, 'Suffix': 0x414870}
TEXT_OPS = ('FullName', 'Name', 'Suffix', 'GetFullName', 'GetName', 'GetSuffix')
# The game's object text ids (0x7488d8 .. 0x749850: id, name text, suffix text; an id 0 ends it).
TEXT_IDS = next([i for i in struct.unpack_from('<%dI' % 990, d, 0x7488d8 - b)[0::3] if i] for b, d in exe_sections() if b <= 0x7488d8 < b + len(d))
TEXT, STRM = RAM + 0x4000, RAM + 0x5000
KEYS = ['NAME', 'SIGNA', 'SIGNB']
BREAKS = ['\r\n', '\r\n', '\n', '\r', '\n\r']
BLANKS = ['', ' ', '\t', '  \t ', ' x']


def cells(r):  # a definition (through a variable) and its cell grid (+0x18): the column and row counts, then cells of 8 bytes, 20 to a row
    obj = bytearray(r.bytes(0x500))
    grid = bytearray(r.bytes(0x400))
    put(grid, 0, 'ii', r.choice([0, 1, 2, 3, 5, 20, 22, r.randrange(-2, 1)]), r.choice([0, 1, 2, 4, r.randrange(-1, 5)]))
    for k in range(8, 0x3f8, 8):
        put(grid, k, 'I', r.choice([9, 10, 9, 10, 8, 11, 0x109, r.getrandbits(32)]) if r.random() < 0.15 else r.choice([0, 1, 2, 3]))
    put(obj, 0x18, 'I', GRID)
    return Call(0x413410, [r.getrandbits(32), REF], {DEF: obj, GRID: grid, REF: struct.pack('<I', DEF)}, conv='t', ret='v')


def load_text(r):
    """ObjectDef_LoadText: a text of key and value lines (keys known, near misses and unknown, values blank or not, blank lines
    between, the four line breaks, a BOM or not) from a memory stream whose size (a recorder's +0x14) is the text's, short of it, odd, 0
    or -2; GlobalAlloc failing now and then."""
    lines = []
    for _ in range(r.choice([0, 1, 2, 3, 3, 4, 6])):
        key = r.choice(KEYS * 4 + ['NAMES', 'SIGN', 'name', 'Signb', 'OTHER', ''])
        lines.append(key + (r.choice([' x', '\tx', ':x', '=x']) if r.random() < 0.15 else ''))
        lines += [r.choice(BLANKS) for _ in range(r.choice([0, 0, 0, 1, 2]))]
        lines.append(r.choice(['Big Wheel', 'A', '', ' ', 'Ghost Train', ' lead', 'NAME']))
        lines += [r.choice(BLANKS) for _ in range(r.choice([0, 0, 0, 1]))]
    text = ''.join(line + r.choice(BREAKS) for line in lines)
    if r.random() < 0.3:
        text = text.rstrip('\r\n')
    data = (b'\xff\xfe' if r.random() < 0.5 else b'') + text.encode('utf-16-le')
    size = r.choice([len(data)] * 6 + [len(data) - 2, len(data) - 1, len(data) + 1, 0, -2, len(data) + 2])
    log = log_region()
    set_return(log, 5, size)
    stream = memory_stream(STRM, TEXT, len(data))
    put(stream, 0x20 + 0x14, 'I', recorder(5, 0))
    regions = {LOG: log, STRM: stream, TEXT: data + r.bytes(8), DEF: r.bytes(0x510),
               HEAP: heap(0x4000, fail=r.choice([0] * 10 + [1, 2, 3])), **global_heap()}
    return Call(0x413600, [DEF, STRM], regions, conv='t', ret='v')


def wtext(r, n=None):
    """Wide text: ASCII, the odd '-' (the separator's test) or character above 0xff, no NUL."""
    n = r.choice([0, 1, 3, r.randrange(12), r.randrange(0x40)]) if n is None else n
    return b''.join(r.choice([r.randrange(0x20, 0x7f), 0x2d, r.randrange(0x100, 0x10000)]).to_bytes(2, 'little') for _ in range(n))


def name_buffers(r):
    """ObjectText's three buffers of 0x40 characters (the formatter, a log stub, writes nothing: what they hold stands for its text): a name,
    blank, ending in '-' (or only '-') or all 0x40 characters (running on into the suffix's), a suffix, blank or not, and a full name of junk."""
    name = r.choice([b'', wtext(r), wtext(r) + b'-\0', b'-\0', wtext(r, 0x3f) + b'-\0', wtext(r, 0x40)])[:0x80]
    suffix = r.choice([b'', b'', wtext(r), b'-\0'])[:0x7e]
    return (name + b'\0\0' + r.bytes(0x80))[:0x80] + (suffix + b'\0\0' + r.bytes(0x80))[:0x80] + r.bytes(0x80)


def object_text(r, op):
    """ObjectText (0x4146b0, 0x4147f0, 0x414870; this the buffers at OTXT) and the definitions' names through it (0x412fb0, 0x413020, 0x4130b0;
    this a definition set at DEFS, its buffers at +0x52c): ids of the game's table at 0x7488d8 or not, now and then a table of the case's own
    (full to 0x749850, or ended by an id 0 early); definitions with the id (cached or not) whose texts +0x4f4.. are L"MISSING" (0x6fd880),
    null or text; out lengths about 0x80; the trim flag 1 or another byte over junk."""
    regions = {LOG: log_region(), 0x78bacc: struct.pack('<I', r.getrandbits(32)), OUTW: r.bytes(0x140)}
    ids = TEXT_IDS
    if r.random() < 0.2:
        rows = [(r.choice([r.randrange(1, 40), r.getrandbits(16), r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32)) for _ in range(330)]
        if r.random() < 0.5:
            rows[r.choice([0, 1, r.randrange(330), 329])] = (0, 0, 0)
        regions[0x7488d8] = b''.join(struct.pack('<3I', *row) for row in rows)
        ids = [row[0] for row in rows]
    id_ = r.choice([r.choice(ids), r.choice(ids), r.choice(ids), r.randrange(40), 0, r.getrandbits(16)])
    size = r.choice([0, 1, 2, 8, 0x3f, 0x40, 0x7f, 0x80, 0x81, 0x90, 0xa0])
    if op in ('FullName', 'Name', 'Suffix'):
        regions[OTXT] = name_buffers(r)
        args = [OTXT, id_ | (r.getrandbits(16) << 16 if r.random() < 0.1 else 0), OUTW, size]
        if op == 'Name':
            args.append(r.choice([0, 1, 1, 2, 0x101, r.getrandbits(32)]))
        return Call(CALLS[op], args, regions, conv='t', ret='b')
    defs = bytearray(r.bytes(0x740))
    defs[0x52c:0x6ac] = name_buffers(r)
    n = r.choice([0, 1, 2, 3, 4])
    texts = [0x6fd880, 0, TXTS, TXTS + 0x80, TXTS + 0x100]
    for i in range(4):
        d = bytearray(r.bytes(0x500))
        put(d, 0, 'H', r.choice([id_, id_, r.randrange(40), r.getrandbits(16)]))
        for k in range(3):
            put(d, 0x4f4 + 4 * k, 'I', r.choice(texts[:1] * 2 + texts[2:] + ([0] if op == 'GetFullName' or r.random() < 0.05 else [])))
        put(defs, 4 * i, 'I', DEFN + 0x500 * i)
        regions[DEFN + 0x500 * i] = bytes(d)
    put(defs, 0x528, 'i', n)
    put(defs, 0x730, 'HxxI', r.choice([id_, id_ ^ 1, r.getrandbits(16)]), r.choice([DEFN, DEFN + 0x500, DEFN + 0x500 * r.randrange(4)]))
    regions[DEFS] = bytes(defs)
    regions[TXTS] = b''.join((wtext(r, r.choice([0, 1, 5, 0x3f, 0x60])) + b'\0\0' + r.bytes(0x80))[:0x80] for _ in range(3))
    return Call(CALLS[op], [DEFS, id_ | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0), OUTW, size], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op in TEXT_OPS:
        return object_text(r, op)
    if op == 'LoadText':
        return load_text(r)
    if op == 'Cells':
        return cells(r)
    n = r.choice([0, 1, r.randrange(2, 20), r.randrange(20, 0x14b), r.randrange(-2, 0)])
    defs = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(0x14a * 0x10))
    for i in range(0x14a):
        put(defs, i * 4, 'I', ITEMS + 0x10 * r.randrange(0x14a))
        put(items, i * 0x10, 'H', r.choice([i, r.randrange(16), r.getrandbits(16)]))
    if op in ('Schema', 'TrackSchema'):
        return Call(CALLS[op], [r.getrandbits(32), r.choice([0, 1, r.randrange(64), r.getrandbits(32)])], {}, conv='t', ret='a')
    if op == 'First':
        put(defs, 0x528, 'i', n)
        return Call(0x412e60, [DEFS], {DEFS: defs, ITEMS: items}, conv='t', ret='a')
    if op == 'Next':  # the cursor about the end, past it, negative
        put(defs, 0x528, 'i', n)
        put(defs, 0x72c, 'i', max(-1, r.choice([-1, 0, n - 2, n - 1, n, r.randrange(-1, 0x14a)])))
        return Call(0x412e70, [DEFS], {DEFS: defs, ITEMS: items}, conv='t', ret='a')
    id_ = r.choice([r.randrange(16), r.randrange(0x14a), r.getrandbits(16)])
    put(defs, 0x528, 'i', n)
    put(defs, 0x730, 'HxxI', r.choice([id_, id_ ^ 1, r.getrandbits(16)]), r.choice([0, ITEMS, r.getrandbits(32)]))
    return Call(CALLS[op], [DEFS, id_ | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)],
                {DEFS: defs, ITEMS: items}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
