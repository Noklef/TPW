# The C library's start-up and shut-down of its files (src/game/core/crtopen): _ioinit 0x687230, _initstdio 0x67e750, _endstdio 0x67e810, _rmtmp 0x68b9f0, against the originals.
# malloc's _nh_malloc, calloc, _amsg_exit, the locks and _fclose_lk are cdecl recorders (CrtInnerEnv); the imports COM recorders in their slots (GetStartupInfoA fills its structure).
# usage: crt_stdio.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, fill, heap, import_table, log_fold, log_region, set_return  # noqa: E402

LOCK, UNLOCK, FCLOSE, CALLOC, AMSG, NHMALLOC, LOCKFILE2, UNLOCKFILE2 = 7, 8, 13, 24, 26, 28, 30, 31
STARTUP, TYPE, STD, COUNT, DELETE = 14, 15, 16, 17, 18
B = Bench([(0x687230, 0x68743c), (0x67e750, 0x67e80c), (0x67e810, 0x67e824), (0x68b9f0, 0x68ba6e), (0x67dcd0, 0x67dcf0), (0x67d160, 0x67d16b), (0x67d170, 0x67d22b),
           (0x6891b0, 0x689243), (0x67b620, 0x67b660)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x67b660: _recorder(FCLOSE, 1, cdecl=True), 0x67fcb0: _recorder(CALLOC, 2, cdecl=True),
                 0x67efc0: _recorder(AMSG, 1, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x682800: _recorder(LOCKFILE2, 2, cdecl=True),
                 0x682870: _recorder(UNLOCKFILE2, 2, cdecl=True), 0x67ad90: _free})
MALLOCS, TYPES, STDS, CALLOCS = (RAM + 0x100 * i for i in range(4))
BLOCKS, PASSED, STREAMS, FILES, TABLE = RAM + 0x1000, RAM + 0x2400, RAM + 0x3000, RAM + 0x3100, RAM + 0x3800
KINDS = ('ioinit', 'initstdio', 'endstdio', 'rmtmp')
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def imports(log):
    return import_table(log, {'startup': (STARTUP, 0x6fd23c, 1, 0, 1), 'type': (TYPE, 0x6fd1a8, 1, 0, 0), 'std': (STD, 0x6fd1ac, 1, 0, 0), 'count': (COUNT, 0x6fd1b0, 1, 0, 0),
                              'delete': (DELETE, 0x6fd0d0, 1, 0, 0)})


def ioinit(r, log, regions):
    blocks = [BLOCKS + 0x480 * i for i in range(r.choice([1, 2, 3, 4, 4, 4]))]   # (then malloc fails)
    if r.random() < 0.05:
        blocks[0] = 0
    regions[MALLOCS] = struct.pack('<40I', *(blocks + [0] * (40 - len(blocks))))
    answers(log, NHMALLOC, MALLOCS)
    regions[TYPES] = struct.pack('<40I', *[r.choice([0, 1, 2, 2, 3, 3, 0x102, 0x203, r.getrandbits(32)]) for _ in range(40)])
    answers(log, TYPE, TYPES)
    regions[STDS] = struct.pack('<40I', *[r.choice([0xffffffff, 0, r.getrandbits(32)]) for _ in range(40)])
    answers(log, STD, STDS)
    log_fold(log)
    regions[BLOCKS] = r.bytes(0x480 * 4)
    regions[0xfb3ba0] = r.bytes(0x104)
    regions[0xfb1548] = r.bytes(4)
    count = r.choice([0, 1, 3, 0x1f, 0x20, 0x21, 0x45, 0x80, 0x81, 0x900, 0xffffffff])
    room = 0xc00 - 4
    flags = bytes(r.choice([0, 1, 1, 9, 0x41, 0x81, 0x89, 8, r.randrange(256)]) for _ in range(min(count, 0x900)))
    handles = struct.pack('<%dI' % 0x81, *[r.choice([0xffffffff, r.getrandbits(32), r.getrandbits(32)]) for _ in range(0x81)])
    passed = struct.pack('<I', count) + (flags + handles if count <= 0x900 else b'')
    regions[PASSED] = passed + r.bytes(4 + room - len(passed))
    info = bytearray(r.bytes(0x44))
    struct.pack_into('<HI', info, 0x32, r.choice([0, 1, 0x2c, 0x8000]), r.choice([PASSED, PASSED, PASSED, 0]))
    fill(log, STARTUP, 0, bytes(info))
    return Call(0x687230, [], regions, conv='c', ret='v')


def initstdio(r, log, regions):
    regions[0xfb4cc0] = struct.pack('<I', r.choice([0, 0, 1, 0x13, 0x14, 0x15, 0x30, 0x200, 0x80000000]))
    table = [TABLE if r.random() < 0.8 else 0, TABLE if r.random() < 0.9 else 0]
    regions[CALLOCS] = struct.pack('<40I', *(table + [0] * 38))
    answers(log, CALLOC, CALLOCS)
    regions[TABLE] = r.bytes(0x60)
    regions[BLOCKS] = b''.join(struct.pack('<I', r.choice([0xffffffff, 0, r.getrandbits(32)])) + r.bytes(0x20) for _ in range(3))
    regions[0xfb3ba0] = struct.pack('<I', BLOCKS)
    regions[0x77e800] = r.bytes(0x280)
    return Call(0x67e750, [], regions, conv='c', ret='v')


def streams(r, regions, flag_choices, names):
    n = r.choice([0, 1, 3, 5, 0x14, 0x16])
    table = []
    for i in range(0x16):
        at = FILES + 0x38 * i
        flags = r.choice(flag_choices) | (r.getrandbits(16) << 16 & 0xff000000)
        regions[at] = r.bytes(0xc) + struct.pack('<I', flags) + r.bytes(0xc) + struct.pack('<I', r.choice(names)) + r.bytes(0x18)
        table.append(at if r.random() < 0.8 else 0)
    regions[STREAMS] = struct.pack('<%dI' % 0x16, *table)
    regions[0xfb3cbc] = struct.pack('<I', STREAMS)
    regions[0xfb4cc0] = struct.pack('<I', n)


def endstdio(r, log, regions):   # (no stream in use: what flushing and closing one does is crt_open.py's and crt_fcloseall.py's)
    regions[HEAP] = heap(0x100)
    streams(r, regions, [0, 0x40, 0x7c, 0x100, 0xff00], [0])
    regions[0xfb1398] = bytes([r.choice([0, 0, 1, 0x80])])
    return Call(0x67e810, [], regions, conv='c', ret='v')


def rmtmp(r, log, regions):
    set_return(log, FCLOSE, r.getrandbits(32))
    streams(r, regions, [0, 1, 2, 0x80, 0x83, 0x40, 0x10, 0x7c], [0, 0, r.getrandbits(32) | 1])
    return Call(0x68b9f0, [], regions, conv='c', ret='a')


def case(r):
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: imports(log)}
    return globals()[r.deal(OPS)](r, log, regions)


if __name__ == '__main__':
    B.main(case)
