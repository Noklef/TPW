# An engine object's model unload (src/game/model/objmodel: 0x57a100) against the original: no model now and then; else 0-3 meshes
# (flag 0x200 or not, 0-3 faces whose first pointers are null or point at words with 0x1000 or not) under the renderer flag 0x20 or
# not, 0-3 texture sets (or none) with 1, 2 or 0 users (their own counts 0: the tracked blocks only), the saved pairs off, the block
# at +0x13c none or one without a texture, the tracked blocks (+0x144, the meshes' +0x74, the sets' +8) as in spritesfree.py's
# table, the model's flag 2 with the procedural texture object (no cache; a tracked block or none) there or not.
# The load (0x579e00): see load(). A holder's part made (0x45bbc0, model/holder): see create(). The object's matrices set (0x57a260):
# see matrices() and matrices_env().
# usage: objmodel.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Unload,Load,Create,Matrices)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, _cdecl_log, blob, fill, global_heap, heap, log_region, put, recorder, set_return, strings  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import engineerrors  # noqa: E402
import loadscreen  # noqa: E402
import texcache  # noqa: E402
import texset  # noqa: E402
import wctimage  # noqa: E402

B = Bench(wctimage.merged([(0x57a100, 0x57a258), (0x579b80, 0x579c13), (0x57b2e0, 0x57b393), (0x57b070, 0x57b130), (0x57b5b0, 0x57b608),
                           (0x575f00, 0x575f8d), (0x56e4e0, 0x56e549), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1), (0x579e00, 0x57a0f2),
                           (0x54dcc0, 0x54dce8), (0x45bbc0, 0x45bc58), (0x579cd0, 0x579df1), (0x5775d0, 0x5775f3), (0x57a260, 0x57a41c)] + texset.RANGES + texcache.RANGES
                          + loadscreen.RANGES), hooks={0x67a7d0: _cdecl_log})
CALLS = {'Unload': 0x57a100, 'Load': 0x579e00, 'Create': 0x45bbc0, 'Matrices': 0x57a260}
OBJ, MODEL, SETS, SET, MESHES, FACES, WORDS, BLOCK, TEX = (RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x1000, RAM + 0x2000,
                                                        RAM + 0x2400, RAM + 0x2600, RAM + 0x3000)
COUNT, TABLE = 0x8bb214, 0x87b1f8
BLOCKS = [HEAP + 0x20, HEAP + 0x40, HEAP + 0x3c]


def block(r):
    return r.choice([0] + BLOCKS + [r.getrandbits(32) | 0x80000000])


def unload(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    table = bytearray(8 * 8)
    for k in range(8):
        b = r.choice(BLOCKS + [r.getrandbits(32) | 0x80000000])
        put(table, 8 * k, '2I', b, r.choice([b, b, b + 4]) & 0xffffffff)
    regions[TABLE] = table
    regions[COUNT] = struct.pack('<I', r.choice([0, 1, 2, 3, 5]))
    obj = bytearray(r.bytes(0x150))
    put(obj, 4, 'I', r.getrandbits(32) & ~0x60000000)
    put(obj, 8, 'I', MODEL if r.random() < 0.9 else 0)
    put(obj, 0x13c, 'I', r.choice([0, BLOCK]))
    put(obj, 0x144, 'I', block(r))
    model = bytearray(r.bytes(0x80))
    sets, meshes = r.choice([0, 1, 2, 3]), r.choice([0, 1, 2, 3])
    put(model, 0x30, 'B', r.getrandbits(8))
    put(model, 0x36, 'H', sets)
    put(model, 0x44, 'H', meshes)
    put(model, 0x50, 'I', SETS)
    put(model, 0x70, 'I', MESHES)
    pairs, sets_ = bytearray(r.bytes(0x20)), bytearray(r.bytes(0x40))
    for k in range(4):
        put(pairs, 8 * k + 4, 'I', SET + 0x10 * k if r.random() < 0.8 else 0)
        put(sets_, 0x10 * k + 8, 'I', block(r))
        put(sets_, 0x10 * k + 0xc, 'hh', 0, r.choice([1, 1, 2, 0]))
    faces = bytearray(r.bytes(0x10 * 12))
    words = bytearray(r.bytes(0x20))
    for k in range(12):
        put(faces, 0x10 * k, 'I', r.choice([0, WORDS + 2 * r.randrange(16)]))
    for k in range(16):
        put(words, 2 * k, 'H', r.getrandbits(16) & ~0x1000 | r.choice([0, 0, 0x1000]))
    mesh = bytearray(r.bytes(0xa0 * 4))
    for k in range(4):
        put(mesh, 0xa0 * k, 'I', r.getrandbits(32) & ~0x200 | r.choice([0, 0, 0x200]))
        put(mesh, 0xa0 * k + 0x5a, 'H', r.choice([0, 1, 2, 3]))
        put(mesh, 0xa0 * k + 0x6c, 'I', FACES + 0x30 * k)
        put(mesh, 0xa0 * k + 0x74, 'I', block(r))
    blk = bytearray(r.bytes(0x20))
    put(blk, 0x18, 'I', 0)
    tex = bytearray(r.bytes(0x234))
    put(tex, 0x230, 'I', 0)
    regions.update({OBJ: obj, MODEL: model, SETS: pairs, SET: sets_, MESHES: mesh, FACES: faces, WORDS: words, BLOCK: blk, TEX: tex,
                    TEX + 0x12970: struct.pack('<I', block(r)), 0x879d60: struct.pack('<I', r.choice([0, TEX])),
                    0x8bd550: struct.pack('<I', r.getrandbits(32))})
    return Call(0x57a100, [OBJ], regions, conv='t', ret='a')


LOBJ, LMESHES, LPARTS, LPAIRS, LINFOS, LDEVICE, LDEVVT = (RAM + 0x40000, RAM + 0x41000, RAM + 0x42000, RAM + 0x43000, RAM + 0x43100,
                                                          RAM + 0x43200, RAM + 0x43280)
LMODEL, LTIME, LFOCUS = RAM + 0x41800, 50, 51


def tick(r, regions):
    """The loading screen's step (0x8c4748) taken now and then (0x8bd508's 1) on its quiet path: past the first step, both images set
    and three or more shown (nothing shown, no bar); timeGetTime a recorder; the restore as renderstate.py's (the focus the window or
    not, the device's slot 1 any result, the error list's surface-lost drop)."""
    log = regions.setdefault(LOG, log_region())
    screen = bytearray(r.bytes(0x48))
    put(screen, 0, 'I', r.randrange(1, 1 << 32))
    put(screen, 0x10, 'II', r.randrange(1, 1 << 32), r.randrange(1, 1 << 32))
    put(screen, 0x2c, 'i', r.choice([3, 4, 0x7fffffff]))
    regions[0x8c4748] = screen
    regions[0x6fd3ec] = struct.pack('<I', recorder(LTIME, 0, True))
    regions[0x6fd328] = struct.pack('<I', recorder(LFOCUS, 0, True))
    set_return(log, LTIME, r.getrandbits(32))
    window = r.getrandbits(32)
    set_return(log, LFOCUS, r.choice([window, r.getrandbits(32)]))
    set_return(log, 21, r.choice([0, 1, r.getrandbits(32)]))
    flags = bytearray(r.bytes(0x28))   # (0x8bd508 the flags, 0x8bd518 the window, 0x8bd52c the device)
    put(flags, 0, 'I', r.getrandbits(32) & ~0x1000001 | (1 if r.random() < 0.4 else 0))
    put(flags, 0x10, 'I', window)
    put(flags, 0x24, 'I', LDEVICE)
    regions.update({0x8bd508: flags, LDEVICE: struct.pack('<I', LDEVVT) + r.bytes(0xc), LDEVVT: struct.pack('<2I', recorder(20, 0), recorder(21, 0)),
                    0x8bcc40: r.bytes(8)})


def model_bytes(r, regions, model, sets):
    """The model's fields: flags (+0x30: 2 off, 0x200 or not), its name (+0x18) "BlueprintMesh" or near it, 0-3 parts (+0x3c, +0x64),
    `sets` texture sets (+0x36; pairs +0x50, infos +0x54), 0-3 meshes (+0x44, +0x70) with 0-3 faces at +0x5e (none at +0x5a: the unload
    scans those), +0x3a at least their sum; the sharp settings (0x8bcbc8's 2, the screen 0x200 / 0x280 wide, Flag54dcc0_Test's
    settings); the object (+0x13c none)."""
    put(model, 0x18, '16s', r.choice([b'BlueprintMesh', b'BlueprintMesh', b'BlueprintMes', b'BlueprintMeshes', b'']))
    parts = r.choice([0, 1, 2, 3])
    put(model, 0x3c, 'H', parts)
    put(model, 0x64, 'I', LPARTS)
    regions[LPARTS] = r.bytes(0x40)
    put(model, 0x36, 'H', sets)
    put(model, 0x50, '2I', LPAIRS, LINFOS)
    meshes = r.choice([0, 1, 2, 3])
    faces = [r.choice([0, 0, 1, 2, 3]) for _ in range(4)]
    mesh = bytearray(r.bytes(0xa0 * 4))
    for k in range(4):
        put(mesh, 0xa0 * k + 0x5a, 'H', 0)
        put(mesh, 0xa0 * k + 0x5e, 'H', faces[k])
    regions[LMESHES] = mesh
    put(model, 0x44, 'H', meshes)
    put(model, 0x3a, 'H', sum(faces[:meshes]) + r.choice([0, 0, 1, 3]))
    put(model, 0x70, 'I', LMESHES)
    put(model, 0x30, 'I', struct.unpack_from('<I', model, 0x30)[0] & ~0x202 | r.choice([0, 0x200]))
    regions[0x8bcbc8] = put(bytearray(regions.get(0x8bcbc8, r.bytes(4))), 0, 'B', r.getrandbits(8))
    regions[0x8bcbd0] = struct.pack('<I', r.choice([0x200, 0x280, 0x200, 0x280, 0x320]))
    regions[0x78d968] = struct.pack('<I', r.choice([0, 1]))
    regions[0x785874] = struct.pack('<I', r.choice([0, 1, 2]))
    regions[0x79d86c] = r.bytes(4)
    regions[LOBJ] = put(bytearray(r.bytes(0x150)), 0x13c, 'I', 0)


def load(r):
    """0x579e00: a terrain model (flag 2: texcache.py's Setup scene with the model as the land, the procedural object [0x879d60] or none,
    the header +0x6c or none; no texture sets; the heap not failing: the unload would find a procedural object with no cache); a model of
    0-2 texture sets as texset.py's set-up (each pair's definition the same, the first's set made now or texset.py's set, the others made
    now; the same info for each); or 1-2 pairs of texset.py's listed set (nothing allocated, so the tracked block can fail, the model then
    unloaded, the set keeping a user). 0x8bd550's 0x100 or not; see model_bytes() and tick()."""
    kind = r.choice(['terrain', 'sets', 'sets', 'listed'])
    if kind == 'terrain':
        while True:   # (not its wrapping sizes: the frame buffer's then fault in both, texcache.py's)
            regions = texcache.view_case(r, 'Setup').regions
            if max(struct.unpack_from('<2I', regions[texcache.HDR], 0x18)) < 0x100:
                break
        regions[HEAP] = heap(0x8000)
        regions[texcache.TABLE - 0x18] += r.bytes(0x20)   # (room for the tracked block's pair past the scene's eight)
        model = regions[texcache.LANDOBJ]
        model_bytes(r, regions, model, 0)
        put(model, 0x30, 'I', struct.unpack_from('<I', model, 0x30)[0] | 2)
        if r.random() < 0.1:
            put(model, 0x6c, 'I', 0)
        regions[0x879d60] = struct.pack('<I', RAM if r.random() < 0.9 else 0)
        regions[0x8bd550] = r.bytes(4)
        regions[LPAIRS] = r.bytes(8)
        address = texcache.LANDOBJ
    else:
        while True:
            regions = texset.init(r, 'Init').regions
            s = bytearray(regions[texset.SET2])
            names = [regions[texset.NAMES][0x14 * k:0x14 * k + 0x14].split(bytes(1))[0] for k in range(4)]
            if all(b'.' in n.lstrip(b'.') for n in names) and (kind == 'sets' or struct.unpack_from('<I', s, 8)[0]):
                break   # (a name without a '.' faults in both: texset.py's)
        regions[HEAP] = heap(0x40000)   # (a set-up failing leaves no list for the sharp textures: texset.py fails its allocations)
        regions[texset.DEFERRED] = put(bytearray(regions[texset.DEFERRED]), 0x400, 'i', 0)   # (only the model's sets wait)
        regions[texset.LIST2] = struct.pack('<4I', texset.TEXS, texset.TEXS + 0x60, texset.TEXS, texset.TEXS + 0x60)   # (as many as sharp asks)
        if kind == 'listed':
            put(s, 0xe, 'h', r.choice([1, 2]))
            regions[texset.SET2] = bytes(s)
            sets_at = [texset.SET2] * r.choice([1, 2])
            if r.random() < 0.4:
                regions[HEAP] = heap(0x40000, fail=1)   # (the tracked block's the first allocation)
        else:
            sets_at = [r.choice([texset.SET2, 0]) if k == 0 else 0 for k in range(r.choice([0, 1, 2, 2]))]
        model = bytearray(r.bytes(0x80))
        model_bytes(r, regions, model, len(sets_at))
        regions[LPAIRS] = b''.join(regions[texset.DEF][:4] + struct.pack('<I', a) for a in sets_at) + r.bytes(8)
        regions[LINFOS] = regions[texset.INFO] * 2
        regions[0x8bd550] = put(bytearray(regions[0x8bd550]), 0, 'I', struct.unpack_from('<I', regions[0x8bd550], 0)[0] & ~0x40000100
                                | r.choice([0, 0x100]))
        regions[LMODEL] = model
        address = LMODEL
    regions[0x879ad0] = engineerrors.lost_list(r)
    tick(r, regions)
    return Call(0x579e00, [LOBJ, address], regions, conv='t', ret='a')


HOLDER, OWNER, LIVE = RAM + 0x48000, RAM + 0x48100, 0x87a6d8


def create(r):
    """0x45bbc0: load()'s case with its model at the holder's +4, the part made first (Mem_Alloc(0x158), its allocation failing now and
    then: 0x579e00 then given no object, a crash in both; load()'s failing allocation one later), constructed (most derived; the live
    objects 0-0x1ff listed) and loaded; the holder's owner (+8) or none, the count (+0x34) its or the holder's."""
    c = load(r)
    regions = c.regions
    first = struct.unpack_from('<I', regions[HEAP], 0)[0]
    fail = 1 if r.random() < 0.05 else (first & 15) + 1 if first & 15 else 0
    regions[HEAP] = put(bytearray(regions[HEAP]), 0, 'I', first & ~15 | fail)
    holder = put(bytearray(r.bytes(0x40)), 4, 'I', c.args[1])
    put(holder, 8, 'I', r.choice([0, OWNER]))
    regions.update({HOLDER: holder, OWNER: r.bytes(0x40), LIVE: put(bytearray(r.bytes(0x808)), 0x800, 'I', r.randrange(0x200))})
    return Call(0x45bbc0, [HOLDER], regions, ret='v')


XF, XFVT, MATRIX = RAM + 0x49000, RAM + 0x49100, RAM + 0x49200


def matrices_env(r, regions):
    """0x57a260's surroundings: the matrix at MATRIX (any dwords); the vertex transformer [0x87b1d8] whose slot 6 is CDECL_LOG4, the copy and
    the flip it is given (the game's frame: the flip, then the copy 0x40 on) hashed through the flip's pointer and the copy's own pointer
    logged as 0 (stack addresses that differ between the sides), the static matrix it is given filled with any bytes now and then; the
    static matrix (0xfb3400) any, built or not (its guard 0xfb33e0)."""
    log = regions.setdefault(LOG, log_region())
    blob(log, 0x3d, 2, 0xffffffff)
    strings(log, 0x3d, 0, out=2)
    if r.random() < 0.7:
        fill(log, 0x3d, 3, r.bytes(0x40), which=1)
    regions.update({0x87b1d8: struct.pack('<I', XF), XF: struct.pack('<I', XFVT) + r.bytes(0xc), XFVT: put(bytearray(r.bytes(0x20)), 0x18, 'I', CDECL_LOG4),
                    MATRIX: r.bytes(0x40), 0xfb33e0: bytes([r.getrandbits(8)]), 0xfb3400: r.bytes(0x40)})
    return regions


def matrices(r):   # 0x57a260: an object (any bytes) given the matrix for its first, second, both or neither (which: a byte, the rest any)
    regions = matrices_env(r, {OBJ: r.bytes(0x100)})
    return Call(0x57a260, [OBJ, r.choice([0, 1, 2, 3, r.getrandbits(32)]), MATRIX], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Load':
        return load(r)
    if op == 'Matrices':
        return matrices(r)
    return create(r) if op == 'Create' else unload(r)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
