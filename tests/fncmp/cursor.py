# The GUI cursors (src/game/ui/cursor: 0x661d96, 0x6590f7, 0x65f11d) against the original. Get: a table of 0-6 entries (the count's upper
# word random), indexes in and out of it (some out only by their high byte; upper words random). SetCursor: the table [0xfaa604] or none,
# its entries cursor objects (slots 4 and 2 recorders, the byte +4 zero or not) or 0, the current id 0x77c484 and the default id 0xfaa600
# in and out of it, -1 or not, the id passed one of those (some out only by the high byte; upper words random). WindowSetCursor: that, the
# window the hovered one [0xfaa5e0] or not. ApplyCursor (0x65f150): the same with the window's word +0x50 the id. Set / ObjSet (0x4a2aa0,
# 0x4814a0): the windows [0x7cb2ac] (mostly), [0x7cc8c0] and [0x7cc8bc] each set or not (one of them now and then the hovered one). Update
# (0x52f950): that, the game mode 0x81ae2c each of the switch's (others too) and the dwords 0x816d48, 0x816d4c, 0x816d5c, 0x816d60 0 (mostly) or not.
# Clear (0x661c36): Get's table (its entries' slot 0 a recorder), the entries now and then none; the heap (Mem_Free). Init (0x661cca): a
# table of random bytes, the count 0 to 0x3ff0 (upper half random), the allocation failing now and then. SetTable (0x659248):
# SetCursor's globals, the current cursor [0xfaa608] one of the objects or 0, the new table that one, another (its count 0-6) or none.
# TableSet (0x661d3f): Get's table, Get's indexes, the cursor one of the objects, 0 or any. LoadAni / LoadTga (0x489720, 0x4899c0): the
# table 0x7b9e88 / 0x7b9ab0 (its count 0x16, past it, short of it or 0; its entries mostly empty); the loaders 0x4895b0 / 0x489670 the cdecl
# recorders k 28 / 29 (the name hashed as a string; driver.cpp's GameCursors env), answering one cursor object each time, the name itself
# (the game's string: the carry and crosshair names mapped, whose +5 / +6 then get written), 0, their hashes or their hashes for the first
# few then 0 (for the .tga ones never past the carry cursor's, whose +5 a hash would be).
# usage: cursor.py [CASES [SEED]] [--wild|--both] [--show N]
# Colour (0x661e30): palcolour.py's Set cases. SysCtor (0x661935): any bytes. Delete (0x661dd0): any bytes, the flags 0-3 or any (the block freed with bit 0).
# Draw (0x65921f) / Arrow (0x661a06): draw_case() below; the point's words small, edges or any (upper halves random now and then).
#   (env OPS=Get,SetCursor,WindowSetCursor,ApplyCursor,Set,ObjSet,Update,Clear,Init,SetTable,TableSet,LoadAni,LoadTga)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, echo, fail_after, heap, log_region, put, set_return, strings, vtable  # noqa: E402

CALLS = {'Get': 0x661d96, 'SetCursor': 0x6590f7, 'WindowSetCursor': 0x65f11d, 'ApplyCursor': 0x65f150, 'Set': 0x4a2aa0, 'ObjSet': 0x4814a0,
         'Update': 0x52f950, 'Clear': 0x661c36, 'Init': 0x661cca, 'SetTable': 0x659248, 'TableSet': 0x661d3f, 'LoadAni': 0x489720,
         'LoadTga': 0x4899c0, 'SysCtor': 0x661935, 'Delete': 0x661dd0,
         'Colour': 0x661e30, 'Draw': 0x65921f, 'Arrow': 0x661a06}
RANGES = [(0x661d96, 0x661dca), (0x6590f7, 0x6591d4), (0x65f11d, 0x65f14e), (0x66190a, 0x66191b), (0x65f150, 0x65f16b),
          (0x4a2aa0, 0x4a2abf), (0x4814a0, 0x4814e3), (0x52f950, 0x52fa1a), (0x527950, 0x527956), (0x661c36, 0x661cca), (0x661cca, 0x661d3f), (0x659248, 0x6592b6),
          (0x661d3f, 0x661d96), (0x489720, 0x4899b5), (0x4899c0, 0x489c71), (0x6618c7, 0x6618e6),
          (0x661935, 0x66195e), (0x661dd0, 0x661dfe), (0x661870, 0x6618a6), (0x6618a6, 0x6618ba),
          (0x661e30, 0x661e54), (0x47ddc0, 0x47ddf2), (0x5f7360, 0x5f76b7),
          (0x65921f, 0x659248), (0x661a06, 0x661a93), (0x5f9780, 0x5f9eb0), (0x61af60, 0x61b040), (0x61b090, 0x61b1f0),
          (0x62f890, 0x62f8e0), (0x62f940, 0x62f990)]
LOADERS = {'LoadAni': (0x7b9e88, 28), 'LoadTga': (0x7b9ab0, 29)}
B = Bench(RANGES, hooks={0x4895b0: _recorder(28, 1, cdecl=True), 0x489670: _recorder(29, 1, cdecl=True)})
TABLE, ENTRIES, CURSORS, CVTABLE, WINDOW = RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x800, RAM + 0x1000
SLOTS, LOADED = RAM + 0x2000, RAM + 0x2200
DRAWN, DRAWVT, DTABLE, DINFO = RAM + 0x3000, RAM + 0x3100, RAM + 0x3200, RAM + 0x3300   # (Draw, Arrow)
DSTATE = 0xfa76b8


def draw_case(r, op, log, regions):
    """Draw: the current cursor [0xfaa608] a recorder object (slot 5; its two words logged as words) or 0. Arrow: display.py's world
    (the clip near the point, the draw table's slot 10 a recorder whose colour index is logged as its byte), the palette random."""
    words = [r.choice([0, 5, 100, 0x7fff, 0x8000, 0xffff, r.randrange(-30, 120) & 0xffff]) | r.choice([0, r.getrandbits(16)]) << 16
             for _ in range(2)]
    if op == 'Draw':
        put(log, LOG_BYTES + 4 * 5, 'I', 3 << 16)
        regions[DRAWN] = struct.pack('<I', DRAWVT) + r.bytes(0x1c)
        regions[DRAWVT] = vtable({5: 2}, 8)
        regions[0xfaa608] = struct.pack('<I', r.choice([DRAWN] * 5 + [0]))
        return Call(CALLS[op], words, regions, ret='v')
    put(log, LOG_BYTES + 4 * 10, 'I', 1 << 7)
    state = bytearray(r.bytes(0x74))
    xs, ys = sorted([r.randrange(-20, 100), r.randrange(-20, 100)]), sorted([r.randrange(-20, 100), r.randrange(-20, 100)])
    put(state, 8, '4i', xs[0], ys[0], xs[1], ys[1])
    put(state, 0x18, 'I', DSTATE + r.choice([0x38, 0x40, 0x44, 0x48]))
    put(state, 0x30, 'I', DINFO)
    for at in (0x38, 0x40, 0x44, 0x48):
        put(state, at, 'I', DTABLE)
    regions.update({DSTATE: state, DTABLE: vtable({10: 8}, 16), DINFO: r.bytes(0x28), 0xfa71f8: r.bytes(0x400), DRAWN: r.bytes(0x20)})
    return Call(CALLS[op], [DRAWN] + words, regions, conv='t', ret='v')


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
    if op in ('Draw', 'Arrow'):
        return draw_case(r, op, log, regions)
    if op == 'Colour':   # (palcolour.py's Set: the colour in, near or off a random palette; junk in the high bits)
        pal = r.bytes(0x400)
        k = r.randrange(256)
        rgb = [pal[k * 4 + i] for i in range(3)] if r.random() < 0.5 else [r.getrandbits(8) for _ in range(3)]
        rgb = [(v + r.choice([0, 0, 1, -1])) & 0xff | (r.getrandbits(24) << 8 if r.random() < 0.3 else 0) for v in rgb]
        return Call(CALLS[op], [CURSORS] + rgb, {CURSORS: r.bytes(0x20), 0xfa71f8: pal}, conv='t', ret='a')
    if op == 'SysCtor':
        return Call(CALLS[op], [CURSORS], {CURSORS: r.bytes(0x20)}, conv='t', ret='a')
    if op == 'Delete':
        return Call(CALLS[op], [CURSORS, r.choice([0, 1, 2, 3, 0xffffffff])], {CURSORS: r.bytes(0x20), HEAP: heap(0x20)}, conv='t', ret='a')
    if op == 'Get':
        n = table(r, regions)
        index = r.choice([r.randrange(n + 2), r.randrange(n + 2), r.randrange(n + 2) | r.randrange(1, 256) << 8, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16)]) << 16
        return Call(CALLS[op], [TABLE, index], regions, conv='t', ret='a')
    if op == 'Init':
        regions = {TABLE: r.bytes(8), HEAP: heap(0x10000, r.choice([0] * 5 + [1]))}
        count = r.choice([0, 1, 2, 6, 0x100, 0x3ff0, r.randrange(0x3ff0)]) | r.choice([0, r.getrandbits(16)]) << 16
        return Call(CALLS[op], [TABLE, count], regions, conv='t', ret='w')
    if op == 'Clear':
        table(r, regions)
        regions[CVTABLE] = vtable({0: 1, 4: 1})
        regions[HEAP] = heap()
        if r.random() < 0.15:
            put(regions[TABLE], 0, 'I', 0)
        return Call(CALLS[op], [TABLE], regions, conv='t', ret='v')
    if op == 'TableSet':
        n = table(r, regions)
        index = r.choice([r.randrange(n + 2), r.randrange(n + 2), r.randrange(n + 2) | r.randrange(1, 256) << 8, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16)]) << 16
        cursor = r.choice([CURSORS + 0x20 * r.randrange(8)] * 4 + [0, r.getrandbits(32)])
        return Call(CALLS[op], [TABLE, index, cursor], regions, conv='t', ret='w')
    if op in LOADERS:
        at, k = LOADERS[op]
        count = r.choice([0x16, 0x16, 0x16, 0x20, 0x15, 0x10, 0, r.randrange(0x30)])
        regions[at] = put(put(bytearray(r.bytes(8)), 0, 'I', SLOTS), 4, 'H', count)
        slots = bytearray(0xc0)
        for i in range(0x30):
            if r.random() < 0.1:
                put(slots, 4 * i, 'I', r.getrandbits(32) | 1)
        regions[SLOTS] = slots
        regions[LOADED] = r.bytes(0x10)
        strings(log, k, 1)
        answer = r.choice(['one', 'echo', 'echo', 'zero', 'some', 'some'] + (['hash'] if op == 'LoadAni' else []))
        if op == 'LoadTga':   # (the carry and crosshair cursors' names, which an echoed name has their +5 / +6 written)
            regions[0x74fff4] = b'Ccar.tga\0'
            regions[0x74ff58] = b'CCro.tga\0'
        if answer == 'echo':
            echo(log, k, 0)
        elif answer == 'one':
            set_return(log, k, LOADED)
        elif answer == 'zero':
            set_return(log, k, 0)
        elif answer == 'some':
            fail_after(log, k, r.randrange(0, 23 if op == 'LoadAni' else 9), 0)
        return Call(CALLS[op], [], regions, ret='v')
    n, current, default = cursor_env(r, regions)
    if op == 'SetCursor':
        return Call(CALLS[op], [ident(r, n, current, default)], regions, ret='v')
    if op == 'SetTable':
        other = RAM + 0x180
        regions[other] = put(put(bytearray(r.bytes(8)), 0, 'I', ENTRIES), 4, 'H', r.choice([0, 1, 3, 6]))
        put(regions[0xfaa5e0], 0x28, 'I', r.choice([CURSORS, CURSORS + 0x20, 0]))
        return Call(CALLS[op], [r.choice([other, other, TABLE, 0])], regions, ret='v')
    if op in ('Set', 'ObjSet', 'Update'):
        wins = [WINDOW + 0x80 * i for i in range(3)]
        for w in wins:
            regions[w] = bytearray(r.bytes(0x60))
        put(regions[0xfaa5e0], 0, 'I', r.choice(wins + [0]))
        game = bytearray(r.bytes(0x38))   # 0x7cb2a8..0x7cb2df
        put(game, 4, 'I', r.choice([wins[0], wins[0], wins[0], 0]))
        regions[0x7cb2a8] = game
        regions[0x7cc8bc] = put(bytearray(8), 0, 'II', r.choice([wins[1], 0]), r.choice([wins[2], 0]))
        if op == 'Update':
            regions[0x81ae2c] = struct.pack('<I', r.choice([1, 3, 4, 0xb, 0x10, 0x15, 0x16, 0x39, 0x3b, 0, 2, 0x3a, 0x3c, r.randrange(0x40),
                                                             r.getrandbits(32)]))
            regions[0x816d48] = bytearray(r.bytes(0x1c))
            for off in (0, 4, 0x14, 0x18):
                put(regions[0x816d48], off, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
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
