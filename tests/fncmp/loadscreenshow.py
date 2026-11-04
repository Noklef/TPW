# The loading screen shown (src/game/gfx/loadscreen: 0x587380 LoadScreen_Show) against the original: the image load, the stamp, the image show and the restore recorders (driver.cpp's kLoadScreenCalls), the image filled in (widths 400, 512, 640, 800,
# 1024 and others, a handle or none), the load answering 0 or not; the first show or later for either count; the screen's old blocks held or not (their handles in the tracked table or not, as loadscreen.py); the name and folder none or a short text, the folder with its backslash or empty; the display's width
# the five named or another; ms 0, small, large or negative; timeGetTime moving on 1000 at each call, so the waits end.
# usage: loadscreenshow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, fill_this, global_heap, heap, log_region, mask_this, put, recorder, strings  # noqa: E402
from texdirs import merged  # noqa: E402

LOAD, STAMP, SHOW, RESTORE, TIME = 1, 2, 3, 4, 40
B = Bench(merged([(0x587380, 0x5879bd), (0x587110, 0x58712f), (0x579b40, 0x579c13), (0x67a400, 0x690000)]),
          hooks={0x586e40: _recorder(LOAD, 1), 0x586f40: _recorder(STAMP, 3), 0x587130: _recorder(SHOW, 0), 0x583730: _recorder(RESTORE, 0, cdecl=True)})
SCREEN, BLOCKS, NAME, FOLDER, TIMES, ANSWERS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x400
COUNT, PAIRS = 0x8bb214, 0x87b1f8


def case(r):
    log = log_region()
    screen = bytearray(r.bytes(0x20))
    blocks = bytearray(r.bytes(0x20))
    handles = []
    for k in range(2):
        h = r.choice([0, r.getrandbits(32)])
        put(blocks, 0x10 * k, 'I', h)
        handles.append(h)
        put(screen, 0x10 + 4 * k, 'I', BLOCKS + 0x10 * k if r.random() < 0.3 else 0)
    pixels = r.choice([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1])
    table = bytearray(8 * 8)
    for k in range(8):
        b = r.choice(handles + [pixels, r.getrandbits(32)])
        put(table, 8 * k, 'II', r.getrandbits(32), r.choice([b, b, (b + 4) & 0xffffffff]))
    width = r.choice([400, 512, 640, 800, 1024, 1, 0x7fffffff])
    fill_this(log, LOAD, struct.pack('<4I', pixels, width, r.getrandbits(32), r.getrandbits(32)))
    answers(log, LOAD, ANSWERS)
    mask_this(log, LOAD)
    mask_this(log, SHOW)
    strings(log, LOAD, 1)
    strings(log, STAMP, 1)
    now = r.getrandbits(32)
    answers(log, TIME, TIMES)
    counts = bytearray(r.bytes(0x10))
    put(counts, 0, 'I', (now - r.choice([3000, 2000, 5000, r.randrange(3000, 1 << 32)])) & 0xffffffff)
    put(counts, 8, 'II', r.choice([0, 0, 1, 2, r.getrandbits(32)]), r.choice([0, 0, 1, 2, r.getrandbits(32)]))
    folder = r.choice([b'', b'data', b'data\\init', b'data\\init\\', b'\\'])
    name = r.choice([b'', b'a', b'loading'])
    regions = {HEAP: heap(0x100, r.choice([0, 0, 0, 0, 0, 0, 0, 0, 0, 1])), **global_heap(), LOG: log, SCREEN: screen, BLOCKS: blocks, PAIRS: table, COUNT: struct.pack('<I', r.choice([0, 1, 2, 4, 8])),
               NAME: name + b'\0', FOLDER: folder + b'\0', TIMES: struct.pack('<40I', *[(now + 1000 * i) & 0xffffffff for i in range(40)]),
               ANSWERS: struct.pack('<40I', *[r.choice([0, 1, r.getrandbits(32)]) for _ in range(40)]), 0x6fd3ec: struct.pack('<I', recorder(TIME, 0, True)), 0x8c4780: counts,
               0x8bcbc8: r.bytes(4), 0x8bd508: r.bytes(4), 0x8bcbd0: struct.pack('<i', r.choice([400, 512, 640, 800, 1024, 0, 1, 1280]))}
    ms = r.choice([0, 0, 1, 500, 1000, 0x1000001, 0x7fffffff, -1, -500, r.getrandbits(31)])
    return Call(0x587380, [SCREEN, r.choice([0, NAME, NAME]), r.choice([0, FOLDER, FOLDER]), ms & 0xffffffff], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
