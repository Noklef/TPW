# The TQI video's macroblocks and their row writers (src/game/gfx/tqivideo: 0x6747d0 (op Blocks: the blocks it leaves on the stack compared, Call(stack=True)), 0x674faf,0x67519d, 0x67538b, 0x675579, 0x676290, 0x675d90, 0x6758c0) against the originals. Rows: values mostly in the tables, the tables random. Macroblocks: valid streams, as tqiblock.py's, six blocks each after its DC code by the game's own tables (x87emu.exe_sections), the dequantiser the IDCT's scales * 8 (quality 100), its AC entries shifted down, DC predictors small; out and its pitch of either sign. [0xfb792c] is 0x6758c0's saved ESP, a stack address, ignored; the port has no such slot.
# usage: tqimacroblock.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Yuy2,Rgb16,Rgb32,RowYuy2,RowRgb16,RowRgb32,Table16,Blocks)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call  # noqa: E402
from tqiblock import exe_bytes, le_dwords, number, pack, stream  # noqa: E402

ROWS = {'RowYuy2': 0x676290, 'RowRgb16': 0x675d90, 'RowRgb32': 0x6758c0}
BLOCKS = {'Yuy2': 0x674faf, 'Rgb16': 0x67519d, 'Rgb32': 0x67538b, 'Table16': 0x675579}
B = Bench([(0x6747d0, 0x675767), (0x6758c0, 0x676478), (0x67673c, 0x67687b), (0x676965, 0x676a6e), (0x673574, 0x67380f)])
STATE, LUMA, CHROMA, BITS, OUT = RAM, RAM + 0x400, RAM + 0x800, RAM + 0x1000, RAM + 0x10000
TABLES = 0xfb7000   # up to 0xfbb598, where the IDCT's constants and the decoder's own tables begin
SCALES = struct.unpack('<64I', exe_bytes(0x705c88, 0x100))
LUMA_DC, CHROMA_DC = exe_bytes(0xfbb92c, 0x80), exe_bytes(0xfbb9ac, 0x100)


def tables(r):
    return r.bytes(0xfbb598 - TABLES)


def dc_code(r, table, width):
    """A DC code: the index's first bits (the byte's low nibble) and the bits that follow (its high nibble)."""
    index = r.randrange(0, len(table))
    entry = table[index]
    length, n = entry & 0xf, entry >> 4
    return format(index, '0%db' % width)[:length].ljust(length, '0') + number(r, n)


def row(r, op):
    pick = lambda: r.choice([r.randrange(0, 0x100), r.randrange(0, 0x100), r.randrange(0, 0x200), r.randrange(-0x100, 0x400) & 0xffffffff])
    regions = {TABLES: tables(r), LUMA: struct.pack('<16I', *[pick() for _ in range(16)]), CHROMA: struct.pack('<%dI' % 0x48, *[pick() for _ in range(0x48)]), OUT: r.bytes(0x40)}
    return Call(ROWS[op], [LUMA, CHROMA, OUT], regions, conv='esiediebp', ignore=[(0xfb792c, 4)])


def macroblock(r, op):
    start = r.randrange(0, 70)
    bits = number(r, start)
    for k in range(6):
        bits += dc_code(r, LUMA_DC, 7) if k < 4 else dc_code(r, CHROMA_DC, 8)
        bits += stream(r, r.choice(['end', 'end', 'few', 'few', 'long', 'escape', 'full']))   # ('zeros' consumes nothing: the next DC code would be read from its bits)
    bits += number(r, 96)
    down = r.choice([0, 1, 2, 3])   # (the levels any: the sums stay in the colour tables as a real stream's do)
    quant = [SCALES[0] * 8 & 0xffffffff] + [(s * 8 & 0xffffffff) >> down for s in SCALES[1:]]
    state = bytearray(struct.pack('<64I', *quant))
    state += struct.pack('<3i', *[r.randrange(-40, 80) for _ in range(3)]) + r.bytes(0xf4)
    state += struct.pack('<II', BITS, start)
    pitch = r.choice([0x40, 0x80, 0x100, -0x40, -0x80])
    size = 0x2000
    out = OUT + (size - 0x100 if pitch < 0 else 0)
    regions = {TABLES: tables(r), STATE: bytes(state), BITS: le_dwords(pack(bits)), OUT: r.bytes(size)}
    if op == 'Blocks':   # 0x6747d0 alone: state, bits and position in registers; it writes the six blocks over its caller's 0x600 bytes above its return address
        return Call(0x6747d0, [STATE, BITS, start] + [r.getrandbits(32) for _ in range(0x180)], {STATE: bytes(state), BITS: regions[BITS]}, conv='esiebpedi', ret='edi', stack=True)
    return Call(BLOCKS[op], [out, pitch & 0xffffffff, STATE], regions, ignore=[(0xfb792c, 4)])


OPS = [k for k in os.environ.get('OPS', ','.join(list(BLOCKS) + list(ROWS) + ['Blocks'])).split(',') if k]


def case(r):
    op = r.deal(OPS)
    return row(r, op) if op in ROWS else macroblock(r, op)


if __name__ == '__main__':
    B.main(case)
