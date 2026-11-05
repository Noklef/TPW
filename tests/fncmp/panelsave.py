# The panel's picture written out (Panel_SavePicture, src/game/gfx/imageops: 0x5d4a30) against the original. Small images (the description's hash reads 0x100 bytes per pitch's unit, so a width of 2 or 3 reaches all that is set and
# stays inside it); now and then the pixels the image's own width and height, whose reversal changes the count while it runs. GuardFile_Open 5 (none now and then), GuardFile_Close 6, the library's 0x64c202 7 (its first argument, a local, given a handle or left alone), 0x64c258 8, 0x64c5c4 9: stdcall recorders.
# usage: panelsave.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, blob_array, fill, log_region, put, set_return, strings  # noqa: E402

OPEN, CLOSE, START, WRITE, FINISH = 5, 6, 7, 8, 9
B = Bench([(0x5d4a30, 0x5d4b6e + 2)], hooks={0x5d4d10: _recorder(OPEN, 1, com=True), 0x5d4fd0: _recorder(CLOSE, 1, com=True), 0x64c202: _recorder(START, 4, com=True), 0x64c258: _recorder(WRITE, 4, com=True), 0x64c5c4: _recorder(FINISH, 1, com=True)})
PANEL, PIXELS, FILE = RAM, RAM + 0x100, RAM + 0x400


def case(r):
    log = log_region()
    width, height = r.choice([0, 1, 2, 2, 2, 3, 3, 3]), r.choice([0, 1, 1, 2, 3, 4])
    panel = bytearray(r.bytes(0x70))
    own = r.randrange(10) == 0
    if own:   # (1 x 1: each swapped to 0x1000000: the count 1, 0x1000000 and then 0)
        width, height = 1, 1
    put(panel, 0x50, '2I', width, height)
    put(panel, 0x5c, 'I', PANEL + 0x50 if own else PIXELS)
    set_return(log, OPEN, r.choice([FILE, FILE, FILE, FILE, 0]))
    set_return(log, CLOSE, r.getrandbits(32))
    strings(log, START, 0, out=1)
    if r.randrange(4):
        fill(log, START, 0, struct.pack('<I', r.getrandbits(32)))
    if own:
        blob(log, WRITE, 1, 0xff)
    else:
        blob_array(log, WRITE, 1, 3, 0x100)
    regions = {LOG: log, PANEL: panel, PIXELS: r.bytes(0x60), FILE: r.bytes(0x10)}
    return Call(0x5d4a30, [PANEL, r.getrandbits(32)], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
