# The thing browser's panel (src/game/ui/thingbrowser: 0x4b3490, 0x4b3580) against the originals: personsprite.py's sprite as the thing
# shown [0x7cc308] (or none), its kind +2 staff (4-8) or another, the count +0x1d8 0 or not, +0x30 0, 1 or any, the mode +0x19c 0-6 or any;
# the panel [0x7cc304] a window whose children (+0x5c, then +0x54) hold the ids 0x147 / 0x148 / 0x14a among others (their vtable's
# slots 9 and 7 recorders of one argument; 0x148's objects +0x98 / +0x9c recorders of its notification, the event's word +4 hashed), the
# last code [0x756b98] 0-3, 9 or any; personsprite.py's clock and camera (ShowName ends on Camera_CentreOnSprite). The text formatter
# 0x6acd60 is a log stub on both sides (driver: TextFormatEnv), so the count's argument object, its fifth argument, is not compared.
# GroupHandler (0x4b3b90): radiogroup.py's group (its children's notifications masked by the looks they end with: Select's states and,
# for b other than 1, the child a set to 1), the message 0x100 mostly (else the default handler +0x118 fncmp's logger), a one of the
# children's ids, the selected one, -1 or another (no child: SetState of 0 faults in both), b 1, 0 or any; the group's parent +0x60 a
# window (its byte +4 refusing posts now and then) or 0, window.py's queues.
# Create (0x4b3cf0): dialogbuild.py's scripted() surroundings ([0xfaa5d8] its parent window) with the scripts generated here: at
# 0x756950 0x95c and 0x963 (kind 1, hidden: a shown one hidden would tell its handler 0x4b3790, whose address the driver cannot run
# natively), each with its child 0x95d among 0-2 decoys, in any order among 0-2 decoys; at 0x756ad8 0-2 plain children, then the panel
# (kind 1, hidden for its handler 0x4b3c10) with 0x147 / 0x148 / 0x149 (kind 1, shown or not) among 0-2 decoys; now and then (3%) one
# of the five missing (a fault in both); the window kinds' slots 5 recorders, listrows.py's registry with "hilight" (or not), the GUI
# manager [0x77c48c] told of hidden windows (slot 13 a recorder). The heap fails now and then (mostly in the build: a fault in both).
# Open (0x4b3f60): ShowName's thing given (or none: the selection cleared), its kind 1 (the window [0x7cc300]), 4-8 (the others, each
# its page and button) or another (no button, the page [0x7cc30c] kept: 6 or other); the chosen window's child 0x95d radiogroup.py's group
# (0-5 children of the ids 0x95e-0x964), the other window the same or a childless one (so the wrong one faults); the panel's children
# (0x140 bytes, no objects, the handler +0x114 fncmp's logger or none), the panel added to the dialog [0x7cb2b8] (childless or with one
# other child, mostly 0x21: posted to unless its dword 2 is set; the panel linked already now and then: not added), the root [0xfaa5d8] none (no hover) and the focus another; advisor.py's
# PostId surroundings for 0x11f / 0xc6 (its message's unwritten bytes masked, +0x16d0 too), [0x7c24c8] none, focus.py's mouse modes (the
# setting 0xfb3b7c 1, the current mode's slot 9 answering 9 mostly, the heap failing now and then).
# OpenShown (0x481790): Open's world, the thing the shown one [0x7c2658] when the flag [0x7cc3c0] is set (else none, [0x7c2658] any).
# usage: thingbrowser.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ShowName,ShowDetail,GroupHandler,Create,Open,OpenShown)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import dialogbuild  # noqa: E402
import focus  # noqa: E402
import listrows  # noqa: E402
import radiogroup  # noqa: E402
import windowfactory  # noqa: E402
import windowscript  # noqa: E402
from cursor import RANGES as CURSOR_RANGES  # noqa: E402
from dialog7cc444 import KIND_VTABLES, merged  # noqa: E402
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, Mask, _cdecl_log4, blob_bytes, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
from msgtags import ADVISOR_RANGES  # noqa: E402
from window import RANGES  # noqa: E402
from personsprite import THING, centre_env, sprite_env  # noqa: E402

CALLS = {'ShowName': 0x4b3490, 'ShowDetail': 0x4b3580, 'GroupHandler': 0x4b3b90, 'Create': 0x4b3cf0, 'Open': 0x4b3f60, 'OpenShown': 0x481790}
B = Bench(merged(RANGES + radiogroup.RANGES + dialogbuild.ALL_RANGES + listrows.REG_RANGES + CURSOR_RANGES + focus.MODES + ADVISOR_RANGES + [
           (0x4b3f60, 0x4b4255), (0x65f447, 0x65f538), (0x65f345, 0x65f410),(0x4b3cf0, 0x4b3f1b), (0x65f538, 0x65f5b7),(0x4b3b90, 0x4b3c09), (0x65d55a, 0x65d56b), (0x475480, 0x4754f0), (0x4d8c50, 0x4d8c70), (0x4f9f00, 0x4fa2b0), (0x4fe900, 0x4fe960), (0x50b350, 0x50b370), (0x510160, 0x510190),
           (0x53c8d0, 0x53c9d0), (0x5da3c0, 0x5da3c1), (0x486860, 0x4868c1), (0x42aab0, 0x42ab1d),
           (0x4b3490, 0x4b3576), (0x4b3580, 0x4b3612), (0x65d7bc, 0x65d832), (0x65d3a3, 0x65d40c), (0x506860, 0x506867), (0x5064f0, 0x506523),
           (0x485ec0, 0x485eea), (0x481790, 0x4817b3)]), hooks={0x6acd60: _cdecl_log4})
PANEL, KIDS, VT, OBJ, OVT = RAM + 0x30000, RAM + 0x30200, RAM + 0x30800, RAM + 0x30900, RAM + 0x30a00
PARENT = RAM + 0x3000   # (GroupHandler's: past radiogroup.py's group and children, before window.py's queues' end)
MANAGER = RAM + 0x30c00
BROWSER, EMPTY, DIALOG, OTHER, OPEN_KIDS = RAM + 0x31000, RAM + 0x31200, RAM + 0x31400, RAM + 0x31600, RAM + 0x34000
CLOSING = RAM + 0x31800   # (Open's [0x7c24c8])
SELECT = {1: 0x964, 4: 0x95f, 5: 0x95e, 6: 0x961, 7: 0x962, 8: 0x960}   # (Open: the kinds' buttons)
dialogbuild.DIALOGS['ThingBrowser_Create'] = (0x7cc304, 0x756ad8, (0x147,), ())   # (scripted()'s script, replaced below)


def group_case(r):
    log = log_region()
    regions = {LOG: log}
    g, kids = radiogroup.group_env(r, regions, log)
    put(g, 0x60, 'I', r.choice([PARENT] * 5 + [0]))
    put(g, 0x118, 'I', CDECL_LOG4)
    g[4] = 0
    parent = bytearray(r.bytes(0x120))
    parent[4] = r.choice([0, 0, 0, 1])
    regions[PARENT] = parent
    selected = int.from_bytes(g[0x138:0x13c], 'little')
    message = r.choice([0x100] * 6 + [0x101, 0, r.getrandbits(32)])
    a = r.choice([k[0] for k in kids] * 3 + [selected, 0xffffffff, 7])
    b = r.choice([1, 1, 0, 0, 2, r.getrandbits(32)])
    i = radiogroup.first(kids, a)
    if i is None and r.random() < 0.85:   # (mostly not the fault)
        b = 1
    states = radiogroup.select_states(kids, selected, a) if message == 0x100 else {}
    if message == 0x100 and b != 1 and i is not None:
        states[i] = 1
    radiogroup.masks(log, kids, states)
    return Call(CALLS['GroupHandler'], [radiogroup.GROUP, message, a, b], regions, ret='a')


def hidden(r, w, ident, script, size=40):
    """dialogbuild.py's child() of kind 1, hidden (its flags' bit 0 clear)"""
    w += [0, 1]
    windowscript.dword(r, w, windowfactory.no_handlers(1, r.choice(windowscript.FLAGS)) & ~1)
    windowscript.dword(r, w, ident)
    dialogbuild.small_rect(r, w, size)
    w += list(script)


def among(r, ids, missing):
    """`ids` less `missing` among 0-2 decoys, shuffled"""
    out = [i for i in ids if i != missing] + [r.choice([0, 1, 0x95e, 0x14b, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(out)
    return out


def create_case(r):
    regions = dialogbuild.scripted(r, 'ThingBrowser_Create')
    log = regions[LOG]
    regions.pop(0x7cc304)
    regions[HEAP] = heap(0x80000, r.choice([0] * 40 + [r.randrange(1, 16)]))   # (a failure lands in the builds: a fault in both)
    regions[0xfaa5d8] = struct.pack('<I', dialogbuild.window.WINDOW2)
    for vt in KIND_VTABLES:   # (the built windows' slots 5: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
    regions[0x77c48c] = struct.pack('<I', MANAGER)   # (the manager told of a hidden window: its slot 13 a recorder)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    listrows.register(r, regions, 'hilight')
    regions[listrows.OBJ98] = put(put(bytearray(r.bytes(0x20)), 0, 'I', listrows.VT98), 0x10, 'HH', r.choice([0, 1, 2]), r.choice([0, 1, 2]))
    regions[0x7cc2f0] = r.bytes(0x18)   # ([0x7cc2f0] and the three windows written)
    missing = r.choice([None] * 190 + [0x95c, 0x963, 0x95d, 0x147, 0x148, 0x149])
    w = []
    for ident in among(r, (0x95c, 0x963), missing):
        if ident in (0x95c, 0x963):
            inner = []
            for kid in among(r, (0x95d,), missing):
                dialogbuild.child(r, inner, kid)
            hidden(r, w, ident, inner + [5], 120)
        else:
            dialogbuild.child(r, w, ident)
    w += [5] * 4
    regions[0x756950] = struct.pack('<%dH' % len(w), *w)
    w, inner = [], []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    for ident in among(r, (0x147, 0x148, 0x149), missing):
        dialogbuild.child(r, inner, ident, kind=1 if ident in (0x147, 0x148, 0x149) else None)
    hidden(r, w, r.choice([0, 1, r.getrandbits(32)]), inner + [5], 160)
    w += [5] * 4
    regions[0x756ad8] = struct.pack('<%dH' % len(w), *w)
    return Call(CALLS['Create'], [], regions, ret='v')


def plain(r, ident, nxt=0, parent=0):
    """a window of 0x140 random bytes with the id, next sibling and parent given, no children, previous sibling or objects, its handler
    +0x114 fncmp's logger or none"""
    w = bytearray(r.bytes(0x140))
    put(w, 0x54, 'III', nxt, 0, 0)
    put(w, 0x60, 'I', parent)
    put(w, 0x8c, 'I', ident)
    put(w, 0x98, 'II', 0, 0)
    put(w, 0x114, 'I', r.choice([0, CDECL_LOG4]))
    return w


def open_case(r, shown=False):
    """Open's world; shown: OpenShown's (the thing [0x7c2658] when [0x7cc3c0] is set, else none)"""
    log = log_region()
    regions = {LOG: log}
    thing = sprite_env(r, regions)
    centre_env(r, regions)
    thing[2] = r.choice([1, 1, 4, 5, 6, 7, 8, 2, 3, 0, 9, r.getrandbits(8)])
    put(thing, 0x1d8, 'I', r.choice([0, 0, 1, 5, r.getrandbits(32)]))
    put(thing, 0x30, 'I', r.choice([0, 0, 1, 2, 0x29b, r.getrandbits(32)]))
    put(thing, 0x19c, 'I', r.choice([0, 1, 2, 3, 4, 5, 6, r.getrandbits(32)]))
    arg = THING if r.random() < 0.9 else 0
    if shown:
        flag = r.choice([0, 1, 1, 1, r.getrandbits(32) | 1])
        arg = arg if flag else 0
        regions[0x7cc3c0] = struct.pack('<I', flag)
        regions[0x7c2658] = struct.pack('<I', arg if flag else r.choice([THING, r.getrandbits(32)]))
    ids = [0x147, 0x148, 0x14a] + r.choice([[], [1], [0x149, 0x147]])
    r.shuffle(ids)
    for i, n in enumerate(ids):
        regions[OPEN_KIDS + 0x140 * i] = put(plain(r, n, OPEN_KIDS + 0x140 * (i + 1) if i + 1 < len(ids) else 0, PANEL), 0, 'I', VT)
    panel = plain(r, r.getrandbits(32))
    put(panel, 0x5c, 'I', OPEN_KIDS)
    if r.random() < 0.05:   # (linked already: not added)
        put(panel, r.choice([0x54, 0x58, 0x60]), 'I', OTHER)
    regions[PANEL] = panel
    regions[VT] = vtable({9: 1, 7: 1})
    dialog = plain(r, r.getrandbits(32))
    other = put(plain(r, r.choice([0x21, 0x21, 1, r.getrandbits(32)]), 0, DIALOG), 0xa8, 'I', r.choice([0, 0, 0, 1]))   # (0x21: posted to unless its dword 2 is set)
    other[4] = r.choice([0, 0, 1])   # (sent to unless its byte +4 is set)
    regions[OTHER] = other
    if r.random() < 0.5:
        put(dialog, 0x5c, 'I', OTHER)
    regions[DIALOG] = dialog
    g, kids = radiogroup.group_env(r, regions, log, ids=[0x95e, 0x95f, 0x960, 0x961, 0x962, 0x963, 0x964])
    put(g, 0x54, 'I', 0)
    put(g, 0x8c, 'I', 0x95d)
    regions[BROWSER] = put(plain(r, 0x95c), 0x5c, 'I', radiogroup.GROUP)
    regions[EMPTY] = plain(r, 0x963)
    chosen = 0x7cc300 if arg and thing[2] == 1 else 0x7cc2fc
    windows = {chosen: BROWSER, chosen ^ 0x7cc300 ^ 0x7cc2fc: r.choice([BROWSER, EMPTY])}
    ident = (SELECT.get(thing[2]) if arg else 0xffffffff)
    selected = struct.unpack_from('<I', g, 0x138)[0]
    radiogroup.masks(log, kids, radiogroup.select_states(kids, selected, ident) if ident is not None else {})
    regions[0x7cc2f0] = struct.pack('<8I', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), windows[0x7cc2fc], windows[0x7cc300], PANEL,
                                    r.choice([THING, 0, r.getrandbits(32)]), r.choice([6, 6, 1, 0, 3, r.getrandbits(32)]))
    regions[0x7cb2b8] = struct.pack('<I', DIALOG)
    regions[0xfaa5d8] = struct.pack('<6I', 0, *[r.getrandbits(32) for _ in range(5)])
    regions[0x790ab0] = r.bytes(4)
    regions[0x756b98] = struct.pack('<I', r.choice([0, 1, 2, 3, 9, r.getrandbits(32)]))
    regions[0x78ba94] = r.bytes(4)
    regions[0x77c48c] = struct.pack('<I', MANAGER)   # (the manager told of a hidden window: its slot 13 a recorder)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    adv = advisor.queue(r, 'PostId')
    for k, v in adv.regions.items():
        if k != LOG:
            regions[k] = v
    if 0xfb34bc in regions:   # (the mode setting's static built already: its first build's atexit call reaches CRT code the ranges leave out)
        regions[0xfb34bc] = bytes([regions[0xfb34bc][0] | 1])
    set_return(log, 20, r.choice([0, 1, 5, 100, -1, r.getrandbits(32)]))
    masks = [Mask(m.region, m.offsets + [0x16d0 + k for k in (2, 3, 0x17)], m.reason) for m in adv.masks]
    regions[0x7c24c8] = struct.pack('<I', r.choice([0, CLOSING]))   # (a window to close: sent 5, then the focus restored to nothing)
    closing = put(plain(r, r.getrandbits(32)), 0x114, 'I', CDECL_LOG4)
    closing[4] = 0
    regions[CLOSING] = closing
    for g in (0x7c2534, 0x7cb2ac, 0x7cb2b0, 0x7cc8bc, 0x7cc8c0, 0xf82884):
        regions[g] = bytes(4)
    regions[HEAP] = heap(0x1000, r.choice([0] * 8 + [1, 2, 3, 4]))
    regions[focus.MODE_VT] = vtable({0: 1})
    regions[focus.MODE] = put(bytearray(r.bytes(0x10)), 0, 'I', focus.MODE_VT)
    set_return(log, 9, r.choice([9, 9, 9, 9, 1, 8, 0, r.getrandbits(32)]))
    regions[0x7b05d4] = put(put(bytearray(r.bytes(0x18)), 4, 'I', r.choice([focus.MODE, focus.MODE, 0])), 0x14, 'I', r.choice([0, focus.HOLDER]))
    regions[0xfb3518] = bytes([r.choice([1, 3, 0xff])])
    regions[0xfb3b7c] = struct.pack('<I', 1)
    if shown:
        return Call(CALLS['OpenShown'], [], regions, ret='v', masks=masks)
    return Call(CALLS['Open'], [arg], regions, ret='v', masks=masks)


def case(r):
    op = r.deal(OPS)
    if op in ('Open', 'OpenShown'):
        return open_case(r, op == 'OpenShown')
    if op == 'GroupHandler':
        return group_case(r)
    if op == 'Create':
        return create_case(r)
    log = log_region()
    regions = {LOG: log}
    thing = sprite_env(r, regions)
    centre_env(r, regions)
    thing[2] = r.choice([4, 5, 6, 7, 8, 1, 2, 3, 9, 0, r.getrandbits(8)])
    put(thing, 0x1d8, 'I', r.choice([0, 0, 1, 5, r.getrandbits(32)]))
    put(thing, 0x30, 'I', r.choice([0, 0, 1, 2, 0x29b, r.getrandbits(32)]))
    put(thing, 0x19c, 'I', r.choice([0, 1, 2, 3, 4, 5, 6, r.getrandbits(32)]))
    regions[0x7cc308] = struct.pack('<I', THING if r.random() < 0.9 else 0)
    ids = [0x147, 0x148, 0x14a] + r.choice([[], [1], [0x149, 0x147]])
    r.shuffle(ids)
    for i, n in enumerate(ids):
        kid = bytearray(r.bytes(0xa0))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', n)
        put(kid, 0x54, 'I', KIDS + 0xa0 * (i + 1) if i + 1 < len(ids) else 0)
        put(kid, 0x98, 'I', r.choice([OBJ, 0]))
        put(kid, 0x9c, 'I', r.choice([OBJ, 0]))
        regions[KIDS + 0xa0 * i] = kid
    regions[PANEL] = put(bytearray(r.bytes(0x90)), 0x5c, 'I', KIDS)
    regions[0x7cc304] = struct.pack('<I', PANEL)
    regions[VT] = vtable({9: 1, 7: 1})
    regions[OBJ] = struct.pack('<I', OVT)
    regions[OVT] = vtable({3: 2})
    blob_bytes(log, 3, 1, 0x30)
    regions[0x756b98] = struct.pack('<I', r.choice([0, 1, 2, 3, 9, r.getrandbits(32)]))
    regions[0x78ba94] = r.bytes(4)
    return Call(CALLS[op], [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
