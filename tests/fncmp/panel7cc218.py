# The game panel's choice (src/game/ui/panel7cc218: Panel7cc218_Choose, 0x4af480) against the original. The money record (the game
# [0x7cf83c]'s handle +0x1da726 in the thing slots 0x7cfb90) a block with its balance +0xc about the costs, the shown thing [0x7c2658].
# Close (close 1) and Open (index -1 to -4): objectstate.py's Open set-up (its object the shown thing; its definition through
# ObjectDefs_Find's cache 0x7897d0, the cost at +0x1b8 - index * 0x40 about the balance); close any other byte than 1 now and then for
# Open. Tile and Short (index 0 or more; the shown thing none or one with no handle +0x30): modeenter.py's Tile15 set-up (the new tile mode made current, the focus cell updated, the
# record on recorded_actions.py's recorder; the index a definition found as mousetile.py's Tile sets them up), the object control (ObjectControls_Find on the game's +0x1d943c: its cache, else a search
# of a few records) with the cost +4 at most the balance (Tile) or over it (Short: soundplay.py's UI sound, the manager's slot 2 a
# recorder).
# usage: panel7cc218.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Close,Open,Tile,Short)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cc5f4  # noqa: E402
import modeenter  # noqa: E402
import objectstate  # noqa: E402
import recorded_actions as ra  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, blob, put, set_return, vtable  # noqa: E402

B = Bench(dialog7cc5f4.merged(modeenter.RANGES + objectstate.RANGES
                              + [(0x4af480, 0x4af5f2), (0x5195d0, 0x51960d), (0x6ad810, 0x6ad814), (0x4d3d10, 0x4d3d80), (0x486ae0, 0x486af9),
                                 (0x51bfc0, 0x51c048), (0x46c5a0, 0x46c5c6), (0x5da3c0, 0x5da3c1), (0x48c940, 0x48c94a), (0x52f880, 0x52f8e0), (0x52fa90, 0x52fac0), (0x51ab20, 0x51ab70), (0x412e90, 0x412f00)]),
          hooks={**modeenter.HOOKS, **objectstate.HOOKS})
OPS = [k for k in os.environ.get('OPS', 'Close,Open,Tile,Short').split(',') if k]
GAME, MONEY, CTL = RAM + 0x60000, RAM + 0x5f000, RAM + 0x5f100
SOUND, SOUND_VT, ITEMS, NDEFS = RAM + 0x5f200, RAM + 0x5f300, RAM + 0x58000, 24
THING = RAM + 0x5f400
HANDLE = 0x300


def money(r, regions):
    """the game's money handle and record: its balance (+0xc) returned"""
    regions[0x7cf83c] = struct.pack('<I', GAME)
    regions[GAME + 0x1da726] = struct.pack('<H', HANDLE)
    at = 0x7cfb90 + 20 * HANDLE
    slots = regions.get(0x7cfb90)
    if slots is not None and len(slots) >= 20 * HANDLE + 4:
        regions[0x7cfb90] = put(bytearray(slots), 20 * HANDLE, 'I', MONEY)
    else:
        regions[at] = struct.pack('<I', MONEY)
    balance = r.choice([0, 1, 100, 5000, -1, -5000, r.getrandbits(31), r.getrandbits(32) - (1 << 31)])
    regions[MONEY] = put(bytearray(r.bytes(0x10)), 0xc, 'i', balance)
    return balance


def cost(r, balance, over):
    """a cost the balance is at most (over 0) or under (over 1), now and then the balance itself"""
    if over:
        return balance + r.choice([1, 1, 2, 100, r.randrange(1 << 20)]) if balance < 0x7fff0000 else balance
    return balance - r.choice([0, 0, 1, 100, r.randrange(1 << 20)]) if balance > -0x7fff0000 else balance


def case(r):
    op = r.deal(OPS)
    if op in ('Close', 'Open'):
        objectstate.OPS = ['Open']
        c = objectstate.case(r)
        regions = c.regions
        balance = money(r, regions)
        obj = regions[objectstate.OBJ]
        regions[0x7c2658] = struct.pack('<I', objectstate.OBJ)
        if op == 'Close':
            return Call(0x4af480, [0x7cc218, r.choice([0, -1, 3, r.getrandbits(32)]), 1 | r.getrandbits(24) << 8], regions, conv='t', ret='a')
        index = r.choice([-1, -1, -2, -3, -4])
        oid = struct.unpack_from('<H', obj, 0xe)[0]
        regions[0x7897d0] = struct.pack('<HHI', oid, r.getrandbits(16), objectstate.DEF)
        regions[objectstate.DEF] = put(bytearray(r.bytes(0x300)), 0x1b8 - index * 0x40, 'i', cost(r, balance, r.random() < 0.4))
        close = r.choice([0, 0, 0, 2, 0xff])
        return Call(0x4af480, [0x7cc218, index, close | r.getrandbits(24) << 8], regions, conv='t', ret='a')
    modeenter.OPS = ['Tile15']
    c = modeenter.case(r)
    regions = c.regions
    balance = money(r, regions)
    regions[0x7c2658] = struct.pack('<I', r.choice([THING, THING, 0]))   # (selected, then the focus cell's: its +0x30 handle none)
    regions[THING] = put(bytearray(r.bytes(0x40)), 0x30, 'H', 0)
    for address, size in {0x81b0c8: 0xc, 0x81eca8: 0xc, 0x8186d8: 0x20, 0x8187f0: 0x10}.items():
        regions[address] = r.bytes(size)
    defs = bytearray(r.bytes(0x740))   # (the new mode's Assign: its definition as mousetile.py's Tile sets it up)
    items = bytearray(r.bytes(NDEFS * 0x300))
    n = r.choice([1, r.randrange(1, NDEFS + 1), NDEFS])
    for i in range(NDEFS):
        put(defs, i * 4, 'I', ITEMS + 0x300 * i)
        put(items, 0x300 * i, 'H', i + 1)
        put(items, 0x300 * i + 0x94, 'I', r.choice([0, 1, 8, 9, 256, r.randrange(1, 257), r.randrange(1, 17)]))
        put(items, 0x300 * i + 0x288, 'I', r.choice([1, 1, 0, 2, r.getrandbits(32)]))
        put(items, 0x300 * i + 0x40, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(defs, 0x528, 'i', n)
    put(defs, 0x730, 'HxxI', 0, 0)   # (no cache hit)
    regions.update({ra.DEFS: defs, ITEMS: items, ra.BUILT: r.bytes(0x20)})
    index = r.randrange(1, n + 1) | r.choice([0, 0, 0, 0x10000, r.getrandbits(15) << 16])   # (found: the original reads through it)
    table = bytearray(0x12d0)
    count = r.choice([1, 2, 4])
    for i in range(count):
        put(table, 0x20 * i, 'H', r.getrandbits(16))
    if r.random() < 0.6:   # the cache
        put(table, 0x12c4, 'HHI', index & 0xffff, 0, CTL)
    else:
        put(table, 0x12c4, 'H', (index + 1) & 0xffff)
        put(table, 0x20 * r.randrange(count), 'H', index & 0xffff)
    put(table, 0x12c0, 'i', count)
    regions[GAME + 0x1d943c] = table
    regions[CTL] = put(bytearray(r.bytes(0x20)), 4, 'i', cost(r, balance, op == 'Short'))
    if op == 'Short':   # (the record found by the search is the table's own: its cost there)
        for i in range(count):
            put(table, 0x20 * i + 4, 'i', cost(r, balance, True))
        log = regions[LOG]
        g = bytearray(r.bytes(0x20))   # 0x802bc0..: the sound on and started, the manager's slot 2 a recorder
        put(g, 8, 'I', 1)
        put(g, 0xc, 'I', SOUND)
        put(g, 0x14, 'I', 1)
        blob(log, 2, 1, 0x7f)
        set_return(log, 2, r.getrandbits(32))
        regions.update({0x802bc0: g, SOUND: struct.pack('<I', SOUND_VT) + r.bytes(0xc), SOUND_VT: vtable({2: 2}, 4), 0x803a2c: r.bytes(4)})
    else:
        for i in range(count):
            put(table, 0x20 * i + 4, 'i', cost(r, balance, False))
    return Call(0x4af480, [0x7cc218, index, r.choice([0, 0, 2, 0xff])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
