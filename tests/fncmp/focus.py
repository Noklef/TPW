# The GUI's keyboard focus (src/game/ui/focus, ui/window, ui/dialogs, ui/dialog7ca1c8) against the originals. SetFocus / KillFocus
# (0x65e59b, 0x65e5fa): window.py's tree and hover (UpdateHover's), the focus [0xfaa5ec] the window, another or none. Dialog7ca1c8_Focus
# (0x498a80): SetFocus's, the window [0x7ca1c8]. FocusRestore (0x4862a0) and HasSource (0x48c8d0): SetFocus's set-up, the globals they
# pick a window from (0x7c2534's +0x10 when its +0xc is set, 0x7cb2ac-0x7cb2b4, 0x7cc8bc / 0x7cc8c0, 0x7c24c8 / 0x7c24cc, [0xf82884] + 4,
# 0x7ca1c8 / 0x7ca1cc) the tree's windows or none, and the three key maps it enables. Close7c24c8 / Close7cb2cc (0x485b40 / 0x4a27c0):
# FocusRestore's, the window [0x7cb2cc] one of the tree or none, the tree's handlers fncmp's logger or none, the hook [0xfaa5fc] too.
# Windows7cc8bc_SelectSecond (0x4bce80): SetFocus's set-up, the two windows [0x7cc8bc] / [0x7cc8c0] of the tree with handlers the logger or none.
# usage: focus.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetFocus,KillFocus,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, LOG, Bench, Call, log_region, put, set_return, vtable  # noqa: E402
from window import FOCUS, RANGES, VTABLE, hover, tree  # noqa: E402

CALLS = {'SetFocus': 0x65e59b, 'KillFocus': 0x65e5fa, 'Dialog7ca1c8_Focus': 0x498a80, 'FocusRestore': 0x4862a0, 'HasSource': 0x48c8d0,
         'Close7c24c8': 0x485b40, 'Close7cb2cc': 0x4a27c0, 'Windows7cc8bc_SelectSecond': 0x4bce80}
B = Bench(RANGES)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
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
    if op in ('FocusRestore', 'HasSource', 'Close7c24c8', 'Close7cb2cc'):
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
        if op in ('Close7c24c8', 'Close7cb2cc'):   # the window closed first (SendMessage): its handler fncmp's logger or none
            for w in wins:
                put(regions[w], 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
            regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
            regions[0x7cb2cc] = struct.pack('<I', some(2))
            ac, b0, b4 = struct.unpack('<3I', regions[0x7cb2ac])   # (FocusRestore faults on [0x7cb2b0] 0 with [0x7cb2ac]: mostly not)
            if ac and not b0 and r.random() < 0.8:
                regions[0x7cb2ac] = struct.pack('<3I', ac, r.choice(wins), b4)
        return Call(CALLS[op], [], regions, ret='a')
    if op == 'Dialog7ca1c8_Focus':   # (the window [0x7ca1c8])
        regions[0x7ca1c8] = struct.pack('<I', window)
        return Call(CALLS[op], [], regions, ret='w')
    return Call(CALLS[op], [window, r.getrandbits(32)], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
