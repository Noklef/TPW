# The resource folder and index (src/game/files/resindex) against the originals. The folder opens its handle and the index its own through
# Handle_Open28, which runs whole (the directory source, Path_Resolve, the strings; fileopen.py's world: the Windows imports recorders,
# GetFileAttributesA a folder, a file or none, InterlockedDecrement now and then 0, the heap failing now and then). The handles have the
# folder handle vtable (0x6fd9e8, resolved), a recorder vtable or the base's, with an object (a RefObject2 with recorders) or none; the
# index the folder holds a recorder vtable (its slot 0, Release, logged).
# FolderCtor (0x6ad130) over random bytes; FolderDtor (0x6ad160) and FolderClose (0x6ad200): an index or none; FolderOpen (0x6ad1b0): +0xc
# 0 mostly (else 1 or any), a path; IndexInit (0x6afbc0): a path, 0-6 records (keys mostly increasing, now and then equal or lower, the
# objects any), the count 0 to two past them.
# usage: resindex.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FolderOpen,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, echo, heap, import_table, log_region, set_return, vtable  # noqa: E402
import fileopen as fo  # noqa: E402

B = Bench(fo.RANGES + [(0x6ad130, 0x6ad230), (0x6afbc0, 0x6afc57)])
THIS, RESULT, OBJ, NAME, INDEX, RECORDS = RAM, RAM + 0x80, RAM + 0xc0, RAM + 0x400, RAM + 0x500, RAM + 0x600
VT_HANDLE, VT_OBJ, VT_INDEX = RAM + 0x200, RAM + 0x280, RAM + 0x300
FOLDER_HANDLE_VTABLE = 0x6fd9e8
OPS = {'FolderCtor': 0x6ad130, 'FolderDtor': 0x6ad160, 'FolderOpen': 0x6ad1b0, 'FolderClose': 0x6ad200, 'IndexInit': 0x6afbc0}
RETURNS = {'FolderDtor': 'v'}
PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]


def world(r):
    """fileopen.py's imports, globals and heap."""
    log = log_region()
    for imp, k in fo.K.items():
        if r.random() < 0.9:
            set_return(log, k, r.choice({'CreateFileA': [0xffffffff, 0x77], 'GetFileAttributesA': [0xffffffff, 0x10, 0x10, 0x20, 0x11, 0x80],
                                         'IsDBCSLeadByte': [0, 0, 1], 'InterlockedDecrement': [0, 1, 1, 2], 'InterlockedIncrement': [1, 2],
                                         'CreateSemaphoreA': [0, 0x44, 0x44], 'WaitForSingleObject': [0, 0, 0x102], 'GetCommandLineA': [0]}
                                        .get(imp, [0, 1, r.getrandbits(32)])))
    if r.random() < 0.8:
        echo(log, fo.K['SetFilePointer'], 1)
    base = r.choice([b'C:\\tp\\', b'D:\\'])
    globals_ = bytearray(r.bytes(0x318))
    globals_[0:8] = b'tp.exe\0\0'
    globals_[0x108:0x109 + len(base)] = base + b'\0'
    return {LOG: log, HEAP: heap(0x4000, fail=r.choice([1, 2]) if r.random() < 0.06 else 0), IMPORT_TABLE: import_table(log, fo.IMPORTS),
            fo.GLOBALS: globals_, fo.PATHS: r.bytes(0x210), fo.LOCALE: bytes(4), VT_HANDLE: vtable(fo.HANDLE_SLOTS), VT_OBJ: vtable(fo.OBJECT_SLOTS),
            VT_INDEX: vtable({0: 0, 1: 0}), OBJ: struct.pack('<4I', VT_OBJ, 0, 0x44, 1) + r.bytes(0x30),
            INDEX: struct.pack('<2I', VT_INDEX, 1) + r.bytes(0x18)}


def handle(r):
    vt = r.choice([FOLDER_HANDLE_VTABLE, FOLDER_HANDLE_VTABLE, VT_HANDLE, fo.HANDLE_VTABLE])
    return struct.pack('<2I', vt, 0 if r.random() < 0.7 else OBJ)


def path(r, regions):
    regions[NAME] = r.choice(fo.PATHS_GIVEN) + b'\0' + r.bytes(0x20)
    return NAME


def case(r):
    op = r.deal(PICK)
    regions = world(r)
    obj = bytearray(r.bytes(0x30))
    stack = []
    if op.startswith('Folder'):
        obj[0:4] = struct.pack('<I', 0 if r.random() < 0.3 else INDEX)
        obj[4:0xc] = handle(r)
        obj[0xc:0x10] = struct.pack('<I', 0 if r.random() < 0.75 else r.choice([1, r.getrandbits(32)]))
        if op in ('FolderOpen', 'FolderClose'):
            stack.append(RESULT)
        if op == 'FolderOpen':
            stack.append(path(r, regions))
    else:   # IndexInit: (&result, path, records, count)
        obj[0:8] = struct.pack('<2I', VT_INDEX, 1)
        obj[0x10:0x18] = handle(r)
        n = r.choice([0, 1, 2, 3, 4, 6])
        recs = bytearray(r.bytes(12 * 9))
        key = r.choice([0, 1, 0x100, r.getrandbits(31)])
        for k in range(9):
            struct.pack_into('<I', recs, 12 * k, key)
            u = r.random()
            key = (key + (r.randrange(1, 0x1000) if u < 0.88 else 0 if u < 0.94 else -r.randrange(1, 4))) & 0xffffffff
        regions[RECORDS] = bytes(recs)
        count = n if r.random() < 0.7 else r.choice([0, n + 1, n + 2, max(n - 1, 0)])
        stack += [RESULT, path(r, regions), RECORDS, count]
    regions[THIS] = bytes(obj)
    regions[RESULT] = r.bytes(8)
    return Call(OPS[op], [THIS] + stack, regions, conv='t', ret=RETURNS.get(op, 'a'))


if __name__ == '__main__':
    B.main(case)
