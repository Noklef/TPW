# The panel [0x7cc218]'s opening (src/game/ui/panel7cc218: 0x4ae8e0, slot 0 of the vtable 0x6ffbe8, and the gauges' set-up 0x4ae980 it
# calls) against the original: panel7cc258.py's surroundings (dialogbuild.py's chain(), panelparts.py's heap and the meshes' vtable
# 0x6feeb8 slots 3 / 5 recorders, the text parts' slot 3 a recorder, the manager's slot 13 a recorder, the park's things and the group
# +4 counted) with the selected thing at [0x7cc208]; the panel's window +0x10 the row whose children are the ones looked up (0x3e26 /
# 0x3e27 hidden when the group counts under 2; the text parts' slot 3 events hashed as window.py's notify_inner(), the gauges 0x3e2d / 0x3e30 (0x705b20 sliders: kid()s grown to 0x160 bytes) each with its
# own children, 0x3e2e among 0-2 decoys, the slider 0x3e2f, the texts 0x3e31 / 0x3e33 / 0x3e32, the hover label 0x3e28 (its object
# +0x98 set)), each a dialogbuild.py kid() shuffled among 0-2 decoys, now and then one missing (none is checked: a fault in both).
# Gauges (0x4ae980 alone): the same, the window +0x10 none now and then.
# Buttons (0x4ad4e0): the same surroundings with the children 0x3e38 / 0x3e36 window705a50.py's buttons (linked and dispatched as the kid
# each replaces; their +0x98 object a recorder k 23, 0x3e36's +0x9c one k 24: the events masked as panel7cc258.py's), the others kid()s
# (their slot 5 a recorder); the thing (thing()) and the panel's mode +8 3, 0x19 / 0x1a, 0x11-0x15 or others.
# Show (0x4af980, slot 10 of the vtable 0x6ffbe8): dialogbuild.py's windowat10() surroundings with the panel's ids (0x3e24, 0x3e15,
# 0x3e25) and handler (0x4af600), the script at 0x755150; the thing's float +0x44 about 0-21 (fractions, negatives, huge, NaNs too);
# advisor.py's PostId surroundings for 0x129 / 0xcf (their records their own mostly; the stack message's unwritten bytes masked).
# Show7ca178 (0x497990, slot 10 of the vtable 0x6ff740, src/game/ui/panel7ca178): the same with that panel's ids (0x15bb8, 0x15bba,
# 0x15bb9), handler (0x497640), script (0x751190), selected thing (0x7ca19c) and posts (0xd5 / 0x12b; the float about 0-30).
# Show7cc3b0 (0x4b6080, slot 10 of 0x6ffe50, src/game/ui/panel7cc3b0): the same with its ids (0x511, 0x512, no inner), handler 0x4b5cb0,
# script 0x756f48, thing 0x7cc3a4 and post 0xc0; the window +0x18 then given the handler 0x4b5c80 (none there: a fault in both).
# Show7cc408 (0x4b79b0, slot 10 of 0x6fff40, src/game/ui/panel7cc408): ids 0x3722, 0x3723 (no inner), handler 0x4b7720, script 0x7575e0,
# thing 0x7cc430, post 0x12e; a sound played first: soundplay.py's globals (0x802bc0, the manager's +8 a recorder k 31, the command
# hashed), the kids' category [0x803a24].
# Show7cc258 / Show7cc298 / Show7cc328 / Show7cc3e0 / Show7ca228 (0x4b0e30 / 0x4b2430 / 0x4b5290 / 0x4b6ae0 / 0x499eb0, slot 10 of
# 0x6ffc68 / 0x6ffce8 / 0x6ffdd8 / 0x6ffec8 / 0x6ff8d0): the same with their ids, handlers, scripts, things and posts (SHOWS).
# ShowThing (0x486920, src/game/ui/thingpanel): the panel route()'s choice (any for none) set up as above at its own address, the
# thing's kind +2, id +0xe, link +0x30 and flags +0x32 set; the definitions 0x7890a0 cached (+0x730 / +0x734) or searched (the first
# +0x528 entries, one of them the definition DEF: kind +0x4ac, +0x284, +0x288); the linked things 0x7cfb90 mostly the thing shown
# (the advisor's entry kept); the thing none now and then.
# usage: panel7cc218.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open,Gauges,Buttons,Show,Show7ca178,
#   Show7cc3b0,Show7cc408,Show7cc258,Show7cc298,Show7cc328,Show7cc3e0,Show7ca228,ShowThing)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import animatormesh  # noqa: E402
import dialogbuild  # noqa: E402
import objectqueue  # noqa: E402
import panel7cc258  # noqa: E402
import panelparts  # noqa: E402
import window  # noqa: E402
import window705a50  # noqa: E402
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, Mask, blob, blob_bytes, put, recorder, set_return, vtable  # noqa: E402
from msgtags import ADVISOR_RANGES  # noqa: E402
from thingenv import ITEM, ITEMS  # noqa: E402

CALLS = {'Open': 0x4ae8e0, 'Gauges': 0x4ae980, 'Buttons': 0x4ad4e0, 'Show': 0x4af980, 'Show7ca178': 0x497990,
         'Show7cc3b0': 0x4b6080, 'Show7cc408': 0x4b79b0, 'Show7cc258': 0x4b0e30, 'Show7cc298': 0x4b2430, 'Show7cc328': 0x4b5290,
         'Show7cc3e0': 0x4b6ae0, 'Show7ca228': 0x499eb0, 'ShowThing': 0x486920}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
QUEUE_RANGES = [   # (objectqueue.py's and object_checks.py's: Object_CanOpen's and Object_TrackNeighboursOk's code)
    (0x4d8300, 0x4d8340), (0x4d8c00, 0x4d8c30), (0x4d96f0, 0x4d99f0), (0x4de4a0, 0x4de5b0), (0x4df290, 0x4df310), (0x4de840, 0x4dede8),
    (0x516330, 0x516380), (0x4ddf40, 0x4de200), (0x4de040, 0x4de100), (0x4de670, 0x4de7f0), (0x502430, 0x502470), (0x50b350, 0x50b370),
    (0x522770, 0x5227a0), (0x522850, 0x522870), (0x536300, 0x53644c), (0x4dd920, 0x4dda20), (0x412e90, 0x412eea), (0x441970, 0x4419b0),
    (0x436860, 0x436880), (0x4dd4e0, 0x4dd510), (0x522770, 0x5227f0), (0x52c2b0, 0x52c430), (0x55a070, 0x55a110), (0x55a390, 0x55a3f0)]
B = Bench(panel7cc258.merged(panel7cc258.RANGES + QUEUE_RANGES + dialogbuild.ALL_RANGES + ADVISOR_RANGES + [
    (0x4ae8e0, 0x4ae93e), (0x4ae980, 0x4aec2b), (0x4ad4e0, 0x4ad71b), (0x4af980, 0x4af9e5), (0x497990, 0x4979e7), (0x4b6080, 0x4b60c5), (0x4b79b0, 0x4b7a00),
    (0x4b0e30, 0x4b0e67), (0x4b2430, 0x4b2467), (0x4b5290, 0x4b52c4), (0x4b6ae0, 0x4b6b14), (0x499eb0, 0x499ee4),
    (0x51bfc0, 0x51c048), (0x486920, 0x486a8e), (0x4e0df0, 0x4e0dfe), (0x4e2710, 0x4e273a), (0x5da3c0, 0x5da3c1)]),
          hooks={**animatormesh.HOOKS, 0x67a7d0: advisor._cdecl_log})   # (advisor.py's: the records' static set-up's atexit logged)
ROOT = dialogbuild.ROW
KIDS, SUBKIDS, PANEL, MANAGER, OBJS, DEFS = 0x31000000, 0x31008000, 0x31010000, 0x31030000, 0x31040000, 0x31100000
THING_SHOWN = 0x31200000
BUTTONS = (0x3e38, 0x3e34, 0x3e2a, 0x3e36, 0x3e2c, 0x3e37, 0x3e35)
BIG = panel7cc258.BIG
GAUGES = (0x3e2d, 0x3e30)
LOOKED = {'Open': (0x3e26, 0x3e27, 0x3e2d, 0x3e30, 0x3e2f, 0x3e31, 0x3e33, 0x3e32, 0x3e28),
          'Gauges': (0x3e2d, 0x3e30, 0x3e2f, 0x3e31, 0x3e33, 0x3e32)}


def decoys(r, near):
    return [r.choice([0, 1, near, r.getrandbits(32)]) for _ in range(r.choice([0, 0, 1, 2]))]


def grow(r, regions, at):
    w = regions[at] = bytearray(regions[at]) + bytearray(r.bytes(BIG - len(regions[at])))
    put(w, 0x114, 'I', r.choice([0, CDECL_LOG4]))
    put(w, 0x5c, 'I', 0)   # (childless unless given some below)
    return w


def thing(r, regions):
    """objectqueue.py's can_open() object (its world, definitions moved to DEFS, links) at RAM, its +0x68 mostly 0, in rows 0-3 with
    object_checks.py's track cells about it; the thing [0x7c2658] (now and then none)"""
    q = dict(objectqueue.can_open(r).regions)
    items = q.pop(ITEMS)
    table = bytearray(q[0x7890a0])
    for off in range(0, len(table) - 3, 4):
        v = struct.unpack_from('<I', table, off)[0]
        if ITEMS <= v < ITEMS + len(items):
            put(table, off, 'I', v - ITEMS + DEFS)
    q[0x7890a0], q[DEFS] = table, items
    obj = q[RAM] = bytearray(q[RAM])
    n = struct.unpack_from('<i', table, 0x528)[0]   # (only the first n definitions are searched: its own one of them)
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, ITEM * r.randrange(n))[0])
    put(obj, 0x68, 'I', r.choice([0, 0, 0, 1, 0x80000000]))
    obj[7] = r.randrange(4)
    world = struct.unpack('<I', q[0x7cf83c])[0]
    x, y = obj[5] & 0x7f, obj[7] + (obj[5] >> 7)
    angle = struct.unpack_from('<i', obj, 0x10)[0]
    offsets = {0: ((4, 1), (-2, 1)), 0x5a: ((1, -5), (1, 1)), 0xb4: ((-5, -2), (1, -2)), 0x10e: ((-2, 4), (-2, -2))}.get(angle, ())
    for dx, dy in offsets:
        cell = bytearray(r.bytes(0x28))
        cell[0xc] = r.choice([1, 2, 4, 0x10, 0x80, 3, 0, 0xff, 0x55])
        cell[0x22] = r.choice([1, 2, 4, 0x10, 0x80, 3, 0, 0xff, 0x55])
        cell[2] = r.choice([2, 2, 0, 1])
        q[world + (((((y + dy) << 7) + x + dx + 1) & 0xffff) + 0x6cde) * 0x28] = cell
    regions.update(q)
    regions[0x7c2658] = struct.pack('<I', RAM if r.random() < 0.97 else 0)


def buttons_case(r):
    regions = dialogbuild.chain(r, 'Open', 0)
    log = regions[LOG]
    panelparts.surroundings(r, regions)
    thing(r, regions)
    ids = list(BUTTONS)
    if r.random() < 0.04:
        ids.remove(r.choice(ids))
    ids += decoys(r, 0x3e2b)
    r.shuffle(ids)
    items = [(KIDS + BIG * k, ident) for k, ident in enumerate(ids)]
    panelparts.kids(r, regions, items, ROOT)
    mode = r.choice([3, 3, 0x19, 0x1a, 0x11, 0x12, 0x13, 0x14, 0x15, 0, 1, 0x10, 0x16, r.getrandbits(32)])
    after = None
    for at, ident in items:
        w = grow(r, regions, at)
        if ident in (0x3e38, 0x3e36):   # (0x705a50 buttons as window705a50.py's, linked and dispatched as the kid they replace)
            btn = window705a50.window(r)
            for offset in (0, 0x54, 0x60, 0x8c, 0x114):
                btn[offset:offset + 4] = w[offset:offset + 4]
            put(btn, 0x5c, 'I', 0)
            put(btn, 0x98, 'I', OBJS if r.random() < 0.8 else 0)
            put(btn, 0x9c, 'I', OBJS + 0x40 if ident == 0x3e36 and r.random() < 0.8 else 0)   # (one notifies: its look known)
            regions[at] = btn
            if ident == 0x3e36:
                after = bytearray(btn)
                after[0x138] = 1 if mode in (0x11, 0x12, 0x13, 0x14, 0x15) else 0
    for i, k in enumerate((23, 24)):
        vt = vtable({5: 1}, size=8)   # (slot 5 told of the button shown or hidden: one argument)
        put(vt, 0xc, 'I', recorder(k, 2))
        regions[OBJS + 0x40 * i] = put(bytearray(r.bytes(0x20)), 0, 'I', OBJS + 0x40 * i + 0x20) + vt
    if after is not None:
        blob_bytes(log, 24, 1, window705a50.NOTIFY_MASK, None if window705a50.look(after) == 1 else window705a50.INNER)
    blob_bytes(log, 23, 1, window705a50.EVENT_MASK)
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 1, 0x80000000]))   # (a button posted 1 when it is set)
    surround(r, regions)
    panel = bytearray(r.bytes(0x30))
    put(panel, 8, 'I', mode)
    put(panel, 0x10, 'I', ROOT)
    regions[PANEL] = panel
    return Call(CALLS['Buttons'], [PANEL], regions, conv='t', ret='v')


def surround(r, regions):
    put(regions[ROOT], 0x114, 'I', r.choice([0, CDECL_LOG4]))
    regions[0x77c48c] = struct.pack('<I', MANAGER)   # (the manager told of a hidden window: its slot 13 a recorder)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    regions[0xfaa540] = r.bytes(8)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([0, ROOT]), r.getrandbits(32), 0, r.getrandbits(32), r.randrange(1, 1 << 32))


def f32(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


SHOWS = {'Show': ((0x4af600, 0x3e24, 0x3e15, 0x3e25), 0x755150, 0x7cc208, (0x129, 0xcf), 22),
         'Show7ca178': ((0x497640, 0x15bb8, 0x15bba, 0x15bb9), 0x751190, 0x7ca19c, (0xd5, 0x12b), 31),
         'Show7cc3b0': ((0x4b5cb0, 0x511, 0x512, 0), 0x756f48, 0x7cc3a4, (0xc0,), 22),
         'Show7cc408': ((0x4b7720, 0x3722, 0x3723, 0), 0x7575e0, 0x7cc430, (0x12e,), 22),
         'Show7cc258': ((0x4b0b30, 0xc06c, 0xc06f, 0xc06d), 0x755908, 0x7cc288, (0xd1,), 22),
         'Show7cc298': ((0x4b2150, 0xa0a2, 0xa091, 0xa0a3), 0x755e80, 0x7cc2c8, (0xd0,), 22),
         'Show7cc328': ((0x4b4fc0, 0x1e84, 0x1e89, 0), 0x756ba0, 0x7cc318, (0x12c,), 22),
         'Show7cc3e0': ((0x4b67f0, 0x520c, 0x520d, 0), 0x7573c8, 0x7cc3d8, (0x12d,), 22),
         'Show7ca228': ((0x499bb0, 0x157e, 0x1583, 0), 0x751a68, 0x7ca218, (0x12a,), 22)}
SOUNDS, SOUNDS_VT = 0x31210000, 0x31210100


def show_case(r, op, panel=None):
    """Show and Show7ca178 (above); with a panel address, the panel there and its regions and masks returned instead"""
    fixed, script, selected, posts, top = SHOWS[op]
    args, regions = dialogbuild.windowat10(r, fixed)
    regions[script] = regions.pop(dialogbuild.WINDOWAT10SCRIPT)
    obj = args[0]
    regions[obj] = regions[obj] + r.bytes(0x10)   # (+0x24 written)
    log = regions[LOG]
    thing = bytearray(r.bytes(0x60))
    put(thing, 0x44, 'I', r.choice([f32(float(r.randrange(0, top)))] * 4 + [f32(r.uniform(-2, top + 1)), f32(r.uniform(-1e6, 1e6)),
                                    f32(256.0 + r.randrange(1, top - 1)), f32(3e19), 0x7fc00000, r.getrandbits(32)]))
    regions[THING_SHOWN] = thing
    regions[selected] = r.bytes(4)
    adv = advisor.queue(r, 'PostId')
    for k, v in adv.regions.items():
        if k != LOG:
            regions[k] = v
    table = bytearray(regions[0x76e300])
    for ident in posts:
        if r.random() < 0.7:   # (the record its own mostly)
            put(table, 0x38 * ident + 4, 'I', ident)
    regions[0x76e300] = table
    set_return(log, 20, r.choice([0, 1, 5, 100, -1, r.getrandbits(32)]))
    masks = [Mask(m.region, sorted(set(m.offsets + [0x16d0 + k for k in (2, 3, 0x17)])), m.reason) for m in adv.masks]
    if op == 'Show7cc3b0':   # (+0x18 none unless built: a preset +0x10 leaves it as it was)
        regions[obj] = put(bytearray(regions[obj]), 0x18, 'I', 0)
    if op == 'Show7cc408':   # (soundplay.py's globals)
        g = bytearray(r.bytes(0x20))
        put(g, 8, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
        put(g, 0xc, 'I', SOUNDS)
        put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
        regions[0x802bc0] = g
        regions[SOUNDS] = struct.pack('<I', SOUNDS_VT) + r.bytes(0xc)
        regions[SOUNDS_VT] = put(vtable({}, 4), 8, 'I', recorder(31, 2))
        regions[0x803a24] = struct.pack('<I', r.choice([0, 1, r.getrandbits(32)]))
        blob(log, 31, 1, 0x7f)
    if panel is not None:   # (the panel at its own address: a selected thing inside it left to the panel's bytes)
        regions[panel] = regions.pop(obj)
        if panel <= selected < panel + len(regions[panel]):
            regions.pop(selected)
        return regions, masks
    return Call(CALLS[op], [obj, THING_SHOWN], regions, conv='t', ret='v', masks=masks)


PANELS = {'Show': 0x7cc218, **{op: int(op[4:], 16) for op in SHOWS if op != 'Show'}}
DEF, DEF_TABLE = 0x31220000, 0x31221000


def route(kind, def_kind, d288, link, w284, flags):
    """the panel ThingPanel_Show (0x486920) shows a thing in, None for none"""
    if kind == 3:
        if def_kind == 0:
            return 'Show7cc3e0' if d288 == 2 else 'Show' if link or w284 == 0 else None
        if def_kind == 3:
            return 'Show7ca178' if flags & 1 else 'Show7cc328' if flags & 2 else 'Show7ca228'
        return {1: 'Show7cc258', 2: 'Show7cc298'}.get(def_kind)
    return 'Show7cc3b0' if kind in (4, 5, 6, 7, 8) else 'Show7cc408' if kind == 1 else None


def show_thing_case(r):
    """ShowThing (0x486920, above)"""
    kind = r.choice([3] * 8 + [4, 5, 6, 7, 8, 1, 1, 0, 2, 9, r.getrandbits(8)])
    def_kind = r.choice([0, 0, 0, 0, 1, 2, 3, 3, 4, r.getrandbits(32)])
    d288 = r.choice([2, 2, 0, 1, r.getrandbits(32)])
    link = r.choice([0, 0, 1, 2, 3])
    w284 = r.choice([0, 0, 1, r.getrandbits(16)])
    flags = r.choice([0, 1, 2, 3, r.getrandbits(16)])
    op = route(kind, def_kind, d288, link, w284, flags) or r.choice(list(SHOWS))
    regions, masks = show_case(r, op, PANELS[op])
    thing = bytearray(regions[THING_SHOWN])
    ident = r.getrandbits(16)
    thing[2] = kind
    put(thing, 0xe, 'H', ident)
    put(thing, 0x30, 'H', link)
    put(thing, 0x32, 'H', flags)
    regions[THING_SHOWN] = thing
    d = bytearray(r.bytes(0x4b0))
    put(d, 0, 'H', ident)
    put(d, 0x284, 'H', w284)
    put(d, 0x288, 'I', d288)
    put(d, 0x4ac, 'I', def_kind)
    regions[DEF] = d
    assert 0x7890a0 not in regions
    defs = bytearray(r.bytes(0x738))   # (the definitions: the cached one, else a search of the first +0x528 for the id)
    hit = r.random() < 0.6
    put(defs, 0x730, 'H', ident if hit else ident ^ 1)
    put(defs, 0x734, 'I', DEF)
    n = r.choice([1, 2, 3])
    table = [DEF_TABLE + 4 * k for k in range(n)]
    table[r.randrange(n)] = DEF
    for k, at in enumerate(table):
        put(defs, 4 * k, 'I', at)
        if at != DEF:
            regions[at] = put(bytearray(r.bytes(4)), 0, 'H', ident ^ 1)
    put(defs, 0x528, 'i', n)
    regions[0x7890a0] = defs
    things = bytearray(r.bytes(0x14 * 16))   # (the linked things: entries mostly the thing shown; the advisor's entry kept)
    for k in range(4):
        put(things, 0x14 * k, 'I', THING_SHOWN if r.random() < 0.95 else r.choice([0, RAM]))
    for at in [at for at in regions if 0x7cfb90 <= at < 0x7cfb90 + len(things)]:
        v = regions.pop(at)
        things[at - 0x7cfb90:at - 0x7cfb90 + len(v)] = v
        if link and (at - 0x7cfb90) // 0x14 == link:   # (a link not the advisor's entry: its masked bytes read as a thing)
            put(thing, 0x30, 'H', r.choice([k for k in (1, 2, 3) if k != link]))
    regions[0x7cfb90] = things
    return Call(CALLS['ShowThing'], [0 if r.random() < 0.03 else THING_SHOWN], regions, ret='v', masks=masks)


def case(r):
    op = r.deal(OPS)
    if op == 'Buttons':
        return buttons_case(r)
    if op == 'ShowThing':
        return show_thing_case(r)
    if op in SHOWS:
        return show_case(r, op)
    regions = dialogbuild.chain(r, 'Open', 0)
    panelparts.surroundings(r, regions)
    panel7cc258.things(r, regions)
    regions[0x7cc208] = regions.pop(0x7cc288)
    ids = list(LOOKED[op])
    if r.random() < 0.04:
        ids.remove(r.choice(ids))
    ids += decoys(r, 0x3e2e)
    r.shuffle(ids)
    items = [(KIDS + BIG * k, ident) for k, ident in enumerate(ids)]
    panelparts.kids(r, regions, items, ROOT)
    sub = 0
    for at, ident in items:
        w = grow(r, regions, at)
        if ident in (0x3e2d, 0x3e30, 0x3e2f):   # (0x705b20: range +0x138 / +0x13c, step +0x144)
            put(w, 0x138, 'iiii', r.choice([0x78, 10, 0, r.getrandbits(31)]), r.choice([0, 0, 1, r.getrandbits(31)]), r.getrandbits(31),
                r.choice([1, -1, 5, -5, 0, r.getrandbits(31)]))
        if ident in GAUGES:   # (its own children: the mesh's 0x3e2e among decoys, now and then missing)
            mine = decoys(r, 0x3e2d) + ([0x3e2e] if r.random() < 0.97 else [])
            r.shuffle(mine)
            subs = [(SUBKIDS + BIG * (sub + k), ident_) for k, ident_ in enumerate(mine)]
            sub += len(mine)
            panelparts.kids(r, regions, subs, at)
            for at_, _ident in subs:
                grow(r, regions, at_)
        elif ident == 0x3e28:
            put(w, 0x98, 'I', dialogbuild.listrows.OBJ98)
    surround(r, regions)
    regions[0x6ff320 + 0xc] = struct.pack('<I', recorder(3, 2))   # (the new text parts told: their slot 3 a recorder too)
    window.notify_inner(regions[LOG])   # (its events hashed with the bytes each notification sets)
    regions[0x7c2658] = r.bytes(0x10)   # (the count's value and count)
    panel = bytearray(r.bytes(0x30))
    put(panel, 4, 'i', r.choice([-1] * 5 + [4, 0x3e9, -3]))
    put(panel, 0x10, 'I', 0 if op == 'Gauges' and r.random() < 0.1 else ROOT)
    regions[PANEL] = panel
    return Call(CALLS[op], [PANEL], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
