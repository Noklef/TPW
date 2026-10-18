# The GUI cursors (src/game/ui/cursor: 0x661d96, 0x6590f7, 0x65f11d) against the original. Get: a table of 0-6 entries (the count's upper
# word random), indexes in and out of it (upper words random). SetCursor: the table [0xfaa604] or none, its entries cursor objects (slots 4
# and 2 recorders, the byte +4 zero or not) or 0, the current id 0x77c484 and the default id 0xfaa600 in and out of it, -1 or not, the id
# passed one of those (upper words random). WindowSetCursor: that, the window the hovered one [0xfaa5e0] or not.
# usage: cursor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Get,SetCursor,WindowSetCursor)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, put, vtable  # noqa: E402

CALLS = {'Get': 0x661d96, 'SetCursor': 0x6590f7, 'WindowSetCursor': 0x65f11d}
RANGES = [(0x661d96, 0x661dca), (0x6590f7, 0x6591d4), (0x65f11d, 0x65f14e), (0x66190a, 0x66191b)]
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
    return r.choice([0xffff, current, current, default, r.randrange(max(n, 1) + 2), r.getrandbits(16)]) | r.choice([0, r.getrandbits(16)]) << 16


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
        index = r.choice([r.randrange(n + 2), r.randrange(n + 2), r.getrandbits(16)]) | r.choice([0, r.getrandbits(16)]) << 16
        return Call(CALLS[op], [TABLE, index], regions, conv='t', ret='a')
    n, current, default = cursor_env(r, regions)
    if op == 'SetCursor':
        return Call(CALLS[op], [ident(r, n, current, default)], regions, ret='v')
    regions[WINDOW] = bytearray(r.bytes(0x60))
    put(regions[0xfaa5e0], 0, 'I', r.choice([WINDOW, WINDOW, WINDOW + 0x20, 0]))
    return Call(CALLS[op], [WINDOW, ident(r, n, current, default)], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
