# The landscape file read (src/game/terrain/land: Lnd_Read, 0x560ff0) against the original. The file through vfs.py's mounts and memory source
# (no name now and then: nothing opened): the version byte (3 mostly), the ten ramp bytes (centres and widths any, small ones often), the control map's
# Array3D header and bytes, then three levels of a u32, a byte and thirteen array headers with their bytes (the level the object's byte +1 names read
# with Lnd_ReadLevel, the others skipped by seeks over their bytes); the stream now and then a few bytes short at the end (in the last level's last
# array, which a read takes as it comes: never in a skipped level's headers, whose short reads would leave uninitialised stack). The object's
# arrays with old blocks or none; byte +1 0-2 mostly, else any. The five ramp tables (0x8793a8, 0x100 bytes each) random before.
# usage: land_read.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, Bench, Call, RAM, global_heap, heap, log_region, put, set_return  # noqa: E402
import vfs  # noqa: E402
from gamestring import LOCALE  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged(vfs.RANGES + [(0x560ff0, 0x561330), (0x55e710, 0x55e790), (0x55f470, 0x55f540), (0x55f630, 0x55f730), (0x5c4d50, 0x5c5020),
                               (0x5c5480, 0x5c5520), (0x5d3860, 0x5d38a0), (0x5e78b0, 0x5e79a0), (0x45b9d0, 0x45ba30), (0x424cf0, 0x424d20),
                               (0x5f6040, 0x5f6160), (0x5f61a0, 0x5f6210), (0x5f7180, 0x5f71a0), (0x5f76c0, 0x5f7740), (0x5f7950, 0x5f7990),
                               (0x5f7d60, 0x5f7e70), (0x67a400, 0x690000)]))
OBJ = RAM + 0x8000


def array(r, skipped):
    w, h, d = r.randrange(0, 4), r.randrange(0, 4), r.randrange(1, 4)
    if not skipped and r.random() < 0.1:
        w, h, d = r.randrange(4, 9), r.randrange(4, 9), r.randrange(1, 5)
    return struct.pack('<3I', w, h, d) + r.bytes(w * h * d)


def ramp_byte(r):
    return r.choice([0, 1, 2, 0x7f, 0x80, 0xfe, 0xff, r.getrandbits(8), r.getrandbits(8)])


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000), LOCALE: bytes(4), **global_heap()}
    root = vfs.mounts(r, log, regions)
    obj = bytearray(r.bytes(0x2b0))
    obj[1] = r.choice([0, 1, 2, 0, 1, 2, 3, r.getrandbits(8)])
    for off in [0x14] + [0x2c + 0xd8 * k + a for k in range(3) for a in range(0, 0xd0, 0x10)]:
        put(obj, off + 0xc, 'I', r.choice([0, 0, HEAP + 0x6000 + off]))
    data = bytes([3 if r.random() < 0.9 else r.choice([0, 2, 4, 0x83])]) + bytes(ramp_byte(r) for _ in range(10))
    data += array(r, False)
    for level in range(3):
        arrays = [array(r, level != obj[1]) for _ in range(13)]
        data += r.bytes(5) + b''.join(arrays)
    cut = 0
    if obj[1] == 2 and len(arrays[-1]) > 12 and r.random() < 0.2:
        cut = r.randrange(1, len(arrays[-1]) - 11)
    vfs.memory_source(r, log, regions, data)
    set_return(log, 8, r.choice([len(data)] * 8 + [len(data) - 4, len(data) + 3]))
    # The memory source's bytes left: the file's, or a few short of it (only where Lnd_ReadLevel's last array reads them).
    src = bytearray(regions[vfs.MEMSRC])
    put(src, 8, 'I', len(data) - cut + 0x100 if not cut else len(data) - cut)
    put(src, 0x10, 'I', len(data) - cut + 0x100 if not cut else len(data) - cut)
    regions[vfs.MEMSRC] = src
    regions[OBJ] = obj
    regions[0x8793a8] = r.bytes(0x500)   # the five ramp tables (0x8793a8 .. 0x8797a8), compared
    regions[vfs.NAME] = vfs.name_under(r, root) + b'.lnd\0' + r.bytes(7)
    return Call(0x560ff0, [OBJ, vfs.NAME if r.random() < 0.97 else 0], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
