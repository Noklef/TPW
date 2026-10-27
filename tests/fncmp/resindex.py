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
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, blob, echo, fill, fill_arg, heap, import_table, log_region, set_return, strings, vtable  # noqa: E402
import fileopen as fo  # noqa: E402

B = Bench(fo.RANGES + [(0x6ad130, 0x6ad230), (0x6afbc0, 0x6afc57), (0x6acfd0, 0x6acff0), (0x6af760, 0x6af770), (0x6af810, 0x6af954),
                       (0x6afa30, 0x6afb5f), (0x6afb70, 0x6afbbc), (0x6ad230, 0x6ad45a)])
THIS, RESULT, OBJ, NAME, INDEX, RECORDS = RAM, RAM + 0x80, RAM + 0xc0, RAM + 0x400, RAM + 0x500, RAM + 0x600
VT_HANDLE, VT_OBJ, VT_INDEX = RAM + 0x200, RAM + 0x280, RAM + 0x300
FOLDER_HANDLE_VTABLE = 0x6fd9e8
OPS = {'FolderCtor': 0x6ad130, 'FolderDtor': 0x6ad160, 'FolderOpen': 0x6ad1b0, 'FolderClose': 0x6ad200, 'IndexInit': 0x6afbc0,
       'BaseCtor': 0x6af760, 'BaseDelete': 0x6acfd0, 'IndexCtor': 0x6af810, 'IndexDtor': 0x6af8a0, 'IndexDelete': 0x6af880,
       'ReleaseEntry': 0x6af900, 'ReleaseAll': 0x6af920, 'GetEntry': 0x6afa30, 'OpenEntry': 0x6afb70,
       'Load': 0x6afa70, 'LoadIndex': 0x6ad2a0, 'LoadNamed': 0x6ad230}
LOADER, VT_LOADER, SOURCE = RAM + 0x780, RAM + 0x800, RAM + 0x900
RETURNS = {'FolderDtor': 'v', 'IndexDtor': 'v', 'ReleaseEntry': 'v', 'ReleaseAll': 'v'}
ENTRY_OBJ, VT_ENTRY, OUT = RAM + 0x700, RAM + 0x380, RAM + 0x90
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


def answer(log, k, which, value=None, held=True):
    """Slot k's recorder answers through its first argument (&slot on the caller's stack: logged as the dword it holds, or as 0 when that is
    itself a stack address or garbage (held False)); its second (a stack address: the stream, the path buffer) logged as 0."""
    if held:
        blob(log, k, 0, 1)
    strings(log, k, 0, out=2 if held else 3)
    echo(log, k, 0)
    fill(log, k, 0, struct.pack('<I', value if value is not None else 0), which=which)


def result(r):
    return r.choice([0, 0, 0, 1, 0xffffffff, 0x80000000, r.getrandbits(32)])


def load_case(r, op, regions):
    log = regions[LOG]
    obj = bytearray(r.bytes(0x30))
    if op == 'Load':   # (&result, record, &object) on an index whose handle answers slot 3 (or its object slot 1)
        obj[0:0x10] = struct.pack('<4I', VT_INDEX, 1, RECORDS, RECORDS + 12 * 4)
        obj[0x10:0x18] = handle(r)
        recs = bytearray(r.bytes(12 * 5))
        for k in range(5):
            struct.pack_into('<H', recs, 12 * k + 4, r.randrange(6) if r.random() < 0.85 else r.choice([6, 7, r.getrandbits(16)]))
        regions[RECORDS] = bytes(recs)
        for k, which in ((1, 0), (3, 1)):
            answer(log, k, which, result(r), held=False)   # (&slot holds &stream)
        regions[0x782d20] = struct.pack('<6I', *[LOADER if r.random() < 0.8 else 0 for _ in range(6)])
        regions[LOADER] = struct.pack('<I', VT_LOADER) + r.bytes(0xc)
        regions[VT_LOADER] = vtable({0: 5})
        answer(log, 0, 2, result(r))
        fill(log, 0, 2, struct.pack('<I', ENTRY_OBJ if r.random() < 0.9 else 0), which=3)
        regions[VT_ENTRY] = vtable({})
        regions[ENTRY_OBJ] = struct.pack('<2I', VT_ENTRY, 1)
        regions[OUT] = r.bytes(4)
        stack = [RESULT, RECORDS + 12 * r.randrange(5), OUT]
    else:   # a folder (+0 an index or none, +4 its handle) and a file system: "ResIdx.dat" read through FileSource from ReadFile's fills
        obj[0:4] = struct.pack('<I', 0 if r.random() < 0.4 else INDEX)
        set_return(log, fo.K['CreateFileA'], r.choice([0x77, 0x77, 0x77, 0xffffffff]))
        set_return(log, fo.K['GetFileAttributesA'], r.choice([0x10, 0x10, 0x10, 0x20, 0xffffffff]))
        set_return(log, fo.K['GetFileSize'], r.choice([0x10, 0x14, 0x20, 0x58, 0, 8]))
        set_return(log, fo.K['ReadFile'], r.choice([1, 1, 1, 0]))
        header = struct.pack('<2I', 0x49524642 if r.random() < 0.85 else r.getrandbits(32), r.choice([0, 1, 1, 2, 2, 3, 5]))
        data = struct.pack('<IHH', r.choice([0, 1, r.getrandbits(32)]), r.randrange(8), r.randrange(0x40))
        fill(log, fo.K['ReadFile'], 1, header + data, records=2)
        if r.random() < 0.75:
            fill_arg(log, fo.K['ReadFile'], 3, 2, which=1)   # (the bytes read: all asked for)
        else:   # (the header's and the records' bytes read: fewer or more than asked: a short header, records past the size)
            fill(log, fo.K['ReadFile'], 3, struct.pack('<2I', r.choice([4, 7, 8, 8, 0x100]), r.choice([1, 4, 0x40, 0x100, 0x1000])), which=1, records=2)
        if op == 'LoadIndex':
            obj[4:0xc] = handle(r)
            stack = [RESULT, path(r, regions)]
        else:   # LoadNamed: the handle a recorder (slot 5 answering and writing a path), the folder handle on a RefObject2 with a path, or none
            u = r.random()
            if u < 0.4:
                obj[4:0xc] = struct.pack('<2I', VT_HANDLE, 0)
                answer(log, 5, 2, result(r), held=False)   # (&r uninitialised)
                fill(log, 5, 1, r.choice([b'C:\\tp\\data\\', b'data', b'x.wad', b'']) + b'\0', which=3)
            else:
                obj[4:0xc] = struct.pack('<2I', FOLDER_HANDLE_VTABLE, 0 if u < 0.5 else SOURCE)
                p = r.choice([b'', b'C:\\tp\\data\\', b'abc', b'D:\\fonts'])
                regions[SOURCE] = struct.pack('<4I5I', fo.REFOBJECT2_VTABLE, 0, 0x44, 1, fo.STRING_VTABLE, len(p), SOURCE + 0x40, len(p) + 8, 0) + \
                    bytes(0x1c) + p + b'\0' + r.bytes(0x10)
            regions[NAME] = r.choice([b'fonts', b'', b'a\\b', b'x.wad']) + b'\0' + r.bytes(0x10)
            stack = [RESULT, NAME]
    regions[THIS] = bytes(obj)
    regions[RESULT] = r.bytes(8)
    return Call(OPS[op], [THIS] + stack, regions, conv='t', ret='a')


def case(r):
    op = r.deal(PICK)
    regions = world(r)
    if op in ('Load', 'LoadIndex', 'LoadNamed'):
        return load_case(r, op, regions)
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
    elif op in ('BaseCtor', 'IndexCtor'):
        pass
    elif op == 'BaseDelete':
        stack.append(r.choice([0, 1, 1, 2, 3, r.getrandbits(32)]))
    elif op != 'IndexInit':   # an index: records (12 bytes: key, kind, name offset, object: ENTRY_OBJ or none) and the last one
        n = r.choice([0, 1, 2, 3, 5])
        recs = bytearray(r.bytes(12 * 6))
        for k in range(6):
            struct.pack_into('<I', recs, 12 * k + 8, ENTRY_OBJ if r.random() < 0.6 else 0)
        regions[RECORDS] = bytes(recs)
        regions[VT_ENTRY] = vtable({})
        regions[ENTRY_OBJ] = struct.pack('<2I', VT_ENTRY, 1)
        records = 0 if r.random() < 0.12 else RECORDS
        last = RECORDS + 12 * (n - 1) if r.random() < 0.85 else r.choice([RECORDS - 12, RECORDS + 1, r.getrandbits(32)])
        obj[0:0x10] = struct.pack('<4I', VT_INDEX, 1, records, last & 0xffffffff)
        obj[0x10:0x18] = handle(r)
        record = 0 if r.random() < 0.1 else RECORDS + 12 * r.randrange(5)
        if op == 'IndexDelete':
            stack.append(r.choice([0, 1, 1, 2, 3, r.getrandbits(32)]))
        elif op == 'ReleaseEntry':
            stack.append(record)
        elif op == 'GetEntry':
            regions[OUT] = r.bytes(4)
            stack += [RESULT, record, OUT]
        elif op == 'OpenEntry':   # the handle's slot 3 (or the object's slot 1 through Handle_Vfn3): its result written through &slot
            log = regions[LOG]
            for k in (1, 3):
                blob(log, k, 0, 1)   # (&slot: the caller's stack; logged as the stream it holds)
                echo(log, k, 0)
                fill(log, k, 0, struct.pack('<I', r.choice([0, 0, 1, 0xffffffff, 0x80000000, r.getrandbits(32)])), which=k // 3)
            stack += [RESULT, r.getrandbits(32), RECORDS + 12 * r.randrange(5)]
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
