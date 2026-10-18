# The GUI cursors (src/game/ui/cursor: 0x661d96, 0x6590f7, 0x65f11d) against the original. Get: a table of 0-6 entries (the count's upper
# word random), indexes in and out of it (some out only by their high byte; upper words random). SetCursor: the table [0xfaa604] or none,
# its entries cursor objects (slots 4 and 2 recorders, the byte +4 zero or not) or 0, the current id 0x77c484 and the default id 0xfaa600
# in and out of it, -1 or not, the id passed one of those (some out only by the high byte; upper words random). WindowSetCursor: that, the
# window the hovered one [0xfaa5e0] or not. ApplyCursor (0x65f150): the same with the window's word +0x50 the id. Set / ObjSet (0x4a2aa0,
# 0x4814a0): the windows [0x7cb2ac], [0x7cc8c0] and [0x7cc8bc] each set or not (one of them now and then the hovered one). Update
# (0x52f950): that, the game mode 0x81ae2c each of the switch's (others too) and the dwords 0x816d48, 0x816d4c, 0x816d5c, 0x816d60 0 or not.
# usage: cursor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Get,SetCursor,WindowSetCursor,ApplyCursor,Set,ObjSet,Update)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, put, vtable  # noqa: E402

CALLS = {'Get': 0x661d96, 'SetCursor': 0x6590f7, 'WindowSetCursor': 0x65f11d, 'ApplyCursor': 0x65f150, 'Set': 0x4a2aa0, 'ObjSet': 0x4814a0,
         'Update': 0x52f950}
RANGES = [(0x661d96, 0x661dca), (0x6590f7, 0x6591d4), (0x65f11d, 0x65f14e), (0x66190a, 0x66191b), (0x65f150, 0x65f16b),
          (0x4a2aa0, 0x4a2abf), (0x4814a0, 0x4814e3), (0x52f950, 0x52fa1a), (0x527950, 0x527956)]
B = Bench(RANGES)
TABLE, ENTRIES, CURSORS, CVTABLE, WINDOW = RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x800, RAM + 0x1000


def table(r, regions):
    """the cursor table at TABLE: its entries cursor objects or 0; returns the count"""
    n = r.choice([0, 1, 2, 3, 4, 5, 6])
    buf = bytearray(r.bytes(8))
    put(buf, 0, 'I', ENTRIES)
    put(buf, 4, 'H', n)
    regions[TABLE] = buf
    entries = bytearray(r.bytes(4 * 8))
    for i in range(8):
        put(entries, 4 * i, 'I', r.choice([CURSORS + 0x20 * i] * 4 + [0]))
        cursor = bytearray(r.bytes(0x20))
        put(cursor, 0, 'I', CVTABLE)
        cursor[4] = r.choice([0, 0, 1, 0x80, r.getrandbits(8)])
        regions[CURSORS + 0x20 * i] = cursor
    regions[ENTRIES] = entries
    regions[CVTABLE] = vtable({4: 1})
    return n


def ident(r, n, current, default):
    return r.choice([0xffff, current, current, default, r.randrange(max(n, 1) + 2), r.randrange(n + 2) | r.randrange(1, 255) << 8, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16)]) << 16


def cursor_env(r, regions):
    """the GUI cursor globals (the table or none, the current and default ids); returns (count, current, default)"""
    n = table(r, regions)
    current = r.choice([0xffff, r.randrange(n + 2), r.randrange(n + 2), r.getrandbits(16)])
    default = r.choice([r.randrange(n + 2), r.randrange(max(n, 1)), 0xffff])
    gui = bytearray(r.bytes(0x30))   # 0xfaa5e0..0xfaa60f
    put(gui, 0x20, 'H', default)
    put(gui, 0x24, 'I', r.choice([TABLE, TABLE, TABLE, 0]))
    regions[0xfaa5e0] = gui
    regions[0x77c484] = put(bytearray(r.bytes(4)), 0, 'H', current)
    return n, current, default


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    if op == 'Get':
        n = table(r, regions)
        index = r.choice([r.randrange(n + 2), r.randrange(n + 2), r.randrange(n + 2) | r.randrange(1, 256) << 8, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16)]) << 16
        return Call(CALLS[op], [TABLE, index], regions, conv='t', ret='a')
    n, current, default = cursor_env(r, regions)
    if op == 'SetCursor':
        return Call(CALLS[op], [ident(r, n, current, default)], regions, ret='v')
    if op in ('Set', 'ObjSet', 'Update'):
        wins = [WINDOW + 0x80 * i for i in range(3)]
        for w in wins:
            regions[w] = bytearray(r.bytes(0x60))
        put(regions[0xfaa5e0], 0, 'I', r.choice(wins + [0]))
        game = bytearray(r.bytes(0x38))   # 0x7cb2a8..0x7cb2df
        put(game, 4, 'I', r.choice([wins[0], 0]))
        regions[0x7cb2a8] = game
        regions[0x7cc8bc] = put(bytearray(8), 0, 'II', r.choice([wins[1], 0]), r.choice([wins[2], 0]))
        if op == 'Update':
            regions[0x81ae2c] = struct.pack('<I', r.choice([1, 3, 4, 0xb, 0x10, 0x15, 0x16, 0x39, 0x3b, 0, 2, 0x3a, 0x3c, r.randrange(0x40),
                                                             r.getrandbits(32)]))
            regions[0x816d48] = bytearray(r.bytes(0x1c))
            for off in (0, 4, 0x14, 0x18):
                put(regions[0x816d48], off, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
            return Call(CALLS[op], [], regions, ret='v')
        args = [ident(r, n, current, default)]
        return Call(CALLS[op], ([RAM] if op == 'ObjSet' else []) + args, regions, conv='t' if op == 'ObjSet' else 'c', ret='v')
    regions[WINDOW] = bytearray(r.bytes(0x60))
    put(regions[0xfaa5e0], 0, 'I', r.choice([WINDOW, WINDOW, WINDOW + 0x20, 0]))
    if op == 'ApplyCursor':
        put(regions[WINDOW], 0x50, 'H', ident(r, n, current, default) & 0xffff)
        return Call(CALLS[op], [WINDOW], regions, conv='t', ret='v')
    return Call(CALLS[op], [WINDOW, ident(r, n, current, default)], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
