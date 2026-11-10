# The C library's current directory (src/game/core/crtopen): _getcwd 0x67b9f0, _getdcwd_lk 0x67ba20, _validdrive 0x67bb50, _fullpath 0x689490, _getdrive 0x689560, _stat 0x67e900, against the originals,
# over malloc and errno; the locks, _getptd, _nh_malloc and tzset cdecl recorders; GetFullPathNameA, GetCurrentDirectoryA, GetDriveTypeA, FindFirstFileA, FindClose and the file time conversions
# COM recorders in their slots (the paths and structures they fill as given).
# usage: crt_cwd.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, fill, heap, import_table, log_region, set_return  # noqa: E402

GETPTD, LOCK, UNLOCK, NHMALLOC, TZSET = 2, 7, 8, 28, 29
FULL, CWD, DRIVE, LASTERR, FIND, CLOSE, LOCALFT, SYSTIME = 14, 15, 16, 17, 18, 19, 20, 21
B = Bench([(0x67b9f0, 0x67ba1f), (0x67ba20, 0x67bb4b), (0x67bb50, 0x67bb90), (0x67dcd0, 0x67dcf0), (0x686a00, 0x686a19), (0x689490, 0x68955a), (0x689560, 0x6895a3), (0x67f1a0, 0x67f222),
           (0x686980, 0x6869f3), (0x67e900, 0x67ec3f), (0x689650, 0x6896fd), (0x67d7f0, 0x67d82a), (0x6895b0, 0x68964e), (0x67ec40, 0x67ecbf), (0x67e830, 0x67e8f2), (0x687c50, 0x687d32),
           (0x687d40, 0x687d72), (0x689400, 0x689490), (0x689700, 0x689712), (0x689720, 0x68975a), (0x67b5f0, 0x67b61e), (0x689250, 0x6893f5),
           (0x67df40, 0x67e010), (0x688f50, 0x689175), (0x689180, 0x6891af), (0x689e90, 0x689edd), (0x689ee0, 0x689f18), (0x689f40, 0x689f72)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True),
                 0x67ad90: _free, 0x687d80: _recorder(TZSET, 0, cdecl=True)})
PTD, OUT, BLOCK, PATH, STAT = RAM, RAM + 0x100, RAM + 0x400, RAM + 0x800, RAM + 0x900
OPS = [n for n in ('getcwd', 'getdcwd', 'getdcwd', 'validdrive', 'fullpath', 'fullpath', 'getdrive', 'stat', 'stat', 'stat') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def filetime(r):
    return struct.pack('<2I', *r.choice([(0, 0), (0, 0), (1, 0), (0, 1), (r.getrandbits(32), r.getrandbits(32))]))


def stat(r, log, regions):
    """_stat: wildcards and drive letters in the path (no lead bytes: _mbctolower's LCMapStringA path is crt_mbstring's); FindFirstFileA's data (times none at times), or none and the root's checks; the time conversions failing now and then
    (their tables the exe's, daylight off, tzset done)."""
    regions[PATH] = r.choice([b'C:\\x', b'c:', b'C:', b'x?y', b'a*', b'\\\\srv\\s', b'.', b'..', b'C:\\', b'abc', b'\\', b'z:\\d\\', b'\x81:', b':', b'\0:']) + b'\0'   # ("\0:": empty, its ':' past the end)
    regions.update({0xfb1870: struct.pack('<I', r.choice([0, 0, 0, 932])), 0xfb1668: bytes(0x101), 0xfb1510: struct.pack('<I', 1), 0x7813dc: bytes(4), 0x7813d8: struct.pack('<i', r.choice([0, -36000, 18000])),
                    STAT: r.bytes(0x24)})
    set_return(log, FIND, r.choice([0x1234, 0xffffffff, 0xffffffff]))
    fill(log, FIND, 1, struct.pack('<I', r.choice([0x10, 0x20, 1, 0x21])) + filetime(r) + filetime(r) + filetime(r) + struct.pack('<2I', r.getrandbits(32), r.getrandbits(32)), which=2)
    set_return(log, LOCALFT, r.choice([1, 1, 1, 1, 0]))
    fill(log, LOCALFT, 1, r.bytes(8), which=3)
    set_return(log, SYSTIME, r.choice([1, 1, 1, 1, 0]))
    fill(log, SYSTIME, 1, b''.join(struct.pack('<8H', r.choice([1969, 1970, 1980, 1998, 2038, 2039, 0x7bc]), r.randrange(1, 13), 0, r.randrange(1, 29), r.randrange(24), r.randrange(60), r.randrange(60), 0) for _ in range(3)), which=4, records=3)
    return Call(0x67e900, [PATH, STAT], regions, conv='c', ret='a')


def path(r):
    text = b'C:\\' + bytes(r.choice(b'abcXYZ\\.') for _ in range(r.choice([0, 1, 5, 0x20, 0x60])))
    return (text + b'\0')[:0x6f] + b'\0'


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, PTD: r.bytes(0x20), OUT: r.bytes(0x200), BLOCK: r.bytes(0x200), HEAP: heap(0x40), 0xfb1440: bytes(4),
               IMPORT_TABLE: import_table(log, {'full': (FULL, 0x6fd214, 4, 0b1, 0b1100), 'cwd': (CWD, 0x6fd118, 2, 0, 0b10), 'drive': (DRIVE, 0x6fd09c, 1, 0b1, 0),
                                                'lasterr': (LASTERR, 0x6fd138, 0, 0, 0), 'find': (FIND, 0x6fd100, 2, 0b1, 0b10), 'close': (CLOSE, 0x6fd104, 1, 0, 0),
                                                'localft': (LOCALFT, 0x6fd15c, 2, 0, 0b11), 'systime': (SYSTIME, 0x6fd0bc, 2, 0, 0b11)})}
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
    if name == 'stat':
        set_return(log, LASTERR, r.choice([2, 3, 5]))
        return stat(r, log, regions)
    if name == 'getdrive':   # (the "C" locale: toupper folds a-z only)
        fill(log, CWD, 1, bytes([r.choice(b'aCz\\1')]) + r.choice([b':', b':', b'\\', b'']) + b'\\x\0', which=1)
        return Call(0x689560, [], regions, conv='c', ret='a')
    drive = r.choice([0, 0, 1, 3, 26, 0xc0, r.getrandbits(32)])
    if name == 'validdrive':
        return Call(0x67bb50, [drive], regions, conv='c', ret='a')
    return Call(0x67ba20, [drive, buffer, size], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
