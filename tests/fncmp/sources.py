# The file and mmio stream sources (src/game/files/sources) against the originals: a FileSource or MmioSource at THIS (handle, position, mapping and view
# each often unset) whose own vtable is recorders (Open closes through +0x24, ReleaseMapping unmaps through +0x1c); every Windows and mmio import a recorder
# (COM-style, stdcall, fncmp import_table()): CreateFileA's and mmioOpenA's path the resolved one in the globals at 0xfaa2c8 / 0xfaa3d0 (logged as its text),
# ReadFile / WriteFile filling the count done through their stack out argument and GetSystemInfo its SYSTEM_INFO (the allocation granularity at +0x1c, from
# 0 to past offset); the base directory a short path, so that Path_Resolve resolves against it.
# usage: sources.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FileMap,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, fill, heap, import_table, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x636e40, 0x637460), (0x5f5690, 0x5f5830), (0x5f5950, 0x5f5a20), (0x618cd0, 0x618d20), (0x6307c0, 0x630800), (0x67a400, 0x690000)])
THIS, RESULT, VTABLE = RAM, RAM + 0x40, RAM + 0x100
PATH, BUFFER = RAM + 0x200, RAM + 0x400
DIRS = 0xfa6fd0   # the base directory (Path_Resolve's)
PATHS = 0xfaa2c8   # the file source's resolved path (0x104), + 0x108 the mmio source's
# The imports: slot, import table address, stack arguments, string arguments (mask), out arguments (mask).
IMPORTS = {'CreateFileA': (40, 0x6fd0d8, 7, 1, 0), 'ReadFile': (41, 0x6fd0dc, 5, 0, 8), 'WriteFile': (42, 0x6fd0e0, 5, 0, 8),
           'SetFilePointer': (43, 0x6fd0e4, 4, 0, 0), 'GetFileSize': (44, 0x6fd0e8, 2, 0, 0), 'GetSystemInfo': (45, 0x6fd0ec, 1, 0, 1),
           'MapViewOfFileEx': (46, 0x6fd0f0, 6, 0, 0), 'CreateFileMappingA': (47, 0x6fd0f4, 6, 0, 0), 'UnmapViewOfFile': (48, 0x6fd0f8, 1, 0, 0),
           'FlushFileBuffers': (49, 0x6fd0fc, 1, 0, 0), 'CloseHandle': (50, 0x6fd13c, 1, 0, 0), 'CreateSemaphoreA': (51, 0x6fd16c, 4, 0, 0),
           'mmioOpenA': (52, 0x6fd3e4, 3, 1, 0), 'mmioClose': (53, 0x6fd3f0, 2, 0, 0), 'mmioRead': (54, 0x6fd3e0, 3, 0, 0),
           'mmioWrite': (55, 0x6fd3dc, 3, 0, 0), 'mmioSeek': (56, 0x6fd3d8, 3, 0, 0), 'mmioSetBuffer': (57, 0x6fd3e8, 4, 0, 0),
           'mmioFlush': (58, 0x6fd3d4, 2, 0, 0)}
K = {name: v[0] for name, v in IMPORTS.items()}
# The source's own slots (thiscall, stack arguments): +0x1c unmap (view), +0x24 close.
SELF_SLOTS = {7: 1}
# name: (address, kind: f file, m mmio; stack arguments: R the result, P a path, M a mode, B the buffer, n a count, o an offset, g an origin, s a size,
# V a view, F flags; returns)
OPS = {
    'FileCtor': (0x636e40, 'f', '', 'a'), 'FileDeletingDtor': (0x636ea0, 'f', 'F', 'a'), 'FileDtor': (0x636ec0, 'f', '', 'v'),
    'FileOpen': (0x636f10, 'f', 'RPM', 'a'), 'ReleaseMapping': (0x636fb0, 'f', '', 'v'), 'FileClose': (0x636fe0, 'f', '', 'v'),
    'FileRead': (0x637000, 'f', 'Bn', 'a'), 'FileWrite': (0x637040, 'f', 'Bn', 'a'), 'FileSeek': (0x637080, 'f', 'og', 'a'),
    'FileSize': (0x6370a0, 'f', '', 'a'), 'FileTell': (0x6370b0, 'f', '', 'a'), 'FileMap': (0x6370c0, 'f', 'os', 'a'),
    'FileUnmap': (0x637180, 'f', 'V', 'v'), 'FileLock': (0x6371a0, 'f', '', 'a'), 'FileFlush': (0x6371b0, 'f', 'R', 'a'),
    'MmioCtor': (0x6371e0, 'm', '', 'a'), 'MmioDeletingDtor': (0x637230, 'm', 'F', 'a'), 'MmioDtor': (0x637250, 'm', '', 'v'),
    'MmioOpen': (0x6372b0, 'm', 'RPM', 'a'), 'MmioClose': (0x637340, 'm', '', 'v'), 'MmioRead': (0x637360, 'm', 'Bn', 'a'),
    'MmioWrite': (0x637390, 'm', 'Bn', 'a'), 'MmioSeek': (0x6373c0, 'm', 'og', 'a'), 'MmioSize': (0x6373f0, 'm', '', 'a'),
    'MmioFlush': (0x637420, 'm', 'R', 'a'), 'MmioMap': (0x637450, 'm', 'os', 'a'),
}
NAMES = [b'', b'a.wad', b'data\\x.wad', b'C:\\x\\y.dat', b'\\abs', b'//srv/f', b'x/']
BASES = [b'C:\\games\\tpw\\', b'D:/x/', b'rel\\', b'Q']


def case(r):
    op = r.choice(PICK)
    address, kind, args, ret = OPS[op]
    log = log_region()
    handle = r.choice([0xffffffff, 0, 0x44, r.getrandbits(32)])
    obj = bytearray(r.bytes(0x40))
    put(obj, 0, 'I', VTABLE)
    put(obj, 4, 'I', 0)   # (no link)
    put(obj, 0xc, 'I', handle)
    put(obj, 0x10, 'I', r.choice([0, 0x10, r.getrandbits(32)]))
    if kind == 'f':   # the mapping and the whole-file view
        put(obj, 0x14, 'I', r.choice([0, 0, 0x55, r.getrandbits(32)]))
        put(obj, 0x18, 'I', r.choice([0, 0, BUFFER, r.getrandbits(32)]))
    count = r.choice([0, 1, 4, 0x10, 0x100, r.getrandbits(32)])
    granularity = r.choice([0x10000, 0x10000, 0x1000, 1, 0x7fffffff, 0x80000000, r.randrange(1, 0x20000), 0])
    offset = r.choice([0, 0, 1, granularity, granularity + 1, granularity - 1, 3 * granularity + 5, 0x80000000, r.getrandbits(32)]) & 0xffffffff
    for imp, k in K.items():
        if r.random() < 0.8:
            set_return(log, k, r.choice({'CreateFileA': [0xffffffff, 0xffffffff, 0x77, r.getrandbits(32)], 'CreateFileMappingA': [0, 0x55, r.getrandbits(32)],
                                         'MapViewOfFileEx': [0, BUFFER, r.getrandbits(32)], 'mmioOpenA': [0, 0, 0x66, r.getrandbits(32)],
                                         'mmioRead': [count, 0, -1, count // 2], 'mmioWrite': [count, 0, -1], 'mmioFlush': [0, 0, 1, 0x100]}.get(imp, [0, 1, r.getrandbits(32)])))
    done = r.choice([count, count, 0, count // 2, r.getrandbits(32)])
    fill(log, K['ReadFile'] if 'Read' in op else K['WriteFile'], 3, struct.pack('<I', done), 0)
    fill(log, K['GetSystemInfo'], 0, r.bytes(0x1c) + struct.pack('<I', granularity) + r.bytes(4), 1)
    base = r.choice(BASES)
    name = r.choice(NAMES) if r.random() < 0.9 else bytes(r.randrange(0x21, 0x7f) for _ in range(r.randrange(0x30)))
    paths = bytearray(r.bytes(0x210))
    regions = {THIS: obj, RESULT: r.bytes(8), VTABLE: vtable(SELF_SLOTS), PATH: name + b'\0' + r.bytes(0x40), BUFFER: r.bytes(0x100), LOG: log, HEAP: heap(),
               IMPORT_TABLE: import_table(log, IMPORTS), DIRS: base + b'\0' + r.bytes(0x103 - len(base)), PATHS: paths}
    stack = []
    for a in args:
        stack.append({'R': lambda: RESULT, 'P': lambda: PATH, 'B': lambda: BUFFER, 'n': lambda: count, 'g': lambda: r.choice([0, 1, 2, 2, 3, r.getrandbits(32)]),
                      'M': lambda: r.choice([0, 1, 2, 3, 0x10, 0x30, 0x50, 0x70, 0x80000001, 0x40000002, 0xc0000013, 0x40000050, r.getrandbits(32)]),
                      'o': lambda: offset, 's': lambda: r.choice([0, 0, 1, 0x100, 0x10000, r.getrandbits(32)]), 'V': lambda: r.choice([0, BUFFER, r.getrandbits(32)]),
                      'F': lambda: r.choice([0, 1, 2, 3, r.getrandbits(32)])}[a]())
    return Call(address, [THIS] + stack, regions, conv='t', ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
