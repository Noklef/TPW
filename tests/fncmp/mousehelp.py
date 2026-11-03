# The help line and cursor for what the mouse is over (src/game/ui/mousehelp: MouseHelp_Update, 0x486d90) against the original. Recorders (their own benches have them; driver.cpp's kMouseHelpEnv): GetAsyncKeyState 7 (import, answers in turn), Wide_FromNarrow 8, the mode holder's slot 9, MouseModes_EnsureHolder 10,
# Highlight_Update 11, Global7c2654_Get 12, HelpBg_SetText 13, GameCursor_Set 14, Global7cc2f0_Get_4b4480 15, Map_TicketBoothPos 16, Map_EntrancePos 17 (each out written in turn), Cell_FirstObject 18, Object_GetDef 19, Object_DefWord284 20, 0x4e2780 21, Staff_Settle 22, Staff_GetName 23, Map_ObjectOrEntranceAt 24, Global81ae2c_Get_52f840 25,
# MapCell_IsType9 26, IsTypeA 27, IsType4 28, IsType0 29, Records_DataIfTerminated 31, Charset_FormatWide 30: the argument (absent for 0x243, 0x22a: the stack's, not compared) as the dwords each entry sets.
# usage: mousehelp.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE as IAT, LOG, RAM, Bench, Call, _recorder, answers, blob, blob_when, fill, import_table, log_region, put, set_return, strings, vtable  # noqa: E402

C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x486d90, 0x487998), (0x5da3c0, 0x5da3c1)],
          hooks={0x40bed0: C(10, 0), 0x469180: C(11, 0), 0x48c930: C(12, 0), 0x4924b0: C(13, 1), 0x4a2aa0: C(14, 1), 0x4b4480: C(15, 0), 0x4d8610: _recorder(16, 2), 0x4d8690: _recorder(17, 2), 0x4d99e0: _recorder(18, 1), 0x4dd4e0: _recorder(19, 0), 0x4e2710: _recorder(20, 1),
                 0x4e2780: _recorder(21, 2), 0x505f50: _recorder(22, 0), 0x506860: _recorder(23, 0), 0x527e80: C(24, 3), 0x52f840: C(25, 0), 0x536340: _recorder(26, 0), 0x536350: _recorder(27, 0), 0x536380: _recorder(28, 0), 0x536390: _recorder(29, 0),
                 0x6acd60: C(30, 5), 0x6ace60: _recorder(31, 1), 0x6adc80: _recorder(8, 4)})
IMPORTS = {'GetAsyncKeyState': (7, 0x6fd320, 1, 0, 0)}
TILES = 250
UNDER, HOLDER, VT, KEYS, DEF, WORD, FIRST, SOURCE, THINGS, WORLD = RAM, RAM + 0x300, RAM + 0x400, RAM + 0x600, RAM + 0x1000, RAM + 0x1800, RAM + 0x1810, RAM + 0x1820, RAM + 0x1900, RAM + 0x80000


def case(r):
    log = log_region()
    regions = {LOG: log, IAT: import_table(log, IMPORTS)}
    held = r.choice([(0, 0, 0)] * 6 + [(1, 0, 0)] * 3 + [(0, 1, 0), (0, 0, 1), (1, 1, 0), (r.randrange(2), r.randrange(2), r.randrange(2))])
    regions[KEYS] = struct.pack('<40I', *[(r.choice([0x8000, 0xffff8000, 0x8001]) if h else r.choice([0, 1, 0x7fff, 0xffff0000])) for h in held] + [0] * 37)
    answers(log, 7, KEYS)
    # the mode
    forced = r.choice([0, 0, 0, 1, r.getrandbits(32)])
    regions[0x7b05d8] = struct.pack('<I', r.choice([HOLDER] * 9 + [0]))
    regions[HOLDER] = struct.pack('<I', VT)
    regions[VT] = vtable({})
    set_return(log, 9, r.choice([0, 0, 0, 0, 0, 0, 1, 1, 8, 3, 3, 9, 9, 2, r.randrange(12)]))
    set_return(log, 25, r.choice([0, 0x15, 0x16, 1]))
    set_return(log, 15, r.choice([0, 0, 0, 0, 2, 2, 1, r.getrandbits(32)]))
    # what is shown
    things = bytearray(r.bytes(0x40 * 5))
    for i in range(5):
        things[0x40 * i + 2] = r.choice([3, 3, 3, 1, r.getrandbits(8)])
        put(things, 0x40 * i + 0x2c, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[THINGS] = things
    table = bytearray(r.bytes(20 * 0x100))
    for i in range(0x100):
        put(table, 20 * i, 'I', THINGS + 0x40 * (i % 5))
    regions[0x7cfb90] = table
    named = r.choice([0, 0, UNDER, THINGS, THINGS + 0x40, r.getrandbits(32)])
    previous = r.choice([0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe])
    regions[0x7c24f4] = struct.pack('<II', previous, named)
    under = bytearray(r.bytes(0x200))
    under[2] = r.choice([1, 1, 4, 5, 6, 7, 8, 0, 2, 3, r.getrandbits(8)])
    put(under, 0x1d8, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions[UNDER] = under
    regions[0x7b05ac] = struct.pack('<I', r.choice([0, 0, 0, UNDER]))
    # the tile
    tile = r.choice([r.randrange(1, TILES), r.randrange(1, TILES), r.randrange(1, TILES), 0x7f, 0x80, 0x81, 1, 0])
    regions[0x7b05cc] = struct.pack('<H', tile)
    regions[0x7cf83c] = struct.pack('<I', WORLD)

    def near(*extra):
        return r.choice([r.randrange(TILES, 0x4000)] * 12 + list(extra) + [0, 0xffff]) & 0xffff
    regions[0x7a1c7c] = struct.pack('<I', r.choice([0, 0, 1]))
    fill(log, 17, 0, struct.pack('<HH', near(tile, tile + 1), near(tile, tile - 1)), records=2)
    fill(log, 16, 0, struct.pack('<HH', near(tile), near(tile)), which=1, records=2)
    for k in (16, 17):
        strings(log, k, 0, out=1)
    fill(log, 24, 0, struct.pack('<HH', r.choice([0, 0, 1, 2, 3, r.randrange(0x100)]), r.getrandbits(16)), which=2)
    strings(log, 24, 0, out=1)
    set_return(log, 24, RAM + 0x800)
    cells = bytearray(r.bytes(0x44 * TILES))
    records = bytearray(r.bytes(40 * TILES))
    for i in range(TILES):
        put(cells, 0x44 * i + 8, 'I', r.choice([1, 3, 4, 9, 0xa, 0x15, 0x15, 0x14, 0, 2, 5, 0x16, r.getrandbits(32)]))
        put(cells, 0x44 * i + 0x10, 'H', r.randrange(TILES))
        put(records, 40 * i + 8, 'I', r.choice([0xb, 0xc, 0x10, 0x19, 0x19, 0x18, 0, 1, r.getrandbits(32)]))
        put(records, 40 * i + 0x10, 'H', r.randrange(TILES))
    regions[WORLD + 0x294] = cells
    regions[WORLD + 0x1102b0] = records
    # the calls' answers
    first = r.choice([0, 1, 2, 3, r.randrange(0x100)])
    fill(log, 18, 0, struct.pack('<H', first), which=3)
    strings(log, 18, 0, out=1)
    set_return(log, 18, FIRST)
    regions[FIRST] = struct.pack('<H', r.choice([first, r.randrange(0x100)]))
    d = bytearray(r.bytes(0x500))
    put(d, 0x288, 'I', r.choice([2, 2, 1, 0]))
    put(d, 0x4ac, 'I', r.choice([0, 0, 1]))
    regions[DEF] = d
    set_return(log, 19, DEF)
    regions[WORD] = struct.pack('<I', r.choice([0, 0, 1, 0x10000]))
    set_return(log, 20, WORD)
    strings(log, 20, 0, out=1)
    set_return(log, 12, r.choice([THINGS, THINGS + 0x40, r.getrandbits(32)]))
    set_return(log, 23, r.choice([RAM + 0x900, r.getrandbits(32)]))
    for k, weights in ((26, [0, 0, 0, 1]), (27, [0, 0, 0, 2]), (28, [0, 0, 0, 1]), (29, [0, 0, 1])):
        set_return(log, k, r.choice(weights))
    # the texts
    regions[0x78ba98] = r.bytes(4)
    regions[0x78ba7c] = r.bytes(4)
    regions[0x78ba80] = struct.pack('<I', r.choice([0, 1, r.getrandbits(32)]))
    regions[SOURCE] = r.bytes(8)
    set_return(log, 31, r.choice([SOURCE, SOURCE, 0]))
    strings(log, 8, 0, out=1)
    blob(log, 30, 4, 0)
    for i, (entry, mask) in enumerate(((0x229, 0x1f), (0x22b, 0xf), (0x226, 0xf), (0x244, 0xf))):
        blob_when(log, 30, 3, entry, mask, i)
    return Call(0x486d90, [forced], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
