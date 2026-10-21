# A sign's style read from its .sgn file (src/game/gfx/signstyle: 0x5ec3a0) against the original. The file through vfs.py's mounts and
# memory source (or no stream: the mounts' answers fail now and then): a version (100, above, below or any), the fields and line styles
# any bytes, the effect and background flags set or not, two fill strips (Array3D_Read's headers, small sizes) and the background (as
# Array3D_Read's for version 100, else Array3D_ReadCompressed's: depth 1, 3 or 4 or another, the codec a recorder); the style's old
# blocks there or not, a failing allocation now and then.
# usage: signstyle.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, fill, global_heap, heap, log_region, put, recorder, set_return  # noqa: E402
import vfs  # noqa: E402
from gamestring import LOCALE  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged(vfs.RANGES + [(0x5ec3a0, 0x5ec690), (0x5c4d50, 0x5c5020), (0x5e78b0, 0x5e7d80), (0x5eb1a0, 0x5eb280), (0x67a400, 0x690000)]))
STYLE, CODEC, CVT = RAM + 0x60000, RAM + 0x60800, RAM + 0x60900


def strip(r):
    """Array3D_Read's bytes: width, height and depth, then the pixels."""
    w, h, d = r.randrange(0, 4), r.randrange(0, 4), r.randrange(1, 5)
    return struct.pack('<3I', w, h, d) + r.bytes(w * h * d)


def compressed(r):
    """Array3D_ReadCompressed's bytes: the header and the depth's part headers and bytes."""
    w, h = r.randrange(0, 4), r.randrange(0, 4)
    depth = r.choice([1, 1, 3, 3, 4, 4, 0, 2])
    head = struct.pack('<3I', w, h, depth)
    lengths = []
    if depth == 1:
        total = r.randrange(0, 0x30)
        lengths = [r.choice([total, r.randrange(0, total + 1)])]
        head += struct.pack('<2I', total, lengths[0])
    elif depth == 3:
        lengths = [r.randrange(0, 0x30)]
        head += struct.pack('<4I', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), lengths[0])
    elif depth == 4:
        lengths = [r.randrange(0, 0x30), r.randrange(0, 0x30)]
        head += struct.pack('<6I', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.randrange(0, 0x40), *lengths)
    return head + b''.join(r.bytes(n) for n in lengths)


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000, r.choice([0] * 12 + [1, 2, 3])), LOCALE: bytes(4), **global_heap()}
    root = vfs.mounts(r, log, regions)
    version = r.choice([100, 100, 100, r.randrange(101, 300), r.randrange(101, 300), 99, 0, -1, -0x80000000, r.getrandbits(31)])
    flags = [r.choice([0, 0, 1, r.getrandbits(32)]) for _ in range(2)]
    background = r.choice([0, 1, 1, r.randrange(2, 0x100)])
    data = struct.pack('<iIB2I', version, r.getrandbits(32), background, *flags) + r.bytes(0x40 + 0x104 + 8 + 0x3c + 0x2c + 0x40 + 0x104 + 8 + 0x3c + 0x2c)
    for flag in flags:
        if flag:
            data += r.bytes(0x14)
    data += strip(r) + strip(r)
    if background:
        data += strip(r) if version == 100 else compressed(r)
    if r.random() < 0.9:
        vfs.memory_source(r, log, regions, data)
        set_return(log, 8, len(data))
    regions[vfs.NAME] = vfs.name_under(r, root) + b'.sgn\0' + r.bytes(7)
    style = bytearray(r.bytes(0x460))
    for off in (0x18, 0x394, 0x400):
        put(style, off + 0xc, 'I', r.choice([0, 0, HEAP + 0x7000 + off]))
    put(style, 0x14, 'I', CODEC)
    regions[STYLE] = style
    regions[CODEC] = struct.pack('<I', CVT) + r.bytes(0xc)
    regions[CVT] = put(bytearray(r.bytes(0x20)), 0xc, 'I', recorder(12, 8))
    if r.random() < 0.5:
        fill(log, 12, 4, r.bytes(r.choice([1, 3, 4])))
    return Call(0x5ec3a0, [STYLE, vfs.NAME], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
