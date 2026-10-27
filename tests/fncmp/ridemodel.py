# A ride's model and motions loaded (src/game/model/ridemodel: 0x461f10, RideModel_Load) against the original: texdirs.py's vfs_case() (vfs.py's
# mounts, files.py's quickload tables, the texture directories; the heap not failing) with the model and its motion files in vfs.py's
# memory_files() under the paths the code makes: the model at the directory (TexDirs_First: the table's first index, or none) and the name
# (holderload.py's model, geometry flag set, no motion block mostly; without flags 8 the part-friendly one, its part made and loaded as in
# holderload.py: wctimage.py's tracked blocks, objmodel.py's live objects); 0-3 of the 12 kinds (the letters at 0x6fe6bc) with motion files
# numbered from 1 (now and then with a gap, or a later one without the first), or one unnumbered, or both: md2file.py's models for flags 0 (a
# motion block mostly; now and then not, or a wrong version or sub-version: then the load fails, or the model is freed) whose tracks name nodes
# of the model (+0x14 below its +0x42), flags 0x1000 / 0x10000 any (0x10000 only on geometry nodes). The model's nodes: names (+0x54) "Sky"
# in some case or another, a tree of all of them (+0xc first child, +8 next) whose root is +0x78 (or a stand-in when there are none), +4 at
# a scratch dword, groups (+0x6c, +0x5a of them) pointing at scratch words or none, 0-3 texture coordinates (+0x5e at +0x68) and no
# triangles (+0x5c) for RideBase_TidyAllUVs (flags 0x400: the other nodes flagged 0x200, skipped), the parts (+0x48 of them from node
# +0x46, records +0x7c) none mostly. The globals the maxima are raised in small, [0x7a445c] 0 mostly (else the quickload tables are asked: slow).
# The name and the directories short (the name now and then starting with its only '.'; each kind's paths are printed with sprintf, slow in the emulator). Flags of the bits the code reads (and now and then
# others); outName a buffer or none; layers 0-4.
# usage: ridemodel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, fb, heap, put  # noqa: E402
import engineerrors  # noqa: E402
import holderload  # noqa: E402
import md2file  # noqa: E402
import objmodel  # noqa: E402
import texdirs  # noqa: E402
import vfs  # noqa: E402
import wctimage  # noqa: E402

B = Bench(texdirs.merged(md2file.RANGES + texdirs.RANGES + objmodel.RANGES + [
    (0x461f10, 0x462870), (0x46dcf0, 0x46dd4c), (0x45ba70, 0x45bad0), (0x44a220, 0x44a290), (0x44a870, 0x44aaf0), (0x45b9d0, 0x45ba30),
    (0x45bbc0, 0x45bc70), (0x461820, 0x4618b0), (0x461920, 0x461a70), (0x467310, 0x467a90), (0x470a00, 0x470b70), (0x472bc0, 0x472c70),
    (0x472f40, 0x472f70), (0x473e30, 0x473f60), (0x5da3c0, 0x5da3c1)]))
KINDS = b'CDILSMEUWBRO'   # (0x6fe6bc: a letter every 8 bytes)
OUT, OUTNAME = RAM + 0x9f000, RAM + 0x9f100
FLAG_BITS = ((1, .2), (2, .4), (4, .3), (8, .5), (0x10, .3), (0x20, .3), (0x40, .3), (0x80, .3), (0x100, .3), (0x200, .15), (0x400, .15),
             (0x800, .3), (0x1000, .3), (0x2000, .3), (0x4000, .3), (0x8000, .3), (0x20000, .5), (0x40000, .3), (0x800000, .3))


def u16(d, at):
    return struct.unpack_from('<H', d, at)[0]


def u32(d, at):
    return struct.unpack_from('<I', d, at)[0]


def first_dir(regions):
    """TexDirs_First's directory as texdirs.vfs_case() sets it: the table's entry at the first index, or None for -1."""
    i = struct.unpack_from('<i', regions[0x74d980], 0)[0]
    if i == -1:
        return None
    text = regions[texdirs.DIRTEXT]
    at = u32(regions[texdirs.DIRS], 4 * i) - texdirs.DIRTEXT
    return bytes(text[at:text.index(b'\0', at)])


def main_path(name, d, flags):
    p = name if d is None or flags & 0x200 else d + name
    dot = p.rfind(b'.')
    return p + b'.md2' if dot <= 0 or p[dot + 1:dot + 2] == b'\\' else p


def base_of(name):
    dot = name.rfind(b'.')
    return name[:dot] if dot >= 0 and name[dot + 1:dot + 2] != b'\\' else name


def ride_model(r, flags):
    """holderload.py's model (the part-friendly one without flags 8), its nodes, parts and root made usable; see the header."""
    d = bytearray(holderload.model(r, 0, not flags & 8))
    geo, total = u16(d, 0x44), u16(d, 0x42)
    nodes = [u32(d, 0x70) + 0xa0 * i for i in range(geo)] + [u32(d, 0x74) + 0x58 * i for i in range(total - geo)]
    while len(d) % 4:
        d.append(0)
    scratch = len(d)
    d += r.bytes(0x40)
    for i, at in enumerate(nodes):
        put(d, at + 4, 'I', scratch + 4 * r.randrange(16))
        put(d, at + 8, 'II', 0, 0)
        if i:
            parent = nodes[r.randrange(i)]
            put(d, at + 8, 'I', u32(d, parent + 0xc))
            put(d, parent + 0xc, 'I', at)
        put(d, at + 0x54, 'I', len(d))
        d += r.choice([b'Sky', b'sky', b'SKY', b'Sky1', b'Sk', b'rock', b'']) + b'\0'
        if i >= geo:
            if flags & 0x400:
                put(d, at, 'I', u32(d, at) | 0x200)
            continue
        if flags & 0x400 and r.random() < 0.2:
            put(d, at, 'I', u32(d, at) | 0x200)
        groups = u32(d, at + 0x6c)
        if not groups:
            put(d, at + 0x5a, 'H', 0)
        for j in range(u16(d, at + 0x5a)):
            put(d, groups + 0x10 * j, 'IHH', r.choice([0, scratch + 2 * r.randrange(32)]), 0, 0)
        verts = u16(d, at + 0x5e) if not flags & 8 else r.choice([0, 0, 1, 2, 3])
        put(d, at + 0x5c, 'HH', 0, verts)
        put(d, at + 0x68, 'I', len(d))
        d += b''.join(struct.pack('<I', fb(r.choice([0.0, 1.0, 0.5, 2.01, -0.995, r.uniform(-3, 3)]))) for _ in range(8))
    if nodes:
        put(d, 0x78, 'I', nodes[0])
    else:
        put(d, 0x78, 'I', len(d))
        d += put(bytearray(0x60), 0x50, 'H', 0xffff)
    put(d, 0x48, 'H', 0)
    if total and r.random() < 0.4:
        first = r.randrange(total)
        parts = r.randint(1, min(3, total - first))
        put(d, 0x46, 'HH', first, parts)
        put(d, 0x7c, 'I', len(d))
        for _ in range(parts):
            d += put(bytearray(r.bytes(0x14)), 0, 'I', r.getrandbits(32) & 0x5007a | r.choice([0, 0x10, 0x20, 0x30]))
            put(d, len(d) - 8, 'II', 0, 0)
    return bytes(d), total, geo


def motion_file(r, total, geo):
    """md2file.py's model for flags 0 with its tracks naming the ride model's nodes; see the header."""
    d = bytearray(md2file.model(r, 0))
    motion = u32(d, 0x98)
    if motion:
        tracks = u32(d, motion + 0x2c)
        if not tracks or not total:
            put(d, motion + 0x12, 'H', 0)
        for i in range(u16(d, motion + 0x12)):
            e = tracks + 0x40 * i
            node = r.randrange(total)
            bits = r.choice([0, 0, 0x1000, 0x10000, 0x11000]) if node < geo else r.choice([0, 0x1000])
            put(d, e + 4, 'I', u32(d, e + 4) & ~0x11000 | bits)
            put(d, e + 0x14, 'H', node)
    return bytes(d)


def case(r):
    flags = 0
    for bit, p in FLAG_BITS:
        if r.random() < p:
            flags |= bit
    if r.random() < 0.1:
        flags |= r.getrandbits(32) & 0xff790000
    regions, root = texdirs.vfs_case(r, [0])
    r.root = root   # (holderload.model's material names)
    text = bytearray()   # (the directories short: see the header)
    for i in range(4):
        text += b'a'
        put(regions[texdirs.DIRS], 4 * i, 'I', texdirs.DIRTEXT + len(text))
        text += r.choice([b'', b'd\\', b'tx\\b\\', b'q']) + b'\0'
    regions[texdirs.DIRTEXT] = text
    log = regions[LOG]
    regions[HEAP] = heap(0x20000)
    directory = first_dir(regions)
    name = r.choice([b'', b'', b'rides\\', b'.']) +bytes(r.choice(b'abcxyz019_') for _ in range(r.randint(1, 6))) +r.choice([b'.md2'] * 6 + [b'', b'.MD2', b'.x\\a'])
    data, total, geo = ride_model(r, flags)
    files = {}
    if r.random() < 0.95:
        files[main_path(name, directory, flags)] = data
    base = (directory if directory is not None else b'(null)') + base_of(name)
    for kind in r.sample(range(12), r.choice([0, 1, 1, 2, 3])):
        letter = KINDS[kind:kind + 1]
        mode = r.choice(['numbered', 'numbered', 'one', 'both'])
        if mode != 'one':
            numbers = list(range(1, r.choice([1, 1, 2, 3]) + 1))
            if r.random() < 0.15:
                numbers.remove(r.choice(numbers))
            for n in numbers:
                files[base + letter + b'%d.md2' % n] = motion_file(r, total, geo)
        if mode != 'numbered':
            files[base + letter + b'.md2'] = motion_file(r, total, geo)
    vfs.memory_files(r, log, regions, files)
    regions[vfs.NAME] = name + b'\0' + r.bytes(7)
    regions[OUT] = r.bytes(4)
    regions[OUTNAME] = r.bytes(0x200)
    regions[0x7a42ac] = struct.pack('<I', r.choice([0, 1, 2, 4, 0x10]))
    regions[0x7afd90] = struct.pack('<4I', r.choice([0, 2, 8]), 0, 0, r.choice([0, 1, 3]))
    regions[0x7afe10] = struct.pack('<I', r.choice([0, 1, 2, 5]))
    regions[0x7b4980] = r.bytes(4)
    regions[0x7a445c] = struct.pack('<I', 0 if r.random() < 0.94 else r.getrandbits(32))   # (the tables' CRT stricmp is slow)
    if not flags & 8:
        wctimage.tracked(r, regions)
        regions[0x879ad0] = engineerrors.lost_list(r)
        regions[0x879d60] = bytes(4)
        regions[0x8bd508] = bytes(4)
        regions[0x8bd550] = put(bytearray(r.bytes(4)), 0, 'I', r.getrandbits(32) & ~0x40000100)
        regions[0x8bcbc8] = r.bytes(4)
        regions[0x8bcbd0] = struct.pack('<I', r.choice([0x200, 0x280, r.getrandbits(32)]))
        regions[objmodel.LIVE] = put(bytearray(r.bytes(0x808)), 0x800, 'I', r.randrange(0x200))
    args = [vfs.NAME, flags, OUT, OUTNAME if r.random() < 0.7 else 0, r.choice([0, 1, 2, 4])]
    return Call(0x461f10, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)
