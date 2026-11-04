# The two steps around a bump texture (src/game/gfx/bumplight: 0x5eb820, 0x5ebf10) against the originals. Their callees are recorders (each has a bench of its own: array3d, image_filters, bumplight):
# their arguments and order are compared, and what they answer steers the rest.
# usage: bumpbuild.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Build,Filter)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region  # noqa: E402

REALLOC, RAMP, BLUR, RENDER, CROP, ERODE, DILATE, LINES, FREE = 1, 2, 3, 4, 5, 6, 7, 8, 9
B = Bench([(0x5eb820, 0x5eb8a0), (0x5ebf10, 0x5ebfc0)],
          hooks={0x5e7840: _recorder(REALLOC, 3), 0x5eb270: _recorder(RAMP, 3), 0x5eb3e0: _recorder(BLUR, 3), 0x5eb8a0: _recorder(RENDER, 1), 0x5d3860: _recorder(FREE, 0),
                 0x5ea3f0: _recorder(CROP, 4, cdecl=True), 0x5ea6d0: _recorder(ERODE, 2, cdecl=True), 0x5ea510: _recorder(DILATE, 2, cdecl=True), 0x5ea8a0: _recorder(LINES, 3, cdecl=True)})
OBJECT, SOURCE, DEST, TABLE = RAM, RAM + 0x100, RAM + 0x140, RAM + 0x200
SIGMAS = [0.0, -0.0, 1.5, 0.25, -1.5, 1e-40, float('inf'), float('-inf'), float('nan')]


def floatbytes(r):
    return struct.pack('<f', r.choice(SIGMAS))


def table(r):
    return struct.pack('<48I', *[r.choice([1, 1, 1, 1, 0, 0x101, 0x100, r.getrandbits(32)]) for _ in range(48)])


def case(r):
    log = log_region()
    regions = {LOG: log}
    obj = bytearray(r.bytes(0x70))
    obj[8:12] = floatbytes(r)
    if r.choice(OPS) == 'Build':
        answers(log, REALLOC, TABLE)
        regions[TABLE] = table(r)
        for at in (0x38, 0x48):
            if r.random() < 0.2:
                obj[at:at + 4] = bytes(4)
        regions[OBJECT] = obj
        return Call(0x5eb820, [OBJECT], regions, conv='t', ret='v')
    for k, slot in ((CROP, 0), (ERODE, 1), (DILATE, 2), (LINES, 3)):
        answers(log, k, TABLE + 0x100 * slot)
        regions[TABLE + 0x100 * slot] = table(r)
    source = bytearray(r.bytes(0x20))
    if r.random() < 0.1:
        source[0xc:0x10] = bytes(4)
    obj[4:8] = struct.pack('<i', r.choice([0, 0, 1, 5, -1, -5, -0x80000000, r.randrange(-0x8000, 0x8000)]))
    regions[OBJECT], regions[SOURCE], regions[DEST] = obj, source, r.bytes(0x20)
    return Call(0x5ebf10, [OBJECT, SOURCE, DEST], regions, conv='t', ret='b')


OPS = [k for k in os.environ.get('OPS', 'Build,Filter').split(',') if k]

if __name__ == '__main__':
    B.main(case)
