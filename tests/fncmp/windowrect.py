# A GUI window's rectangles (src/game/ui/window, ui/guirect) against the originals. RectAssign / SaveRect (0x65aa20, 0x65bc90): random
# rectangles (the window's at +8 and +0x2c). GetRect / SetSavedRect (0x65d42e, 0x65cfd4): a GUI rectangle, a recorder or none, the insets
# small or edges. SaveInsetRect (0x485e30): random rectangles and insets. SetHitArea (0x65cfff): the old hit area +0x44 none, the window's +8
# or a shape whose slot 4 is a recorder; the new one a shape or none. SetInner / OffsetInner (0x65bb10, 0x65bb3f): the rectangle +0x138 of a
# kind 4 window (a GUI rectangle, its vtable 0x705920) given random words (their upper halves random).
# usage: windowrect.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=RectAssign,SaveRect,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, log_region, put, vtable  # noqa: E402
from window import MISC, RANGES, VTABLE, WINDOW  # noqa: E402

CALLS = {'RectAssign': 0x65aa20, 'SaveRect': 0x65bc90, 'GetRect': 0x65d42e, 'SetSavedRect': 0x65cfd4, 'SaveInsetRect': 0x485e30,
         'SetHitArea': 0x65cfff, 'SetInner': 0x65bb10, 'OffsetInner': 0x65bb3f}
B = Bench(RANGES + [(0x65cfff, 0x65d070), (0x65bb10, 0x65bb6b), (0x659dac, 0x659de5)])


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
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
    if op in ('SetInner', 'OffsetInner'):
        win = bytearray(r.bytes(0x160))
        put(win, 0x138, 'I', 0x705920)
        regions[WINDOW] = win
        words = [r.choice([0, 1, 5, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]) | r.getrandbits(16) << 16 for _ in range(4 if op == 'SetInner' else 2)]
        return Call(CALLS[op], [WINDOW] + words, regions, conv='t', ret='a' if op == 'SetInner' else 'v')
    regions[MISC] = r.bytes(0x10)   # RectAssign / SaveRect: random rectangles (the window's at +8 and +0x2c)
    regions[MISC + 0x40] = r.bytes(0x10)
    regions[WINDOW] = r.bytes(0x40)
    if op == 'SaveRect':
        return Call(CALLS[op], [WINDOW], regions, conv='t', ret='a')
    return Call(CALLS[op], [MISC, MISC + 0x40], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
