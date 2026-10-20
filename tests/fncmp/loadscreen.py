# The loading screen's close, atexit destructor and image show (src/game/gfx/loadscreen: 0x5879c0, 0x586d70, 0x587130) against the
# originals. Close / StaticDtor: shown or not,
# timeGetTime (a recorder) 3000 ms or more past the time shown (less would wait forever: the recorder's answer does not move); the
# blocks at +0x10 / +0x14 set or not, their handles 0 or set and in the tracked table (as texturemem.py) or not. ShowImage: images of
# 0-24 x 0-24 (pixels null now and then), 2 or 4 bytes a pixel (or 0, 1, 3 / other: 4); driver kind 1 (the surface's Lock a COM recorder
# filling the description: rows / columns -2-20, pitch at or past the row; the rest of the description logged as a hash; failing or
# not), 4 (rows / columns -2-20 into a packed buffer) or another. The progress bar (0x587a70): see bar(). A loading step (0x587c80): see tick().
# usage: loadscreen.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Close,StaticDtor,ShowImage,DrawBar,Tick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, blob, echo, fb, fill, global_heap, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import engineerrors  # noqa: E402

B = Bench([(0x5879c0, 0x587a64), (0x586d70, 0x586e17), (0x579b40, 0x579c13), (0x67acb0, 0x67acb1), (0x587130, 0x587372), (0x587a70, 0x587c77), (0x67a830, 0x67a860),
           (0x587c80, 0x587d67), (0x678d30, 0x678d66), (0x583720, 0x58372a), (0x583730, 0x583786), (0x56afc0, 0x56b02a)])
CALLS = {'Close': 0x5879c0, 'StaticDtor': 0x586d70, 'ShowImage': 0x587130, 'DrawBar': 0x587a70, 'Tick': 0x587c80}
SCREEN, BLOCKS, COUNT, PAIRS, TIME = 0x8c4748, RAM + 0x100, 0x8bb214, 0x87b1f8, 40
IMAGE, DISP, SURF, VT, RENDER, PIX, OUT = RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x1000, RAM + 0x3000


def count(r):
    return r.choice([0, 1, 2, 7, 20, r.randrange(1, 21), r.randrange(1, 21)]) if r.random() > 0.05 else r.choice([-1, -2])


def picture(r):
    """An image of 0-24 x 0-24 (its pixels at PIX, null now and then), 2 or 4 bytes a pixel (or 0, 1, 3 / other: 4)."""
    image = bytearray(r.bytes(0x10))
    put(image, 0, 'IIII', PIX if r.random() > 0.05 else 0, r.choice([0, 1, 24, r.randrange(1, 25)]), r.choice([0, 1, 24, r.randrange(1, 25)]),
        r.choice([2, 2, 4, 4, 3, 1, 0, r.getrandbits(32)]))
    return image


def display(r, log, regions, kind, rows, cols, size=4, buffer=0):
    """The regions ShowImage draws through: driver kind `kind`; the surface's Lock (slot 0x19) filling the description (rows, cols, a
    pitch at or past `size` bytes a column) or failing; the renderer's packed buffer OUT (+8) and fill buffer `buffer` (+4); the pixels."""
    pitch = max(cols, 0) * size + r.choice([0, 0, 2, 4, 6])
    regions.update({PIX: r.bytes(24 * 24 * 4), OUT: r.bytes(max(rows, 1) * max(pitch, 4 * max(cols, 1))),
                    0x8bd524: struct.pack('<I', kind), 0x879a24: struct.pack('<I', DISP), DISP: r.bytes(0x14) + struct.pack('<I', SURF),
                    SURF: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({}, 33, com={0x19: 5, 0x20: 2}),
                    0x879dac: struct.pack('<I', RENDER), RENDER: r.bytes(4) + struct.pack('<II', buffer, OUT), 0x8bcbd0: struct.pack('<ii', cols, rows)})
    desc = bytearray(r.bytes(0x28))
    put(desc, 8, 'iiI', rows, cols, pitch)
    put(desc, 0x24, 'I', OUT)
    fill(log, 0x19, 2, desc)
    blob(log, 0x19, 2, 0x7ffffc00)
    if r.random() < 0.85:
        set_return(log, 0x19, 0)


def show_image(r):
    log = log_region()
    image = picture(r)
    regions = {RAM: r.bytes(0x10), IMAGE: image, LOG: log}
    display(r, log, regions, r.choice([1, 1, 1, 4, 4, 4, 0, 2, 3]), count(r), count(r), 2 if struct.unpack_from('<I', image, 0xc)[0] == 2 else 4)
    return Call(0x587130, [IMAGE], regions, conv='t', ret='a')


# screen heights whose second (EDGE2) or third (EDGE3) rectangle's y0 is 0.0 with the earlier ones' not (446H / 480 = -(11H / 480 / 5), or
# -(4 * (11H / 480) / 5), found by search): a renderer echoing y0 refuses that one only
EDGE2 = [0xd181a80a, 0xa3035014, 0x7484f81e, 0x4606a028]
EDGE3 = [0xd18176a8, 0xa302ed50, 0x5cfd12b0, 0x2e7e8958]


def bar(r, log, regions, screen, renderer, vt, sized=True):
    """The progress bar's inputs: the step count (+0) and its scale (+0x18) about 100 % or anything; the screen (0x8bcbd0, unless not
    `sized`) the usual sizes, small, anything or an EDGE height; the renderer `renderer` (its vtable at `vt`), its rectangle (slot 0x28)
    taking (its hash), refusing (0), or answering its y0 (refusing on an EDGE height's)."""
    put(screen, 0, 'i', r.choice([0, 1, 50, 99, 100, 101, 200, r.randrange(-50, 300), r.getrandbits(32) - (1 << 31)]))
    put(screen, 0x18, 'I', r.choice([fb(1.0), fb(0.5), fb(1.01), fb(0.99), r.f(), r.pos(3)]))
    w, h = r.choice([(640, 480), (800, 600), (1024, 768), (r.randrange(-5, 2000), r.randrange(-5, 2000)), (r.getrandbits(32), r.getrandbits(32))])
    answer = r.choice(['hash', 'hash', 'zero', 'echo'])
    if answer == 'echo' and r.random() < 0.7:
        h = r.choice(EDGE2 + EDGE3)
    if answer == 'zero':
        set_return(log, 10, 0)
    elif answer == 'echo':
        echo(log, 10, 1)
    if sized:
        regions[0x8bcbd0] = struct.pack('<II', w & 0xffffffff, h & 0xffffffff)
    regions.update({0x879ab0: struct.pack('<I', renderer), renderer: put(bytearray(r.bytes(0x10)), 0, 'I', vt), vt: vtable({7: 0, 10: 6, 8: 0})})


def draw_bar(r):
    """0x587a70: see bar()."""
    log = log_region()
    screen = bytearray(r.bytes(0x20))
    regions = {RAM + 0x800: screen, LOG: log}
    bar(r, log, regions, screen, RENDER, VT)
    return Call(0x587a70, [RAM + 0x800], regions, conv='t', ret='a')


FILL, DRAWER, DRAWVT, DEVICE, DEVVT, TICKED = RAM + 0x4000, RAM + 0x700, RAM + 0x780, RAM + 0x900, RAM + 0x980, RAM + 0x800
TICK_TIME, FOCUS = 40, 0


def tick(r):
    """0x587c80: the screen's step count (+0) 0 half the time (else the bar's); its images (+0x10, +0x14) set or not (as picture()); its
    image count (+0x2c) 0-4 or any; the count of shows (0x8c478c) 0, 1, 2 or any; driver kind 1, 4 (the screen then 1-20 rows and -2-20
    columns, the fill buffer set or not) or another (the screen the bar's); timeGetTime a recorder; the bar as bar(); the restore as
    renderstate.py's (the focus the window or not, the device's slot 1 any result, engineerrors.py's lost_list()); the device's slot 0
    (RenderDevice_Call0) any result."""
    log = log_region()
    screen = bytearray(r.bytes(0x30))
    regions = {TICKED: screen, LOG: log, IMAGE: picture(r), IMAGE + 0x20: picture(r)}
    kind = r.choice([1, 1, 4, 4, 4, 0, 2])
    rows, cols = (r.randrange(1, 21), r.choice([r.randrange(1, 21), 1, 8, 16, r.randrange(-2, 1)])) if kind == 4 else (count(r), count(r))
    buffer = r.choice([FILL, FILL, 0]) if cols > 0 else 0   # (a fill of no columns steps its rows backwards, below the buffer)
    display(r, log, regions, kind, rows, cols, buffer=buffer)
    bar(r, log, regions, screen, DRAWER, DRAWVT, sized=kind != 4)
    if r.random() < 0.5:
        put(screen, 0, 'I', 0)
    put(screen, 0x10, 'II', r.choice([IMAGE, 0]), r.choice([IMAGE + 0x20, 0]))
    put(screen, 0x2c, 'i', r.choice([0, 1, 2, 3, 4, r.getrandbits(31) - (1 << 30)]))
    regions[FILL] = r.bytes(20 * 48 + 0x40)
    regions[0x77e580] = r.bytes(8)
    regions[0x8c4780] = put(bytearray(r.bytes(0x10)), 0xc, 'I', r.choice([0, 1, 1, 2, r.getrandbits(32)]))
    regions[0x6fd3ec] = struct.pack('<I', recorder(TICK_TIME, 0, True))
    set_return(log, TICK_TIME, r.getrandbits(32))
    window = r.getrandbits(32)
    set_return(log, FOCUS, r.choice([window, window, r.getrandbits(32)]))
    set_return(log, 1, r.choice([0, 0, 1, r.getrandbits(32)]))
    set_return(log, 2, r.getrandbits(32))
    flags = bytearray(r.bytes(0x28))   # (0x8bd508 the flags, 0x8bd518 the window, 0x8bd524 the driver kind, 0x8bd52c the device)
    put(flags, 0x10, 'I', window)
    flags[0x1c:0x20] = regions.pop(0x8bd524)
    put(flags, 0x24, 'I', DEVICE)
    regions.update({0x8bd508: flags, DEVICE: struct.pack('<I', DEVVT) + r.bytes(0xc), DEVVT: struct.pack('<2I', recorder(2, 0), recorder(1, 0)),
                    0x6fd328: struct.pack('<I', recorder(FOCUS, 0, True)), 0x879ad0: engineerrors.lost_list(r), 0x8bcc40: r.bytes(8)})
    return Call(0x587c80, [TICKED], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'ShowImage':
        return show_image(r)
    if op == 'DrawBar':
        return draw_bar(r)
    if op == 'Tick':
        return tick(r)
    log = log_region()
    regions = {HEAP: heap(), **global_heap(), LOG: log, 0x6fd3ec: struct.pack('<I', recorder(TIME, 0, True)), 0x8bcbc8: r.bytes(4)}
    now = r.getrandbits(32)
    set_return(log, TIME, now)
    globs = bytearray(r.bytes(0x48))   # (the static screen, the time shown at +0x38, the count of shows at +0x44)
    put(globs, 0x44, 'I', r.choice([0, 0, 1, 2, r.getrandbits(32)]))
    put(globs, 0x38, 'I', (now - r.choice([3000, 3001, 5000, r.randrange(3000, 1 << 32)])) & 0xffffffff)
    screen = bytearray(r.bytes(0x18))
    blocks = bytearray(r.bytes(0x20))
    handles = []
    for k in range(2):
        h = r.choice([0, r.getrandbits(32)])
        put(blocks, 0x10 * k, 'I', h)
        handles.append(h)
        put(screen, 0x10 + 4 * k, 'I', BLOCKS + 0x10 * k if r.random() < 0.8 else 0)
    table = bytearray(8 * 8)
    for k in range(8):
        b = r.choice(handles + [r.getrandbits(32)])
        put(table, 8 * k, 'II', r.getrandbits(32), r.choice([b, b, (b + 4) & 0xffffffff]))
    regions.update({PAIRS: table, COUNT: struct.pack('<I', r.choice([0, 1, 2, 4, 8])), BLOCKS: blocks})
    if op == 'StaticDtor':
        globs[:0x18] = screen
        regions[SCREEN] = globs
        return Call(0x586d70, [], regions)
    regions[SCREEN] = globs
    regions[RAM] = screen
    return Call(0x5879c0, [RAM], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
