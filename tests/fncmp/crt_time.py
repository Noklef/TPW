# The C library's time text and fields (src/game/core/crttime: _Strftime and its parts, the copies of the locale's names, __tzset, cvtdate, gmtime) against the originals:
# the locale's lock calls COM recorders, the locks, _getptd, _tzset and _nh_malloc cdecl recorders; the names, the struct tm and the pictures made here.
# usage: crt_time.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, import_table, log_region, set_return  # noqa: E402

GETPTD, LOCK, UNLOCK, NHMALLOC, TZSET, INC, DEC = 2, 7, 8, 28, 29, 14, 15
B = Bench([(0x6902b0, 0x690394), (0x6903a0, 0x69048a), (0x690490, 0x6907f2), (0x690800, 0x690941), (0x690950, 0x690eee), (0x690ef0, 0x690f1d), (0x690f20, 0x690fb3), (0x690fc0, 0x691025), (0x691030, 0x6912dc), (0x687d40, 0x687d72),
           (0x688300, 0x68849c), (0x6884a0, 0x688608), (0x67df40, 0x67e010), (0x67dcd0, 0x67dcf0)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x687d80: _recorder(TZSET, 0, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True)})
WHEN, PTD, OUTVARS, OUT, FORMAT, NAMES, TZ, PCTYPE, STRINGS, BLOCK = RAM, RAM + 0x40, RAM + 0x90, RAM + 0x100, RAM + 0x400, RAM + 0x600, RAM + 0x700, RAM + 0x800, RAM + 0xc00, RAM + 0x3000
OPS = [n for n in ('strftime', 'strftime', 'strftime', 'expandtime', 'store_winword', 'store_str', 'store_num', 'store_number', 'gettnames', 'getdays', 'getmonths', 'tzset', 'cvtdate', 'gmtime') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
PICTURES = [b'd', b'dd', b'ddd', b'dddd', b'ddddd', b'M', b'MM', b'MMM', b'MMMM', b'MMMMM', b'y', b'yy', b'yyy', b'yyyy', b'H', b'HH', b'HHH', b'h', b'hh', b'hhh', b'm', b'mm', b'mmm', b's', b'ss', b'sss', b't', b'tt', b'AM/PM', b'am/pm',
            b'A/P', b'a/p', b'A', b'a', b'aXm', b"'", b"''", b"'''", b"'te", b"xt'", b' ', b'/', b':', b'\x81x', b'\x81d', b"'\x81x'"]
SPECS = b'aAbBcdHIjmMpSUwWxXyYzZ%q'
LEAP = [-1, 30, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366, 0x7fffffff]
PLAIN = [-1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365, 0x7fffffff]
FOUR, YEAR, LEAPYEAR, DAY = 0x7861f80, 0x1e13380, 0x1e28500, 86400


def picture(r):
    return b''.join(r.choice(PICTURES) for _ in range(r.randrange(6)))


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0)})}
    regions[PCTYPE] = struct.pack('<256H', *[0x8000 if i >= 0x81 else 0x20 for i in range(256)])
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[0xfb1440] = bytes(4)
    regions[0xfb3ca4] = struct.pack('<2I', r.choice([0, 0, 1]), r.randrange(4))
    regions[0xfb1510] = struct.pack('<I', r.choice([0, 0, 1]))
    regions[0xfb192c] = struct.pack('<2I', r.randrange(2), r.randrange(2))
    regions[0xfb1548] = struct.pack('<I', r.randrange(2))
    strings = bytearray()
    pointers = []
    for i in range(43):
        pointers.append(STRINGS + len(strings))
        strings += (picture(r) if i >= 40 else r.choice([b'', b'n', b'Sunday', b'name%d' % i])) + b'\0'
    regions[STRINGS] = bytes(strings) + bytes(8)
    regions[NAMES] = struct.pack('<43I', *pointers)
    regions[0x781f8c] = struct.pack('<I', NAMES)
    regions[0x781468] = struct.pack('<2I', TZ, TZ + 0x10)
    regions[TZ] = b'PST\0'.ljust(0x10, b'\0') + b'PDT and more\0'
    set_return(log, NHMALLOC, r.choice([BLOCK, BLOCK, BLOCK, 0]))
    regions[BLOCK] = bytes(0x800)
    if name in ('gettnames', 'getdays', 'getmonths'):
        return Call({'gettnames': 0x690490, 'getdays': 0x6902b0, 'getmonths': 0x6903a0}[name], [], regions, conv='c', ret='a')
    if name == 'tzset':
        return Call(0x687d40, [], regions, conv='c')
    if name == 'cvtdate':
        regions[0x781470] = r.bytes(0x20)
        regions[0x7813e0] = struct.pack('<i', r.choice([0, 3600, 18000, -36000, -3600, 86400, 0x7000]))
        regions[0x781514] = struct.pack('<14i', *LEAP) + struct.pack('<14i', *PLAIN)
        year = r.choice([70, 71, 72, 99, 100, 101, 104, 199, 200, r.randrange(70, 300)])
        return Call(0x688300, [r.choice([0, 1, 1, 2]), r.choice([0, 1, 1, 1, 2]), year, r.randrange(12), r.choice([1, 2, 3, 4, 5, 5]), r.randrange(7), r.choice([0, 1, 15, 31, 366]),
                               r.choice([0, 1, 2, 23, 24, 0xffffffff]), r.choice([0, 30, 59]), r.choice([0, 59, 86399]), r.choice([0, 1, 999])], regions, conv='c')
    if name == 'gmtime':
        regions[PTD] = struct.pack('<I', r.choice([0, 0, BLOCK + 0x100])).rjust(0x44, b'\0')
        set_return(log, GETPTD, PTD)
        regions[0xfb1518] = r.bytes(0x24)
        regions[BLOCK + 0x100] = r.bytes(0x24)
        regions[0x781514] = struct.pack('<14i', *LEAP) + struct.pack('<14i', *PLAIN)
        years = r.randrange(0, 0x10)
        when = r.choice([0, 1, DAY - 1, DAY, YEAR - 1, YEAR, 2 * YEAR, 2 * YEAR - 1, 2 * YEAR + LEAPYEAR - 1, 2 * YEAR + LEAPYEAR, FOUR - 1, FOUR, years * FOUR + 2 * YEAR + r.randrange(LEAPYEAR), years * FOUR + r.randrange(FOUR), 0x7fffffff, -1, -0x80000000, r.getrandbits(31)])
        regions[WHEN] = struct.pack('<i', when)
        return Call(0x6884a0, [WHEN], regions, conv='c', ret='a')
    regions[WHEN] = struct.pack('<9i', r.choice([0, 5, 59, 123, -1]), r.choice([0, 7, 59, -3]), r.choice([0, 1, 11, 12, 13, 23, 24, -1, -12]), r.choice([1, 9, 10, 31, 0]), r.randrange(12), r.choice([0, 5, 69, 99, 100, 101, 199, 200, 205, -1, -101, 1000, 8100]),
                                r.randrange(7), r.choice([0, 1, 5, 6, 7, 8, 13, 14, 99, 100, 365]), r.choice([0, 1, -1]))
    regions[OUT] = r.bytes(0x200)
    size = r.choice([0, 1, 2, 3, 4, 5, 8, 0x10, 0x40, 0x100])
    regions[OUTVARS] = struct.pack('<2I', OUT + 8, size)
    if name == 'store_str':
        regions[FORMAT] = r.choice([b'', b'n', b'long enough text']) + b'\0'
        return Call(0x690ef0, [FORMAT, OUTVARS, OUTVARS + 4], regions, conv='c')
    if name == 'store_num':
        return Call(0x690f20, [r.choice([0, 7, 12, 123, 4567, 0x7fffffff, 0xffffffff, 0xffffff85]), r.choice([1, 2, 3, 4, 8]), OUTVARS, OUTVARS + 4], regions, conv='c')
    if name == 'store_number':
        return Call(0x690fc0, [r.choice([0, 7, 12, 123, 4567, 0x7fffffff, 0xffffffff, 0xffffff85]), OUTVARS, OUTVARS + 4], regions, conv='c')
    if name == 'expandtime':
        if size == 0:   # (a %% with no room leaves the count below 0: strftime never asks it)
            regions[OUTVARS] = struct.pack('<2I', OUT + 8, 1)
        return Call(0x690950, [r.choice(SPECS) | r.choice([0, 0x4100]), WHEN, OUTVARS, OUTVARS + 4, NAMES], regions, conv='c')
    if name == 'store_winword':
        regions[FORMAT] = picture(r) + bytes(0x10)
        return Call(0x691030, [FORMAT, WHEN, OUTVARS, OUTVARS + 4, NAMES], regions, conv='c')
    pieces = []
    for _ in range(r.randrange(1, 8)):
        kind = r.randrange(5)
        if kind < 3:
            pieces.append(b'%' + (b'#' if r.random() < 0.3 else b'') + bytes([r.choice(SPECS)]))
        elif kind == 3:
            pieces.append(r.choice([b'lit', b' ', b'\x81x', b'%%']))
        else:
            pieces.append(b'\xe9')
    regions[FORMAT] = b''.join(pieces) + r.choice([b'', b'', b'%', b'%#']) + bytes(0x10)
    return Call(0x690800, [OUT + 8, size, FORMAT, WHEN, r.choice([NAMES, NAMES, 0])], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
