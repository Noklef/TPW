# The game panel class of the object at 0x7cc298 (src/game/ui/panel7cc298: 0x4b1ee0) against the original. Update: the panel's slots 5, 6 and
# 13 recorders (13's result set), its object +0x20 one with slot 9 a recorder; the thing [0x7c2658] with its dwords +0x68 (0 or not), +0x188,
# +0x194 and byte +0x190 any, its definition (gObjectDefs 0x7890a0 as object.py's: a few, the thing's found, the cache hit or not) with its
# dword +0x10c 0 or not; the window +0x10's children (dialog7cb398.py's lists: decoys about the children 0xa08e, 0xa08f, 0xa090, 0xa0a5-0xa0a7,
# their slots 7 / 9 recorders and their own children none or window.py's tree for Window_SetShown) and the 0x705a50 button 0xa08a (as in
# window705a50.py, now and then missing), its +0x98 / +0x9c objects recorders (k 23 / 24) masked as that bench says by the look it gets,
# [0xfaa5d4] set or not (its post of 1 to the queues).
# The texts 0x33 / 0x34 as language.py's (Language_WideText): a record table about them, the converter on or off.
# Stats (0x4b16c0): the window +0x14 (now and then none) with the children 0xa09d / 0xa0a0 / 0xa09e and each of 0xa096 / 0xa093 / 0xa092 /
# 0xa099 / 0xa094 mostly (slots 7 / 9 recorders) among decoys; the thing's six CircBufU32s of 30 (+0x70, +0xf8, +0x1a8, +0x230, +0x2b8,
# +0x340: values small or any, cursors and sizes about 0 and 30, wrapped or not), object.py's clock and date for the scrap refund, its
# definition (as Update's, 0x500 bytes) routed for Object_Status (+0x19c 4; or the kinds +0x9c 1 / 2 with +0x2c 0; or past every test to
# the count's end, +0x68 0 or not) and for the rating (+0x4ac 2 a sideshow, else a ride without a track: never a coaster), its cell
# (bytes +5 / +7) with analyser.py's people about it for radius 1, the game's +0x1da720 a slot of 0x7cfb90; the panel's +8 one of the
# statuses or any, its settings +0x24 / +0x28 / +0x2c about 0-100 or any. The formatter 0x6acd60 the log stub with its two number
# arguments hashed (text_args()). The two surviving mutants of the cell id's upper half (0, or the slot of another sum) are equivalent:
# ParkAnalyser_AveragePersonStat19c reads the cell's low word only.
# Stats7cc258 (0x4b01d0, slot 5 of the panel 0x7cc258's vtable 0x6ffc68, src/game/ui/panel7cc258): the same world with that panel's
# children 0xc070 / 0xc076-0xc079 / 0xc07c / 0xc07d (always there) among its decoys, the definition's kind +0x158 for
# Object_SettingsValue (1 / 3 negating its second setting) 0-3 or any.
# Stats7cc3b0 (0x4b58b0, slot 5 of the panel 0x7cc3b0's vtable 0x6ffe50, src/game/ui/panel7cc3b0): the window +0x14 (now and then none)
# with the children 0x51b / 0x51e / 0x514 / 0x51f / 0x517 / 0x513 / 0x515 (always there; slot 9 hashing the 32 bytes of the text it is
# sent, 0x51f's +0x98 a recorder (k 25: the event's word +4) or none) among decoys; the staff member [0x7c2658] as stafflist.py's staff()
# with its float +0x1fc as +0x1f8 and its hire date +0x1f0 object.py's clock() date; stafflist.py's texts() (the record table [0x78baa4],
# the wage table 0x785300). The formatter the log stub with its number argument hashed (text_args()).
# Update7cc3b0 (0x4b5ab0, slot 1 of 0x6ffe50): the staff member (staff(); none now and then) with its cell (bytes +5 / +7, the map
# [0x8023a0]'s cell type +8 about 0, 1, 3, 9 and 0xa as analysercell.py's) and its patrol region, the panel's slots 5 / 6 recorders, its
# object +0x20 one with slot 9 a recorder (the name's address), the window +0x10's children 0x510 (window705a50.py's SetState surroundings:
# its objects, queues and masks) and 0x50e (slot 5 a recorder) among decoys; scriptchain.py's advisor_env() for 0xd4.
# Patrol7cc3b0 (0x4b5b50, thiscall (on)): modeenter.py's mode set-up (focus.py's Enter6fead0: the holder set or not, the current mode a
# recorder or none, the heap failing now and then, the setting 1, kept so after advisor_env() for 0xc1), the tile [0x7b05cc] for the new
# tile mode's Assign (its flag clear: the env's recorders 30 / 24), the staff member's patrol region, the panel's slot 11 a recorder; on
# 1 mostly, 0, or any.
# usage: panel7cc298.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Update,Stats,Stats7cc258,Stats7cc3b0,Update7cc3b0,
#   Patrol7cc3b0)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, _cdecl_log4, blob_bytes, log_region, put, recorder, set_return, text_args, vtable  # noqa: E402
from analyser import people  # noqa: E402
from dialog7cc5f4 import merged  # noqa: E402
from funnytime import RANGES as CLOCK_RANGES  # noqa: E402
from object import clock  # noqa: E402
from dialog7cb398 import KVTABLE, children, window  # noqa: E402
from language import text  # noqa: E402
from window import RANGES as WINDOW_RANGES, shown_env  # noqa: E402
from window705a50 import EVENT_MASK, INNER, NOTIFY_MASK, RANGES as BUTTON_RANGES, look, window as button  # noqa: E402
import focus  # noqa: E402
import modeenter  # noqa: E402
import scriptchain  # noqa: E402
import stafflist  # noqa: E402
import window705a50  # noqa: E402
from fncmp import _cdecl_log  # noqa: E402
from window import queues  # noqa: E402

CALLS = {'Update': 0x4b1ee0, 'Stats': 0x4b16c0, 'Stats7cc258': 0x4b01d0, 'Stats7cc3b0': 0x4b58b0, 'Update7cc3b0': 0x4b5ab0,
         'Patrol7cc3b0': 0x4b5b50}
PATROL_RANGES = modeenter.RANGES + [
    (0x46c350, 0x46c460), (0x46c580, 0x46c5b0), (0x46c6a0, 0x46c6c0), (0x4814a0, 0x481500), (0x48c950, 0x48c970), (0x4a2aa0, 0x4a2ad0),
    (0x4b5b50, 0x4b5c90), (0x506e90, 0x506ec0), (0x6590f7, 0x6591e4), (0x65f11d, 0x65f160), (0x66190a, 0x66192b), (0x661d96, 0x661de0),
    (0x67dcd0, 0x67ddb0), (0x67efc0, 0x67f1b0), (0x6828a0, 0x6830a0), (0x68a170, 0x68a450)]
UPDATE_RANGES = [
    (0x401030, 0x401260), (0x401280, 0x401710), (0x4017a0, 0x402a30), (0x405710, 0x405760), (0x409cd0, 0x409cf0), (0x41a980, 0x41a9c0),
    (0x486b00, 0x486b50), (0x4b5ab0, 0x4b5b60), (0x4fa990, 0x4faa10), (0x505bd0, 0x505c60), (0x506860, 0x506880), (0x506eb0, 0x506ee0),
    (0x5194d0, 0x519520), (0x536310, 0x536370), (0x536390, 0x5363b0), (0x550ca0, 0x550d90), (0x59a940, 0x59aa80), (0x59abc0, 0x59ae30),
    (0x59b590, 0x59b630), (0x59bea0, 0x59c0b0), (0x59c2b0, 0x59c690), (0x5f5660, 0x5f56a0), (0x5f56f0, 0x5f5960), (0x5f5ad0, 0x5f5bc0),
    (0x5f5c70, 0x5f5ce0), (0x5f5e20, 0x5f5e60), (0x5f7180, 0x5f71a0), (0x658ccc, 0x658daf), (0x659710, 0x659740), (0x65d7bc, 0x65d832),
    (0x668820, 0x6688c3), (0x668ad3, 0x668c99), (0x6697e3, 0x6698b4), (0x67a480, 0x67a600), (0x67a650, 0x67a800), (0x67ad90, 0x67af10),
    (0x67dcd0, 0x67dd00), (0x67f020, 0x67f1b0), (0x67fa20, 0x67fa50), (0x680430, 0x6804e0), (0x681740, 0x681970), (0x682720, 0x6827d0),
    (0x682b40, 0x682e50), (0x682fc0, 0x6830a0), (0x688710, 0x688740), (0x5da3c0, 0x5da3c1), (0x5d1750, 0x5d1751)] + window705a50.RANGES
STAFF_RANGES = [(0x485ec0, 0x485f70), (0x4b58b0, 0x4b5a70), (0x4f8690, 0x4f8740), (0x505b70, 0x505be0), (0x5064f0, 0x506590), (0x506870, 0x506900),
                (0x506eb0, 0x506ee0), (0x5fc530, 0x5fc580), (0x5fc660, 0x5fc750), (0x65d3a3, 0x65d41c), (0x65d7bc, 0x65d832), (0x67c320, 0x67c3e0),
                (0x67c7f0, 0x67c840), (0x6acb70, 0x6accb0), (0x6adc80, 0x6add40), (0x6b00e0, 0x6b0140), (0x5da3c0, 0x5da3c1), (0x67a830, 0x67a860)]
STATS_RANGES = [(0x412e90, 0x412f00), (0x43e0b0, 0x43e0f0), (0x441910, 0x441980), (0x485f60, 0x4861e0), (0x495cf0, 0x495d70), (0x4b16c0, 0x4b1b60),
                (0x4cd880, 0x4cd9d0), (0x4dd4e0, 0x4dd510), (0x4de4a0, 0x4de680), (0x4e0220, 0x4e03a0), (0x4e03d0, 0x4e0400), (0x4e0440, 0x4e0460),
                (0x4e0560, 0x4e0870), (0x4e1c70, 0x4e1e10), (0x4e1e30, 0x4e2190), (0x4e2290, 0x4e2450), (0x4e2710, 0x4e2750), (0x4f8690, 0x4f8740),
                (0x509990, 0x5099b0), (0x519450, 0x5194a0), (0x519510, 0x519560), (0x545310, 0x545460), (0x55a4e0, 0x55a540), (0x5fc550, 0x5fc580),
                (0x65d7bc, 0x65d832), (0x67c320, 0x67c3e0), (0x6acb70, 0x6accb0), (0x6acd60, 0x6ace70), (0x6b00e0, 0x6b0140), (0x5da3c0, 0x5da3c1),
                (0x4b01d0, 0x4b063e), (0x436860, 0x436890), (0x4e1a30, 0x4e1b50), (0x506580, 0x506660), (0x50b350, 0x50b370)]
B = Bench(merged([(0x4b1ee0, 0x4b20a4), (0x4e0440, 0x4e0450), (0x4e21b0, 0x4e21b7), (0x4e1a00, 0x4e1a07), (0x4e1a10, 0x4e1a17), (0x4dd4e0, 0x4dd4fb),
                  (0x412e90, 0x412eea), (0x485b00, 0x485b40), (0x6adc80, 0x6add25), (0x6ace60, 0x6acea0)] + WINDOW_RANGES + BUTTON_RANGES
                 + STATS_RANGES + CLOCK_RANGES + STAFF_RANGES + UPDATE_RANGES + PATROL_RANGES),
          hooks={**modeenter.HOOKS, 0x6acd60: _cdecl_log4, 0x67a7d0: _cdecl_log})
PANEL, OBJECT, THING, ROOT, KIDS, DECOYS = 0x33100000, 0x33110000, 0x33120000, 0x33130000, 0x33140000, 0x33150000
OBJS, DEFS, ITEMS, TEXTS = 0x33160000, 0x7890a0, 0x33170000, 0x33180000
IDS = [0xa08e, 0xa08f, 0xa090, 0xa0a5, 0xa0a6, 0xa0a7]


def defs(r, regions, thing, size=0x200):
    """gObjectDefs with 1-4 definitions (size bytes each), the thing's id one of theirs; returns the thing's"""
    table = bytearray(r.bytes(0x740))
    n = r.randrange(1, 5)
    ids = r.sample(range(0x10000), n)
    for i in range(n):
        put(table, 4 * i, 'I', ITEMS + size * i)
        item = put(bytearray(r.bytes(size)), 0, 'H', ids[i])
        put(item, 0x10c, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        regions[ITEMS + size * i] = item
    put(table, 0x528, 'i', n)
    k = r.randrange(n)
    put(thing, 0xe, 'H', ids[k])
    put(table, 0x730, 'HHI', r.choice([ids[k], r.getrandbits(16)]), 0, ITEMS + size * k)
    regions[DEFS] = table
    return regions[ITEMS + size * k]


def texts(r, regions, first=0x33, n=2, buffer=None):
    """the language object's converter (on or off) and record table [0x78ba94] about the n indexes from first (0x33 / 0x34); the wide
    buffer 0x7ba058 the given bytes or 0x55s"""
    lang = bytearray(r.bytes(0x600))
    conv = bytearray(r.bytes(0x10))
    put(conv, 8, 'BBHI', r.choice([0x80, 0x81, 0xa0, 0, 0xff, r.getrandbits(8)]), r.getrandbits(8), r.choice([0, 3, 0x80, 0x400, 0x7ff]), TEXTS + 0x4000)
    put(lang, 0x54c, 'II', TEXTS + 0x800, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    table = bytearray(r.bytes(0x20))
    put(table, 8, 'II', r.choice([first + n] * 3 + [first + n - 1, 5]), TEXTS + 0xb00 - 4 * first)
    put(lang, 0x564, 'I', TEXTS + 0xa00)
    pool = bytearray()
    for i in range(n):
        body = text(r) + bytes([0]) * r.choice([1, 1, 2])
        head = r.choice([1] * 6 + [2, 0x101]) | (len(body) << 8 if r.random() < 0.9 else r.randrange(0x40) << 8)
        rec = bytearray(struct.pack('<I', head) + body + bytes(0x40 - 4 - len(body)))
        if r.random() < 0.15:
            rec[4 + ((len(body) + 3) & ~3)] = 1   # (not terminated)
        pool += rec
    regions.update({0x78b530: lang, TEXTS + 0x800: conv, TEXTS + 0x4000: r.bytes(0x1000), 0x7ba058: buffer or bytes([0x55]) * 0x404, TEXTS + 0xa00: table,
                    TEXTS + 0xc00: bytes(pool), TEXTS + 0xb00: struct.pack('<%dI' % n, *[TEXTS + 0xc00 + 0x40 * i for i in range(n)])})


STATS_IDS = [0xa09d, 0xa096, 0xa093, 0xa092, 0xa099, 0xa094, 0xa0a0, 0xa09e]
OPTIONAL = {0xa096, 0xa093, 0xa092, 0xa099, 0xa094}
# the panel 0x7cc258's (Stats7cc258): (its children, their decoys); none of them checked
STATS258_IDS = ([0xc070, 0xc076, 0xc077, 0xc079, 0xc078, 0xc07d, 0xc07c], [0, 0xc071, 0xc075, 0xc07a, 0xc07b])
GAME = 0x32000000


def ring(r, thing, at):
    """a CircBufU32 of 30 at thing + at: its values small or any, its cursor and size about 0 and 30, wrapped or not"""
    big = r.random() < 0.25
    for i in range(30):
        put(thing, at + 4 + 4 * i, 'I', r.getrandbits(32) if big and r.random() < 0.5 else r.choice([0, 1, r.randrange(1000), r.randrange(1 << 20)]))
    cursor = r.choice([-1, 0, 1, 15, 28, 29, r.randrange(30)])
    size = r.choice([30, 30, max(cursor + 1, 0), r.randrange(31), r.randrange(31, 34)])
    put(thing, at + 0x7c, 'iiB', cursor, size, r.choice([0, 1, 1, r.getrandbits(8)]))


def stats_case(r, op='Stats'):
    """Stats' (or Stats7cc258's) world (the module comment)"""
    thing = bytearray(r.bytes(0x400))
    regions = {}
    clock(r, regions, thing)   # (its log the case's)
    log = regions[LOG]
    text_args(log, 2)
    for at in (0x70, 0xf8, 0x1a8, 0x230, 0x2b8, 0x340):
        ring(r, thing, at)
    item = defs(r, regions, thing, 0x500)
    put(thing, 0x28, 'I', 0)   # (no track: the rating's ride path without the pieces)
    put(item, 0x158, 'i', r.choice([0, 1, 2, 3, -1, r.getrandbits(31)]))
    route = r.choice(['3', '19', 'end'])
    put(item, 0x4ac, 'i', r.choice([2, 2, 0, 1, 7]) if route != 'end' else r.choice([2, 1, 7]))
    put(item, 0x9c, 'i', {'3': r.choice([0, 1, 2, 5]), '19': r.choice([1, 2]), 'end': r.choice([0, 5])}[route])
    put(thing, 0x19c, 'I', 4 if route == '3' else r.choice([0, 3, 5]))
    if route == '19':
        put(thing, 0x2c, 'I', 0)
    if route == 'end':
        put(thing, 0x64, 'I', 0)
        thing[0x32] &= ~8
        put(thing, 0x68, 'I', r.choice([0, 1, r.getrandbits(32)]))
    put(thing, 0x1a0, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    thing[5], thing[7] = (r.randrange(128), r.randrange(128)) if r.random() < 0.9 else (r.getrandbits(8), r.getrandbits(8))
    regions.update(people(r, ((thing[7] << 7) + thing[5] + 1) & 0xffff, 1))
    regions[0x7cf83c] = struct.pack('<I', GAME)
    regions[GAME + 0x1da720] = struct.pack('<H', r.randrange(40))
    regions[THING] = thing
    regions[0x7c2658] = struct.pack('<I', THING)
    regions[0x78ba94] = struct.pack('<I', r.getrandbits(32))
    regions[0x7ba058] = bytes([0x55]) * 0x40
    ids, decoys = (STATS_IDS, [0, 0xa08b, 0xa0a4, 0xa08d, 0xa091]) if op == 'Stats' else STATS258_IDS
    kids = [(KIDS + 0x200 * k, ident) for k, ident in enumerate(ids) if ident not in OPTIONAL or r.random() < 0.85]
    for addr, _ in kids:
        window(r, regions, addr, ROOT, 0)
    regions[KVTABLE] = vtable({7: 1, 9: 1})
    window(r, regions, ROOT, 0, 0)
    put(regions[ROOT], 0x5c, 'I', children(r, regions, kids, ROOT, DECOYS, decoys))
    panel = bytearray(r.bytes(0x40))
    put(panel, 0, 'I', PANEL + 0x40)
    put(panel, 8, 'I', r.choice([0, 1, 3, 0x19, r.getrandbits(32)]))
    put(panel, 0x14, 'I', ROOT if r.random() < 0.9 else 0)
    for off in (0x24, 0x28, 0x2c):
        put(panel, off, 'i', r.choice([0, 50, 100, r.randrange(-20, 130), r.getrandbits(32) - (1 << 31)]))
    regions[PANEL] = panel
    regions[PANEL + 0x40] = vtable({})
    return Call(CALLS[op], [PANEL], regions, conv='t', ret='v')


STAFF_IDS = [0x51b, 0x51e, 0x514, 0x51f, 0x517, 0x513, 0x515]


def staff_case(r):
    """Stats7cc3b0's world (the module comment)"""
    regions = {}
    date = bytearray(0x60)
    clock(r, regions, date)
    log = regions[LOG]
    text_args(log, 1)
    blob_bytes(log, 9, 0, 0xffffffff)
    blob_bytes(log, 25, 1, 0x30)
    staff = bytearray(r.bytes(0x220))
    stafflist.staff(r, staff, 0)
    staff[0x1fc:0x200] = staff[0x1f8:0x1fc] if r.random() < 0.3 else struct.pack('<f', r.choice([0.0, 1.0, 50.5, 99.9, 100.0, 255.9, 256.0, -1.0]))
    staff[0x1f0:0x1f8] = date[0x18:0x20]
    regions[THING] = staff
    regions[0x7c2658] = struct.pack('<I', THING)
    stafflist.texts(r, regions)
    regions[0x78ba94] = struct.pack('<I', r.getrandbits(32))
    kids = [(KIDS + 0x200 * k, ident) for k, ident in enumerate(STAFF_IDS)]
    for addr, ident in kids:
        window(r, regions, addr, ROOT, 0)
        if ident == 0x51f and r.random() < 0.8:
            put(regions[addr], 0x98, 'I', OBJS + 0x200)
    regions[OBJS + 0x200] = struct.pack('<I', OBJS + 0x240) + r.bytes(0x3c)
    regions[OBJS + 0x240] = put(vtable({}, size=8), 0xc, 'I', recorder(25, 2))
    regions[KVTABLE] = vtable({7: 1, 9: 1})
    window(r, regions, ROOT, 0, 0)
    put(regions[ROOT], 0x5c, 'I', children(r, regions, kids, ROOT, DECOYS, [0, 0x516, 0x518, 0x51a, 0x51c, 0x51d]))
    panel = bytearray(r.bytes(0x40))
    put(panel, 0x14, 'I', ROOT if r.random() < 0.95 else 0)
    regions[PANEL] = panel
    return Call(CALLS['Stats7cc3b0'], [PANEL], regions, conv='t', ret='v')


CELLMAP = 0x33190000


def update_case(r):
    """Update7cc3b0's world (the module comment)"""
    log = log_region()
    regions = {LOG: log}
    masks = scriptchain.advisor_env(r, regions, 0xd4)
    staff = bytearray(r.bytes(0x220))
    stafflist.staff(r, staff, 0)
    staff[5], staff[7] = r.choice([0, 1, 127, 128, 255, r.randrange(256)]), r.randrange(4)
    cells = bytearray(r.bytes(0x44 * (4 * 128 + 256)))
    put(cells, ((staff[7] << 7) + staff[5]) * 0x44 + 8, 'I', r.choice([0, 1, 2, 3, 4, 5, 8, 9, 0xa, 0xb, 0x109, r.getrandbits(32)]))
    regions[CELLMAP] = cells
    regions[0x8023a0] = struct.pack('<I', CELLMAP)
    regions[THING] = staff
    regions[0x7c2658] = struct.pack('<I', THING if r.random() < 0.95 else 0)
    patrol = struct.unpack_from('<HH', staff, 0x20a) != (1, 0x4000)
    btn = window705a50.window(r)
    window705a50.objects(r, regions, log, btn, [(0x98, window705a50.OBJ98), (0x9c, window705a50.OBJ9C)])
    shown = bytearray(btn)   # (the look the notification sees: after the state is set)
    shown[0x138] = int(patrol)
    blob_bytes(log, 22, 1, window705a50.NOTIFY_MASK, None if window705a50.look(shown) == 1 else window705a50.INNER)
    blob_bytes(log, 3, 1, window705a50.EVENT_MASK)
    queues(r, regions)
    regions[0xfaa5d4] = put(bytearray(4), 0, 'I', r.choice([0, 1, 0x80000000]))
    regions[KIDS] = btn
    window(r, regions, KIDS + 0x200, ROOT, 0)
    regions[KVTABLE] = vtable({5: 1, 7: 1, 9: 1})
    window(r, regions, ROOT, 0, 0)
    put(regions[ROOT], 0x5c, 'I', children(r, regions, [(KIDS, 0x510), (KIDS + 0x200, 0x50e)], ROOT, DECOYS, [0, 0x50d, 0x50f, 0x511]))
    panel = bytearray(r.bytes(0x40))
    put(panel, 0, 'I', PANEL + 0x40)
    put(panel, 0x10, 'I', ROOT)
    put(panel, 0x20, 'I', OBJECT)
    regions[PANEL] = panel
    regions[PANEL + 0x40] = vtable({})
    regions[OBJECT] = struct.pack('<I', KVTABLE) + r.bytes(0x1c)
    return Call(CALLS['Update7cc3b0'], [PANEL], regions, conv='t', ret='v', masks=masks)


def patrol_case(r):
    """Patrol7cc3b0's world (the module comment)"""
    regions = focus.case(r, ['Enter6fead0']).regions
    masks = scriptchain.advisor_env(r, regions, 0xc1)
    regions[0xfb3b7c] = struct.pack('<I', 1)   # (focus.py's setting, which advisor_env() replaced)
    regions[0x7b05cc] = struct.pack('<H', r.choice([0, 1, 0x80, 0x81, r.getrandbits(16)]))
    staff = bytearray(r.bytes(0x220))
    put(staff, 0x20a, 'HH', *((1, 0x4000) if r.random() < 0.3 else (r.getrandbits(16), r.getrandbits(16))))
    regions[THING] = staff
    regions[0x7c2658] = struct.pack('<I', THING)
    regions[PANEL] = put(bytearray(r.bytes(0x40)), 0, 'I', PANEL + 0x40)
    regions[PANEL + 0x40] = vtable({})
    on = r.choice([1, 1, 1, 0, 0, 2, r.getrandbits(32)])
    return Call(CALLS['Patrol7cc3b0'], [PANEL, on], regions, conv='t', ret='a', masks=masks)


def case(r):
    op = r.deal(OPS)
    if op == 'Patrol7cc3b0':
        return patrol_case(r)
    if op == 'Update7cc3b0':
        return update_case(r)
    if op == 'Stats7cc3b0':
        return staff_case(r)
    if op != 'Update':
        return stats_case(r, op)
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
    regions[0xfaa5d4] = put(bytearray(4), 0, 'I', r.choice([0, 1, 0x80000000]))   # (the button posted 1 when it is set)
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
