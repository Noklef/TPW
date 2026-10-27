# A GUI window's rectangles (src/game/ui/window, ui/guirect) against the originals. RectAssign / SaveRect (0x65aa20, 0x65bc90): random
# rectangles (the window's at +8 and +0x2c). GetRect / SetSavedRect (0x65d42e, 0x65cfd4): a GUI rectangle, a recorder or none, the insets
# small or edges. SaveInsetRect (0x485e30): random rectangles and insets. SetHitArea (0x65cfff): the old hit area +0x44 none, the window's +8
# or a shape whose slot 4 is a recorder; the new one a shape or none. SetInner / OffsetInner (0x65bb10, 0x65bb3f): the rectangle +0x138 of a
# kind 4 window (a GUI rectangle, its vtable 0x705920) given random words (their upper halves random); SetInner7059f0 (0x6651ef) the kind 7's.
# SetColumns (0x663554): the kind 7 window's columns none yet (or some), counts of 0 to 8 or negative, the allocation now and then failing
# (only with no columns to make). SetColumn (0x663662): a column array of up to 6 (or none), the index about the count. SetRange705bc8
# (0x66cad3): the kind 11 window's range ends any (near each other mostly), the new ends given or not. SetRange2705bc8 (0x66cb69): the
# same for its second range, its value +0x158 near the ends (or any).
# SetRect20 / SetRect14 / KeepRect / GetRect20 / GetRect14 / GetSavedRect / GetSize14 (0x65cef7, 0x65cf17, 0x65cf3d, 0x65d4a0, 0x65d4cb,
# 0x65d4f6, 0x65d8e7): random rectangles +8 / +0x14 / +0x20 / +0x2c, the other one random or none (not for SetRect20), the out pointers
# each set or 0.
# usage: windowrect.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=RectAssign,SaveRect,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, Bench, Call, heap, log_region, put, vtable  # noqa: E402
from window import MISC, RANGES, VTABLE, WINDOW  # noqa: E402

CALLS = {'GetSize': 0x65d865, 'RectAssign': 0x65aa20, 'SaveRect': 0x65bc90, 'GetRect': 0x65d42e, 'SetSavedRect': 0x65cfd4, 'SaveInsetRect': 0x485e30,
         'SetHitArea': 0x65cfff, 'SetInner': 0x65bb10, 'OffsetInner': 0x65bb3f,
         'SetInner7059f0': 0x6651ef, 'SetColumns': 0x663554, 'SetColumn': 0x663662, 'SetRange705bc8': 0x66cad3,
         'SetRange2705bc8': 0x66cb69, 'SetRect20': 0x65cef7, 'SetRect14': 0x65cf17, 'KeepRect': 0x65cf3d, 'GetRect20': 0x65d4a0,
         'GetRect14': 0x65d4cb, 'GetSavedRect': 0x65d4f6, 'GetSize14': 0x65d8e7}
B = Bench(RANGES + [(0x65d865, 0x65d8e7), (0x659e3e, 0x659e83), (0x65cfff, 0x65d070), (0x65bb10, 0x65bb6b), (0x659dac, 0x659de5), (0x6651ef, 0x66521e),
                       (0x663554, 0x663662), (0x663662, 0x6636b2), (0x66cad3, 0x66cb69), (0x66cb69, 0x66cc4d), (0x67acf0, 0x67ada0), (0x5da3c0, 0x5da3c1),
                       (0x65cef7, 0x65cf5e), (0x65d4a0, 0x65d521), (0x65d8e7, 0x65d90b)])


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if op == 'GetSize':   # the rectangle +8 and the insets small or edges; the two out pointers each set or 0; inner's low word 0 or not
        win = bytearray(r.bytes(0x120))
        for k in list(range(0xc, 0x14, 2)) + list(range(0xf8, 0x100, 2)):
            put(win, k, 'H', r.choice([0, 1, 2, 5, 40, 0x7fff, 0x8000, 0xffff, r.randrange(-50, 50) & 0xffff, r.getrandbits(16)]))
        regions[WINDOW] = win
        regions[MISC] = r.bytes(8)
        inner = r.choice([0, 1, 0x10000, 0xffff0000, r.getrandbits(32)])
        return Call(CALLS[op], [WINDOW, r.choice([MISC, MISC, 0]), r.choice([MISC + 4, MISC + 4, 0]), inner], regions, conv='t', ret='w')
    if op in ('SetRect20', 'SetRect14', 'KeepRect', 'GetRect20', 'GetRect14', 'GetSavedRect', 'GetSize14'):
        regions[WINDOW] = r.bytes(0x40)
        regions[MISC] = r.bytes(0x10)
        if op == 'GetSize14':
            return Call(CALLS[op], [WINDOW, r.choice([MISC, MISC, 0]), r.choice([MISC + 4, MISC + 4, 0])], regions, conv='t', ret='w')
        args = [] if op == 'KeepRect' else [MISC if op == 'SetRect20' else r.choice([MISC] * 6 + [0])]
        return Call(CALLS[op], [WINDOW] + args, regions, conv='t', ret='w')
    if op in ('GetRect', 'SetSavedRect'):   # the rectangle a GUI one, a recorder (slot 3: four) or 0; insets small or edges; inner's low word 0 or not
        win = bytearray(r.bytes(0x120))
        for k in range(0xf8, 0x100, 2):
            put(win, k, 'H', r.choice([0, 1, 2, 5, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
        regions[WINDOW] = win
        rect = bytearray(r.bytes(0x10))
        put(rect, 0, 'I', r.choice([0x705920, 0x705920, VTABLE]))
        regions[MISC] = rect
        regions[VTABLE] = vtable({3: 4})
        inner = r.choice([0, 1, 0x10000, 0xffff0000, r.getrandbits(32)])
        args = [r.choice([MISC] * 6 + [0])] + ([inner] if op == 'GetRect' else [])
        return Call(CALLS[op], [WINDOW] + args, regions, conv='t', ret='w')
    if op == 'SaveInsetRect':   # the window's rectangles (+8 and the saved +0x2c) and insets random (small or edges), the insets dx / dy too
        win = bytearray(r.bytes(0x120))
        for k in list(range(0xc, 0x14, 2)) + list(range(0xf8, 0x100, 2)):
            put(win, k, 'H', r.choice([0, 1, 2, 5, 0x7fff, 0x8000, 0xffff, r.randrange(-50, 50) & 0xffff, r.getrandbits(16)]))
        regions[WINDOW] = win
        return Call(CALLS[op], [WINDOW] + [r.choice([0, 1, 3, -2, 0x7fff, -0x8000, 0x12345, r.getrandbits(32)]) & 0xffffffff for _ in range(2)], regions, ret='v')
    if op == 'SetHitArea':
        win = bytearray(r.bytes(0x60))
        put(win, 0x44, 'I', r.choice([0, WINDOW + 8, MISC, MISC]))
        regions[WINDOW] = win
        regions[MISC] = struct.pack('<I', VTABLE) + r.bytes(0xc)
        regions[VTABLE] = vtable({4: 1})
        return Call(CALLS[op], [WINDOW, r.choice([0, MISC + 0x40, MISC + 0x40])], regions, conv='t', ret='a')
    if op == 'SetColumns':
        win = bytearray(r.bytes(0x180))
        put(win, 0x164, 'H', 0 if r.random() < 0.8 else r.choice([1, 3, 0xffff]))
        regions[WINDOW] = win
        count = r.choice([0, 1, 2, 3, 4, 8, 0xffff, 0x8000])
        regions[HEAP] = heap(0x1000, 1 if count in (0, 0xffff, 0x8000) and r.random() < 0.3 else 0)
        return Call(CALLS[op], [WINDOW, count | r.getrandbits(16) << 16], regions, conv='t')
    if op == 'SetColumn':
        win = bytearray(r.bytes(0x180))
        n = r.choice([0, 1, 2, 6, 0xffff])
        put(win, 0x164, 'H', n)
        put(win, 0x16c, 'I', MISC if r.random() < 0.85 else 0)
        regions[WINDOW] = win
        regions[MISC] = r.bytes(0x60)
        index = r.choice([0, 1, 2, 5, n, (n - 1) & 0xffff, 0xffff, 0x8000, r.getrandbits(16)])   # (below the count: in the array, or before it)
        words = [index] + [r.getrandbits(32) for _ in range(2)]
        return Call(CALLS[op], [WINDOW] + words, regions, conv='t', ret='a')
    if op in ('SetRange705bc8', 'SetRange2705bc8'):
        win = bytearray(r.bytes(0x160))
        low = r.choice([0, -5, 100, 0x7fffffff, -0x80000000, r.randrange(-50, 50)])
        at = 0x138 if op == 'SetRange705bc8' else 0x14c
        put(win, at, 'II', low & 0xffffffff, (low + r.choice([0, 1, -1, 30, -30]) if r.random() < 0.7 else r.getrandbits(32)) & 0xffffffff)
        put(win, 0x158, 'I', (r.choice([low, low - 1, low + 1, r.randrange(-60, 60), 0x7fffffff, -0x80000000]) if r.random() < 0.85
                              else r.getrandbits(32)) & 0xffffffff)
        regions[WINDOW] = win
        regions[MISC] = struct.pack('<ii', r.randrange(-50, 50), r.choice([r.randrange(-50, 50), 0x7fffffff, -0x80000000]))
        return Call(CALLS[op], [WINDOW, r.choice([0, MISC]), r.choice([0, MISC + 4])], regions, conv='t', ret='a')
    if op in ('SetInner', 'OffsetInner', 'SetInner7059f0'):
        win = bytearray(r.bytes(0x160))
        put(win, 0x138, 'I', 0x705920)
        regions[WINDOW] = win
        words = [r.choice([0, 1, 5, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]) | r.getrandbits(16) << 16 for _ in range(2 if op == 'OffsetInner' else 4)]
        return Call(CALLS[op], [WINDOW] + words, regions, conv='t', ret='v' if op == 'OffsetInner' else 'a')
    regions[MISC] = r.bytes(0x10)   # RectAssign / SaveRect: random rectangles (the window's at +8 and +0x2c)
    regions[MISC + 0x40] = r.bytes(0x10)
    regions[WINDOW] = r.bytes(0x40)
    if op == 'SaveRect':
        return Call(CALLS[op], [WINDOW], regions, conv='t', ret='a')
    return Call(CALLS[op], [MISC, MISC + 0x40], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
