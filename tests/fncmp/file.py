# The file and system layer (src/game/files/file, 0x5f5660-0x5f5fb0) against the originals: every Windows import it calls through the import table is a
# recorder (COM-style, stdcall) whose path arguments log as a hash of the string (fncmp strings(): the resolved paths live in stack buffers), FindFirstFile
# filling its WIN32_FIND_DATA and the file stream's open slot its result (fncmp fill()); the command line, base directory and path buffer globals at
# 0xfa6ec8 / 0xfa6fd0 / 0xfa70d8 (the base often empty, so that Dir_GetBase builds it from the command line); the clock at 0xfa71e0 as thunks_static's;
# streams of the file stream's vtable (0x6fd930) made of recorders; Mem_Alloc the bench heap (driver.cpp's Files env).
# usage: file.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Load,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import IMPORT_TABLE as IAT, LOG, Bench, Call, HEAP, RAM, counter, fill, heap, import_table, put, set_return, strings, vtable  # noqa: E402
from thunks_static import CLOCK, QUERY_COUNTER, QUERY_FREQUENCY, TIME_GET_TIME, clock  # noqa: E402

B = Bench([(0x5f5660, 0x5f5fc0), (0x5f7180, 0x5f7190), (0x67a400, 0x690000)])
GLOBALS = 0xfa6ec8   # the command line (0x104), + 0x108 the base directory, + 0x210 the path buffer; the clock follows at 0xfa71e0
BASE = GLOBALS + 0x108
FILE_STREAM_VTABLE = 0x6fd930
PATH, PATH2, COMMAND_LINE = RAM + 0x10, RAM + 0x210, RAM + 0x410
RESULT, DONE, OUT_STREAM, STREAM, TIMES = RAM + 0x800, RAM + 0x810, RAM + 0x820, RAM + 0x900, RAM + 0xc00
OUT, BUFFER = RAM + 0x1000, RAM + 0x2000
# The imports: slot (from 40, clear of the stream vtable's), import table address, stack arguments, string arguments (mask), out arguments (mask).
IMPORTS = {'timeGetTime': (40, 0x6fd3ec, 0, 0, 0), 'GetCommandLineA': (41, 0x6fd1e4, 0, 0, 0), 'CreateDirectoryA': (42, 0x6fd120, 2, 1, 0),
           'RemoveDirectoryA': (43, 0x6fd114, 1, 1, 0), 'SetFileAttributesA': (44, 0x6fd108, 2, 1, 0), 'FindFirstFileA': (45, 0x6fd100, 2, 1, 2),
           'FindClose': (46, 0x6fd104, 1, 0, 0), 'GetFileAttributesA': (47, 0x6fd10c, 1, 1, 0), 'DeleteFileA': (48, 0x6fd0c4, 1, 1, 0),
           'MoveFileA': (49, 0x6fd11c, 2, 3, 0), 'CopyFileA': (50, 0x6fd158, 3, 3, 0), 'CompareFileTime': (51, 0x6fd110, 2, 0, 0),
           'Sleep': (52, 0x6fd1d4, 1, 0, 0), 'GlobalAlloc': (53, 0x6fd1cc, 2, 0, 0), 'GlobalFree': (54, 0x6fd26c, 1, 0, 0)}
K = {name: v[0] for name, v in IMPORTS.items()}
# The stream's slots (thiscall, stack arguments): 0 the deleting destructor, +4 close, +8 read, +0xc write, +0x10 seek, +0x18 tell, +0x24 open (result, path, mode).
STREAM_SLOTS = {0: 1, 2: 2, 3: 2, 4: 2, 9: 3}
OPEN = 9
PATHS = [b'', b'a', b'tp.exe', b'data\\x.wad', b'C:\\games\\tpw\\', b'C:/x', b'c:x', b'C:', b'\\\\srv\\share\\f', b'//srv/f', b'\\/x', b'/\\x', b'\\abs\\path',
         b'/abs', b'\\', b'/', b'x\\', b'x/', b'a:\\', b'::\\']
BASES = [b'', b'', b'C:\\games\\tpw\\', b'D:/x/', b'\\\\srv\\s\\', b'rel\\', b'C:', b'Q', b'C:x\\']
COMMAND_LINES = [b'"C:\\Program Files\\TPW\\tp.exe" -nocd', b'C:\\TPW\\tp.exe -w', b'tp.exe', b'"tp.exe"', b'"', b'', b'C:/a/b/tp.exe', b'C:\\a/b\\c',
                 b'"C:\\a b\\c', b'C:\\x\\ "y\\z"', b'/', b'\\']
# name: (address, stack arguments: R the result, O a buffer for the path, M its size, P a path or none, Q a second path, U the buffer, m a size, D the count's
# out or none, S a stream out, X the stream, d a dword, o an origin, T two file times or none, C the clock (ECX), N a count; conv; returns; paths: s short
# (the resolved path is a stack buffer of 0x104), l any length)
OPS = {
    'GlobalAlloc': (0x5f5660, 'd', 'c', 'a', ''), 'GlobalFree': (0x5f5680, 'd', 'c', 'a', ''), 'SetBase': (0x5f5690, 'P', 'c', 'v', 'l'),
    'GetBase': (0x5f56f0, '', 'c', 'a', 'l'), 'Resolve': (0x5f5710, 'ROMP', 'c', 'a', 'l'), 'Load': (0x5f5830, 'RPUmD', 'c', 'a', 's'),
    'InitBase': (0x5f5950, '', 'c', 'v', 'l'), 'Open': (0x5f5a20, 'RSPd', 'c', 'a', 's'), 'DirCreate': (0x5f5a50, 'RP', 'c', 'a', 'l'),
    'DirRemove': (0x5f5a90, 'RP', 'c', 'a', 'l'), 'StreamOpen': (0x5f5ad0, 'RSPd', 'c', 'a', 's'), 'Close': (0x5f5b80, 'RX', 'c', 'a', ''),
    'Tell': (0x5f5bb0, 'X', 'c', 'a', ''), 'TimeCompare': (0x5f5bc0, 'TT', 'c', 'a', ''), 'Seek': (0x5f5bf0, 'Xdo', 'c', 'a', ''),
    'SetAttributes': (0x5f5c10, 'RPd', 'c', 'a', 's'), 'GetSize': (0x5f5c70, 'P', 'c', 'a', 's'), 'Exists': (0x5f5cd0, 'P', 'c', 'a', 's'),
    'IsDirectory': (0x5f5d10, 'P', 'c', 'a', 's'), 'Delete': (0x5f5d60, 'RP', 'c', 'a', 's'), 'Move': (0x5f5dc0, 'RPQ', 'c', 'a', 's'),
    'Copy': (0x5f5df0, 'RPQ', 'c', 'a', 's'), 'Read': (0x5f5e20, 'RXUND', 'c', 'a', ''), 'Write': (0x5f5e50, 'RXUND', 'c', 'a', ''),
    'TimerInit': (0x5f5e80, 'C', 'f', 'v', ''), 'TimerGetMs': (0x5f5f10, 'C', 'f', 'a', ''), 'TimeInit': (0x5f5f90, '', 'c', 'v', ''),
    'TimeGetMs': (0x5f5fa0, '', 'c', 'a', ''), 'Sleep': (0x5f5fb0, 'd', 'c', 'v', ''),
}


def text(r, choices, room):
    """A NUL-terminated string from choices (or random bytes), at most room - 1 long, padded to room with random bytes."""
    s = r.choice(choices) if r.random() < 0.85 else bytes(r.randrange(0x20, 0x7f) for _ in range(r.randrange(0x20)))
    if r.random() < 0.15:
        s = s + bytes(r.choice(b'ab\\/:.') for _ in range(r.randrange(room)))
    s = s[:room - 1]
    return s + b'\0' + r.bytes(room - len(s) - 1)


def case(r):
    name = r.choice(PICK)
    address, args, conv, ret, paths = OPS[name]
    regions = {HEAP: heap()}
    clock(r, regions, name in ('TimerGetMs', 'TimeGetMs'))   # (the clock, its log and timeGetTime at slot 40)
    log = regions[LOG]
    if name in ('TimerInit', 'TimeInit'):   # the frequency (the counter's first value) at Timer_Init's bounds: the divisor halved to 125 or past it, 10 and 32000000 ticks
        freq = r.choice([0, 1, 9999, 10000, 10999, 3579545, 1193182, 32000000000, 32000000007, 32000000008, 32000001000, 31999999999, 2 ** 34, 2 ** 34 + r.getrandbits(34),
                         2 ** 35 - 1, 2 ** 35, 2 ** 32 - 1, 2 ** 32, r.getrandbits(r.choice([16, 32, 36, 40, 64]))])
        counter(log, freq, r.getrandbits(r.choice([8, 32, 64])), r.getrandbits(2) if r.random() < 0.4 else 3)
    iat = import_table(log, IMPORTS)
    for at in (QUERY_FREQUENCY, QUERY_COUNTER, TIME_GET_TIME):   # (clock()'s slots move into the table: the counter, and timeGetTime the same recorder)
        iat[at - IAT:at - IAT + 4] = regions.pop(at)
    regions[IAT] = iat
    # The globals: a long base only where the resolved path goes to OUT or the path buffer (the stack buffers are 0x104).
    long = paths == 'l'
    base = text(r, BASES + [b'B' * 0x102 + b'\\', b'C:' + b'b' * 0x100], 0x104) if long else text(r, BASES, 0x60) + r.bytes(0xa4)
    glob = bytearray(text(r, COMMAND_LINES, 0x104) + r.bytes(4) + base + r.bytes(4) + r.bytes(0x104) + r.bytes(4))
    regions[GLOBALS] = glob
    path_room = 0x1f0 if long else 0x40
    strs = bytearray(r.bytes(0x800))
    strs[0x10:0x10 + path_room] = text(r, PATHS + ([b'P' * 0x103, b'P' * 0x102 + b'\\', b'P' * 0x104, b'P' * 0x103 + b'\\', b'P' * 0x101 + b'/'] if long else []), path_room)
    strs[0x210:0x250] = text(r, PATHS, 0x40)
    strs[0x410:0x610] = (text(r, COMMAND_LINES + [b'"' + b'x' * 0x150 + b'\\y"', b'x' * 0x110 + b'\\y'], 0x200) if long else
                         text(r, COMMAND_LINES, 0x60) + r.bytes(0x1a0))   # (an empty base is built from it)
    regions[RAM] = strs
    regions[RAM + 0x800] = bytearray(r.bytes(0x800))   # the result, the count, the stream out, the stream, the file times
    put(regions[RAM + 0x800], 0x100, 'I', FILE_STREAM_VTABLE)
    regions[OUT] = r.bytes(0x400)
    regions[BUFFER] = r.bytes(0x400)
    regions[FILE_STREAM_VTABLE] = vtable(STREAM_SLOTS)
    # What the imports and the stream return.
    count = r.choice([0, 1, 4, 0x10, 0x100, r.getrandbits(32)])
    set_return(log, K['GetCommandLineA'], COMMAND_LINE)
    size = r.choice([0, 1, 0x40, 0x3ff, 0x400, 0x401, 0xffffffff, 0x7fffffff, 0x80000000, r.getrandbits(32)])
    fill(log, K['FindFirstFileA'], 1, r.bytes(0x20) + struct.pack('<I', size) + r.bytes(0x10), 0)
    fill(log, OPEN, 0, struct.pack('<i', r.choice([0, 0, -1, 1, -5, -0x80000000])), 1)
    strings(log, OPEN, 2, 1)
    choices = {'FindFirstFileA': [0xffffffff, 0xffffffff, 0x1234, r.getrandbits(32)], 'GetFileAttributesA': [0xffffffff, 0x10, 0, 0x20, 0x11, 0x80, r.getrandbits(32)],
               'GlobalAlloc': [0, HEAP + 0x100]}
    for imp, k in K.items():
        if imp not in ('GetCommandLineA', 'timeGetTime') and r.random() < 0.8:
            set_return(log, k, r.choice(choices.get(imp, [0, 1, 1, r.getrandbits(32)])))
    for slot, values in [(2, [count, count, 0, count // 2, -1]), (3, [count, 0, -1]), (4, [0, 1, r.getrandbits(32)]), (6, [0, 0x40, r.getrandbits(32)])]:
        if r.random() < 0.85:
            set_return(log, slot, r.choice(values))
    stack = []
    for a in args:
        stack.append({'R': lambda: RESULT, 'O': lambda: OUT, 'M': lambda: 0x104, 'P': lambda: PATH if name != 'Resolve' or r.random() < 0.9 else 0,
                      'Q': lambda: PATH2, 'U': lambda: BUFFER, 'm': lambda: r.choice([0, 0x40, 0x3ff, 0x400, 0x7fffffff, -1, r.getrandbits(32)]),
                      'D': lambda: DONE if r.random() < 0.8 else 0, 'S': lambda: OUT_STREAM, 'X': lambda: STREAM, 'o': lambda: r.choice([0, 1, 2, 3]),
                      'd': lambda: r.choice([0, 1, 0x10, 0x80000001, r.getrandbits(32)]), 'N': lambda: count, 'C': lambda: CLOCK,
                      'T': lambda: TIMES + 8 * r.randrange(2) if r.random() < 0.8 else 0}[a]())
    return Call(address, stack, regions, conv=conv, ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
