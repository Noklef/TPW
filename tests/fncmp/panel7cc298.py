# The game panel class of the object at 0x7cc298 (src/game/ui/panel7cc298: 0x4b1ee0) against the original. Update: the panel's slots 5, 6 and
# 13 recorders (13's result set), its object +0x20 one with slot 9 a recorder; the thing [0x7c2658] with its dwords +0x68 (0 or not), +0x188,
# +0x194 and byte +0x190 any, its definition (gObjectDefs 0x7890a0 as object.py's: a few, the thing's found, the cache hit or not) with its
# dword +0x10c 0 or not; the window +0x10's children (dialog7cb398.py's lists: decoys about the children 0xa08e, 0xa08f, 0xa090, 0xa0a5-0xa0a7,
# their slots 7 / 9 recorders and their own children none or window.py's tree for Window_SetShown) and the 0x705a50 button 0xa08a (as in
# window705a50.py, now and then missing), its +0x98 / +0x9c objects recorders (k 23 / 24) masked as that bench says by the look it gets.
# The texts 0x33 / 0x34 as language.py's (Language_WideText): a record table about them, the converter on or off.
# usage: panel7cc298.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Update)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, blob_bytes, log_region, put, recorder, set_return, vtable  # noqa: E402
from dialog7cb398 import KVTABLE, children, window  # noqa: E402
from language import text  # noqa: E402
from window import RANGES as WINDOW_RANGES, shown_env  # noqa: E402
from window705a50 import EVENT_MASK, INNER, NOTIFY_MASK, RANGES as BUTTON_RANGES, look, window as button  # noqa: E402

CALLS = {'Update': 0x4b1ee0}
B = Bench([(0x4b1ee0, 0x4b20a4), (0x4e0440, 0x4e0450), (0x4e21b0, 0x4e21b7), (0x4e1a00, 0x4e1a07), (0x4e1a10, 0x4e1a17), (0x4dd4e0, 0x4dd4fb),
           (0x412e90, 0x412eea), (0x485b00, 0x485b40), (0x6adc80, 0x6add25), (0x6ace60, 0x6acea0)] + WINDOW_RANGES + BUTTON_RANGES)
PANEL, OBJECT, THING, ROOT, KIDS, DECOYS = 0x33100000, 0x33110000, 0x33120000, 0x33130000, 0x33140000, 0x33150000
OBJS, DEFS, ITEMS, TEXTS = 0x33160000, 0x7890a0, 0x33170000, 0x33180000
IDS = [0xa08e, 0xa08f, 0xa090, 0xa0a5, 0xa0a6, 0xa0a7]


def defs(r, regions, thing):
    """gObjectDefs with 1-4 definitions, the thing's id one of theirs; returns the thing's"""
    table = bytearray(r.bytes(0x740))
    n = r.randrange(1, 5)
    ids = r.sample(range(0x10000), n)
    for i in range(n):
        put(table, 4 * i, 'I', ITEMS + 0x200 * i)
        item = put(bytearray(r.bytes(0x200)), 0, 'H', ids[i])
        put(item, 0x10c, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        regions[ITEMS + 0x200 * i] = item
    put(table, 0x528, 'i', n)
    k = r.randrange(n)
    put(thing, 0xe, 'H', ids[k])
    put(table, 0x730, 'HHI', r.choice([ids[k], r.getrandbits(16)]), 0, ITEMS + 0x200 * k)
    regions[DEFS] = table


def texts(r, regions):
    """the language object's converter (on or off) and record table [0x78ba94] about the indexes 0x33 / 0x34"""
    lang = bytearray(r.bytes(0x600))
    conv = bytearray(r.bytes(0x10))
    put(conv, 8, 'BBHI', r.choice([0x80, 0x81, 0xa0, 0, 0xff, r.getrandbits(8)]), r.getrandbits(8), r.choice([0, 3, 0x80, 0x400, 0x7ff]), TEXTS + 0x4000)
    put(lang, 0x54c, 'II', TEXTS + 0x800, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    table = bytearray(r.bytes(0x20))
    put(table, 8, 'II', r.choice([0x35, 0x35, 0x35, 0x34, 5]), TEXTS + 0xb00 - 4 * 0x33)
    put(lang, 0x564, 'I', TEXTS + 0xa00)
    pool = bytearray()
    for i in range(2):
        body = text(r) + bytes([0]) * r.choice([1, 1, 2])
        head = r.choice([1] * 6 + [2, 0x101]) | (len(body) << 8 if r.random() < 0.9 else r.randrange(0x40) << 8)
        rec = bytearray(struct.pack('<I', head) + body + bytes(0x40 - 4 - len(body)))
        if r.random() < 0.15:
            rec[4 + ((len(body) + 3) & ~3)] = 1   # (not terminated)
        pool += rec
    regions.update({0x78b530: lang, TEXTS + 0x800: conv, TEXTS + 0x4000: r.bytes(0x1000), 0x7ba058: bytes([0x55]) * 0x404, TEXTS + 0xa00: table,
                    TEXTS + 0xc00: bytes(pool), TEXTS + 0xb00: struct.pack('<2I', TEXTS + 0xc00, TEXTS + 0xc40)})


def case(r):
    r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    wins = shown_env(r, regions, log)
    kids = [(KIDS + 0x200 * k, ident) for k, ident in enumerate(IDS)]
    for addr, _ in kids:
        window(r, regions, addr, ROOT, r.choice([0, 0, wins[0]]))
    regions[KVTABLE] = vtable({7: 1, 9: 1})
    btn = button(r)
    if r.random() < 0.9:
        put(btn, 0x98, 'I', OBJS if r.random() < 0.8 else 0)
        put(btn, 0x9c, 'I', OBJS + 0x40 if r.random() < 0.8 else 0)
        put(btn, 0x5c, 'I', 0)
        regions[KIDS + 0x1000] = btn
        kids.append((KIDS + 0x1000, 0xa08a))
    for i, k in enumerate((23, 24)):
        vt = vtable({}, size=8)
        put(vt, 0xc, 'I', recorder(k, 2))
        regions[OBJS + 0x40 * i] = put(bytearray(r.bytes(0x20)), 0, 'I', OBJS + 0x40 * i + 0x20) + vt
    window(r, regions, ROOT, 0, 0)
    put(regions[ROOT], 0x5c, 'I', children(r, regions, kids, ROOT, DECOYS, [0, 0xa08b, 0xa0a4, 0xa08d, 0xa091]))
    thing = bytearray(r.bytes(0x200))
    put(thing, 0x68, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    defs(r, regions, thing)
    regions[THING] = thing
    regions[0x7c2658] = struct.pack('<I', THING)
    texts(r, regions)
    after = bytearray(btn)   # (the look the notification sees: after the state is set)
    after[0x138] = 1 if struct.unpack_from('<I', thing, 0x68)[0] == 0 else 0
    blob_bytes(log, 23, 1, EVENT_MASK)
    blob_bytes(log, 24, 1, NOTIFY_MASK, None if look(after) == 1 else INNER)
    panel = bytearray(r.bytes(0x40))
    put(panel, 0, 'I', PANEL + 0x40)
    put(panel, 0x10, 'I', ROOT)
    put(panel, 0x20, 'I', OBJECT)
    regions[PANEL] = panel
    regions[PANEL + 0x40] = vtable({})
    regions[OBJECT] = struct.pack('<I', OBJECT + 0x40) + r.bytes(0x1c)
    regions[OBJECT + 0x40] = vtable({9: 1})
    set_return(log, 13, r.getrandbits(32))
    return Call(CALLS['Update'], [PANEL], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
