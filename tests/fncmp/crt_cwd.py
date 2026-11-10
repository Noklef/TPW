# The C library's current directory (src/game/core/crtopen): _getcwd 0x67b9f0, _getdcwd_lk 0x67ba20, _validdrive 0x67bb50, _fullpath 0x689490, _getdrive 0x689560, against the originals, over malloc and errno;
# the locks, _getptd and _nh_malloc cdecl recorders; GetFullPathNameA, GetCurrentDirectoryA and GetDriveTypeA COM recorders in their slots (the paths they fill as given).
# usage: crt_cwd.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, fill, heap, import_table, log_region, set_return  # noqa: E402

GETPTD, LOCK, UNLOCK, NHMALLOC = 2, 7, 8, 28
FULL, CWD, DRIVE, LASTERR = 14, 15, 16, 17
B = Bench([(0x67b9f0, 0x67ba1f), (0x67ba20, 0x67bb4b), (0x67bb50, 0x67bb90), (0x67dcd0, 0x67dcf0), (0x686a00, 0x686a19), (0x689490, 0x68955a), (0x689560, 0x6895a3), (0x67f1a0, 0x67f222),
           (0x686980, 0x6869f3)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True),
                 0x67ad90: _free})
PTD, OUT, BLOCK, PATH = RAM, RAM + 0x100, RAM + 0x400, RAM + 0x800
OPS = [n for n in ('getcwd', 'getdcwd', 'getdcwd', 'validdrive', 'fullpath', 'fullpath', 'getdrive') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def path(r):
    text = b'C:\\' + bytes(r.choice(b'abcXYZ\\.') for _ in range(r.choice([0, 1, 5, 0x20, 0x60])))
    return (text + b'\0')[:0x6f] + b'\0'


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, PTD: r.bytes(0x20), OUT: r.bytes(0x200), BLOCK: r.bytes(0x200), HEAP: heap(0x40), 0xfb1440: bytes(4),
               IMPORT_TABLE: import_table(log, {'full': (FULL, 0x6fd214, 4, 0b1, 0b1100), 'cwd': (CWD, 0x6fd118, 2, 0, 0b10), 'drive': (DRIVE, 0x6fd09c, 1, 0b1, 0),
                                                'lasterr': (LASTERR, 0x6fd138, 0, 0, 0)})}
    set_return(log, GETPTD, PTD)
    set_return(log, NHMALLOC, r.choice([BLOCK, BLOCK, 0]))
    set_return(log, DRIVE, r.choice([0, 1, 2, 3, 3, 5]))
    for k in (FULL, CWD):
        set_return(log, k, r.choice([0, 3, 0x10, 0x20, 0x60, 0x103, 0x104, 0x200, r.getrandbits(32)]))
    fill(log, FULL, 2, path(r), which=0)
    fill(log, CWD, 1, path(r), which=1)
    regions[0xfb1548] = struct.pack('<I', r.randrange(2))
    buffer = r.choice([OUT, OUT, 0])
    size = r.choice([0, 1, 4, 0x10, 0x20, 0x61, 0x104, 0x200, 0xffffffff, r.getrandbits(32)])
    if name == 'getcwd':
        return Call(0x67b9f0, [buffer, size], regions, conv='c', ret='a')
    if name == 'fullpath':
        regions[PATH] = r.choice([b'', b'a\\b', b'C:\\x\\..\\y', b'\\\\srv\\s']) + b'\0'
        set_return(log, LASTERR, r.choice([2, 3, 5]))
        return Call(0x689490, [buffer, r.choice([PATH, PATH, PATH, 0]), size], regions, conv='c', ret='a')
    if name == 'getdrive':   # (the "C" locale: toupper folds a-z only)
        fill(log, CWD, 1, bytes([r.choice(b'aCz\\1')]) + r.choice([b':', b':', b'\\', b'']) + b'\\x\0', which=1)
        return Call(0x689560, [], regions, conv='c', ret='a')
    drive = r.choice([0, 0, 1, 3, 26, 0xc0, r.getrandbits(32)])
    if name == 'validdrive':
        return Call(0x67bb50, [drive], regions, conv='c', ret='a')
    return Call(0x67ba20, [drive, buffer, size], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
