# The window at 0x7cb398 (src/game/ui/dialog7cb398: 0x4a4ab0, 0x4a52a0) against the originals. ShowTopThree: thingcount.py's park
# (thingenv.py) with values 0-22 at +0x30 (the game record's handle +0x1da720 of [0x7cf83c] one of the slots); the window's children (a few
# others with them, the child 0x4a68d1 now and then missing) and that child's 0x4a68d2..0x4a68d4 (their slot 7 a recorder, their own
# children none or window.py's tree for Window_SetShown, shown_env).
# Create (0x4a52a0): dialogbuild.py's focused() surroundings (the window none mostly; the script's parent [0x7cb2ac]) with the script at
# 0x7537d0 generated here: 0-2 plain children, then the dialog (kind 1) whose children are the title 0x4a68ce, 0x4a68d1 (its children
# 0x4a68d2-d4), the panel 0x4a68bb (missing a quarter of the time: checked) with the radio group 0x4a68c1 (kind 8, missing a quarter of the
# time: checked; its children 0x4a68c3 / c4 / c2 kind 2) and the labels 0x4a68bc-c0 (kind 2), 0x4a68c9 (its children 0x4a68cb / cc / cd /
# ca) and 0x4a68d0 / 0x4a68cf, each list in any order among 0-2 decoys, now and then (2%) one unchecked child missing (a fault in both);
# the window kinds' slots 5, 7 and 9 recorders (game code the driver cannot run natively), the text parts' slots 3 and 9 too (unported),
# the meshes as dialog7cca94.py's (global_heap(), their vtable 0x6feeb8's slots 3 / 5 recorders); the language texts 0x97-0x9e
# (panelparts.py's texts(); 0x16a-0x16c past its table), the panel's RendererPart's slots 3 / 5 the meshes' recorders (unported); [0x7cb39c]
# 0-2 mostly, the byte [0x753c48] any. The park as ShowTopThree's (3, 4 or 6 things, the first three of kind 1 on plain cells with three
# values 1..22, so no top child is hidden: that tells its handler 0x4a4700, whose address the driver cannot run natively) with its cells (Things_CountKind1OnPlainCell) and the things' floats +0x19c 0..100 mostly (the percentages), the analyser (slot 12 of the
# table 0x7cfb90, its total +0x21c08 any) and advisor.py's PostId surroundings (slot 13; the stack message's unwritten bytes masked), the
# world's time +0x1da70c; the GUI manager [0x77c48c] told of hidden windows (slot 13 a recorder). The heap fails now and then (mostly in
# the build: a fault in both). The log stays under its 40 entries (about 25-37).
# TopHandler (0x4a4700): see top_handler().
# usage: dialog7cb398.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ShowTopThree,Create,TopHandler)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import animatormesh  # noqa: E402
import dialogbuild  # noqa: E402
import focus  # noqa: E402
import panelparts  # noqa: E402
import radiogroup  # noqa: E402
import window705a50  # noqa: E402
from dialog7cc5f4 import merged  # noqa: E402
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, Mask, _cdecl_log, blob_bytes, global_heap, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
from msgtags import ADVISOR_RANGES, TIMER_RANGES  # noqa: E402
from thingenv import CELLS, SLOTS, THING, park  # noqa: E402
from window import RANGES as WINDOW_RANGES, node, shown_env  # noqa: E402

CALLS = {'ShowTopThree': 0x4a4ab0, 'Create': 0x4a52a0, 'TopHandler': 0x4a4700}
THINGS = [(0x4a4ab0, 0x4a4b8a), (0x4c8430, 0x4c85a8), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1),
          (0x4fa990, 0x4fa9fa), (0x536300, 0x53644c), (0x4d8300, 0x4d8328), (0x519510, 0x519550)]
B = Bench(merged(THINGS + WINDOW_RANGES + dialogbuild.ALL_RANGES + panelparts.ALL_RANGES + focus.MODES + focus.RANGES + radiogroup.RANGES +
                 window705a50.RANGES + TIMER_RANGES + ADVISOR_RANGES + [
    (0x4a52a0, 0x4a5e0e), (0x4a4700, 0x4a4748), (0x4a0810, 0x4a083d), (0x4c9130, 0x4c91d0), (0x4c94c0, 0x4c94c7), (0x4c9670, 0x4c9744), (0x4c9750, 0x4c9824),
    (0x51a280, 0x51a287), (0x4e0440, 0x4e0450), (0x4e2710, 0x4e2740), (0x4dd4e0, 0x4dd500), (0x412e90, 0x412eea), (0x41a960, 0x41a970),
    (0x5161b0, 0x516200), (0x481f80, 0x481fa0), (0x481fc0, 0x482030), (0x4cd300, 0x4cd331), (0x41a990, 0x41a9a7), (0x506eb0, 0x506ecc),
    (0x67a400, 0x6902c0), (0x485d20, 0x485e30), (0x48d290, 0x48d36d), (0x48f690, 0x48f6b0), (0x492d20, 0x492d3b), (0x6589f9, 0x658abb),
    (0x658ccc, 0x658e04), (0x659710, 0x659740), (0x659d00, 0x659d53), (0x659d90, 0x659df5), (0x659e3e, 0x659e93), (0x65a9d0, 0x65aa00),
    (0x65aa20, 0x65aaa0), (0x65c388, 0x65c4c9), (0x65c5d5, 0x65c682), (0x65c7fe, 0x65c989), (0x65c9bd, 0x65cda5), (0x65d070, 0x65d1d8),
    (0x65d42e, 0x65d4b0), (0x65d773, 0x65d832), (0x65db25, 0x65dc00), (0x65e526, 0x65e559), (0x65e6d5, 0x65e73d), (0x65f5fe, 0x65f655),
    (0x65f66e, 0x65f6e1), (0x6616b0, 0x661780), (0x6617f0, 0x661820), (0x6697e3, 0x6698b4), (0x66bff7, 0x66c01b), (0x65da8d, 0x65db25),
    (0x65f5b7, 0x65f643), (0x661525, 0x6615a3), (0x485ac0, 0x485b00)]), hooks={**animatormesh.HOOKS, 0x67a7d0: _cdecl_log})
ROOT, PANEL, KIDS, KVTABLE, DECOYS, POOL = 0x33000000, 0x33010000, 0x33020000, 0x33030000, 0x33040000, 0x34000000   # (window.py's tree is at RAM + 0x3000)
NODE = 0x120
KIND_VTABLES = (0x705980, 0x705a50, 0x705b20, 0x705950, 0x705a20, 0x705c58, 0x7059f0, 0x705ac0, 0x705c28, 0x705bf8, 0x705bc8, 0x705af0,
                0x705b98)   # (the factory's kinds 1-13)
ANALYSER, MANAGER = 0x35000000, 0x35100000
dialogbuild.DIALOGS['Dialog7cb398_Create'] = (0x7cb398, 0x7537d0, (0x4a68ce,), (0x7cb2a0, 0x7cb360, 0x7cb378))   # (scripted()'s script, replaced)


def window(r, regions, at, parent, children):
    buf = bytearray(r.bytes(NODE))
    put(buf, 0, 'I', KVTABLE)
    buf[0x48] = (buf[0x48] & 0xfe) | r.choice([0, 1, 1])
    put(buf, 0x54, 'I', 0)
    put(buf, 0x5c, 'I', children)
    put(buf, 0x60, 'I', parent)
    for off in (0x98, 0x9c, 0x114):
        put(buf, off, 'I', 0)
    regions[at] = buf
    return buf


def children(r, regions, ids, parent, base, decoys):
    """a child list of the windows `ids` (address, id) shuffled among decoy nodes (ids from decoys) from base on; returns its head"""
    items = list(ids) + [(base + NODE * i, r.choice(decoys + [r.getrandbits(32)])) for i in range(r.choice([0, 1, 2, 3]))]
    r.shuffle(items)
    for i, (addr, ident) in enumerate(items):
        if addr not in regions:
            regions[addr] = node(r, ident, 0)
            put(regions[addr], 0x60, 'I', parent)
        put(regions[addr], 0x54, 'I', items[i + 1][0] if i + 1 < len(items) else 0)
        put(regions[addr], 0x8c, 'I', ident)
    return items[0][0] if items else 0


def things(r, regions, count):
    """the park (ShowTopThree's, with its cells; the things' values +0x30 and floats +0x19c); returns its handles"""
    park_regions, handles, _ = park(r, count=count, kinds=(1, 1, 1, 2, 3), defs=False, cells=True, pool=POOL)
    regions.update(park_regions)
    pool = bytearray(regions[POOL])
    for i in range(count):
        put(pool, THING * i + 0x30, 'i', r.choice([0, 1, 2, 3, 4, 5, 10, 21, 22, r.randrange(23)]))
        put(pool, THING * i + 0x19c, 'f', r.choice([r.uniform(0, 100)] * 6 + [0.0, 33.0, 66.0, -5.0, 300.0, 1e30]))
    regions[POOL] = pool
    return handles


def top_three(r):
    log = log_region()
    regions = {LOG: log}
    wins = shown_env(r, regions, log)
    count = r.choice([0, 1, 2, 3, 4, 6])
    handles = things(r, regions, count)
    regions[RAM + 0x1da720] = struct.pack('<H', r.choice(handles + [0]) if handles else 0)
    kids = [(KIDS + NODE * k, 0x4a68d2 + k) for k in range(3)]
    for addr, _ in kids:
        window(r, regions, addr, PANEL, r.choice([0, 0, wins[0]]))
    regions[KVTABLE] = vtable({7: 1})
    window(r, regions, ROOT, 0, 0)
    window(r, regions, PANEL, ROOT, 0)
    put(regions[PANEL], 0x5c, 'I', children(r, regions, kids, PANEL, DECOYS + 0x1000, [0, 0x4a68d1, 0x4a68d5]))
    put(regions[ROOT], 0x5c, 'I', children(r, regions, [(PANEL, 0x4a68d1)] if r.random() < 0.9 else [], ROOT, DECOYS, [0, 0x4a68d0, 0x4a68d2, 0x4a68d4]))
    regions[0x7cb398] = struct.pack('<I', ROOT)
    return Call(CALLS['ShowTopThree'], [], regions, ret='v')


def script(r, items, missing):
    """the script words of the children `items` ((id, kind, their own items or None, checked)) in any order among 0-2 decoys; one
    unchecked child of all left out when `missing` names it, a checked one a quarter of the time"""
    words, ids = [], [it for it in items if not (it[3] and r.random() < 0.25) and it[0] != missing]
    ids += [(r.choice([0, 1, 0x4a68b0, 0x4a68e0, r.getrandbits(32)]), None, None, False) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(ids)
    for ident, kind, inner, _ in ids:
        dialogbuild.child(r, words, ident, (script(r, inner, missing) if inner else []) + [5], kind=kind)
    return words


def create(r):
    regions = dialogbuild.focused(r, 'Dialog7cb398_Create')
    log = regions[LOG]
    regions[HEAP] = heap(0x100000, r.choice([0] * 40 + [r.randrange(1, 24)]))   # (a failure in the build faults in both)
    regions[0x77c48c] = struct.pack('<I', MANAGER)   # (the manager told of a hidden window: its slot 13 a recorder)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    regions.update(global_heap())
    regions[panelparts.MESH_VTABLE + 0xc] = struct.pack('<I', recorder(26, 2))
    blob_bytes(log, 26, 1, 0)   # (its event a stack pointer, logged as a hash of none of its bytes, as listrows.py's)
    regions[panelparts.MESH_VTABLE + 0x14] = struct.pack('<I', recorder(27, 1))
    regions[0x6ff2d8 + 0xc] = struct.pack('<I', recorder(26, 2))   # (the panel's RendererPart told the same: unported)
    regions[0x6ff2d8 + 0x14] = struct.pack('<I', recorder(27, 1))
    for vt in KIND_VTABLES:   # (the built windows' slots 5, 7 and 9: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    for vt in (0x6ff320, 0x6ff350):   # (the text parts' layout and text: unported)
        if vt != 0x6ff320:
            regions[vt + 0xc] = struct.pack('<I', recorder(3, 2))
        regions[vt + 0x24] = struct.pack('<I', recorder(9, 1))
    panelparts.texts(r, regions, list(range(0x97, 0x9f)))
    regions[0x7cb39c] = struct.pack('<I', r.choice([0, 1, 2] * 3 + [3, r.getrandbits(32)]))
    regions[0x753c48] = bytes([r.getrandbits(8)])
    # the park, the analyser (slot 12) and the advisor (slot 13)
    things(r, regions, r.choice([3, 4, 6]))
    pool, grid = bytearray(regions[POOL]), bytearray(regions[CELLS])
    for i, value in enumerate(r.sample(range(1, 23), 3)):   # (three tops 1..22: a top child hidden tells its handler 0x4a4700, which the driver cannot run natively)
        pool[THING * i + 2] = 1
        put(pool, THING * i + 0x30, 'i', value)
        put(grid, ((pool[THING * i + 7] << 7) + pool[THING * i + 5]) * 0x44 + 8, 'I', 1)
    regions[POOL], regions[CELLS] = pool, grid
    regions[SLOTS] =bytes(regions[SLOTS]) + struct.pack('<5I5I', ANALYSER, 12, 0, 0, 0, advisor.ADVISOR, 13, 0, 0, 0)
    regions[ANALYSER + 0x21c08] = struct.pack('<I', r.choice([0, 1, 7, 100, r.getrandbits(32)]))
    adv = advisor.queue(r, 'PostId')
    masks = [Mask(m.region, m.offsets + [0x16d0 + k for k in (2, 3, 0x17)], m.reason) for m in adv.masks]   # (199's message copied to +0x16d0 too)
    for k, v in adv.regions.items():
        if k not in (LOG, 0x7cf83c, 0x80239c, advisor.WORLD) and not 0x7cfb90 <= k < 0x7cfb90 + 20 * 16:
            regions[k] = v
    set_return(log, 20, r.choice([0, 1, 5, 100, -1, r.getrandbits(32)]))
    table = bytearray(regions[0x76e300])
    if r.random() < 0.7:   # (199's record its own mostly)
        put(table, 0x38 * 199 + 4, 'I', 199)
    regions[0x76e300] = table
    world = bytearray(r.bytes(0x18))
    put(world, 0, 'I', struct.unpack('<I', adv.regions[advisor.NOW])[0] if advisor.NOW in adv.regions else r.getrandbits(32))
    put(world, 0x10, 'HHH', 13, r.getrandbits(16), 12)
    regions[RAM + 0x1da70c] = world
    # the script: 0-2 plain children, then the dialog
    missing = r.choice([None] * 49 + [r.choice([0x4a68ce, 0x4a68d1, 0x4a68d2, 0x4a68c3, 0x4a68bc, 0x4a68c0, 0x4a68c9, 0x4a68cc, 0x4a68d0,
                                                0x4a68cf])])
    radio = [(0x4a68c3, 2, None, False), (0x4a68c4, 2, None, False), (0x4a68c2, 2, None, False)]
    panel = [(0x4a68c1, 8, radio, True)] + [(i, 2, None, False) for i in range(0x4a68bc, 0x4a68c1)]
    items = [(0x4a68ce, 1, None, False), (0x4a68d1, 1, [(i, 1, None, False) for i in (0x4a68d2, 0x4a68d3, 0x4a68d4)], False),
             (0x4a68bb, 1, panel, True), (0x4a68c9, 1, [(i, 1, None, False) for i in (0x4a68cb, 0x4a68cc, 0x4a68cd, 0x4a68ca)], False),
             (0x4a68d0, 1, None, False), (0x4a68cf, 1, None, False)]
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), script(r, items, missing) + [5], 160, 1)
    w += [5] * 4
    regions[0x7537d0] = struct.pack('<%dH' % len(w), *w)
    return Call(CALLS['Create'], [], regions, ret='v', masks=masks)


def top_handler(r):
    """0x4a4700: a window whose slot 8 (a recorder) answers about the record table's indexes 0-5 (or any), its default handler +0x118
    fncmp's logger; panelparts.py's texts() with [0x78ba9c] a copy of its table counting 0, 1, 3 or 6; the message 0xc mostly"""
    log = log_region()
    regions = {LOG: log, 0x78b530: r.bytes(0x600)}
    panelparts.texts(r, regions, [0, 1, 2, 3, 4, 5])
    table = bytearray(regions[panelparts.TEXTS + 0xa00])   # ([0x78ba9c]: a copy of [0x78ba94]'s table with another count, mostly none)
    put(table, 8, 'I', r.choice([0, 0, 0, 1, 3, 6]))
    regions[panelparts.TEXTS + 0xa40] = table
    put(regions[0x78b530], 0x56c, 'I', panelparts.TEXTS + 0xa40)
    w = bytearray(r.bytes(NODE))
    put(w, 0, 'I', KVTABLE)
    put(w, 0x118, 'I', CDECL_LOG4)
    regions[KIDS], regions[KVTABLE] = w, vtable({8: 0})
    set_return(log, 8, r.choice([0, 1, 2, 3, 4, 5, -1, r.getrandbits(32)]))
    regions[0x7cb310] = r.bytes(8)
    message = r.choice([0xc] * 4 + [0, 0xb, 0xd, 0x10c, 0x14, r.getrandbits(32)])
    return Call(CALLS['TopHandler'], [KIDS, message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Create':
        return create(r)
    if op == 'TopHandler':
        return top_handler(r)
    return top_three(r)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
