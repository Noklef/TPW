# The loading screen's close, atexit destructor and image show (src/game/gfx/loadscreen: 0x5879c0, 0x586d70, 0x587130) against the
# originals. Close / StaticDtor: shown or not,
# timeGetTime (a recorder) 3000 ms or more past the time shown (less would wait forever: the recorder's answer does not move); the
# blocks at +0x10 / +0x14 set or not, their handles 0 or set and in the tracked table (as texturemem.py) or not. ShowImage: images of
# 0-24 x 0-24 (pixels null now and then), 2 or 4 bytes a pixel (or 0, 1, 3 / other: 4); driver kind 1 (the surface's Lock a COM recorder
# filling the description: rows / columns -2-20, pitch at or past the row; the rest of the description logged as a hash; failing or
# not), 4 (rows / columns -2-20 into a packed buffer) or another. The progress bar (0x587a70): see draw_bar().
# usage: loadscreen.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Close,StaticDtor,ShowImage,DrawBar)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, blob, echo, fb, fill, global_heap, heap, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x5879c0, 0x587a64), (0x586d70, 0x586e17), (0x579b40, 0x579c13), (0x67acb0, 0x67acb1), (0x587130, 0x587372), (0x587a70, 0x587c77), (0x67a830, 0x67a860)])
CALLS = {'Close': 0x5879c0, 'StaticDtor': 0x586d70, 'ShowImage': 0x587130, 'DrawBar': 0x587a70}
SCREEN, BLOCKS, COUNT, PAIRS, TIME = 0x8c4748, RAM + 0x100, 0x8bb214, 0x87b1f8, 40
IMAGE, DISP, SURF, VT, RENDER, PIX, OUT = RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x1000, RAM + 0x3000


def count(r):
    return r.choice([0, 1, 2, 7, 20, r.randrange(1, 21), r.randrange(1, 21)]) if r.random() > 0.05 else r.choice([-1, -2])


def show_image(r):
    log = log_region()
    w, h = r.choice([0, 1, 24, r.randrange(1, 25)]), r.choice([0, 1, 24, r.randrange(1, 25)])
    bpp = r.choice([2, 2, 4, 4, 3, 1, 0, r.getrandbits(32)])
    size = 2 if bpp == 2 else 4
    image = bytearray(r.bytes(0x10))
    put(image, 0, 'IIII', PIX if r.random() > 0.05 else 0, w, h, bpp)
    kind = r.choice([1, 1, 1, 4, 4, 4, 0, 2, 3])
    rows, cols = count(r), count(r)
    pitch = max(cols, 0) * size + r.choice([0, 0, 2, 4, 6])
    regions = {RAM: r.bytes(0x10), IMAGE: image, LOG: log, PIX: r.bytes(max(w * h * 4, 4)), OUT: r.bytes(max(rows, 1) * max(pitch, 4 * max(cols, 1))),
               0x8bd524: struct.pack('<I', kind), 0x879a24: struct.pack('<I', DISP), DISP: r.bytes(0x14) + struct.pack('<I', SURF),
               SURF: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({}, 33, com={0x19: 5, 0x20: 2}),
               0x879dac: struct.pack('<I', RENDER), RENDER: r.bytes(8) + struct.pack('<I', OUT), 0x8bcbd0: struct.pack('<ii', cols, rows)}
    desc = bytearray(r.bytes(0x28))
    put(desc, 8, 'iiI', rows, cols, pitch)
    put(desc, 0x24, 'I', OUT)
    fill(log, 0x19, 2, desc)
    blob(log, 0x19, 2, 0x7ffffc00)
    if r.random() < 0.85:
        set_return(log, 0x19, 0)
    return Call(0x587130, [IMAGE], regions, conv='t', ret='a')


# screen heights whose second (EDGE2) or third (EDGE3) rectangle's y0 is 0.0 with the earlier ones' not (446H / 480 = -(11H / 480 / 5), or
# -(4 * (11H / 480) / 5), found by search): a renderer echoing y0 refuses that one only
EDGE2 = [0xd181a80a, 0xa3035014, 0x7484f81e, 0x4606a028]
EDGE3 = [0xd18176a8, 0xa302ed50, 0x5cfd12b0, 0x2e7e8958]


def draw_bar(r):
    """0x587a70: the step count (+0) and its scale (+0x18) about 100 % or anything; the screen the usual sizes, small, anything or an EDGE
    height; the renderer's rectangle (slot 0x28) taking (its hash), refusing (0), or answering its y0 (refusing on an EDGE height's)."""
    log = log_region()
    screen = bytearray(r.bytes(0x20))
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
    regions = {RAM + 0x800: screen, 0x8bcbd0: struct.pack('<II', w & 0xffffffff, h & 0xffffffff), LOG: log, 0x879ab0: struct.pack('<I', RENDER),
               RENDER: put(bytearray(r.bytes(0x10)), 0, 'I', VT), VT: vtable({7: 0, 10: 6, 8: 0})}
    return Call(0x587a70, [RAM + 0x800], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'ShowImage':
        return show_image(r)
    if op == 'DrawBar':
        return draw_bar(r)
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
