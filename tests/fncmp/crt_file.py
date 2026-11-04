# The C library's low-level files and write side (src/game/core/crtfile: 0x686a10, 0x686980, 0x68ead0, 0x68eb20, 0x68eb90, 0x686aa0, 0x686a20, 0x68c320, 0x6864c0, 0x686440, 0x687530, 0x67d0f0, 0x680ad0, 0x67b710, 0x67b6d0) against the originals:
# the Win32 calls COM recorders in their import slots (WriteFile's bytes hashed, up to 0x100), the locks, _getptd and _nh_malloc cdecl recorders, the handle table made here.
# usage: crt_file.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob_array, fill, fill_arg, import_table, log_region, set_return  # noqa: E402

GETPTD, LOCK, UNLOCK, LOCKFILE, UNLOCKFILE, NHMALLOC = 2, 7, 8, 11, 12, 28
CSINIT, CSENTER, CSLEAVE, WRITE, SEEK, ERROR = 14, 15, 16, 17, 18, 19
B = Bench([(0x686a00, 0x686a19), (0x686980, 0x6869f3), (0x68ead0, 0x68eb19), (0x68eb20, 0x68eb85), (0x68eb90, 0x68ebb2), (0x686440, 0x6864b2), (0x6864c0, 0x6866ca), (0x686a20, 0x686a92), (0x686aa0, 0x686b20),
           (0x68c320, 0x68c34b), (0x687530, 0x68758d), (0x67d0f0, 0x67d15e), (0x680ad0, 0x680c00), (0x67b710, 0x67b856), (0x67b6d0, 0x67b707), (0x67dcd0, 0x67dcf0)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x6827c0: _recorder(LOCKFILE, 1, cdecl=True),
                 0x682830: _recorder(UNLOCKFILE, 1, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True)})
PTD, FILE, ANSWERS, SEEKS = (RAM + 0x100 * i for i in range(4))
INFO0, INFO1, DATA, BUFFER, BLOCK = RAM + 0x1000, RAM + 0x1800, RAM + 0x2000, RAM + 0x3000, RAM + 0x5000
OPS = [n for n in ('doserrno', 'dosmaperr', 'osfhandle', 'lock', 'unlock', 'lseek', 'lseek_lk', 'isatty', 'write', 'write_lk', 'getbuf', 'flush', 'flsbuf', 'fwrite', 'fwrite_lk') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
FDS = [0, 1, 2, 5, 0x1f, 0x20, 0x21, 0x3f, 0x40, 0x50]


def table(r):
    out = bytearray(0x480)
    for i in range(0x20):
        struct.pack_into('<IB', out, i * 0x24, 0x1000 + i, r.choice([1, 0x81, 0xc1, 0x21, 0xa1, 0x41, 0xe1, 0xa3, 0, 0x80]))
        struct.pack_into('<I', out, i * 0x24 + 8, r.choice([0, 1, 1]))
    return out


def text(r, size):
    return bytes(r.choice([0xa, 0xa, 0x41, 0x1a, r.randrange(256)]) for _ in range(size))


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, PTD: bytearray(0x20), INFO0: table(r), INFO1: table(r),
               IMPORT_TABLE: import_table(log, {'init': (CSINIT, 0x6fd144, 1, 0, 0), 'enter': (CSENTER, 0x6fd140, 1, 0, 0), 'leave': (CSLEAVE, 0x6fd130, 1, 0, 0), 'write': (WRITE, 0x6fd0e0, 5, 0, 0b1000),
                                                'seek': (SEEK, 0x6fd0e4, 4, 0, 0), 'error': (ERROR, 0x6fd138, 0, 0, 0)})}
    set_return(log, GETPTD, PTD)
    regions[0xfb3ba0] = struct.pack('<64I', *[INFO1 if i == 1 else INFO0 for i in range(64)])
    regions[0xfb3ca0] = struct.pack('<I', 0x40)
    regions[0xfb1350] = struct.pack('<I', r.randrange(4))
    regions[0xfb1548] = struct.pack('<I', r.randrange(2))
    regions[0x7813b0] = struct.pack('<IB', 0, r.choice([0, 0x20]))
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([1, 1, 1, 1, 1, 0]) for _ in range(40)])
    regions[SEEKS] = struct.pack('<40I', *[r.choice([0x100, 0, 0xffffffff]) for _ in range(40)])
    answers(log, WRITE, ANSWERS)
    answers(log, SEEK, SEEKS)
    blob_array(log, WRITE, 1, 2, 1)
    if r.random() < 0.7:
        fill_arg(log, WRITE, 3, 2)
    else:
        fill(log, WRITE, 3, struct.pack('<4I', *[r.choice([0, 1, 2, 8, 0x400, 0x401]) for _ in range(4)]), records=4)
    set_return(log, ERROR, r.choice([5, 2, 6, 0x13, 0x24, 0x25, 0xbc, 0xca, 0xcb, 0x57, 1, 0]))
    set_return(log, NHMALLOC, r.choice([BLOCK, BLOCK, BLOCK, 0]))
    if name == 'doserrno':
        return Call(0x686a10, [], regions, conv='c', ret='a')
    if name == 'dosmaperr':
        return Call(0x686980, [r.choice([0, 1, 2, 5, 0x12, 0x13, 0x24, 0x25, 0xbb, 0xbc, 0xca, 0xcb, 0x57, r.getrandbits(32), r.randrange(0x100)])], regions, conv='c')
    fd = r.choice(FDS)
    if name == 'osfhandle':
        return Call(0x68ead0, [fd], regions, conv='c', ret='a')
    if name == 'lock':
        return Call(0x68eb20, [r.choice(FDS[:8])], regions, conv='c')
    if name == 'unlock':
        return Call(0x68eb90, [r.choice(FDS[:8])], regions, conv='c')
    if name in ('lseek', 'lseek_lk'):
        return Call(0x686a20 if name == 'lseek' else 0x686aa0, [fd, r.choice([0, 4, 0xffffffff]), r.randrange(3)], regions, conv='c', ret='a')
    if name == 'isatty':
        return Call(0x68c320, [fd], regions, conv='c', ret='a')
    size = r.choice([0, 1, 2, 3, 8, 0x20, 0x80, 0x3ff, 0x400, 0x500])
    regions[DATA] = text(r, 0x600)
    if name in ('write', 'write_lk'):
        return Call(0x686440 if name == 'write' else 0x6864c0, [fd if name == 'write' else r.choice(FDS[:8]), DATA, size], regions, conv='c', ret='a')
    file = r.choice([FILE, FILE, FILE, FILE, 0x77e820, 0x77e840])
    flags = r.choice([0x2, 0x82, 0x2 | 0x8, 0x2 | 0x100, 0x2 | 4, 0x82 | 0x8, 0x1 | 0x10, 0x81 | 0x10, 0x1, 0x40 | 2, 0x10, 0x83 | 8, 0, 8, 0x1 | 0x10 | 8, 0x20 | 2 | 8])
    bufsize = r.choice([0, 1, 4, 0x10, 0x1000, -1])
    fd = r.choice(FDS[:8] + [-1, 0x40]) if name == 'flsbuf' else r.choice(FDS[:8] + [0x40])
    regions[BUFFER] = r.bytes(0x100)
    regions[BLOCK] = r.bytes(0x20)
    regions[file] = struct.pack('<6Ii', BUFFER + r.choice([0, 0, 1, 2, 8, 0x40, -1 & 0xffffffff]) & 0xffffffff, r.choice([0xffffffff, 0, 0, 1, 5, 0x20]), BUFFER, flags, fd & 0xffffffff, r.getrandbits(32), bufsize) + bytes(4)
    if name == 'getbuf':
        return Call(0x687530, [file], regions, conv='c')
    if name == 'flush':
        return Call(0x67d0f0, [file], regions, conv='c', ret='a')
    if name == 'flsbuf':
        return Call(0x680ad0, [r.choice([0x41, 0xe9, 0xa, 0x141, 0xffffffff]), file], regions, conv='c', ret='a')
    return Call(0x67b6d0 if name == 'fwrite' else 0x67b710, [DATA, r.choice([0, 1, 1, 2, 4]), r.choice([0, 1, 3, 8, 0x20, 0x100]), file], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
