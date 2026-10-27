# The file library's open paths against the originals: StreamBase_OpenMemory, Handle_Open28 / OpenE0 (a directory or WAD source made and mounted /
# opened), FileStream_OpenPath (the folder opened as a directory source, the name through it), StreamBase_OpenIn / OpenBuffered (src/game/files/handles,
# filestream, streamopen), and RefObject2's constructor, destructors and Vfn5 (the base of the sources those make). The whole chain below them runs (the
# sources, the WAD directory, the strings, Path_Resolve); the Windows and mmio imports are recorders (COM-style, fncmp import_table()): paths logged as
# text, CreateFileA a handle or INVALID_HANDLE_VALUE, GetFileAttributesA a file, a folder or none, SetFilePointer mostly where it is sent, ReadFile filling
# a WAD header (DWFB, versions 0-3, mostly no entries and no name block) and reporting all it was asked for read (fill_arg()), IsDBCSLeadByte one answer
# for a case, InterlockedDecrement now and then 0 (the object freed). The stream or handle at THIS has a source / object or not; its vtable is the
# game's or recorders. AddFolder / AddWad (src/game/core/class6fd9d8) open a new folder / WAD handle on a path String and append it to a list of 0-2 nodes; AddBoth
# (0x41cc40) adds the path as a folder and, with the game's ".wad" string (0x78bb08) appended, as a WAD; AddIn (0x41cd50) the same under the game's
# second root (the String at 0xf816d4: empty, ending in a backslash or not), the global backslash String (0xf80be0) put between; AddBothIn (0x41cc20)
# AddBoth and then AddIn. MakeFolders (0x5c7ac0, files/gamepaths): a
# root (a wide String; WideCharToMultiByte a recorder answering one path, the GlobalAlloc / GlobalFree stubs) with the String at 0xf81b48 ("\"
# mostly) and each of 0-3 listed names (0x786b90) appended, opened as a folder and, when that fails, created (CreateDirectoryA a recorder).
# SetEntry (0x41ec30, core/class6fd9d8): a Class6fdaa4 (its name and path Strings of a few texts, its handle at +0x2c as THIS's handle case)
# from a directory entry whose name ends in ".wad" (either case) or not, and a path String as AddFolder's; the ".wad" Strings 0x78bb08 and 0xf80808.
# Ctor6fdaa4 (0x41dc80): SetEntry's case on a Class6fdaa4 constructed first over random bytes (its Strings and handle new).
# usage: fileopen.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open28,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, echo, fill, fill_arg, global_heap, heap, import_table, log_region, put, set_return, vtable  # noqa: E402

RANGES = [(0x41c9e0, 0x41ce90), (0x5c5480, 0x5c5570), (0x424cf0, 0x424d20), (0x4036a0, 0x4036c0), (0x5da3c0, 0x5da3d0), (0x638ef0, 0x639230), (0x62fa40, 0x630000), (0x636e40, 0x637460), (0x45f940, 0x45fa40), (0x61a060, 0x61af40), (0x45d820, 0x45d880),
          (0x4217e0, 0x421800), (0x637460, 0x638f00), (0x641500, 0x642400), (0x5f5660, 0x5f9000), (0x618000, 0x61a060), (0x6307c0, 0x630900),
          (0x6ad800, 0x6ad820), (0x67a400, 0x690000), (0x405000, 0x406000), (0x5e4000, 0x5e5000), (0x636b00, 0x636e40),
          (0x5c7ac0, 0x5c7ba6), (0x424db0, 0x424e10), (0x41ec30, 0x41edd3), (0x41eb90, 0x41ec25), (0x424d30, 0x424db0), (0x5c53c0, 0x5c5480), (0x41dc80, 0x41dcf2)]
B = Bench(RANGES)
THIS, RESULT, OTHER, OBJ, SRC = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xc0, RAM + 0x100
VT_THIS, VT_OBJ, VT_SRC = RAM + 0x200, RAM + 0x280, RAM + 0x300
NAME, CONTAINER, TEXT, BUF = RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700
LIST, NODES, PATHSTR = RAM + 0x800, RAM + 0x840, RAM + 0x880   # the held-handle list at +0x2c, its nodes, a path String
WAD_EXT = 0x78bb08   # the game's ".wad" String
ROOT2, SLASH = 0xf816d4, 0xf80be0   # the game's second root and the global backslash (Strings)
GLOBALS = 0xfa6ec8   # the command line (0x104), + 0x108 the base directory, + 0x210 the path buffer
PATHS = 0xfaa2c8   # the file source's resolved path (0x104), + 0x108 the mmio source's
LOCALE = 0xfb1440   # the CRT's LC_CTYPE handle: 0, the C locale
STREAM_VTABLE, FILESTREAM_VTABLE, HANDLE_VTABLE, REFOBJECT2_VTABLE, STRING_VTABLE = 0x703100, 0x6fd930, 0x703180, 0x703898, 0x703090
# The recorder vtables' stack arguments: a Handle's (Reset, Vfn1 .. Vfn10, the deleting destructor), a RefObject2's (the deleting destructor, the
# forwarders' targets, the attach, the teardown), a RefObject's.
HANDLE_SLOTS = {0: 0, 1: 3, 2: 2, 3: 5, 4: 3, 5: 4, 6: 0, 7: 3, 8: 2, 9: 2, 10: 3, 11: 1}
OBJECT_SLOTS = {0: 1, 1: 5, 2: 3, 3: 2, 4: 3, 5: 4, 7: 3, 8: 2, 9: 2, 10: 3, 11: 1}
# The imports: slot, import table address, stack arguments, string arguments (mask), out arguments (mask), BYTE arguments (mask).
IMPORTS = {'CreateFileA': (40, 0x6fd0d8, 7, 1, 0), 'GetFileSize': (41, 0x6fd0e8, 2, 0, 2), 'CloseHandle': (42, 0x6fd13c, 1, 0, 0),
           'CreateSemaphoreA': (43, 0x6fd16c, 4, 8, 0), 'mmioOpenA': (44, 0x6fd3e4, 3, 1, 0), 'mmioClose': (45, 0x6fd3f0, 2, 0, 0),
           'mmioSetBuffer': (46, 0x6fd3e8, 4, 0, 0), 'FindFirstFileA': (47, 0x6fd100, 2, 1, 2), 'FindClose': (48, 0x6fd104, 1, 0, 0),
           'GetFileAttributesA': (49, 0x6fd10c, 1, 1, 0), 'InterlockedIncrement': (50, 0x6fd0c0, 1, 0, 0), 'InterlockedDecrement': (51, 0x6fd12c, 1, 0, 0),
           'GetCommandLineA': (52, 0x6fd1e4, 0, 0, 0), 'SetFilePointer': (53, 0x6fd0e4, 4, 0, 4), 'ReadFile': (54, 0x6fd0dc, 5, 0, 0xa),
           'WaitForSingleObject': (55, 0x6fd128, 2, 0, 0), 'ReleaseSemaphore': (56, 0x6fd170, 3, 0, 4), 'IsDBCSLeadByte': (57, 0x6fd154, 1, 0, 0, 1),
           'WriteFile': (58, 0x6fd0e0, 5, 0, 8), 'FlushFileBuffers': (59, 0x6fd0fc, 1, 0, 0), 'mmioRead': (60, 0x6fd3e0, 3, 0, 0),
           'mmioSeek': (61, 0x6fd3d8, 3, 0, 0), 'WideCharToMultiByte': (62, 0x6fd1dc, 8, 0, 0), 'CreateDirectoryA': (63, 0x6fd120, 2, 1, 0)}
K = {name: v[0] for name, v in IMPORTS.items()}
PATHS_GIVEN = [b'x.wad', b'data\\x.wad', b'C:\\tp\\data\\x.wad', b'data/x.wad', b'\\x.wad', b'/x', b'a\\b/c.bin', b'', b'x.wad*\\data', b'C:\\tp\\',
               b'\x81\\x.bin', b'a\x81\\b', b'data\\', b'D:/y/z.txt']
CONTAINERS = [None, b'', b'C:\\tp\\data', b'x.wad', b'data\\x.wad', b'C:\\tp\\x.wad', b'rel\\dir\\']
# name: (address, this: s a stream, h a handle, o a RefObject2, x room for one; stack arguments: R the result, N a path, C a container, m a mode, f a
# flags word, d a dword, F the deleting destructor's flags, O the other stream, c a capacity, B a text buffer, T a C string or none; returns)
OPS = {
    'OpenMemory': (0x5f72c0, 's', 'Rd', 'a'), 'Open28': (0x5f8510, 'h', 'RN', 'a'), 'OpenE0': (0x5f85d0, 'h', 'RNf', 'a'),
    'OpenPath': (0x5f7ed0, 's', 'RNm', 'a'), 'OpenIn': (0x62fa40, 's', 'RNC', 'a'), 'OpenBuffered': (0x62fd30, 's', 'ROc', 'a'),
    'RefObject2Ctor': (0x619730, 'x', '', 'a'), 'RefObject2DeletingDtor': (0x619790, 'o', 'F', 'a'), 'RefObject2Dtor': (0x6197b0, 'o', '', 'v'),
    'RefObject2Vfn5': (0x6198b0, 'o', 'RBTc', 'a'), 'AddFolder': (0x41c9e0, 'l', 'P', 'v'), 'AddWad': (0x41cb10, 'l', 'P', 'v'),
    'AddBoth': (0x41cc40, 'l', 'P', 'v'), 'AddIn': (0x41cd50, 'l', 'P', 'v'), 'AddBothIn': (0x41cc20, 'l', 'P', 'v'),
    'MakeFolders': (0x5c7ac0, 'x', 'W', 'v'), 'SetEntry': (0x41ec30, 'e', 'DP', 'v'), 'Ctor6fdaa4': (0x41dc80, 'e', 'DP', 'a'),
}
ENTRY, ENTRY_NAME, OWN = RAM + 0xc00, RAM + 0xc80, RAM + 0xd00   # SetEntry: the entry, its name, the object's two Strings' texts
ROOT, FOLDERS, SEP = RAM + 0x900, RAM + 0xa00, 0xf81b48   # MakeFolders: the root, the list's nodes and names, the separator String


def wad_header(r):
    """A WAD header: DWFB (mostly), the version, the entry count at +0x48 (mostly none), the name block's offset and size (mostly none)."""
    h = bytearray(r.bytes(0x58))
    struct.pack_into('<2I', h, 0, 0x42465744 if r.random() < 0.85 else r.getrandbits(32), r.choice([0, 1, 2, 2, 3, 0xffffffff]))
    struct.pack_into('<3I', h, 0x48, 0 if r.random() < 0.85 else 1, r.choice([0, 0, 0x58]), r.choice([0, 0, 4]))
    return h


def text(r, at, regions, s):
    regions[at] = s + b'\0' + r.bytes(0x40)
    return at


def case(r):
    op = r.choice(PICK)
    address, kind, args, ret = OPS[op]
    log = log_region()
    for imp, k in K.items():
        if r.random() < 0.9:
            set_return(log, k, r.choice({'CreateFileA': [0xffffffff, 0x77, 0x77, 0x77], 'GetFileAttributesA': [0xffffffff, 0x10, 0x20, 0x20, 0x11, 0x80],
                                         'ReadFile': [1, 1, 0], 'IsDBCSLeadByte': [0, 0, 1], 'mmioOpenA': [0, 0x66, 0x66], 'FindFirstFileA': [0xffffffff, 0x99],
                                         'InterlockedDecrement': [0, 1, 1, 2], 'InterlockedIncrement': [1, 2], 'CreateSemaphoreA': [0, 0x44, 0x44],
                                         'WaitForSingleObject': [0, 0, 0x102], 'GetCommandLineA': [0], 'GetFileSize': [0, 0x58, 0x1000]}
                                        .get(imp, [0, 1, r.getrandbits(32)])))
    if r.random() < 0.8:
        echo(log, K['SetFilePointer'], 1)   # (where it is sent)
    fill(log, K['ReadFile'], 1, bytes(wad_header(r)), 0)
    fill_arg(log, K['ReadFile'], 3, 2, 1)   # (the bytes read: all asked for)
    base = r.choice([b'C:\\tp\\', b'D:\\'])
    globals_ = bytearray(r.bytes(0x318))
    globals_[0:8] = b'tp.exe\0\0'
    globals_[0x108:0x109 + len(base)] = base + b'\0'
    heap_region = heap(0x10000, fail=r.choice([1, 2, 3]) if r.random() < 0.08 else 0)   # (now and then an allocation answers 0)
    obj = bytearray(r.bytes(0x40))   # an object at THIS
    if kind == 's':
        struct.pack_into('<2I', obj, 0, r.choice([FILESTREAM_VTABLE, STREAM_VTABLE]), 0 if r.random() < 0.8 else SRC)
    elif kind == 'h':
        struct.pack_into('<2I', obj, 0, VT_THIS if r.random() < 0.6 else HANDLE_VTABLE, 0 if r.random() < 0.75 else OBJ)
    elif kind == 'o':   # a RefObject2: its vtable, a link or none, the semaphore, the count, the path (a String in RAM)
        path = r.choice([b'', b'C:\\tp\\data\\', b'abc'])
        struct.pack_into('<4I5I', obj, 0, REFOBJECT2_VTABLE if r.random() < 0.7 else VT_OBJ, r.choice([0, 0, OBJ]), r.choice([0, 0x44]), r.randrange(3),
                         STRING_VTABLE, len(path), TEXT, len(path) + r.choice([0, 8]), 0)
    regions = {THIS: obj, RESULT: r.bytes(0x10), LOG: log, HEAP: heap_region, IMPORT_TABLE: import_table(log, IMPORTS), GLOBALS: globals_,
               PATHS: r.bytes(0x210), LOCALE: bytes(4), VT_THIS: vtable(HANDLE_SLOTS),
               VT_OBJ: vtable(OBJECT_SLOTS),
               VT_SRC: vtable({0: 1, 1: 2, 2: 2, 3: 2, 6: 2, 7: 1, 8: 1, 10: 1}),
               OBJ: struct.pack('<4I', VT_OBJ, 0, 0x44, 1) + r.bytes(0x30), SRC: struct.pack('<3I', VT_SRC, 0, 1) + r.bytes(0x34)}
    if kind == 'o':
        regions[TEXT] = path + b'\0' + r.bytes(0x20)
    if kind == 'e':
        if r.random() < 0.9:   # (the handles' releases: a count left, mostly; 0 destroys the opened object through slot 0x30, a pure call)
            set_return(log, K['InterlockedDecrement'], r.choice([1, 1, 2]))
        for k, off in enumerate((4, 0x18)):
            s = r.choice([b'', b'old', b'x.wad', b'p_a_b'])
            struct.pack_into('<5I', obj, off, STRING_VTABLE, len(s), OWN + 0x40 * k, len(s) + 8, 8)
            regions[OWN + 0x40 * k] = s + b'\0' + r.bytes(0x10)
        struct.pack_into('<2I', obj, 0x2c, VT_THIS if r.random() < 0.6 else HANDLE_VTABLE, 0 if r.random() < 0.6 else OBJ)
        regions[0xf80808] = struct.pack('<5I', STRING_VTABLE, 4, OWN + 0x80, 8, 8)
        regions[OWN + 0x80] = b'.wad\0' + r.bytes(8)
    if kind == 'l':   # a Class6fd9d8: the list at +0x2c (vtable, head, tail, count) of 0-2 nodes {object, next, prev}
        n = r.choice([0, 0, 1, 2])
        nodes = bytearray(r.bytes(0x18))
        for k in range(n):
            struct.pack_into('<3I', nodes, 0xc * k, r.getrandbits(32), NODES + 0xc * (k + 1) if k + 1 < n else 0, NODES + 0xc * (k - 1) if k else 0)
        struct.pack_into('<I', obj, 0x2c, LIST)
        regions[THIS] = obj
        regions[LIST] = struct.pack('<4I', r.getrandbits(32), NODES if n else 0, NODES + 0xc * (n - 1) if n else 0, n)
        regions[NODES] = nodes
    stack = []
    for x in args:
        if x == 'R':
            stack.append(RESULT)
        elif x == 'N':
            stack.append(text(r, NAME, regions, r.choice(PATHS_GIVEN)))
        elif x == 'C':
            c = r.choice(CONTAINERS)
            stack.append(0 if c is None else text(r, CONTAINER, regions, c))
        elif x == 'm':
            stack.append(r.choice([0x80000001, 0x80000001, 0x80000002, 0x80008001, 0x40000000, 0xc0000001, 1, 0, r.getrandbits(32)]))
        elif x == 'f':
            stack.append(r.choice([0, 0, 0x8000, 1, r.getrandbits(32)]))
        elif x == 'd':
            stack.append(r.choice([0, 1, 0x10, 0x100, r.getrandbits(32)]))
        elif x == 'F':
            stack.append(r.choice([0, 1, 2, 3]))
        elif x == 'O':
            regions[OTHER] = struct.pack('<2I', STREAM_VTABLE, 0 if r.random() < 0.2 else SRC) + r.bytes(0x38)
            stack.append(OTHER)
        elif x == 'c':
            if op == 'RefObject2Vfn5':   # (the text buffer's capacity: BUF holds 0x100)
                stack.append(r.choice([0, 1, 2, 4, 0x10, 0x40, 0x100]))
            else:
                stack.append(r.choice([0, 1, 4, 0x10, 0x100, 0x1000, 0x80000000, r.getrandbits(32)]))
        elif x == 'B':
            regions[BUF] = r.bytes(0x100)
            stack.append(BUF)
        elif x == 'P':   # a path String
            p = r.choice(PATHS_GIVEN)
            text(r, NAME, regions, p)
            regions[PATHSTR] = struct.pack('<5I', STRING_VTABLE, len(p), NAME, len(p) + 8, 8)
            regions[WAD_EXT] = struct.pack('<5I', STRING_VTABLE, 4, PATHSTR + 0x40, 8, 8)   # (the game's ".wad" string; AddBoth appends it)
            regions[PATHSTR + 0x40] = b'.wad\0' + r.bytes(8)
            if op in ('AddIn', 'AddBothIn'):   # (the root: a String; the backslash a String)
                root = r.choice([b'', b'C:\\tp', b'C:\\tp\\', b'data', b'\\'])
                regions[ROOT2] = struct.pack('<5I', STRING_VTABLE, len(root), ROOT2 + 0x20, len(root) + 8, 8) + bytes(0xc) + root + b'\0' + r.bytes(8)
                regions[SLASH] = struct.pack('<5I', STRING_VTABLE, 1, SLASH + 0x20, 8, 8) + bytes(0xc) + r.choice([b'\\', b'\\', b'/']) + b'\0' + r.bytes(8)
            stack.append(PATHSTR)
        elif x == 'W':   # MakeFolders' root and list
            root = r.choice([b'C:\\tp\\save', b'D:', b'', b'C:\\tp\\'])
            set_return(log, K['WideCharToMultiByte'], len(root) + 1 if r.random() < 0.9 else r.choice([0, 1]))
            fill(log, K['WideCharToMultiByte'], 4, root + bytes(1))
            regions[ROOT] = struct.pack('<5I', 0x7030c8, len(root), ROOT + 0x20, len(root) + 8, 8) + bytes(0xc) + root.decode().encode('utf-16-le') + bytes(2)
            table = regions[IMPORT_TABLE]
            for at, stub in global_heap().items():
                table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
            sep = r.choice([b'\\', b'\\', b'\\', b'', b'/x\\'])
            regions[SEP] = struct.pack('<5I', STRING_VTABLE, len(sep), SEP + 0x20, len(sep) + 8, 8) + bytes(0xc) + sep + bytes(8)
            n = r.choice([0, 1, 1, 2, 3])
            folders = bytearray(r.bytes(0x100))
            for k in range(n):   # node k at +0x10 * k {name, next}; name k's String at +0x40 + 0x14 * k, its text at +0xc0 + 0x10 * k
                struct.pack_into('<2I', folders, 0x10 * k, FOLDERS + 0x40 + 0x14 * k, FOLDERS + 0x10 * (k + 1) if k + 1 < n else 0)
                name = r.choice([b'users', b'online', b'', b'a\\b'])
                struct.pack_into('<5I', folders, 0x40 + 0x14 * k, STRING_VTABLE, len(name), FOLDERS + 0xc0 + 0x10 * k, len(name) + 8, 8)
                folders[0xc0 + 0x10 * k:0xc0 + 0x10 * k + len(name) + 1] = name + bytes(1)
            regions[FOLDERS] = bytes(folders)
            regions[0x786b90] = struct.pack('<I', FOLDERS if n else 0)
            stack.append(ROOT)
        elif x == 'D':   # a directory entry: its name at +0x20
            name = r.choice([b'x.wad', b'X.WAD', b'p_park_1.wad', b'park_1_x', b'data', b'a.wa', b'', b'p_a_b'])
            regions[ENTRY_NAME] = name + b'\0' + r.bytes(8)
            regions[ENTRY] = put(bytearray(r.bytes(0x40)), 0x20, 'I', ENTRY_NAME)
            stack.append(ENTRY)
        elif x == 'T':
            stack.append(0 if r.random() < 0.2 else text(r, NAME, regions, r.choice([b'', b'x.wad', b'data\\*.bin'])))
    return Call(address, [THIS] + stack, regions, conv='t', ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
