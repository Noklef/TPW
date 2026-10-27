# A holder made from a .md2 file (0x46dcf0, src/game/model/holder: Holder_LoadMd2) against the original: texdirs.py's vfs_case() (vfs.py's
# mounts, files.py's quickload tables, the texture directories; the heap not failing) with md2file.py's made-up model in vfs.py's memory
# source (its size the file's mostly), its geometry flag set and its materials' names (+0xc) at the file's end (files.py's names mostly, now
# and then one under the root) for TextureSet_Load; flag 0 or not (MD2_Load's flags 2 or 6, the model's motion block mostly as they ask).
# With noPart 0 (some two cases in five) the part is made and loaded (Holder_CreateAt20, ObjModel_Load): then the model has no materials
# (ObjModel_Load's texture sets not set up here), its geometry nodes 0-3 colours (+0x5e) and the word +0x3a 0-3 (the tracked block's size),
# the procedural flag (+0x30 bit 1) any with no procedural object (the error pushed on objmodel.py's lost list), the tracked blocks as
# wctimage.py's, the live objects as objmodel.py's, no loading screen (0x8bd508 0) and no deferred loads (0x8bd550's 0x100 clear).
# usage: holderload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, Bench, Call, heap, put, set_return  # noqa: E402
import engineerrors  # noqa: E402
import md2file  # noqa: E402
import objmodel  # noqa: E402
import texdirs  # noqa: E402
import vfs  # noqa: E402
import wctimage  # noqa: E402

B = Bench(texdirs.merged(md2file.RANGES + texdirs.RANGES + objmodel.RANGES + [(0x46dcf0, 0x46dd4c), (0x45ba70, 0x45ba92)]))


def model(r, flag, part):
    """md2file.py's model for MD2_Load's flags, the geometry flag set; the materials named (none with a part); see the header."""
    d = bytearray(md2file.model(r, 6 if flag else 2))
    put(d, 0x30, 'I', struct.unpack_from('<I', d, 0x30)[0] | 1)
    if part:
        put(d, 0x36, 'H', 0)
        put(d, 0x3a, 'H', r.randrange(4))
        nodes, count = struct.unpack_from('<IH', d, 0x70)[0], struct.unpack_from('<H', d, 0x44)[0]
        for i in range(count if nodes else 0):
            put(d, nodes + 0xa0 * i + 0x5e, 'H', r.choice([0, 0, 1, 2, 3]))
        return d
    table, count = struct.unpack_from('<I', d, 0x54)[0], struct.unpack_from('<H', d, 0x36)[0]
    for i in range(count if table else 0):
        put(d, table + 0x10 * i + 0xc, 'I', len(d))
        d += (texdirs.files.name(r) if r.random() < 0.8 else vfs.name_under(r, r.root)) + b'\0'
    return d


def case(r):
    part = r.random() < 0.4
    flag = r.choice([0, 0, 1, r.getrandbits(32)])
    regions, r.root = texdirs.vfs_case(r, [0])
    log = regions[LOG]
    regions[HEAP] = heap(0x10000)
    data = bytes(model(r, flag, part))
    vfs.memory_source(r, log, regions, data)
    set_return(log, 8, r.choice([len(data)] * 8 + [len(data) - 4, len(data) + 3]))
    regions[vfs.NAME] = vfs.name_under(r, r.root) + b'.md2\0' + r.bytes(7)
    if part:
        wctimage.tracked(r, regions)
        regions[0x879ad0] = engineerrors.lost_list(r)
        regions[0x879d60] = bytes(4)
        regions[0x8bd508] = bytes(4)
        regions[0x8bd550] = put(bytearray(r.bytes(4)), 0, 'I', r.getrandbits(32) & ~0x40000100)
        regions[0x8bcbc8] = r.bytes(4)
        regions[0x8bcbd0] = struct.pack('<I', r.choice([0x200, 0x280, r.getrandbits(32)]))
        regions[objmodel.LIVE] = put(bytearray(r.bytes(0x808)), 0x800, 'I', r.randrange(0x200))
    args = [vfs.NAME if r.random() < 0.97 else 0, r.getrandbits(32), 0 if part else r.choice([1, r.getrandbits(32) | 1]), r.getrandbits(32), flag]
    return Call(0x46dcf0, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)
