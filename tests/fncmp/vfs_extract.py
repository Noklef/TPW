# A file copied out of the virtual file system (src/game/files/vfs: 0x5c5010 VFS_ExtractFile) against the original: the source through vfs.py's
# mounts and memory source (no file now and then), its size (the source's size recorder: the data's, 0 or a few off) and bytes read into a
# Mem_GlobalAlloc block (none for size 0); the file written through a FileStream (vtable 0x6fd930) of recorders (k 20 delete, 21 close,
# 22 write: the block's first 32 dwords hashed, 23 open: the path's text, its result filled in any sign, the result's address returned);
# an allocation failing now and then (the FileStream's: both fault).
# usage: vfs_extract.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, Bench, Call, RAM, blob, echo, fill, global_heap, heap, log_region, put, recorder, set_return, strings  # noqa: E402
import vfs  # noqa: E402
from gamestring import LOCALE  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged(vfs.RANGES + [(0x5c5010, 0x5c510f), (0x5c4d50, 0x5c5020), (0x5f5660, 0x5f56a0), (0x5f6040, 0x5f6160), (0x5f61a0, 0x5f6210),
                               (0x5f7180, 0x5f71a0), (0x5f76c0, 0x5f7740), (0x5f7950, 0x5f7990), (0x5f7d60, 0x5f7e70), (0x67a400, 0x690000)]))
TO = RAM + 0x8000
FILE_STREAM_VTABLE = 0x6fd930


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000, fail=r.choice([0] * 40 + [1, 2, 3, 4, 5])), LOCALE: bytes(4), **global_heap()}
    root = vfs.mounts(r, log, regions)
    data = r.bytes(r.choice([0, 1, 4, 7, 40, 100, 128, r.randrange(1, 300)]))
    if r.random() < 0.95:
        vfs.memory_source(r, log, regions, data)
        set_return(log, 8, r.choice([len(data)] * 6 + [0, len(data) + 3, max(len(data) - 2, 0)]))
    vt = bytearray(r.bytes(0x40))
    put(vt, 0, '2I', recorder(20, 1), recorder(21, 0))
    put(vt, 0xc, 'I', recorder(22, 2))
    put(vt, 0x24, 'I', recorder(23, 3))
    regions[FILE_STREAM_VTABLE] = vt
    blob(log, 22, 0, 0xffffffff)
    strings(log, 23, 2, 1)   # (the open: the path as text, the result's address 0)
    fill(log, 23, 0, struct.pack('<i', r.choice([0, 0, 0, 1, -1, -0x80000000, 0x7fffffff])), which=2)
    echo(log, 23, 0)
    regions[vfs.NAME] = vfs.name_under(r, root) + b'.ttf\0' + r.bytes(7)
    regions[TO] = r.choice([b'C:\\tp\\font.ttf', b'x', b'']) + b'\0' + r.bytes(7)
    return Call(0x5c5010, [vfs.NAME, TO], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
