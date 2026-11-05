# The screen grab saved (src/game/gfx/screengrab: 0x561370, 0x561760) against the originals: the .tga's header (hashed), rows from the last, no file; the save's steps recorders failing each in turn, no name set. The C library's file calls and the
# engine's errors recorders; the pixels' pointers hashed by their bytes.
# usage: screengrabsave.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, set_return  # noqa: E402

FOPEN, FWRITE, FCLOSE, PUSH, TAKE, TORGB, FREE = 1, 2, 3, 4, 5, 6, 8
B = Bench([(0x561370, 0x561460), (0x561760, 0x5617bd)], hooks={0x67b8a0: _recorder(FOPEN, 2, cdecl=True), 0x67b6d0: _recorder(FWRITE, 4, cdecl=True), 0x67b620: _recorder(FCLOSE, 1, cdecl=True), 0x56af20: _recorder(PUSH, 2),
                                                               0x561610: _recorder(TAKE, 0), 0x561460: _recorder(TORGB, 0), 0x579b80: _recorder(FREE, 1, cdecl=True)})
GRAB, PIXELS, PATH = RAM, RAM + 0x200, RAM + 0x100
OPS = ['Write', 'Write', 'Write', 'No file', 'Save', 'Save', 'Save', 'Save', 'Save', 'Unnamed']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    width, height = r.choice([1, 2, 5, 0x101]) if op != 'Write' else r.choice([1, 2, 5]), r.choice([0, 1, 2, 3, -1])
    regions = {LOG: log, GRAB: r.bytes(0x90), PIXELS: r.bytes(0x200), PATH: b'Scr00001.tga\0', 0x8bcbd0: struct.pack('<ii', width, height)}
    grab = bytearray(regions[GRAB])
    struct.pack_into('<I', grab, 4, PIXELS)
    struct.pack_into('<I', grab, 0x8c, 0 if op == 'Unnamed' else PATH)
    regions[GRAB] = grab
    blob_bytes(log, FWRITE, 0, (1 << 18) - 1)
    if op in ('Write', 'No file'):
        set_return(log, FOPEN, 0 if op == 'No file' else 0x11110000)
        return Call(0x561370, [GRAB, PATH], regions, conv='t', ret='a')
    answers = r.choice([(1, 1, 1), (1, 1, 1), (0, 1, 1), (1, 0, 1), (1, 1, 0), (7, 0x100, 1)])
    set_return(log, TAKE, answers[0])
    set_return(log, TORGB, answers[1])
    set_return(log, FOPEN, 0x11110000 if answers[2] else 0)
    regions[0x8bcbd0] = struct.pack('<ii', 1, 1)
    return Call(0x561760, [GRAB], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
