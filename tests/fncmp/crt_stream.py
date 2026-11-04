# The C library's stream reading and wide characters (src/game/core/crtstream, wctomb and mbtowc in core/crt, _flswbuf in core/crtfile) against the originals:
# the Win32 calls COM recorders in their import slots, the locks, _getptd and _nh_malloc cdecl recorders, the handle table and the buffers made here.
# usage: crt_stream.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob_array, fill, fill_arg, import_table, log_region, set_return  # noqa: E402

M2W, GETPTD, LOCK, UNLOCK, LOCKFILE, UNLOCKFILE, NHMALLOC = 3, 2, 7, 8, 11, 12, 28
CSINIT, CSENTER, CSLEAVE, WRITE, SEEK, ERROR, READ, INC, DEC, W2M = 14, 15, 16, 17, 18, 19, 20, 29, 30, 31
B = Bench([(0x686a00, 0x686a19), (0x686980, 0x6869f3), (0x68ead0, 0x68eb19), (0x68eb20, 0x68eb85), (0x68eb90, 0x68ebb2), (0x686440, 0x6864b2), (0x6864c0, 0x6866ca), (0x686a20, 0x686a92), (0x686aa0, 0x686b20),
           (0x68c320, 0x68c34b), (0x687530, 0x68758d), (0x680ad0, 0x680c00), (0x67dcd0, 0x67dcf0),
           (0x686ff0, 0x687137), (0x687140, 0x6871ac), (0x6871b0, 0x687222), (0x687690, 0x687701), (0x687710, 0x68780e), (0x67c9f0, 0x67ca1e), (0x67ca20, 0x67cb85), (0x67cb90, 0x67cbda), (0x687440, 0x68752f),
           (0x687590, 0x68768d), (0x688a50, 0x688ac2), (0x688ad0, 0x688cf5), (0x67cbe0, 0x67cc0d), (0x67cc10, 0x67cc93), (0x67cca0, 0x67ccce), (0x67ccd0, 0x67cded), (0x67cdf0, 0x67ce19), (0x67ce20, 0x67cf40),
           (0x67e1d0, 0x67e207), (0x67e210, 0x67e34c), (0x67e350, 0x67e378), (0x67e380, 0x67e524)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x6827c0: _recorder(LOCKFILE, 1, cdecl=True),
                 0x682830: _recorder(UNLOCKFILE, 1, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True)})
PTD, FILE, ANSWERS, SEEKS, OUT = (RAM + 0x100 * i for i in range(5))
INFO0, INFO1, PCTYPE, BUFFER, BLOCK = RAM + 0x1000, RAM + 0x1800, RAM + 0x2000, RAM + 0x3000, RAM + 0x5000
TEXT = RAM + 0x6000
OPS = [n for n in ('flswbuf', 'fputwc', 'fputwc_lk', 'wctomb', 'wctomb_lk', 'mbtowc', 'mbtowc_lk', 'fgetc', 'filbuf', 'filwbuf', 'read', 'read_lk', 'ungetc', 'ungetc_lk', 'ungetwc', 'ungetwc_lk', 'fgetwc', 'fgetwc_lk', 'fread', 'fread_lk', 'ftell', 'ftell_lk')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
FDS = [0, 1, 2, 5, 0x1f, 0x20, 0x21, 0x3f]


def table(r):
    out = bytearray(0x480)
    for i in range(0x20):
        struct.pack_into('<IBB', out, i * 0x24, 0x1000 + i, r.choice([1, 0x81, 0xc1, 0x21, 0xa1, 0x41, 0xe1, 0xa3, 0x83, 0x89, 0xc9, 0x85, 0x82, 9, 0]), r.choice([0xa, 0xa, 0x41, 0xd]))
        struct.pack_into('<I', out, i * 0x24 + 8, r.choice([0, 1, 1]))
    return out


def text(r, size):
    block = bytes(r.choice([0xa, 0xd, 0xd, 0x41, 0x1a, 0x81, r.randrange(256)]) for _ in range(0x40))
    return (block * (size // 0x40 + 1))[:size]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, PTD: bytearray(0x20), INFO0: table(r), INFO1: table(r), PCTYPE: struct.pack('<256H', *[0x8000 if i >= 0x81 else 0x20 for i in range(256)]),
               IMPORT_TABLE: import_table(log, {'init': (CSINIT, 0x6fd144, 1, 0, 0), 'enter': (CSENTER, 0x6fd140, 1, 0, 0), 'leave': (CSLEAVE, 0x6fd130, 1, 0, 0), 'write': (WRITE, 0x6fd0e0, 5, 0, 0b1010),
                                                'seek': (SEEK, 0x6fd0e4, 4, 0, 0), 'error': (ERROR, 0x6fd138, 0, 0, 0), 'read': (READ, 0x6fd0dc, 5, 0, 0b1010),
                                                'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0), 'w2m': (W2M, 0x6fd1dc, 8, 0, 0b10010100), 'm2w': (M2W, 0x6fd174, 6, 0, 0b10100)})}
    set_return(log, GETPTD, PTD)
    regions[0xfb3ba0] = struct.pack('<64I', *[INFO1 if i == 1 else INFO0 for i in range(64)])
    regions[0xfb3ca0] = struct.pack('<3I', 0x40, r.choice([0, 0, 1]), r.randrange(4))
    regions[0xfb1350] = struct.pack('<I', r.randrange(4))
    regions[0xfb1548] = struct.pack('<I', r.randrange(2))
    regions[0xfb1440] = struct.pack('<5I', r.choice([0, 0, 0x409, 0x411]), 0, 0, 0, r.choice([932, 1252]))
    regions[0x77e7dc] = struct.pack('<i', r.choice([1, 1, 2, 3, 0]))
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[0x7813b0] = struct.pack('<IB', 0, r.choice([0, 0x20, 0x80, 0x82]))
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([1, 1, 1, 1, 1, 0]) for _ in range(40)])
    regions[SEEKS] = struct.pack('<40I', *[r.choice([0x100, 0, 0xffffffff]) for _ in range(40)])
    answers(log, WRITE, ANSWERS)
    answers(log, READ, ANSWERS)
    answers(log, SEEK, SEEKS)
    blob_array(log, WRITE, 1, 2, 1)
    blob_array(log, W2M, 2, 3, 2, which=1)
    blob_array(log, M2W, 2, 3, 1, which=2)
    fill_arg(log, WRITE, 3, 2)
    if r.random() < 0.7:
        fill_arg(log, READ, 3, 2, which=1)
    else:
        fill(log, READ, 3, struct.pack('<4I', *[r.choice([0, 1, 2, 3, 8, 0x20]) for _ in range(4)]), which=1, records=4)
    fill(log, READ, 1, bytes(r.choice([0xa, 0xd, 0x41]) for _ in range(4)), which=2, records=4)
    if name not in ('ungetwc', 'ungetwc_lk') or r.random() < 0.5:   # (not written: _ungetwc_lk's bytes are the character's own, _getwc_lk's answer the file argument's low word)
        fill(log, W2M, 4, r.bytes(2), which=3)
    fill(log, W2M, 7, struct.pack('<I', r.choice([0, 0, 0, 1])), which=4)
    if name not in ('fgetwc', 'fgetwc_lk') or r.random() < 0.5:
        fill(log, M2W, 4, r.bytes(2), which=5)
    set_return(log, W2M, r.choice([1, 1, 2, 2, 0]))
    set_return(log, M2W, r.choice([1, 1, 2, 0]))
    set_return(log, ERROR, r.choice([5, 2, 6, 0x6d, 0x13, 0x57, 0]))
    set_return(log, NHMALLOC, r.choice([BLOCK, BLOCK, BLOCK, 0]))
    regions[OUT] = r.bytes(0x10)
    c = r.choice([0x41, 0xe9, 0x8140, 0xffff, 0x10041, 0xa, 0xffffffff, 0x100, 0xff])
    if name in ('wctomb', 'wctomb_lk'):
        return Call(0x687140 if name == 'wctomb' else 0x6871b0, [r.choice([OUT, OUT, OUT, 0]), c], regions, conv='c', ret='a')
    if name in ('mbtowc', 'mbtowc_lk'):
        regions[TEXT] = bytes([r.choice([0, 0x41, 0x81, 0x90, 0xff]), r.choice([0, 0x41, 0x81])]) + r.bytes(6)
        return Call(0x687690 if name == 'mbtowc' else 0x687710, [r.choice([OUT, OUT, OUT, 0]), r.choice([TEXT, TEXT, TEXT, 0]), r.choice([0, 1, 2, 3, 0xffffffff])], regions, conv='c', ret='a')
    regions[BUFFER] = text(r, 0x1400)
    regions[BLOCK] = text(r, 0x1000)
    if name in ('read', 'read_lk'):
        return Call(0x688a50 if name == 'read' else 0x688ad0, [r.choice(FDS + [0x40, 0x50]) if name == 'read' else r.choice(FDS), BUFFER, r.choice([0, 1, 2, 3, 8, 0x20, 0x80])], regions, conv='c', ret='a')
    file = r.choice([FILE, FILE, FILE, FILE, 0x77e820, 0x77e840])
    flags = r.choice([1, 1 | 8, 0x81, 0x81 | 8, 1 | 0x100, 1 | 4, 0x80, 0x80 | 8, 0x82, 0x82 | 8, 2, 2 | 8, 0x83 | 8, 0x40, 0x41, 0x49, 0, 8, 1 | 8 | 0x400, 1 | 0x10 | 8, 0x81 | 0x10])
    size = r.choice([0, 1, 4, 0x10, 0x200, 0x1000, -1])
    fd = r.choice(FDS + [-1, 0x40])
    base = r.choice([BUFFER, BUFFER, BUFFER, BUFFER, 0])
    regions[file] = struct.pack('<6Ii', (BUFFER + r.choice([0, 0, 1, 2, 8, 0x40, 0xffffffff])) & 0xffffffff, r.choice([0xffffffff, 0, 0, 1, 2, 3, 5, 0x20]), base, flags, fd & 0xffffffff, r.getrandbits(32), size) + bytes(4)
    if name in ('fgetc', 'filbuf', 'filwbuf', 'fgetwc', 'fgetwc_lk'):
        at = {'fgetc': 0x67cb90, 'filbuf': 0x687440, 'filwbuf': 0x687590, 'fgetwc': 0x67cdf0, 'fgetwc_lk': 0x67ce20}[name]
        return Call(at, [file], regions, conv='c', ret='w' if name in ('fgetwc', 'fgetwc_lk') else 'a')
    if name in ('ftell', 'ftell_lk'):
        return Call(0x67e350 if name == 'ftell' else 0x67e380, [file], regions, conv='c', ret='a')
    if name in ('fread', 'fread_lk'):
        regions[TEXT] = r.bytes(0x600)
        return Call(0x67e1d0 if name == 'fread' else 0x67e210, [TEXT, r.choice([0, 1, 1, 2, 4]), r.choice([0, 1, 3, 0x10, 0x80, 0x100]), file], regions, conv='c', ret='a')
    if name in ('ungetc', 'ungetc_lk'):
        return Call(0x67cbe0 if name == 'ungetc' else 0x67cc10, [r.choice([0x41, 0xd, 0xa, 0xe9, 0x141, 0xffffffff]), file], regions, conv='c', ret='a')
    if name == 'flswbuf':
        return Call(0x686ff0, [c & 0xffff, file], regions, conv='c', ret='a')
    at = {'fputwc': 0x67c9f0, 'fputwc_lk': 0x67ca20, 'ungetwc': 0x67cca0, 'ungetwc_lk': 0x67ccd0}[name]
    return Call(at, [c, file], regions, conv='c', ret='w')


if __name__ == '__main__':
    B.main(case)
