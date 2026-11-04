# The C library's opening, closing and flushing of files (src/game/core/crtopen) against the originals: the Win32 calls COM recorders in their import slots, the locks, _getptd and _nh_malloc cdecl recorders,
# free the heap's hash, the handle table and the streams' table made here. A section entered may set its slot's open flag, as another thread would (the section's first dword points at the flag).
# usage: crt_open.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, blob, blob_array, fill, fill_arg, heap, import_table, log_region, set_return  # noqa: E402

GETPTD, LOCK, UNLOCK, LOCKFILE, UNLOCKFILE, NHMALLOC, LOCKFILE2, UNLOCKFILE2 = 2, 7, 8, 11, 12, 28, 30, 31
CSINIT, CSENTER, CSLEAVE, WRITE, SEEK, ERROR, READ, CREATE, TYPE, CLOSE, SETSTD, SEOF, FLUSHFB = 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26
B = Bench([(0x686a00, 0x686a19), (0x686980, 0x6869f3), (0x68ead0, 0x68eb19), (0x68eb20, 0x68eb85), (0x68eb90, 0x68ebb2), (0x686440, 0x6864b2), (0x6864c0, 0x6866ca), (0x686aa0, 0x686b20), (0x67d0f0, 0x67d15e), (0x67dcd0, 0x67dcf0),
           (0x688ad0, 0x688cf5), (0x67b110, 0x67b13f), (0x67b860, 0x67b897), (0x67b8a0, 0x67b8b5), (0x6868a0, 0x686979), (0x6866d0, 0x68682b), (0x68b640, 0x68b991), (0x68e810, 0x68e978), (0x68e980, 0x68ea21), (0x68ea30, 0x68eac3),
           (0x686300, 0x686368), (0x686370, 0x686400), (0x686400, 0x686433), (0x68f630, 0x68f77e), (0x691610, 0x691674), (0x67cf90, 0x67d067), (0x67d070, 0x67d0a7), (0x67d0b0, 0x67d0e5), (0x67d160, 0x67d16b), (0x67d170, 0x67d22b),
           (0x687b20, 0x687bbe)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x6827c0: _recorder(LOCKFILE, 1, cdecl=True), 0x682830: _recorder(UNLOCKFILE, 1, cdecl=True),
                 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x682800: _recorder(LOCKFILE2, 2, cdecl=True), 0x682870: _recorder(UNLOCKFILE2, 2, cdecl=True), 0x67ad90: _free})
PTD, ANSWERS, SEEKS, READS = (RAM + 0x100 * i for i in range(4))
INFO0, INFO1, TEXT, STREAMS, FILES, GIVEN, BLOCK = RAM + 0x1000, RAM + 0x1800, RAM + 0x2000, RAM + 0x2100, RAM + 0x2200, RAM + 0x2800, RAM + 0x3000
BUFFER, MODE = HEAP + 0x100, RAM + 0x2f00
OPS = [n for n in ('fopen', 'fsopen', 'getstream', 'openfile', 'sopen', 'sopen', 'alloc', 'set', 'free', 'close', 'close_lk', 'chsize', 'setmode', 'freebuf', 'setvbuf', 'fflush', 'fflush_lk', 'flsall', 'flushall', 'commit')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
FDS = [0, 1, 2, 5, 0x1f, 0x20, 0x21, 0x3f, 0x40, 0x50]
MODES = ['r', 'w', 'a', 'x', '', 'r+', 'w+', 'a+', 'rb', 'rt', 'wb', 'r+b', 'rb+', 'rbt', 'rtb', 'rc', 'rn', 'rcn', 'wc', 'rbc', 'rS', 'rR', 'rSR', 'rT', 'rTT', 'rD', 'rDD', 'r++', 'rz', 'rzb', 'r+D', 'r bS', 'wcbSTD', 'a+tnRTD', 'r\xff', 'rb\x2a', 'r,', 'ru']


def table(r):
    out = bytearray(0x480)
    for i in range(0x20):
        struct.pack_into('<IB', out, i * 0x24, r.choice([0x1000 + i, 0x1000 + i, 0xffffffff]), r.choice([1, 0x81, 0xc1, 0x21, 0xa1, 0x41, 0xe1, 0xa3, 0, 0x80]))
        struct.pack_into('<I', out, i * 0x24 + 8, r.choice([0, 1, 1]))
    return out


def stream(r, regions, at, fd):
    flags = r.choice([0, 0, 2, 0x82, 2 | 8, 2 | 0x100, 2 | 4, 0x82 | 8, 1 | 0x10, 0x81 | 0x10, 1, 0x40 | 2, 0x10, 0x83 | 8, 8, 1 | 8, 0x20 | 2 | 8, 2 | 0x4000, 2 | 8 | 0x4000, 0x83 | 8 | 0x4000, 0x4000, 0x3000 | 0xc00 | 2 | 8 | 4])
    regions[at] = struct.pack('<6Ii', BUFFER + r.choice([0, 0, 1, 2, 8, 0x40]), r.choice([0xffffffff, 0, 0, 1, 5, 0x20]), BUFFER, flags, fd & 0xffffffff, r.getrandbits(32), r.choice([0, 1, 4, 0x10, 0x1000, -1])) + r.bytes(0x1c)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, PTD: bytearray(0x20), INFO0: table(r), INFO1: table(r), HEAP: heap(0x1000),
               IMPORT_TABLE: import_table(log, {'init': (CSINIT, 0x6fd144, 1, 0, 0), 'enter': (CSENTER, 0x6fd140, 1, 0, 0), 'leave': (CSLEAVE, 0x6fd130, 1, 0, 0), 'write': (WRITE, 0x6fd0e0, 5, 0, 0b1000),
                                                'seek': (SEEK, 0x6fd0e4, 4, 0, 0), 'error': (ERROR, 0x6fd138, 0, 0, 0), 'read': (READ, 0x6fd0dc, 5, 0, 0b1010), 'create': (CREATE, 0x6fd0d8, 7, 1, 0),
                                                'type': (TYPE, 0x6fd1a8, 1, 0, 0), 'close': (CLOSE, 0x6fd13c, 1, 0, 0), 'setstd': (SETSTD, 0x6fd268, 2, 0, 0), 'seof': (SEOF, 0x6fd27c, 1, 0, 0), 'flush': (FLUSHFB, 0x6fd0fc, 1, 0, 0)})}
    set_return(log, GETPTD, PTD)
    regions[0xfb3ba0] = struct.pack('<64I', *[INFO1 if i == 1 else INFO0 for i in range(64)])
    regions[0xfb3ca0] = struct.pack('<I', 0x40)
    regions[0xfb1350] = struct.pack('<I', r.randrange(4))
    regions[0xfb1360] = struct.pack('<I', r.choice([0, 0x12, 0x80, 0x1ff]))
    regions[0xfb1548] = struct.pack('<I', r.randrange(2))
    regions[0xfb1894] = struct.pack('<I', r.choice([0, 0, 0x4000]))
    regions[0xfb1914] = struct.pack('<I', r.choice([0x8000, 0x4000, 0]))
    regions[0x77ea84] = struct.pack('<I', r.choice([1, 1, 2]))
    regions[0x7813b0] = struct.pack('<IB', 0, r.choice([0, 0x20]))
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([1, 1, 1, 1, 1, 0]) for _ in range(40)])
    regions[SEEKS] = struct.pack('<40I', *[r.choice([0x100, 0x100, 0, 0xffffffff]) for _ in range(40)])
    regions[READS] = struct.pack('<40I', *[r.choice([1, 1, 1, 1, 0]) for _ in range(40)])
    regions[TEXT] = (b'C:\\some\\file.dat\0' + bytes(0x100))[:0x100]
    regions[BLOCK] = bytes(0x480)   # (zero: a section's fill through its first dword does nothing here)
    regions[GIVEN] = r.bytes(0x20)
    answers(log, WRITE, ANSWERS)
    answers(log, SEEK, SEEKS)
    answers(log, READ, READS)
    blob_array(log, WRITE, 1, 2, 1)
    if r.random() < 0.7:
        fill_arg(log, WRITE, 3, 2)
    else:
        fill(log, WRITE, 3, struct.pack('<4I', *[r.choice([1, 2, 8, 0x400, 0x401]) for _ in range(4)]), records=4)
    fill(log, READ, 1, bytes([r.choice([0x1a, 0x1a, 0x41])]), which=1)
    fill(log, READ, 3, struct.pack('<I', r.choice([1, 1, 0])), which=2)
    blob(log, CREATE, 3, 0b111)
    set_return(log, ERROR, r.choice([5, 2, 6, 0x83, 0x83, 0x13, 0x24, 0xbc, 0x57, 1, 0]))
    set_return(log, NHMALLOC, r.choice([BLOCK, BLOCK, BLOCK, 0]))
    set_return(log, CREATE, r.choice([0x2222, 0x2222, 0x2222, 0x2222, 0xffffffff]))
    set_return(log, TYPE, r.choice([1, 1, 1, 1, 0, 2, 3, 4]))
    set_return(log, CLOSE, r.choice([1, 1, 0]))
    set_return(log, SEOF, r.choice([1, 1, 0]))
    set_return(log, FLUSHFB, r.choice([1, 1, 0]))
    count = r.choice([0, 1, 2, 4, 6])
    for i in range(6):
        stream(r, regions, FILES + 0x38 * i, r.choice(FDS[:8]))
    regions[STREAMS] = struct.pack('<6I', *[FILES + 0x38 * i if r.random() < 0.85 else 0 for i in range(6)])
    regions[0xfb3cbc] = struct.pack('<I', STREAMS)
    regions[0xfb4cc0] = struct.pack('<I', count)
    fd = r.choice(FDS)
    mode = r.choice(MODES).encode('latin-1') + b'\0'
    share = r.choice([0x10, 0x20, 0x30, 0x40, 0x40, 0x40, 0x40, 0, 0x50, 0x11, 0x41, 0x3f])
    regions[MODE] = mode + bytes(8)
    if name in ('alloc', 'sopen', 'fopen', 'fsopen', 'openfile'):   # (the first block now and then all open, so the search goes on; now and then no more blocks)
        for block in (INFO0, INFO1):
            data = regions[block]
            full = r.random() < (0.5 if block == INFO0 else 0.2)
            for i in range(0x20):
                if full:
                    data[i * 0x24 + 4] = 1
                struct.pack_into('<I', data, i * 0x24 + 0xc, block + i * 0x24 + 4)   # (the section: what its entering fills)
        fill(log, CSENTER, 0, bytes(r.choice([0, 0, 0, 1]) for _ in range(8)), which=3, records=8, through=0)
        blocks = r.choice([1, 2, 2, 0x40])
        regions[0xfb3ba0] = struct.pack('<64I', *[(INFO1 if i == 1 else INFO0) if i < blocks else 0 for i in range(64)])
        regions[0xfb3ca0] = struct.pack('<I', 0x20 * blocks)
    if name == 'fopen':
        return Call(0x67b8a0, [TEXT, MODE], regions, conv='c', ret='a')
    if name == 'fsopen':
        return Call(0x67b860, [TEXT, MODE, share], regions, conv='c', ret='a')
    if name == 'getstream':
        return Call(0x6868a0, [], regions, conv='c', ret='a')
    if name == 'openfile':
        return Call(0x6866d0, [TEXT, MODE, share, FILES], regions, conv='c', ret='a')
    if name == 'sopen':
        oflag = r.choice([0, 1, 2, 2, 2, 3 if r.random() < 0.2 else 2]) | r.randrange(8) << 8
        for bit in (0x80, 0x8000, 0x4000, 0x40, 0x1000, 0x20, 0x10, 8):
            if r.random() < 0.25:
                oflag |= bit
        return Call(0x68b640, [TEXT, oflag, share, r.choice([0x1a4, 0x80, 0, 0x100, 0x180])], regions, conv='c', ret='a')
    if name == 'alloc':
        return Call(0x68e810, [], regions, conv='c', ret='a')
    if name == 'set':
        return Call(0x68e980, [fd, 0x3333], regions, conv='c', ret='a')
    if name == 'free':
        return Call(0x68ea30, [fd], regions, conv='c', ret='a')
    if name == 'close':
        return Call(0x686300, [fd], regions, conv='c', ret='a')
    if name == 'close_lk':
        if r.random() < 0.5:   # (1 and 2 sharing a handle)
            struct.pack_into('<I', regions[INFO0], 0x24, 0x1001)
            struct.pack_into('<I', regions[INFO0], 0x48, 0x1001)
        return Call(0x686370, [r.choice([0, 1, 1, 2, 2, 5, 0x20])], regions, conv='c', ret='a')
    if name == 'chsize':
        return Call(0x68f630, [r.choice(FDS[:8]), r.choice([0, 0x80, 0x100, 0x100, 0x101, 0x1800, 0x3000])], regions, conv='c', ret='a')
    if name == 'setmode':
        return Call(0x691610, [r.choice(FDS[:8]), r.choice([0x8000, 0x8000, 0x4000, 0x4000, 0, 0x8001])], regions, conv='c', ret='a')
    if name == 'freebuf':
        return Call(0x686400, [FILES], regions, conv='c')
    if name == 'setvbuf':
        return Call(0x67cf90, [FILES, r.choice([0, 0, GIVEN]), r.choice([0, 0, 0x40, 0x40, 4, 4, 1, 2, 0x80, 0x44]), r.choice([0, 1, 2, 3, 0x100, 0x1001, 0x7fffffff, 0x80000000, 0xffffffff])], regions, conv='c', ret='a')
    if name == 'fflush':
        return Call(0x67d070, [r.choice([FILES, FILES, FILES, 0])], regions, conv='c', ret='a')
    if name == 'fflush_lk':
        return Call(0x67d0b0, [FILES], regions, conv='c', ret='a')
    if name == 'flsall':
        return Call(0x67d170, [r.choice([0, 0, 1, 1, 2, 0xffffffff])], regions, conv='c', ret='a')
    if name == 'flushall':
        return Call(0x67d160, [], regions, conv='c', ret='a')
    return Call(0x687b20, [fd], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
