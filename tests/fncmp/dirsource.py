# The directory source (src/game/files/dirsource) against the originals: a DirSource at THIS (the game's vtable, which the driver points at the replacements;
# its folder a String in RAM, now and then empty; a search open or not) over the file layer and the file / mmio stream sources, whose Windows and mmio
# imports are all recorders (COM-style, stdcall, fncmp import_table()): path arguments logged as their text (they are stack buffers, the heap's String data
# or the resolved-path globals), FindFirstFileA / FindNextFileA filling the WIN32_FIND_DATA through their stack out argument (attributes, times, sizes, a
# name of up to 0x40 bytes), InterlockedDecrement now and then 0 (the failed source freed). The stream Open is handed has a recorder vtable and usually no
# source; the base directory a short path, so that Path_Resolve resolves against it.
# usage: dirsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, fill, heap, import_table, log_region, set_return, vtable  # noqa: E402

B = Bench([(0x4217e0, 0x4217f0), (0x619950, 0x61a060), (0x5f5660, 0x5f5fc0), (0x636e40, 0x637460), (0x618cd0, 0x619950), (0x6307c0, 0x630800), (0x5f5f00, 0x5f9000),
           (0x67a400, 0x690000)])
THIS, RESULT, ENTRY, FIND = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x300
STREAM, SRC, VT_STREAM, NAME, NAME2, FOLDER = RAM + 0x480, RAM + 0x4c0, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x800
GLOBALS = 0xfa6ec8   # the command line (0x104), + 0x108 the base directory, + 0x210 the path buffer
PATHS = 0xfaa2c8   # the file source's resolved path (0x104), + 0x108 the mmio source's
DIR_VTABLE, STRING_VTABLE = 0x7038d0, 0x703090
# The imports: slot, import table address, stack arguments, string arguments (mask), out arguments (mask).
IMPORTS = {'CreateFileA': (40, 0x6fd0d8, 7, 1, 0), 'GetFileSize': (41, 0x6fd0e8, 2, 0, 0), 'CloseHandle': (42, 0x6fd13c, 1, 0, 0),
           'CreateSemaphoreA': (43, 0x6fd16c, 4, 8, 0), 'mmioOpenA': (44, 0x6fd3e4, 3, 1, 0), 'mmioClose': (45, 0x6fd3f0, 2, 0, 0),
           'mmioSetBuffer': (46, 0x6fd3e8, 4, 0, 0), 'CreateDirectoryA': (47, 0x6fd120, 2, 1, 0), 'RemoveDirectoryA': (48, 0x6fd114, 1, 1, 0),
           'SetFileAttributesA': (49, 0x6fd108, 2, 1, 0), 'FindFirstFileA': (50, 0x6fd100, 2, 1, 2), 'FindClose': (51, 0x6fd104, 1, 0, 0),
           'GetFileAttributesA': (52, 0x6fd10c, 1, 1, 0), 'DeleteFileA': (53, 0x6fd0c4, 1, 1, 0), 'MoveFileA': (54, 0x6fd11c, 2, 3, 0),
           'FindNextFileA': (55, 0x6fd0c8, 2, 0, 2), 'InterlockedIncrement': (56, 0x6fd0c0, 1, 0, 0), 'InterlockedDecrement': (57, 0x6fd12c, 1, 0, 0),
           'GetCommandLineA': (58, 0x6fd1e4, 0, 0, 0), 'SetFilePointer': (59, 0x6fd0e4, 4, 0, 0)}
K = {name: v[0] for name, v in IMPORTS.items()}
FOLDERS = [b'C:\\tp\\data\\', b'C:\\tp\\', b'D:/x/', b'rel\\', b'', b'C:\\tp\\data']
NAMES = [b'x.wad', b'a', b'data\\x.wad', b'..\\up.txt', b'', b'*.wad', b'sub/', b'C:\\abs.txt', b'readme.txt', b'*', b'a/b/*.bin']
# name: (address, stack arguments: R the result, S the stream, N a name, M a second name, m a mode, a attributes, d a dword, E the entry, F the find data,
# P a path to mount; conv; returns)
OPS = {
    'Ctor': (0x619950, '', 't', 'a'), 'DeletingDtor': (0x619970, 'f', 't', 'a'), 'Dtor': (0x619990, '', 't', 'v'),
    'FindFirst': (0x6199e0, 'RNE', 't', 'a'), 'FindNext': (0x619b30, 'RE', 't', 'a'), 'Close': (0x619ba0, '', 't', 'v'),
    'FillEntry': (0x619bc0, 'EF', 's', 'v'), 'Open': (0x619c20, 'RSNmd', 't', 'a'), 'Mount': (0x619d70, 'RP', 't', 'a'),
    'SetAttributes': (0x619e10, 'RNa', 't', 'a'), 'MakeDir': (0x619e90, 'RN', 't', 'a'), 'Remove': (0x619f10, 'RN', 't', 'a'),
    'Rename': (0x619fb0, 'RNM', 't', 'a'),
}


def find_data(r):
    """WIN32_FIND_DATA's first 0x70 bytes: attributes, the times, the sizes, reserved, a name (NUL-terminated within them: the rest of a filled one is the
    stack's)."""
    name = r.choice([b'x.wad', b'.', b'..', b'data', b'readme.txt', b'', b'a' * 0x40, b'Long Name With Spaces.dat'])
    d = bytearray(r.bytes(0x70))
    struct.pack_into('<I', d, 0, r.choice([0x10, 0x20, 0x80, 0x11, r.getrandbits(32)]))
    d[0x2c:0x2d + len(name)] = name + b'\0'
    return bytes(d)


def case(r):
    op = r.choice(PICK)
    address, args, conv, ret = OPS[op]
    log = log_region()
    folder = r.choice(FOLDERS)
    obj = bytearray(r.bytes(0x40))
    struct.pack_into('<4I', obj, 0, DIR_VTABLE, 0, r.getrandbits(32), r.randrange(4))
    struct.pack_into('<5I', obj, 0x10, STRING_VTABLE, len(folder), FOLDER, len(folder) + r.choice([0, 0, 8]), r.choice([0, 8]))
    struct.pack_into('<I', obj, 0x24, r.choice([0xffffffff, 0xffffffff, 0x1234, 0]))
    for imp, k in K.items():
        if r.random() < 0.85:
            set_return(log, k, r.choice({'CreateFileA': [0xffffffff, 0x77, 0x77], 'FindFirstFileA': [0xffffffff, 0x99, 0x99, 0],
                                         'FindNextFileA': [0, 1, 1], 'GetFileAttributesA': [0xffffffff, 0x10, 0x20, 0x11],
                                         'mmioOpenA': [0, 0x66, 0x66], 'InterlockedDecrement': [0, 1, 1, 2], 'InterlockedIncrement': [1, 2],
                                         'GetCommandLineA': [0]}.get(imp, [0, 1, r.getrandbits(32)])))
    fill(log, K['FindFirstFileA'], 1, find_data(r), 0)
    fill(log, K['FindNextFileA'], 1, find_data(r), 1)
    base = r.choice([b'C:\\tp\\', b'D:\\'])
    globals_ = bytearray(r.bytes(0x318))
    globals_[0:8] = b'tp.exe\0\0'
    globals_[0x108:0x109 + len(base)] = base + b'\0'
    heap_region = heap(0x4000)
    if r.random() < 0.03:
        struct.pack_into('<I', heap_region, 0, 0)   # (the next allocation answers 0)
    regions = {THIS: obj, RESULT: r.bytes(0x10), FOLDER: folder + b'\0' + r.bytes(0x40), LOG: log, HEAP: heap_region, IMPORT_TABLE: import_table(log, IMPORTS),
               GLOBALS: globals_, PATHS: r.bytes(0x210), VT_STREAM: vtable({0: 1, 2: 2, 3: 2, 4: 2, 10: 1}),
               SRC: struct.pack('<3I', VT_STREAM, 0, 1) + r.bytes(0x34)}
    stack = []
    for x in args:
        if x == 'R':
            stack.append(RESULT)
        elif x == 'f':
            stack.append(r.choice([0, 1, 2, 3]))
        elif x == 'S':
            regions[STREAM] = struct.pack('<2I', VT_STREAM, 0 if r.random() < 0.85 else SRC) + r.bytes(0x38)
            stack.append(STREAM)
        elif x in 'NMP':
            at = {'N': NAME, 'M': NAME2, 'P': NAME}[x]
            text = r.choice(FOLDERS + [b'C:\\tp\\data\\sub']) if x == 'P' else r.choice(NAMES)
            regions[at] = text + b'\0' + r.bytes(0x40)
            stack.append(at if r.random() < 0.97 or x != 'N' or op != 'FindFirst' else 0)
        elif x == 'm':
            stack.append(r.choice([0x80000001, 0x80000002, 0x80008001, 0x8000, 1, 2, 0, 0x80000000, r.getrandbits(32)]))
        elif x == 'a':
            stack.append(r.choice([0, 0x10, 0x20, 0x80, 1, r.getrandbits(32)]))
        elif x == 'd':
            stack.append(r.getrandbits(32))
        elif x == 'E':   # a DirEntry: random, a name pointer into it
            e = bytearray(r.bytes(0x200))
            struct.pack_into('<I', e, 0x20, ENTRY + 0x24 + r.choice([0, 0, 4, 0x10]))
            regions[ENTRY] = e
            stack.append(ENTRY)
        elif x == 'F':
            regions[FIND] = find_data(r) + r.bytes(0xd0)
            stack.append(FIND)
    return Call(address, ([] if conv == 's' else [THIS]) + stack, regions, conv=conv, ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
