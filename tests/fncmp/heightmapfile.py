# The height map read from its file or built and written (src/game/model/heightmap: HeightMap_ReadOrBuild, 0x451640) against the original: no file, no block, a short read, bad magic or version, a file of the size or of
# another, a build that fails (a fault in both); no file to write. The file's calls, the allocator and HeightMap_Build recorders; the block written is compared as a region.
# usage: heightmapfile.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, set_return  # noqa: E402

CALLS = {0x5c4d50: (16, 2), 0x5c5000: (17, 1), 0x45b9d0: (3, 2), 0x5c4f60: (18, 4), 0x5c4f80: (19, 1), 0x45ba00: (20, 1), 0x451880: (21, 5), 0x67b8a0: (22, 2), 0x67b6d0: (23, 4), 0x67b620: (24, 1)}
OPEN, SIZE, ALLOC, READ, BUILD, FOPEN = 16, 17, 3, 18, 21, 22
B = Bench([(0x451640, 0x45187b), (0x5da3c0, 0x5da3c1)], hooks={addr: _recorder(k, n, cdecl=True) for addr, (k, n) in CALLS.items()})
INSTANCE, TABLE, FILE, BLOCK, MAP, GRIDS, OWNER, PLACE, PATH = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x4000, RAM + 0x4100, RAM + 0x4200
OPS = ['same', 'same', 'other size', 'other size', 'no file', 'no block', 'short read', 'bad magic', 'bad version', 'build fails', 'no write']


def header(r, width, height, a, b, c, magic=0xab1e0003, version=0x640005):
    return struct.pack('<IIHHIII', magic, version, width, height, a, b, c) + r.bytes(0x18)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    width, height = r.choice([1, 2, 3]), r.choice([1, 2, 4])
    cells = width * height
    args_w, args_h = (width, height) if op not in ('other size', 'build fails', 'no write', 'no file') else (r.choice([width, width + 1]), height + 1)
    file = header(r, width, height, 0x30, 0x60, 0x70, 0xab1e0004 if op == 'bad magic' else 0xab1e0003, 0x640006 if op == 'bad version' else 0x640005)
    fill(log, READ, 0, file)
    set_return(log, OPEN, 0 if op == 'no file' else 0x11110000)
    set_return(log, SIZE, r.randrange(0x30, 0x200))
    set_return(log, READ, 0 if op == 'short read' else r.choice([1, 0x40]))
    table = struct.pack('<40I', *([0 if op == 'no block' else FILE] + [BLOCK] * 39)) if op not in ('no file',) else struct.pack('<40I', *([BLOCK] * 40))
    answers(log, ALLOC, TABLE)
    set_return(log, BUILD, 0 if op == 'build fails' else MAP)
    set_return(log, FOPEN, 0 if op == 'no write' else 0x22220000)
    w2, h2 = r.choice([args_w, 2]), r.choice([args_h, 3])
    n2 = w2 * h2
    grid, small, last = r.bytes(25 * n2), r.bytes(n2), r.bytes(n2)
    offs = [GRIDS if r.random() < 0.9 else 0, GRIDS + 25 * n2 if r.random() < 0.9 else 0, GRIDS + 26 * n2 if r.random() < 0.9 else 0]
    regions = {LOG: log, TABLE: table, INSTANCE: struct.pack('<II', r.getrandbits(32), r.getrandbits(32)) + struct.pack('<I', OWNER) + r.bytes(0xcc), OWNER: r.bytes(4) + struct.pack('<I', PLACE), PLACE: r.bytes(0xa0), FILE: bytes(0x80),
               BLOCK: r.bytes(0x30 + 27 * n2 + 8), MAP: header(r, w2, h2, offs[0], offs[1], offs[2]), GRIDS: grid + small + last, PATH: b'a.map\0'}
    return Call(0x451640, [INSTANCE, args_w, args_h, PATH, r.choice([0, RAM + 0x5000]), r.choice([0, 1])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
