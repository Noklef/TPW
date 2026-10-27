# The GUI's keyboard focus (src/game/ui/focus, ui/window, ui/dialogs, ui/dialog7ca1c8) against the originals. SetFocus / KillFocus
# (0x65e59b, 0x65e5fa): window.py's tree and hover (UpdateHover's), the focus [0xfaa5ec] the window, another or none. Dialog7ca1c8_Focus
# (0x498a80): SetFocus's, the window [0x7ca1c8]. FocusRestore (0x4862a0) and HasSource (0x48c8d0): SetFocus's set-up, the globals they
# pick a window from (0x7c2534's +0x10 when its +0xc is set, 0x7cb2ac-0x7cb2b4, 0x7cc8bc / 0x7cc8c0, 0x7c24c8 / 0x7c24cc, [0xf82884] + 4,
# 0x7ca1c8 / 0x7ca1cc) the tree's windows or none, and the three key maps it enables. Close7c24c8 / Close7cb2cc (0x485b40 / 0x4a27c0):
# FocusRestore's, the window [0x7cb2cc] one of the tree or none, the tree's handlers fncmp's logger or none, the hook [0xfaa5fc] too.
# Windows7cc8bc_SelectSecond (0x4bce80): SetFocus's set-up, the two windows [0x7cc8bc] / [0x7cc8c0] of the tree with handlers the logger or none.
# Overlay7cb2b4_Toggle / Obj7b51f0_ToggleOverlay / Obj7b51f0_CloseOverlay (0x4a29d0, 0x481490, 0x4815d0): see overlay_case.
# Enter6fead0 / Set7c24c8 (0x481a10 / 0x485b70, the mouse mode and the window [0x7c24c8]): see modes_case.
# CaptureMouse / ReleaseMouse (0x65e549 / 0x65e56e): SetFocus's set-up, the capture [0xfaa5e8] the window, another or none.
# WindowAt10_Show (0x493130, thiscall (flag); src/game/ui/dialogs): see show_case.
# usage: focus.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetFocus,KillFocus,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, blob_bytes, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
from window import FOCUS, MISC, OBJECT98, RANGES, VTABLE, hover, shown_env, tree  # noqa: E402
from windowfactory import FACTORY, timer_env  # noqa: E402

CALLS = {'SetFocus': 0x65e59b, 'KillFocus': 0x65e5fa, 'Dialog7ca1c8_Focus': 0x498a80, 'FocusRestore': 0x4862a0, 'HasSource': 0x48c8d0,
         'Close7c24c8': 0x485b40, 'Close7cb2cc': 0x4a27c0, 'Windows7cc8bc_SelectSecond': 0x4bce80,
         'Overlay7cb2b4_Toggle': 0x4a29d0, 'Obj7b51f0_ToggleOverlay': 0x481490, 'Obj7b51f0_CloseOverlay': 0x4815d0,
         'Enter6fead0': 0x481a10, 'Set7c24c8': 0x485b70, 'CaptureMouse': 0x65e549, 'ReleaseMouse': 0x65e56e, 'WindowAt10_Show': 0x493130}
MODES = [(0x40cb40, 0x40cb90), (0x46c350, 0x46c460), (0x46c6a0, 0x46c6c0), (0x46cff0, 0x46d010), (0x4814a0, 0x481500), (0x481a10, 0x481b00),
         (0x485b40, 0x485d30), (0x4862a0, 0x486420), (0x4a25f0, 0x4a2690), (0x4a2aa0, 0x4a2ad0), (0x550ca0, 0x550d90), (0x6590f7, 0x6591e4),
         (0x65e6d5, 0x65e73d), (0x65f11d, 0x65f160), (0x66190a, 0x66192b), (0x661d96, 0x661de0), (0x668ad3, 0x668c99), (0x67a740, 0x67a800),
         (0x67fa20, 0x67fa50), (0x681740, 0x681970), (0x5da3c0, 0x5da3c1), (0x46ce80, 0x46ceca), (0x46cfb0, 0x46cfc0), (0x40f3a0, 0x40f3a6),
         (0x40f440, 0x40f446), (0x5d1410, 0x5d1416), (0x46d590, 0x46d59d), (0x46c710, 0x46c72e), (0x46ced0, 0x46ced7), (0x5d1750, 0x5d1751)]
B = Bench(sorted(set(RANGES + FACTORY + MODES + [(0x4a29d0, 0x4a2a8a), (0x4a2a90, 0x4a2a96), (0x481490, 0x481495), (0x4815d0, 0x4815df),
                                            (0x65e549, 0x65e56e), (0x65e56e, 0x65e59b),
    (0x493130, 0x49319f), (0x659409, 0x65959a), (0x659730, 0x659760), (0x65d547, 0x65d56a), (0x65d56b, 0x65d59d),   # (WindowAt10_Show's:
    (0x65f447, 0x65f538), (0x65f345, 0x65f410), (0x65f16b, 0x65f246), (0x65d9dd, 0x65da8d), (0x65d90b, 0x65d9dd)])))   # top depth, AddChild)
OVERLAY, OVERLAY_VT = RAM + 0xb000, RAM + 0xb040   # (an overlay to delete: its slot 2 a recorder)
MODE, MODE_VT, HOLDER = RAM + 0xc000, RAM + 0xc100, RAM + 0xc200   # (the current mouse mode, a recorder object; the holder, never read)


def modes_case(r, op, regions, log, wins):
    """Enter6fead0 / Set7c24c8 (0x481a10 / 0x485b70): Close7c24c8's set-up with [0x7cb2ac] 0x30 bytes (the dialog [0x7cb2b8] none, [0x7cb2cc]
    and the cursor id 0x7cb2d8 in it), the GUI cursor's default id [0xfaa600] (no table) and current 0x77c484, a heap (an allocation now
    and then failing); the mouse mode [0x7b05d8] a recorder object (slot 9 answering 9 mostly) or none, the holder [0x7b05e8] set or not,
    the setting 0xfb3b7c 1 with its guard set (so a CMMOnline base, whose Assign 0x46c6b0 is not ported, is always replaced by a
    CMMOnline); the three key maps with 0-6 entries. Set7c24c8's window one of the tree or none."""
    regions[HEAP] = heap(0x1000, r.choice([0] * 8 + [1, 2, 3, 4]))
    game = bytearray(r.bytes(0x30))
    game[0:0xc] = regions[0x7cb2ac]
    put(game, 0xc, 'I', 0)
    game[0x20:0x24] = regions.pop(0x7cb2cc)
    regions[0x7cb2ac] = game
    regions[0xfaa600] = struct.pack('<HHI', r.choice([0, 1, 0xffff, r.getrandbits(16)]), r.getrandbits(16), 0)
    regions[0x77c484] = r.bytes(4)
    regions[MODE_VT] = vtable({0: 1})
    regions[MODE] = put(bytearray(r.bytes(0x10)), 0, 'I', MODE_VT)
    set_return(log, 9, r.choice([9, 9, 9, 1, 7, 0, r.getrandbits(32)]))
    regions[0x7b05d4] = put(put(bytearray(r.bytes(0x18)), 4, 'I', r.choice([MODE, MODE, 0])), 0x14, 'I', r.choice([0, HOLDER]))
    regions[0xfb3518] = bytes([r.choice([1, 3, 0xff])])
    regions[0xfb3b7c] = struct.pack('<I', 1)
    for k in range(3):   # (the key maps' entries, KeyMap_Reset's: 0-6 of them)
        m, entries = FOCUS + 0x80 + 0x10 * k, RAM + 0xc400 + 0x100 * k
        regions[m] = put(put(bytearray(regions[m]), 0, 'I', entries), 4, 'H', r.choice([0, 1, 2, 3, 6]))
        regions[entries] = r.bytes(6 * 0x14)
    if op == 'Enter6fead0':
        return Call(CALLS[op], [], regions, ret='v')
    return Call(CALLS[op], [r.choice(wins + [0, 0])], regions, ret='a')


def overlay_case(r, op, regions, log):
    """Overlay7cb2b4_Toggle and its callers: window.py's shown_env (SetShown's tree, the root [0xfaa5d8]), windowfactory.py's timer_env and
    a heap (CreateChild's, an allocation now and then failing: a fault in both), the focus [0xfaa5ec] one of the tree or none; [0x7cb2ac]
    one of the tree (now and then none), the toggle [0x7cb2e8] 0, 1 or other, the byte 0x790ab0 with flags 0x16 or others; [0x7cb2b4] a
    recorder object while the toggle is set, else none or one of the tree; FocusRestore's globals as below ([0x7cb2b0] one of the tree)."""
    wins = shown_env(r, regions, log)
    timer_env(r, regions, log)
    regions[HEAP] = heap(0x2000, r.choice([0] * 30 + [1]))
    regions[0xfaa5ec] = struct.pack('<I', r.choice(wins + [0]))
    on = r.choice([0, 0, 1, 1, 2, 0xffffffff])
    regions[0x7cb2e8] = struct.pack('<I', on)
    regions[0x790ab0] = bytes([r.choice([0, 0, 0, 2, 4, 0x10, 1, 8, 0xe9, r.getrandbits(8)])])
    regions[OVERLAY_VT] = vtable({2: 1})
    regions[OVERLAY] = struct.pack('<I', OVERLAY_VT)
    overlay = r.choice([OVERLAY, OVERLAY, 0]) if on else r.choice([0, 0, 0] + wins)
    src, frame, maps = FOCUS, FOCUS + 0x40, [FOCUS + 0x80 + 0x10 * k for k in range(3)]
    regions[0x7c2534] = struct.pack('<I', r.choice([src, 0, 0]))
    regions[src] = put(put(bytearray(r.bytes(0x20)), 0xc, 'I', r.choice([0, 1])), 0x10, 'I', r.choice(wins))
    root = struct.unpack_from('<I', regions[0xfaa5d8])[0]   # ([0x7cb2ac] not the root: showing it would send the new overlay a message)
    others = [w for w in wins if w != root]
    ac = r.choice(others * 6 + [0]) if others else wins[0] if on or overlay else 0
    regions[0x7cb2ac] = struct.pack('<3I', ac, r.choice(wins), overlay)
    regions[0x7cc8bc] = struct.pack('<2I', r.choice(wins + [0]), r.choice(wins + [0]))
    regions[0x7c24c8] = struct.pack('<2I', r.choice(wins + [0, 0]), r.choice(wins + [0, 0, 0]))
    regions[0xf82884] = struct.pack('<I', frame)
    regions[frame] = put(bytearray(r.bytes(8)), 4, 'I', r.choice(wins))
    regions[0x7ca1c8] = struct.pack('<2I', r.choice(wins), r.choice([0, 1]))
    for at, m in zip((0x787c14, 0x787a60, 0x78733c), maps):
        regions[at] = struct.pack('<I', m)
        regions[m] = r.bytes(8)
    if op == 'Overlay7cb2b4_Toggle':
        return Call(CALLS[op], [], regions, ret='a')
    return Call(CALLS[op], [0x7b51f0], regions, conv='t', ret='a')


SHOWN, SHOWN_KID, SHOWN_OBJ, SHOWN_VT = RAM + 0xd000, RAM + 0xd200, RAM + 0xd400, RAM + 0xd440


def show_case(r):
    """WindowAt10_Show: window.py's shown_env() tree (previous links +0x58 set, as windowtree.py's AddChild), its root [0xfaa5d8] mostly
    set; the windows' kinds (slot 3: a recorder of its own, 27, answering 1, 2, 4 or 11, never 7) for Gui_TopDepth, its globals random;
    the holder's +0xc 0 mostly, its window +0x10 a new one (no links now and then one set, a leaf child or not, objects +0x98 / +0x9c set or
    not), one of the tree's below the root or none; the flag 0 or not; FocusRestore's globals (the window [0x7c24c8] one of the tree or
    none, the handlers fncmp's logger or none); the focus [0xfaa5ec] the new window, one of the tree or none; the objects' events hashed."""
    log = log_region()
    regions = {LOG: log}
    wins = shown_env(r, regions, log)
    for w in wins:
        prev, c = 0, struct.unpack_from('<I', regions[w], 0x5c)[0]
        while c:
            put(regions[c], 0x58, 'I', prev)
            prev, c = c, struct.unpack_from('<I', regions[c], 0x54)[0]
        put(regions[w], 0, 'I', SHOWN_VT)
    put(regions[wins[0]], 0x58, 'I', 0)
    root = bytearray(regions[0xfaa5d8])
    put(root, 0, 'I', r.choice([wins[0]] * 12 + [0]))
    regions[0xfaa5d8] = root
    regions[VTABLE] = vtable({5: 1, 3: 2})
    regions[SHOWN_VT] = put(vtable({}), 0xc, 'I', recorder(27, 0))
    set_return(log, 27, r.choice([4, 4, 1, 2, 11]))
    blob_bytes(log, 3, 1, 0)
    blob_bytes(log, 22, 1, 0)
    put(regions[MISC + 0x80], 0xc, 'I', recorder(22, 2))
    regions[0xfaa570] = r.bytes(4)
    regions[0xfaa594] = struct.pack('<i', r.choice([0, -1, 5, 0x7fffffff, -0x80000000, r.getrandbits(31)]))
    regions[0xfaa5c8] = r.bytes(4)
    kid = bytearray(r.bytes(0x120))
    put(kid, 0, 'I', SHOWN_VT)
    for off in (0x60, 0x54, 0x58):
        put(kid, off, 'I', 0 if r.random() < 0.85 else r.choice(wins))
    put(kid, 0x5c, 'I', r.choice([0, 0, SHOWN_KID]))
    put(kid, 0x98, 'I', r.choice([OBJECT98, 0]))
    put(kid, 0x9c, 'I', r.choice([MISC + 0x40, 0]))
    put(kid, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
    regions[SHOWN] = kid
    leaf = bytearray(r.bytes(0x120))
    put(leaf, 0, 'I', SHOWN_VT)
    put(leaf, 0x54, 'I', 0)
    put(leaf, 0x5c, 'I', 0)
    put(leaf, 0x98, 'I', r.choice([OBJECT98, 0]))
    put(leaf, 0x9c, 'I', r.choice([MISC + 0x40, 0]))
    regions[SHOWN_KID] = leaf
    window = r.choice([SHOWN] * 8 + wins[1:] + [0])
    regions[SHOWN_OBJ] = put(put(bytearray(r.bytes(0x20)), 0xc, 'I', r.choice([0] * 6 + [1, r.getrandbits(32)])), 0x10, 'I', window)
    regions[0xfaa5ec] = struct.pack('<I', r.choice([SHOWN, SHOWN, 0] + wins))
    regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))

    def some(none=3):
        return r.choice(wins + [0] * none)
    src, frame = FOCUS, FOCUS + 0x40
    regions[0x7c2534] = struct.pack('<I', r.choice([src, 0]))
    regions[src] = put(put(bytearray(r.bytes(0x20)), 0xc, 'I', r.choice([0, 0, 0, 1])), 0x10, 'I', some())
    regions[0x7cb2ac] = struct.pack('<3I', 0, some(), 0)
    regions[0x7cc8bc] = struct.pack('<2I', some(), some())
    regions[0x7c24c8] = struct.pack('<2I', some(2), some(4))
    regions[0xf82884] = struct.pack('<I', r.choice([frame, 0]))
    regions[frame] = put(bytearray(r.bytes(8)), 4, 'I', some(1))
    regions[0x7ca1c8] = struct.pack('<2I', some(1), 0)
    for at, m in zip((0x787c14, 0x787a60, 0x78733c), [FOCUS + 0x80 + 0x10 * k for k in range(3)]):
        regions[at] = struct.pack('<I', m)
        regions[m] = r.bytes(8)
    flag = r.choice([0, 0, 1, 1, r.getrandbits(32)])
    return Call(CALLS['WindowAt10_Show'], [SHOWN_OBJ, flag], regions, conv='t', ret='v')


def case(r, ops=None):
    op = r.deal(ops or OPS)
    if op == 'WindowAt10_Show':
        return show_case(r)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if 'Overlay' in op:
        return overlay_case(r, op, regions, log)
    wins = tree(r, regions)   # the hover as UpdateHover's; the focus [0xfaa5ec] the window, another of the tree or none
    hover(r, regions, wins)
    window = r.choice(wins)
    regions[0xfaa5ec] = struct.pack('<I', r.choice([window, window, 0] + wins))
    if op == 'Windows7cc8bc_SelectSecond':   # the two windows of the tree (now and then none), their handlers fncmp's logger or none
        for w in wins:
            put(regions[w], 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
        regions[0x7cc8bc] = struct.pack('<II', r.choice(wins + [0]), r.choice(wins * 4 + [0]))
        regions[0x7cc6a0] = r.bytes(4)
        if r.random() < 0.5:
            set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
        return Call(CALLS[op], [], regions, ret='a')
    if op in ('FocusRestore', 'HasSource', 'Close7c24c8', 'Close7cb2cc', 'Enter6fead0', 'Set7c24c8'):
        def some(none=3):
            return r.choice(wins + [0] * none)
        src, frame, maps = FOCUS, FOCUS + 0x40, [FOCUS + 0x80 + 0x10 * k for k in range(3)]
        regions[0x7c2534] = struct.pack('<I', r.choice([src, src, 0]))
        regions[src] = put(put(bytearray(r.bytes(0x20)), 0xc, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)])), 0x10, 'I', some())
        regions[0x7cb2ac] = struct.pack('<3I', some(4), some(), some())
        regions[0x7cc8bc] = struct.pack('<2I', some(), some())
        regions[0x7c24c8] = struct.pack('<2I', some(), some(4))
        regions[0xf82884] = struct.pack('<I', r.choice([frame, frame, 0]))
        regions[frame] = put(bytearray(r.bytes(8)), 4, 'I', some(1))
        regions[0x7ca1c8] = struct.pack('<2I', some(1), r.choice([0, 0, 1, r.getrandbits(32)]))
        for at, m in zip((0x787c14, 0x787a60, 0x78733c), maps):
            regions[at] = struct.pack('<I', m)
            regions[m] = r.bytes(8)
        for w in wins:   # (Fn_65d57c's flags: bit 0 half the time)
            put(regions[w], 0x48, 'I', struct.unpack_from('<I', regions[w], 0x48)[0] & ~1 | r.choice([0, 1]))
        if op in ('Close7c24c8', 'Close7cb2cc', 'Enter6fead0', 'Set7c24c8'):   # the window closed first (SendMessage): its handler fncmp's logger or none
            for w in wins:
                put(regions[w], 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
            regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
            regions[0x7cb2cc] = struct.pack('<I', some(2))
            ac, b0, b4 = struct.unpack('<3I', regions[0x7cb2ac])   # (FocusRestore faults on [0x7cb2b0] 0 with [0x7cb2ac]: mostly not)
            if ac and not b0 and r.random() < 0.8:
                regions[0x7cb2ac] = struct.pack('<3I', ac, r.choice(wins), b4)
        if op in ('Enter6fead0', 'Set7c24c8'):
            return modes_case(r, op, regions, log, wins)
        return Call(CALLS[op], [], regions, ret='a')
    if op in ('CaptureMouse', 'ReleaseMouse'):
        regions[0xfaa5d8] = put(bytearray(regions[0xfaa5d8]), 0x10, 'I', r.choice([window, window, 0] + wins))
        return Call(CALLS[op], [window], regions, conv='t', ret='w')
    if op == 'Dialog7ca1c8_Focus':   # (the window [0x7ca1c8])
        regions[0x7ca1c8] = struct.pack('<I', window)
        return Call(CALLS[op], [], regions, ret='w')
    return Call(CALLS[op], [window, r.getrandbits(32)], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
