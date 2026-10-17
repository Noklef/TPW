# The WAD archive source (src/game/files/wadsource) against the originals: a WadSource at THIS built by the case (the game's WadSource vtable, which the
# driver points at the replacements; strings, string list, header and directory tree in an arena as waddir.py / wadfind.py build them), its FileStream's
# vtable, the stream OpenEntry is handed and the archive's source all recorders: the file's open (+0x24) answers a status (the path it is given logged as
# text) and its read fills the header ("DWFB", versions 0-3 and wild); the stream's read fills an entry record (flags 0, 1, 2 zlib, 4 RefPack, 6), the
# source's read the packed bytes (a small valid RefPack or zlib stream now and then), its GetLock (+0x2c) a semaphore. The Shared / Slice / Buffer sources
# OpenEntryStream makes are the real ones (the driver points their vtables at the replacements), so their attach slots run the decoders
# (BufferSource_AttachRefPack / AttachZlib). The Windows imports are recorders; the base directory a short path, so that Path_Resolve resolves against it.
# usage: wadsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open,...)
import os
import struct
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, echo, fill, heap, import_table, log_region, recorder, set_return, strings  # noqa: E402
from waddir import ARENA, LOCALE, Arena, pool, strings as string_list  # noqa: E402
from wadfind import STRING_VTABLE, finder, name_hash, normalized, part, string, tree  # noqa: E402

B = Bench([(0x45f8e0, 0x45fa40), (0x61a060, 0x61af40), (0x45d820, 0x45d880), (0x4217e0, 0x421800), (0x637460, 0x638f00), (0x641500, 0x641900), (0x5f5690, 0x5f5a20), (0x5f5f00, 0x5f9000),
           (0x618000, 0x61a060), (0x6307c0, 0x630900), (0x6ad800, 0x6ad820), (0x641900, 0x642400), (0x67a400, 0x690000), (0x405000, 0x406000),
           (0x5e4000, 0x5e5000), (0x636b00, 0x636d00)])
THIS, RESULT, ENTRY = RAM, RAM + 0x100, RAM + 0x200
STREAM, SRC, SEM, STATUS = RAM + 0x400, RAM + 0x440, RAM + 0x4c0, RAM + 0x4d0
VT_FILE, VT_STREAM, VT_SRC = RAM + 0x500, RAM + 0x540, RAM + 0x580
NAME = RAM + 0x600
DIRS = 0xfa6fd0   # the base directory (Path_Resolve's)
WAD_VTABLE, FILE_VTABLE = 0x703908, 0x6fd930
# The recorders: the file's slot i is recorder i, the stream's 10 + i, the source's 20 + i; their stack arguments.
FILE_SLOTS = {0: 1, 2: 2, 3: 2, 4: 2, 8: 1, 9: 3}
STREAM_SLOTS = {0: 1, 2: 2, 3: 2, 4: 2, 8: 1}
SRC_SLOTS = {0: 1, 1: 2, 2: 2, 3: 2, 6: 2, 7: 1, 8: 1, 10: 1}
K_FILE_READ, K_FILE_SEEK, K_FILE_OPEN, K_STREAM_READ, K_SRC_READ, K_SRC_SIZE, K_SRC_TELL, K_SRC_LOCK = 2, 4, 9, 12, 21, 24, 25, 31
IMPORTS = {'CreateSemaphoreA': (40, 0x6fd16c, 4, 8, 0), 'CloseHandle': (41, 0x6fd13c, 1, 0, 0), 'GetFileAttributesA': (42, 0x6fd10c, 1, 1, 0),
           'InterlockedIncrement': (43, 0x6fd0c0, 1, 0, 0), 'InterlockedDecrement': (44, 0x6fd12c, 1, 0, 0),
           'WaitForSingleObject': (45, 0x6fd128, 2, 0, 0), 'ReleaseSemaphore': (46, 0x6fd170, 3, 0, 0)}
FOLDERS = [b'', b'', b'data\\', b'rides\\', b'rides\\coaster\\', b'a\\', b'Data\\']
PATHS = [b'x.wad', b'data/x.wad', b'x.wad*\\data', b'x.wad*\\rides\\coaster', b'x.wad*\\data\\', b'x.wad*/data', b'a\\b.wad*\\', b'', b'*\\', b'x.wad*',
         b'x.wad*\\nope', b'X.WAD*\\RIDES']
# name: (address, this: w the source, l its list, n a list node, x space for one; stack arguments; returns)
OPS = {
    'Ctor': (0x61a060, 'x', '', 'a'), 'DeletingDtor': (0x61a110, 'w', 'F', 'a'), 'Dtor': (0x61a130, 'w', '', 'v'), 'Close': (0x61a200, 'w', '', 'v'),
    'Open': (0x61a250, 'w', 'RPg', 'a'), 'FindFirst': (0x61a550, 'w', 'RTE', 'a'), 'FillEntry': (0x61a6c0, 'w', 'E', 'v'),
    'FindNext': (0x61a700, 'w', 'RE', 'a'), 'OpenEntry': (0x61a760, 'w', 'RSNgd', 'a'), 'OpenEntryStream': (0x61a800, 'w', 'RSN', 'a'),
    'Vfn7': (0x61adb0, 'w', 'Rdd', 'a'), 'Vfn8': (0x61adc0, 'w', 'Rd', 'a'), 'Vfn9': (0x61add0, 'w', 'Rd', 'a'),
    'ListDtor': (0x45d820, 'l', '', 'v'), 'ListClear': (0x61aeb0, 'l', '', 'v'), 'ListDeletingDtor': (0x61a7e0, 'l', 'F', 'a'),
    'NodeDeletingDtor': (0x61af00, 'n', 'F', 'a'), 'ListBaseDeletingDtor': (0x45f8e0, 'l', 'F', 'a'),
}


def vt(base, slots, size=16):
    return bytearray(struct.pack('<%dI' % size, *[recorder(base + i, slots.get(i, 0)) for i in range(size)]))


def refpack(r):
    """A small RefPack stream: the header (0x10fb, the size big-endian), literal runs of 4 (0xe0 + n), the stop (0xfc + 0-3 literals)."""
    runs = r.choice([0, 1, 2])
    body = b''.join(bytes([0xe0]) + r.bytes(4) for _ in range(runs))
    tail = r.randrange(4)
    size = 4 * runs + tail
    return b'\x10\xfb' + struct.pack('>I', size)[1:] + body + bytes([0xfc + tail]) + r.bytes(tail)


def case(r):
    op = r.choice(PICK)
    address, kind, args, ret = OPS[op]
    a = Arena(r)
    log = log_region()
    folder = r.choice(FOLDERS)
    name = r.choice([part(r), part(r) + b'\\' + part(r), b'car.bin', b'coaster\\track.bin', b'', b'*.bin'])
    hashes = [name_hash(normalized(folder + name)) if folder + name else 0, name_hash(normalized(name)) if name else 0]
    obj = bytearray(r.bytes(0x100))
    if kind == 'w':
        struct.pack_into('<4I', obj, 0, WAD_VTABLE, 0, r.getrandbits(32), r.randrange(4))
        obj[0x10:0x24] = a.data[string(a, r, r.choice([b'', b'x.wad*', b'C:\\tp\\x.wad*data\\'])) - ARENA:][:0x14]
        obj[0x24:0x38] = a.data[string(a, r, folder) - ARENA:][:0x14]
        struct.pack_into('<i', obj, 0x38, -1)
        string_list(a, r, ARENA + 0xef10)
        obj[0x3c:0x4c] = a.data[0xef10:0xef20]
        obj[0x4c:0x60] = a.data[string(a, r, r.choice([b'', b'data\\*.bin'])) - ARENA:][:0x14]
        struct.pack_into('<2I', obj, 0x60, 0x42465744 if r.random() < 0.9 else r.getrandbits(32), r.choice([0, 1, 2, 2, 3]))
        struct.pack_into('<3I', obj, 0xb8, VT_FILE if r.random() < 0.95 or op == 'Open' else FILE_VTABLE, r.choice([0, SRC]), r.choice([0, SRC, SRC, SRC]))
        struct.pack_into('<3I', obj, 0xc4, tree(a, r, 2, hashes) if r.random() < 0.85 else 0, a.text(b'names') if r.random() < 0.5 else 0,
                         finder(a, r, hashes) if r.random() < 0.7 else 0)
        pool(a, r, ARENA + 0xef00, 'f')
        pool(a, r, ARENA + 0xef08, 'w')   # (the two pools are back to back)
        obj[0xd0:0xe0] = a.data[0xef00:0xef10]
        if obj[0xb8:0xbc] == struct.pack('<I', FILE_VTABLE):
            struct.pack_into('<I', obj, 0xbc, 0)   # (the game's FileStream: its slots forward to a source; none here)
    elif kind == 'l':
        string_list(a, r, ARENA + 0xef10)
        obj[:0x10] = a.data[0xef10:0xef20]
    elif kind == 'n':
        n = a.alloc(0x1c)
        a.put(n, '5I2I', STRING_VTABLE, 3, a.text(b'abc') if r.random() < 0.8 else 0, 4, 0, r.getrandbits(32), r.getrandbits(32))
        obj[:0x1c] = a.data[n - ARENA:][:0x1c]
    # The recorders' answers.
    status = r.choice([0, 0, 0xffffffff, 1])
    set_return(log, K_FILE_OPEN, STATUS)
    strings(log, K_FILE_OPEN, 2, 1)   # (the resolved path, on the stack: its text; &status: 0)
    # The header: every read of the file fills these bytes, so the directory load reads them as its header, its records (a name offset and length at +4
    # and +0x2c, small) and its name block too.
    header = bytearray(r.bytes(0x58))
    struct.pack_into('<3I', header, 0, 0x42465744 if r.random() < 0.85 else r.getrandbits(32), r.choice([0, 1, 2, 2, 3, 0xffffffff]), r.randrange(7))
    struct.pack_into('<2I', header, 0x2c, r.randrange(9), r.randrange(7))
    struct.pack_into('<3I', header, 0x48, r.choice([0, 1, 2]), 0x58 + r.choice([0, 0x28, 0x50]), r.choice([0, 4, 8]))
    if op == 'Open' and r.random() < 0.7:   # (a file that reads what it is asked for and seeks where it is sent; the stack header of a load
        # without a tree in OpenEntryStream is unfilled: its seek answers a hash, and the load stops there)
        echo(log, K_FILE_READ, 1)
        echo(log, K_FILE_SEEK, 0)
    else:
        set_return(log, K_FILE_READ, r.choice([0x58, 0x58, 0, 0x57]))
    strings(log, K_FILE_READ, 0, 1)   # (the load reads its header into the stack: 0)
    set_return(log, 42, r.choice([0xffffffff, 0, 0x20, 0x10, 0x11]))   # GetFileAttributesA
    set_return(log, 40, r.choice([0, 0x44]))
    set_return(log, K_SRC_LOCK, SEM)
    set_return(log, K_SRC_TELL, r.choice([0, 0, 0x58, r.getrandbits(32)]))
    flags = r.choice([0, 0, 1, 2, 2, 4, 4, 6, 0x101, r.getrandbits(32)])
    if flags & 4 and r.random() < 0.7:
        packed = refpack(r)
    elif flags & 2 and r.random() < 0.7:
        packed = zlib.compress(r.bytes(r.choice([0, 1, 4, 0x10])))
    else:
        packed = r.bytes(r.choice([0, 4, 0x10]))
    size = len(packed) if r.random() < 0.85 else r.choice([0, 1, 0x100])
    unpacked = r.choice([0, 0x10, 0x40, 0x100]) if r.random() < 0.3 else 0x40
    record = bytearray(r.bytes(0x28))
    struct.pack_into('<5I', record, 0xc, r.choice([0, 0x58, 0x1000]), size, flags, unpacked, 0)
    set_return(log, K_STREAM_READ, r.choice([0x28, 0x28, 0]))
    strings(log, K_STREAM_READ, 0, 1)   # (&record, on the stack: 0)
    set_return(log, K_SRC_READ, r.choice([len(packed), len(packed), 0, size]))
    if op == 'Open':
        fill(log, K_FILE_READ, 0, bytes(header))
    elif op.startswith('OpenEntry'):   # (a load without a tree reads nothing through the file)
        fill(log, K_STREAM_READ, 0, bytes(record))
        fill(log, K_SRC_READ, 0, packed, 1)
    regions = {HEAP: heap(0x10000), LOCALE: bytes(4), RESULT: r.bytes(0x10), LOG: log, IMPORT_TABLE: import_table(log, IMPORTS),
               VT_FILE: vt(0, FILE_SLOTS), VT_STREAM: vt(10, STREAM_SLOTS), VT_SRC: vt(20, SRC_SLOTS),
               SRC: struct.pack('<3I', VT_SRC, 0, 1) + r.bytes(0x34), SEM: struct.pack('<I', r.choice([0, 0x55])) + r.bytes(0xc),
               STATUS: struct.pack('<I', status) + r.bytes(0xc), DIRS: r.choice([b'C:\\tp\\', b'D:\\']) + b'\0' + r.bytes(0x100)}
    stack = []
    for x in args:
        if x == 'R':
            stack.append(RESULT)
        elif x == 'F':
            stack.append(r.choice([0, 1, 2, 3]))
        elif x == 'P':
            stack.append(a.text(r.choice(PATHS)))
        elif x == 'g':
            stack.append(r.choice([0, 0x8000, 0x10, 2, 0x40000000, r.getrandbits(32)]))
        elif x == 'd':
            stack.append(r.getrandbits(32))
        elif x == 'T':   # a pattern
            stack.append(a.text(r.choice([b'*', b'*.bin', name, b'coaster\\*', b'a/b', b'', b'*.*'])) if r.random() < 0.95 else 0)
        elif x == 'E':   # a DirEntry: the pattern text (usually none), a name pointer
            e = bytearray(r.bytes(0x200))
            text = r.choice([b'', b'', b'', b'x', b'data\\x.bin', folder + b'car.bin'])
            e[0x24:0x25 + len(text)] = text + b'\0'
            struct.pack_into('<I', e, 0x20, ENTRY + 0x24)
            regions[ENTRY] = e
            stack.append(ENTRY)
        elif x == 'S':   # the stream (a StreamBase): usually without a source
            regions[STREAM] = struct.pack('<2I', VT_STREAM, 0 if r.random() < 0.85 else SRC) + r.bytes(0x38)
            stack.append(STREAM)
        elif x == 'N':
            regions[NAME] = name + b'\0' + r.bytes(0x40)
            stack.append(NAME)
    regions[THIS] = obj
    regions[ARENA] = a.data
    return Call(address, [THIS] + stack, regions, conv='t', ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
