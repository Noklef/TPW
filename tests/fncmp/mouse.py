# The mouse handlers of src/game/input/mouse (0x46c210, 0x46c2a0) against the originals: the mouse object at [0x7b05d8] is a recorder (the log shows which slot is called with which
# arguments), the last position and the button flags are random. GuiButton (0x486c70, with ui/window's 0x658af1 / 0x658b5b and the GUI's
# message queue): see gui(). GuiMove (0x6588ef): window.py's tree and hover (the capture [0xfaa5e8] mostly 0), the GUI up or not
# ([0x77c488]), the point a point() (in the windows mostly) or any. GuiWheel (0x658bcb): gui()'s, the capture [0xfaa5e8] the window or 0,
# the delta small or any, x and y as gui()'s.
# usage: mouse.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Button,Moved,GuiButton,GuiMove,GuiWheel)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, LOG, RAM, log_region, put, vtable  # noqa: E402
from window import RANGES as WINDOW_RANGES, hover, point, tree  # noqa: E402

B = Bench([(0x46c210, 0x46c2a0), (0x46c2a0, 0x46c330), (0x5da3c0, 0x5da3c1), (0x486c70, 0x486d8f), (0x658af1, 0x658b5b), (0x658b5b, 0x658bcb),
           (0x658ccc, 0x658d9f), (0x6697e3, 0x6698a4), (0x659710, 0x659721),
           (0x658bcb, 0x658c38)] +
          sorted(set(WINDOW_RANGES + [(0x6588ef, 0x6589f9)])))
MOUSE, VT = RAM + 0x2000, RAM + 0x2100
WINDOW, QUEUE = RAM + 0x3000, RAM + 0x3200


def gui(r):
    """0x486c70: the GUI up, the hovered window and the buttons' windows the window or none, a queue of 1-4 entries (as window.py's) or none, the
    scale words around 640 x 480 (negative too), the three held flags; x and y small or anything"""
    win = bytearray(r.bytes(0x120))
    win[4] = r.choice([0, 0, 1])
    cap = r.choice([1, 2, 4])
    entries = bytearray(r.bytes(16 * cap))
    for i in range(cap):
        if r.random() < 0.6:
            put(entries, 16 * i, 'I', 0)
    regions = {LOG: log_region(), WINDOW: win, QUEUE + 0x40: entries,
               QUEUE: struct.pack('<5I', QUEUE + 0x40, cap, 0, r.randrange(cap), r.getrandbits(8)),
               0xfaa5f0: struct.pack('<I', r.choice([QUEUE] * 5 + [0])), 0x77c488: struct.pack('<I', r.choice([1, 1, 1, 0])),
               0xfaa588: struct.pack('<3I', *[r.choice([WINDOW, WINDOW, 0]) for _ in range(3)]), 0xfaa5bc: r.bytes(2),
               0xfaa5c0: struct.pack('<h', r.choice([480, 600, 768, 1, -480, 0x7fff])), 0xfaa5c4: struct.pack('<h', r.choice([640, 800, 1024, 3, -640])),
               0xfaa5e0: struct.pack('<I', r.choice([WINDOW, WINDOW, 0])),
               0x7c24dc: struct.pack('<3I', *[r.choice([0, 0, 1, r.getrandbits(32)]) for _ in range(3)])}
    event = r.choice([0xf0, 0xf1, 0xf2, 0xf0, 0xf1, 0xf2, 0xef, 0xf3, 0x1f0])
    xy = [r.choice([0, 1, 320, 639, -5, r.randrange(-0x10000, 0x10000), r.getrandbits(32)]) & 0xffffffff for _ in range(2)]
    return Call(0x486c70, [event, r.choice([1, 0, 0, 2, 0x100]), r.getrandbits(32)] + xy, regions, ret='a')


def case(r):
    name = r.choice(OPS)
    if name == 'GuiButton':
        return gui(r)
    if name == 'GuiWheel':
        c = gui(r)
        c.regions[0xfaa5e8] = struct.pack('<I', r.choice([WINDOW, 0, 0]))
        x, y = c.args[3:5]
        delta = r.choice([1, -1, 120, -120, 0, r.getrandbits(32)]) & 0xffffffff
        return Call(0x658bcb, [delta, x, y], c.regions, ret='w')
    if name == 'GuiMove':
        regions = {LOG: log_region()}
        wins = tree(r, regions)
        hover(r, regions, wins)
        regions[0xfaa5d8] = put(bytearray(regions[0xfaa5d8]), 0x10, 'I', r.choice([0] * 6 + wins))   # (the capture: a window or none)
        regions[0x77c488] = struct.pack('<I', r.choice([1, 1, 1, 1, 0]))
        x, y = point(r, regions, wins)
        return Call(0x6588ef, [x, y], regions, ret='w')
    glob = bytearray(struct.pack('<IIIIIIII', *[r.choice([0, 1, 5, 0x80000000]) for _ in range(8)]))  # 0x7b05d4 .. 0x7b05f4: the last position, the mouse object, the flags
    put(glob, 4, 'I', MOUSE)
    put(glob, 0x10, 'I', r.choice([0, 1, 5]))
    put(glob, 0x1c, 'I', r.choice([0, 1, 5]))
    regions = {MOUSE: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({1: 2, 2: 2, 3: 2, 4: 2, 6: 4, 7: 2, 8: 2}), LOG: log_region(), 0x7b05d4: glob}
    if name == 'Button':
        return Call(0x46c210, [r.choice([0xf0, 0xf1, 0xf0, 0xf1, 0, 0xef]), r.choice([1, 0, 2, 1]), r.getrandbits(32), r.getrandbits(32)], regions, conv='c', ret='a')
    return Call(0x46c2a0, [r.choice([0, 1, 5, 0x80000000, 0xffffffff]), r.choice([0, 1, 5, 7])], regions, conv='s', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Button,Moved,GuiButton,GuiMove,GuiWheel').split(',') if k]

if __name__ == '__main__':
    B.main(case)
