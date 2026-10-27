# The help background (src/game/frontend/helpbg: 0x4922d0, 0x491fe0) against the original. Clear: any object bytes. Handler: the window's
# default handler +0x118 fncmp's logger (its result now and then set), its object A0[0] a recorder (slot 15), the message 0x14 often.
# SetInterval / FrameSetInterval (0x492160, src/game/ui/frame's 0x66a084): the words +4 / +0xc set or not, slot 0 and the clock recorders. (Its HelpBg_Refresh, 0x492320, is
# benched with the virtual call forwarders in vcall_forwards.py.)
# Close / Show / Hide / FrameStop (0x492130, 0x4922e0, 0x492300, ui/frame's 0x66a016): window.py's tree for Window_SetShown, the words set or not.
# MoveTo (0x492330): window.py's tree for Window_Offset (offset_env), the window +0x18 one of it or none, x and y words 0xffff or not.
# SetText (0x4924b0): the background at 0x7ca0c0 (its words +4 / +0xc set or not, slot 1 a recorder), its window one of window.py's notify_env
# tree (the window's slot 9 a recorder), a wide text of 0-20 characters, '\n' among them now and then.
# RestoreRect (0x4923d0): windowtree.py's rect_env() tree, the window +0x18 one of it or none, its rectangle +0x14's words the same as
# +8's, moved, sized or any (Window_SetRect's cases).
# Place (0x492530, cdecl (x, y)): RestoreRect's tree with both low words 0xffff a third of the time, else MoveTo's; the object at 0x7ca0c0.
# usage: helpbg.py [CASES [SEED]] [--wild|--both] [--show N]
#   (env OPS=Clear,Handler,SetInterval,FrameSetInterval,Close,Show,Hide,FrameStop,MoveTo,SetText,RestoreRect)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, Bench, Call, RAM, log_region, put, set_return, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from window import RANGES as WINDOW_RANGES, VTABLE as WINDOW_VTABLE, notify_blobs, notify_env, offset_env, shown_env, word  # noqa: E402

CALLS = {'Clear': 0x4922d0, 'Handler': 0x491fe0, 'SetInterval': 0x492160, 'FrameSetInterval': 0x66a084, 'Close': 0x492130, 'Show': 0x4922e0,
         'Hide': 0x492300, 'FrameStop': 0x66a016, 'MoveTo': 0x492330, 'SetText': 0x4924b0, 'RestoreRect': 0x4923d0, 'Place': 0x492530}
PLACED = 0x7ca0c0   # (Place's object)
B = Bench([(0x4922d0, 0x4922e0), (0x491fe0, 0x492014), (0x65e705, 0x65e73d), (0x65f66e, 0x65f697), (0x492160, 0x49217c), (0x66a084, 0x66a0b9),
           (0x65968e, 0x6596a2), (0x492130, 0x492151), (0x4922e0, 0x492320), (0x66a016, 0x66a040), (0x669f94, 0x669fa8),
           (0x66a002, 0x66a016), (0x492330, 0x4923c5), (0x4924b0, 0x492522), (0x67c2f0, 0x67c330), (0x67acd0, 0x67ad00),
           (0x4923d0, 0x49243e), (0x65c9bd, 0x65ce10), (0x659e3e, 0x659e93), (0x659d00, 0x659d53), (0x659d90, 0x659df5), (0x65a9d0, 0x65aa00),
           (0x65aa20, 0x65aaa0), (0x65d4cb, 0x65d4f6), (0x66bff7, 0x66c01b),
           (0x661700, 0x66176a), (0x661770, 0x661800), (0x492530, 0x49255b)] + WINDOW_RANGES)
BG_VTABLE, TEXT = 0x33400000, 0x33500000
WINDOW, OBJECT, VTABLE = RAM + 0x100, RAM + 0x300, RAM + 0x400
FRAME = 0x30008000


def case(r):
    op = r.deal(OPS)
    if op == 'SetText':
        log = log_region()
        regions = {LOG: log}
        wins = notify_env(r, regions, log)
        notify_blobs(log, 0, 0x30)   # (NotifyC00's bytes +4 / +5)
        regions[WINDOW_VTABLE] = vtable({5: 1, 3: 2, 0: 1, 9: 1})
        bg = bytearray(r.bytes(0x20))
        put(bg, 0, 'I', BG_VTABLE)
        for off in (4, 0xc):
            put(bg, off, 'H', r.choice([0, 1, 1, 1, 0x100, 0x8000, r.getrandbits(16)]))
        window = r.choice(wins)
        put(regions[window], 0, 'I', WINDOW_VTABLE)   # (the tree's windows' own vtable pointers are random)
        put(bg, 0x18, 'I', window)
        regions[0x7ca0c0] = bg
        regions[BG_VTABLE] = vtable({}, size=8)
        n = r.choice([0, 0, 1, 2, 5, 20])
        chars = [0x0a if r.random() < 0.12 else r.choice([0x41, 0x10a, 0xa00, r.getrandbits(16) | 1]) for _ in range(n)]
        regions[TEXT] = struct.pack('<%dH' % (n + 1), *chars, 0)
        return Call(CALLS[op], [TEXT], regions, ret='v')
    if op == 'Place':   # (0x492530: both low words 0xffff a third of the time, RestoreRect's tree; else MoveTo's; the object at 0x7ca0c0)
        log = log_region()
        regions = {LOG: log}
        if r.random() < 0.35:
            from windowtree import rect_env
            wins = rect_env(r, regions, log)
            x, y = (0xffff | r.choice([0, 0xffff, r.getrandbits(16)]) << 16 for _ in range(2))
        else:
            wins = offset_env(r, regions, log)
            x, y = (r.choice([word(r), word(r), 0xffff | r.getrandbits(16) << 16]) for _ in range(2))
            if x & 0xffff == 0xffff and y & 0xffff == 0xffff:   # (MoveTo's case: offset_env's hashes are not RestoreRect's)
                y ^= r.choice([1, 0x8000, 0xffff])
        window = r.choice(wins + [0])
        regions[PLACED] = put(bytearray(r.bytes(0x20)), 0x18, 'I', window)
        return Call(CALLS[op], [x, y], regions, ret='v' if window else 'a')
    if op == 'RestoreRect':
        from windowtree import rect_env
        log = log_region()
        regions = {LOG: log}
        wins = rect_env(r, regions, log)
        window = r.choice(wins[:1] * 3 + wins + [0])
        if window:
            left, top, right, bottom = struct.unpack_from('<hhhh', regions[window], 0xc)
            how = r.choice(['same', 'moved', 'moved', 'sized', 'sized', 'any'])
            if how == 'moved':
                dx, dy = r.choice([0, 3, -7]), r.choice([0, 2, -5])
                left, top, right, bottom = left + dx, top + dy, right + dx, bottom + dy
            elif how == 'sized':
                left, top, right, bottom = [v + r.choice([0, 0, 1, -2, 6]) for v in (left, top, right, bottom)]
            elif how == 'any':
                left, top, right, bottom = [r.randrange(-0x8000, 0x8000) for _ in range(4)]
            put(regions[window], 0x18, 'hhhh', left, top, right, bottom)
        obj = bytearray(r.bytes(0x20))
        put(obj, 0x18, 'I', window)
        regions[FRAME] = obj
        # (with the window EAX is the rectangle's destructor's: a stack address, which differs between the sides)
        return Call(CALLS[op], [FRAME], regions, conv='t', ret='v' if window else 'a')
    if op == 'MoveTo':
        log = log_region()
        regions = {LOG: log}
        wins = offset_env(r, regions, log)
        obj = bytearray(r.bytes(0x20))
        window = r.choice(wins + [0])
        put(obj, 0x18, 'I', window)
        regions[FRAME] = obj
        x, y = (r.choice([word(r), word(r), 0xffff | r.getrandbits(16) << 16]) for _ in range(2))
        # (with the window EAX is the rectangle's destructor's: a stack address, which differs between the sides)
        return Call(CALLS[op], [FRAME, x, y], regions, conv='t', ret='v' if window else 'a')
    if op in ('Close', 'Show', 'Hide', 'FrameStop'):   # window.py's tree for Window_SetShown; the words +4 / +6 / +0xc set or not, the window +0x18 one of
        log = log_region()  # the tree's (or none for Close), the object's slot 1 a recorder
        regions = {LOG: log}
        wins = shown_env(r, regions, log)
        obj = bytearray(r.bytes(0x20))
        put(obj, 0, 'I', FRAME + 0x40)
        for off in (4, 6, 0xc):
            put(obj, off, 'H', r.choice([0, 0, 1, 1, 0x100, 0x8000, r.getrandbits(16)]))
        put(obj, 0x18, 'I', r.choice(wins + ([0] if op == 'Close' else [])))
        regions[FRAME] = obj
        regions[FRAME + 0x40] = vtable({}, size=8)
        if r.random() < 0.5:
            set_return(log, 1, r.getrandbits(32))
        return Call(CALLS[op], [FRAME], regions, conv='t', ret='a' if op == 'FrameStop' else 'v')
    if op in ('SetInterval', 'FrameSetInterval'):   # the words +4 / +0xc 0 or not (upper bytes too), slot 0 and the clock [0x77c490] recorders
        log = log_region()
        obj = bytearray(r.bytes(0x20))
        put(obj, 0, 'I', VTABLE)
        put(obj, 4, 'H', r.choice([0, 1, 0x100, 0x8000, r.getrandbits(16)]))
        put(obj, 0xc, 'H', r.choice([0, 0, 1, 0x100, 0x8000, r.getrandbits(16)]))
        if r.random() < 0.7:
            set_return(log, 0, r.getrandbits(32))
        regions = {LOG: log, OBJECT: obj, VTABLE: vtable({}), 0x77c490: struct.pack('<I', OBJECT + 0x40), OBJECT + 0x40: struct.pack('<I', VTABLE)}
        return Call(CALLS[op], [OBJECT, r.getrandbits(32)], regions, conv='t', ret='v')
    if op == 'Handler':
        log = log_region()
        win = bytearray(r.bytes(0x120))
        put(win, 0x118, 'I', CDECL_LOG4)
        put(win, 0xa0, 'I', OBJECT)
        if r.random() < 0.5:
            set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
        if r.random() < 0.5:
            set_return(log, 15, r.getrandbits(32))
        regions = {LOG: log, WINDOW: win, OBJECT: struct.pack('<I', VTABLE) + r.bytes(0x1c), VTABLE: vtable({})}
        args = [r.choice([0x14, 0x14, 0x15, 0, 4, r.getrandbits(32)])] + [r.choice([0, 1, 4, r.getrandbits(32)]) for _ in range(2)]
        return Call(CALLS[op], [WINDOW] + args, regions, ret='a')
    return Call(CALLS[op], [RAM + 0x20], {RAM: r.bytes(0x60)}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
