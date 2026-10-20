# (With terrain/land's 0x450a80, which frees the instance behind [0x79fcb4].)
# The holder functions of src/game/model/holder that go through the renderer (0x45bdd0 Holder_Sleep, 0x4254c0 Animator_Sleep, 0x45bc60 Holder_Destroy, 0x45bac0 Holder_Free) against the originals: a
# holder whose part sits in a renderer's slot lists (tags 0x10, 0x20, 0x70) or not; an animator holding such a holder; and a holder whose part has a vtable (a recorder) giving the
# virtual base offset; a holder disposed of (and the owner that holds it); a holder added to the renderer's grid (grid_masks.py's environment, the part reached through its virtual base offset); a user of the holder behind [0x79fcb4] taking it out by its rectangle (rem_rect()'s grid) or by sleeping, and putting it back (Wake: the slot list; Acquire / RideAcquire: the grid rectangle or Wake).
# WakeMatrix (0x45bd30): Wake's environment with the part room for its matrices, given one (objmodel.py's matrices_env), into the grid or
# not; AnimWake (0x425440): an animator holding that holder, with that matrix or none (Holder_Wake then). Matrices: objmodel.py's
# 0x57a260 cases (port.py takes one bench).
# usage: holder.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Matrices,WakeMatrix,AnimWake,Sleep,Animator,Destroy,Release,Grid,Dispose,Owner,AnimGrid,Children,Wake,Acquire,RideAcquire,AddObj,SetPart,Unlink,AnimDispose,InstDispose,Free,InstFree,FreeAll,TerrainFree,RemRect,RemRect14,Clear0)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import grid_masks  # noqa: E402
import objmodel  # noqa: E402
import vobj3  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, _cdecl_log, global_heap, heap, log_region, put, vtable  # noqa: E402

RANGES = [(0x424ed0, 0x424ee9), (0x425490, 0x4254c0), (0x43eac0, 0x43eb00), (0x455a50, 0x455ae0), (0x45ba00, 0x45ba30), (0x4254c0, 0x4254f0), (0x45bc60, 0x45bcb0), (0x45bdd0, 0x45be90), (0x45be80, 0x45bed0), (0x576540, 0x5765d0), (0x5766e0, 0x57681a), (0x5d8820, 0x5d8870), (0x45bcb0, 0x45bd10), (0x576620, 0x5766f0), (0x576270, 0x576320), (0x5e2a80, 0x5e2b80), (0x67acb0, 0x67acb1), (0x590730, 0x590750), (0x5da3c0, 0x5da3c1), (0x45bd00, 0x45bd30), (0x576410, 0x576550), (0x5d8130, 0x5d8190), (0x5d87d0, 0x5d8820), (0x56b2f0, 0x56b340), (0x48f5c0, 0x48f650),
           (0x43bd20, 0x43bdc0), (0x44b270, 0x44b2c0), (0x44b4c0, 0x44b520), (0x475550, 0x4755a0), (0x5f5680, 0x5f56a0), (0x4254f0, 0x4255c0), (0x455ae0, 0x455c00),
           (0x45bac0, 0x45bbbc), (0x57a100, 0x57a258), (0x44aae0, 0x44ab30),
           (0x463db0, 0x463ec5), (0x463ed0, 0x464140), (0x452540, 0x452560), (0x46b3b0, 0x46b3d0), (0x468200, 0x468246), (0x45baa0, 0x45bab4),
           (0x450a80, 0x450abf), (0x427bb0, 0x427bfb), (0x45bd30, 0x45bd99), (0x425440, 0x425483), (0x57a260, 0x57a41c)]
HOOKS = {0x691de0: vobj3.lock, 0x691ea0: lambda e: None, 0x67a7d0: _cdecl_log}
B = Bench(RANGES, hooks=HOOKS)
ANIM, OWNER, TARGET, USER = RAM + 0x2000, RAM + 0x2500, RAM + 0x3000, RAM + 0x3200
UMODEL, UPARTS, UPS, UENTRIES, USCRIPTS = RAM + 0x3400, RAM + 0x3500, RAM + 0x3700, RAM + 0x3740, RAM + 0x3800   # (Unlink's)
ACQ_TARGET, ACQ_USER, AWAKE = RAM + 0x7400, RAM + 0x7600, RAM + 0x7800   # clear of the grid environment (grid_masks.py's vtables sit at RAM + 0x3000)


def sleep(r):
    obj = bytearray(r.bytes(0x400))
    put(obj, 0, 'I', r.getrandbits(32))
    put(obj, 0x1c, 'I', r.choice([0, RAM + 0x100, RAM + 0x100]))
    put(obj, 0x100, 'I', RAM + 0x300)
    offset = r.choice([0, 0x10, 0x20])
    put(obj, 0x304, 'I', offset)
    renderer = bytearray(r.bytes(0x100))
    part = RAM + 0x100 + offset
    nodes = bytearray(0x40)
    for k, slot in enumerate((0x14, 0x18, 0x2c)):
        put(nodes, 8 * k, 'II', part if r.random() < 0.6 else r.getrandbits(32), 0)
        put(renderer, slot, 'I', RAM + 0x1400 + 8 * k if r.random() < 0.8 else 0)
    put(renderer, 4, 'I', 0)
    put(renderer, 0xc, 'I', 5)
    regions = {0x87a6d0: put(bytearray(r.bytes(0x10)), 0, 'I', RAM + 0x1000), RAM: obj, RAM + 0x1000: renderer, RAM + 0x1400: nodes, 0x8bd500: r.bytes(0x20)}
    return Call(0x45bdd0, [RAM], regions, conv='c', ret='v')


def animator(r):
    c = sleep(r)
    c.regions[ANIM] = put(put(bytearray(r.bytes(0x40)), 0, 'I', r.getrandbits(32)), 0x24, 'I', RAM)
    return Call(0x4254c0, [ANIM], c.regions, conv='c', ret='a')


def destroy(r):
    c = sleep(r)
    obj = c.regions[RAM]
    offset = struct.unpack_from('<I', obj, 0x304)[0]
    vt = vtable({0: 1})
    put(vt, 4, 'I', offset)
    obj[0x300:0x300 + len(vt)] = vt
    put(obj, 0x100 + offset, 'I', RAM + 0x300)
    put(obj, 0x8, 'I', r.choice([0, OWNER]))
    c.regions[OWNER] = r.bytes(0x40)
    c.regions[LOG] = log_region()
    return Call(0x45bc60, [RAM], c.regions, conv='c', ret='v')


def release(r):
    if r.random() < 0.5:   # the rectangle path: rem_rect()'s renderer grid
        c = rem_rect(r, 'RemRect')
        regions, lo, hi, flags = dict(c.regions), c.args[1], c.args[2], 0x4000 | r.getrandbits(32)
    else:                  # the sleep path: a holder in the renderer's slot lists
        c = sleep(r)
        regions, lo, hi, flags = dict(c.regions), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32) & ~0x4000
    regions[TARGET] = put(put(put(put(bytearray(r.bytes(0x100)), 4, 'I', flags), 8, 'I', RAM), 0xc0, 'I', lo), 0xc4, 'I', hi)
    regions[USER] = put(bytearray(r.bytes(0x20)), 4, 'I', r.choice([0, 1, 5, 0xffffffff]))
    regions[0x79fcb4] = struct.pack('<I', TARGET)
    return Call(0x5d8820, [USER], regions, conv='t', ret='v')


def dispose(r):
    c = destroy(r)
    c.regions.update(global_heap())
    c.regions[HEAP] = heap()
    return Call(0x455a50, [RAM], c.regions, conv='c', ret='a')


def dispose_owner(r):
    c = destroy(r)
    owner, counter = RAM + 0x5000, RAM + 0x5200
    obj = c.regions[RAM]
    put(obj, 0x24, 'I', r.choice([0, 0x1234, RAM + 0x5400]))
    c.regions[owner] = put(put(put(bytearray(r.bytes(0x60)), 0x24, 'I', RAM), 0x3c, 'I', counter), 0x34, 'I', r.choice([0, 0x4321]))
    c.regions[counter] = r.bytes(0x40)
    c.regions.update(global_heap())
    c.regions[HEAP] = heap()
    return Call(0x455a80, [owner], c.regions, conv='c', ret='a')


def animator_grid(r):
    c = add_to_grid(r)
    anim = RAM + 0x7000
    c.regions[anim] = put(put(bytearray(r.bytes(0x40)), 0, 'I', r.getrandbits(32)), 0x24, 'I', c.args[0])
    return Call(0x425490, [anim] + c.args[1:], c.regions, conv='c', ret='a')


def children(r):
    c = dispose_owner(r)
    n = r.choice([0, 1, 2, 3])
    root = RAM + 0x8000
    kids = [RAM + 0x5000 + 0x1000 * (k + 1) for k in range(n)]
    parent = bytearray(r.bytes(0x60))
    put(parent, 0x38, 'I', kids[0] if n else 0)
    c.regions[root] = parent
    owner = c.args[0]
    for k, addr in enumerate(kids):
        child = bytearray(c.regions[owner])
        put(child, 0x38, 'I', kids[k + 1] if k + 1 < n else 0)
        c.regions[addr] = child
    return Call(0x43eac0, [root], c.regions, conv='c', ret='v')


def add_to_grid(r):
    grid_masks.OPS = ['Grid']
    c = grid_masks.case(r)
    regions = dict(c.regions)
    obj, part, vtp, holder = c.args[5], c.args[5] - 0x100, RAM + 0x6000, RAM + 0x6100
    regions[part] = struct.pack('<I', vtp)
    regions[vtp] = put(bytearray(8), 4, 'I', 0x100)    # the part's virtual base offset
    regions[holder] = put(put(bytearray(r.bytes(0x40)), 0, 'I', r.getrandbits(32)), 0x1c, 'I', part)
    regions[0x87a6d0] = struct.pack('<I', c.args[0])
    xs = [r.choice([0, 1, 2, 3, 5, 0xfffe, 0xffff]) for _ in range(4)]
    return Call(0x45bcb0, [holder, (xs[0] << 16) | xs[1], (xs[2] << 16) | xs[3]], regions, conv='c', ret='v')


def clean_slots(r, c):
    """The grid environment with the renderer's slot heads (+0x14..+0x2f) empty, or the last one the grid's first node."""
    regions = dict(c.regions)
    renderer_at = struct.unpack('<I', regions[0x87a6d0])[0]
    renderer = bytearray(regions[renderer_at])
    renderer[0x14:0x30] = bytes(0x1c)
    if r.random() < 0.5 and struct.unpack_from('<I', regions[grid_masks.NODES])[0]:
        put(renderer, 0x2c, 'I', grid_masks.NODES)
    regions[renderer_at] = renderer
    return Call(c.address, c.args, regions, conv=c.conv, ret=c.ret)


def wake(r):
    c = clean_slots(r, add_to_grid(r))
    return Call(0x45bd00, c.args[:1], c.regions, conv='c', ret='v')


def wake_matrix(r):
    c = clean_slots(r, add_to_grid(r))
    regions = dict(c.regions)
    holder = c.args[0]
    part = struct.unpack_from('<I', regions[holder], 0x1c)[0]
    regions[part] = put(bytearray(r.bytes(0x98)), 0, 'I', struct.unpack_from('<I', regions[part])[0])   # (room for its flags and matrices)
    objmodel.matrices_env(r, regions)
    return Call(0x45bd30, [holder, objmodel.MATRIX, r.choice([0, 0, 1, 0x100, r.getrandbits(32)])], regions, conv='c', ret='v')


def animator_wake(r):
    c = wake_matrix(r)
    regions = dict(c.regions)
    regions[AWAKE] = put(put(bytearray(r.bytes(0x40)), 0xc, 'I', r.choice([0, objmodel.MATRIX, objmodel.MATRIX])), 0x24, 'I', c.args[0])
    return Call(0x425440, [AWAKE], regions, conv='c', ret='a')


def acquire(r, ride):
    c = clean_slots(r, add_to_grid(r))
    regions = dict(c.regions)
    lo, hi = c.args[1], c.args[2]
    flags = r.getrandbits(32) | 0x4000 if r.random() < 0.5 else r.getrandbits(32) & ~0x4000
    regions[ACQ_TARGET] = put(put(put(put(bytearray(r.bytes(0x100)), 4, 'I', flags), 8, 'I', c.args[0]), 0xc0, 'I', lo), 0xc4, 'I', hi)
    done = r.choice([0, 0, 1, 5, 0xffffffff])
    if ride:
        index = r.choice([0, 1, 2])
        regions[0x7a4610] = b''.join(struct.pack('<I', ACQ_TARGET if k == index else r.getrandbits(32)) for k in range(3))
        regions[ACQ_USER] = put(put(bytearray(r.bytes(0x20)), 4, 'I', index), 8, 'I', done)
        return Call(0x5d8130, [ACQ_USER], regions, conv='t', ret='v')
    regions[0x79fcb4] = struct.pack('<I', ACQ_TARGET)
    regions[ACQ_USER] = put(bytearray(r.bytes(0x20)), 4, 'I', done)
    return Call(0x5d87d0, [ACQ_USER], regions, conv='t', ret='v')


def add_obj(r):   # an object {flags, part} (render/renderer Obj_AddToRenderer) in the renderer's tag 0x60 list: the holder's cell stands in, its part pointer copied to +4
    c = clean_slots(r, add_to_grid(r))
    holder = c.args[0]
    regions = dict(c.regions)
    obj = bytearray(regions[holder])
    put(obj, 0, 'I', r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    if r.random() < 0.9:
        put(obj, 4, 'I', struct.unpack_from('<I', obj, 0x1c)[0])
    else:
        put(obj, 4, 'I', 0)
    regions[holder] = obj
    if r.random() < 0.1:
        regions[0x87a6d0] = struct.pack('<I', 0)
    return Call(0x56b2f0, [holder], regions, conv='t', ret='a')


def set_part(r):   # an object whose part at +0xb4 is the grid environment's part (offset 0x100 behind its vtable), the words at +8 and +0x44
    c = clean_slots(r, add_to_grid(r))
    regions = dict(c.regions)
    holder = c.args[0]
    part = struct.unpack_from('<I', regions[holder], 0x1c)[0]    # the part pointer the holder keeps (a vtable pointer behind it, offset 0x100)
    me = part - 0xb4
    cur = r.choice([0, 5, 9])
    value = r.choice([0, 0, 5, 9, 0x10005, r.getrandbits(16)])
    for k, v in regions.items():
        for a, n in ((me + 8, 2), (me + 0x44, 2)):
            assert a + n <= k or k + len(v) <= a, 'overlap'
    regions[me + 8] = struct.pack('<H', cur)
    regions[me + 0x44] = struct.pack('<H', r.choice([0, 1, 0xffff]))
    return Call(0x48f5c0, [me, value], regions, conv='t', ret='v')


def unlink(r):   # 0x43bd20: Release's holder (behind [0x7a4610 + 4 * index]), its model's parts with flag 0x80 whose part states are active released
    c = release(r)
    regions = c.regions
    n = r.choice([0, 1, 3, 6, 8])
    model = put(bytearray(r.bytes(0x80)), 0x48, 'H', n)
    put(model, 0x7c, 'I', UPARTS)
    parts = bytearray(r.bytes(0x14 * 8))
    for k in range(8):
        put(parts, 0x14 * k, 'I', r.getrandbits(32) & ~0x80 | r.choice([0, 0x80, 0x80]))
    entries = bytearray(r.bytes(0x14 * 8))
    for k in range(8):
        put(entries, 0x14 * k, 'I', r.getrandbits(32) & ~2 | r.choice([0, 2, 2]))
        put(entries, 0x14 * k + 0xc, 'i', r.choice([-1, 0, 1, 2, 3, 4, 5]))
    ps = put(put(bytearray(r.bytes(0x10)), 4, 'I', UENTRIES), 8, 'I', r.choice([0, 1, 2, 5, 0xffffffff]))
    holder = regions[RAM]   # (the holder Release sleeps or takes out of the grid: its model at +4, its part states at +0x28)
    put(holder, 4, 'I', UMODEL)
    put(holder, 0x28, 'I', UPS)
    slots = r.choice([1, 3, 6])
    table = struct.pack('<6I', *[r.choice([0, USCRIPTS + 0x40, USCRIPTS + 0x80]) for _ in range(6)])
    regions.update({UMODEL: model, UPARTS: parts, UENTRIES: entries, UPS: ps, USCRIPTS: table + r.bytes(0xa8),
                    0x7b49f0: struct.pack('<11I', USCRIPTS, slots, 0, 0, 0, 0, 0, 0, 0, r.choice([1, 1, 0]), 0)})
    regions.update(global_heap())
    regions[HEAP] = heap()
    index = r.randrange(4)
    regions[0x7a4610] = b''.join(struct.pack('<I', TARGET if k == index and r.random() < 0.9 else 0) for k in range(4))
    return Call(0x43bd20, [index], regions, conv='c', ret='v')


IHOLD, INST, IANIM, ICOUNT, IMODEL, IRECS, IPARTS, IPOBJ = RAM + 0x9000, RAM + 0xa000, RAM + 0xa200, RAM + 0xa280, RAM + 0xa300, RAM + 0xa380, RAM + 0xa400, RAM + 0xa500


def holder_at(r, base, owner):   # a holder as destroy()'s at base (its owner at +8 or none); returns it and its adjusted part
    obj = bytearray(r.bytes(0x400))
    put(obj, 0, 'I', r.getrandbits(32))
    put(obj, 8, 'I', r.choice([0, owner]))
    put(obj, 0x1c, 'I', r.choice([0, base + 0x100, base + 0x100]))
    put(obj, 0x24, 'I', r.choice([0, 0x1234]))
    offset = r.choice([0, 0x10, 0x20])
    vt = vtable({0: 1})
    put(vt, 4, 'I', offset)
    obj[0x300:0x300 + len(vt)] = vt
    put(obj, 0x100, 'I', base + 0x300)
    put(obj, 0x100 + offset, 'I', base + 0x300)
    return obj, base + 0x100 + offset


def renderer_env(r, parts):   # [0x87a6d0]'s renderer with both a grid (rem_rect()'s) and the slot lists of tags 0x10, 0x20, 0x70 (sleep()'s), their nodes the given parts or others
    width, height = r.choice([(4, 3), (3, 3), (2, 5), (6, 2)])
    renderer = bytearray(r.bytes(0x100))
    put(renderer, 0x38, 'ii', width, height)
    put(renderer, 0x10, 'I', RAM + 0x1200)
    put(renderer, 4, 'I', 0)
    put(renderer, 0xc, 'I', 7)
    grid, nodes, snodes = bytearray(4 * width * height + 16), bytearray(0x60), bytearray(0x18)
    for c in range(width * height):
        if r.random() < 0.6:
            put(nodes, 8 * c, 'II', r.choice(parts) if r.random() < 0.7 else r.getrandbits(32), 0)
            put(grid, 4 * c, 'I', RAM + 0x1400 + 8 * c)
    for k, slot in enumerate((0x14, 0x18, 0x2c)):
        put(snodes, 8 * k, 'II', r.choice(parts) if r.random() < 0.6 else r.getrandbits(32), 0)
        put(renderer, slot, 'I', RAM + 0x1500 + 8 * k if r.random() < 0.8 else 0)
    return {0x87a6d0: put(bytearray(r.bytes(0x10)), 0, 'I', RAM + 0x1000), RAM + 0x1000: renderer, RAM + 0x1200: grid, RAM + 0x1400: nodes, RAM + 0x1500: snodes,
            0x8bd500: r.bytes(0x20), LOG: log_region(), HEAP: heap(), **global_heap()}


def animator_at(r):   # an animator (flags +0, its holder IHOLD at +0x24, blocks at +0x20 / +0x34, the counter ICOUNT at +0x3c)
    anim = put(put(bytearray(r.bytes(0x40)), 0, 'I', r.getrandbits(32)), 0x24, 'I', IHOLD)
    put(anim, 0x3c, 'I', ICOUNT)
    put(anim, 0x20, 'I', r.choice([0, 0x4321]))
    put(anim, 0x34, 'I', r.choice([0, 0x4322]))
    return anim


def animator_dispose(r):   # 0x425510: the animator's holder (sleeping when flag 0x4000) destroyed; at a zero count its model's record blocks freed
    return Call(0x425510, [IANIM], animator_env(r), conv='c', ret='a')


def animator_env(r):   # the regions of an animator at IANIM for its disposal (animator_dispose's)
    anim, part = animator_parts(r)
    regions = renderer_env(r, [part])
    regions.update(anim)
    return regions


def animator_parts(r):   # animator_env's regions but the renderer, and the animator's holder's part
    hold, part = holder_at(r, IHOLD, OWNER)
    model = put(put(bytearray(r.bytes(0x60)), 0x36, 'H', r.choice([0, 1, 3, 5])), 0x54, 'I', IRECS)
    put(hold, 4, 'I', IMODEL)
    return {IHOLD: hold, OWNER: r.bytes(0x40), IANIM: animator_at(r), ICOUNT: put(bytearray(r.bytes(0x20)), 0x1c, 'i', r.choice([1, 1, 2, 0, -1])),
            IMODEL: model, IRECS: r.bytes(0x50)}, part


def instance_dispose(r):   # 0x455ad0: an instance with an animator (or none) at +0xb4, parts at +0xac and its holder at +8 (RAM), both holders' parts in the renderer
    h1, p1 = holder_at(r, IHOLD, OWNER)
    h2, p2 = holder_at(r, RAM, OWNER + 0x100)
    regions = renderer_env(r, [p1, p2])
    pos = lambda: (r.randrange(-1, 7) & 0xffff) << 16 | (r.randrange(-1, 6) & 0xffff)
    inst = bytearray(r.bytes(0x100))
    put(inst, 4, 'I', r.getrandbits(32))
    put(inst, 8, 'I', RAM)
    put(inst, 0xa8, 'I', r.choice([0, 1, 2, 4]))
    put(inst, 0xac, 'I', IPARTS)
    put(inst, 0xb4, 'I', r.choice([0, IANIM, IANIM]))
    put(inst, 0xc0, 'II', pos(), pos())
    put(inst, 0xe0, 'I', r.choice([0, 0x5678]))
    parts = b''.join(struct.pack('<I', r.choice([0, IPOBJ + 0x50 * k, IPOBJ + 0x50 * k])) for k in range(4))
    regions.update({RAM: h2, IHOLD: h1, OWNER: r.bytes(0x40), OWNER + 0x100: r.bytes(0x40), INST: inst, IANIM: animator_at(r), ICOUNT: r.bytes(0x20),
                    IPARTS: parts, IPOBJ: r.bytes(0x140)})
    return Call(0x455ad0, [INST], regions, conv='c', ret='a')


FMODEL, FPARTS, FOBJ, FSTATES, FRECS = RAM + 0xb000, RAM + 0xb100, RAM + 0xb800, RAM + 0xb900, RAM + 0xb940


def holder_free(r):   # 0x45bac0: a holder (destroy()'s, its part in the renderer) with an object model at +0x20 (no model at its +8: ObjModel_Unload's short
    # path) behind a virtual base like the part's, the count +0x34 that Destroy may bring to 0, a model of 0-5 parts some with flag 0x1000 (+0x2c one of them or not), part states or none
    hold, part = free_holder(r, RAM)
    regions = renderer_env(r, [part])
    regions.update({RAM: hold, **free_shared(r)})
    return Call(0x45bac0, [RAM], regions, conv='c', ret='v')


def free_holder(r, base):   # holder_free's holder at base (sharing free_shared's model, object model and part states); returns it and its part
    hold, part = holder_at(r, base, OWNER)
    put(hold, 0x34, 'i', r.choice([0, 0, 1, 1, 2, -1]))
    put(hold, 0x1c, 'I', r.choice([0, struct.unpack_from('<I', hold, 0x1c)[0] or base + 0x100]))
    put(hold, 4, 'I', FMODEL)
    put(hold, 0x14, 'I', r.choice([0, r.getrandbits(32)]))
    put(hold, 0x20, 'I', r.choice([0, FOBJ, FOBJ]))
    put(hold, 0x28, 'I', r.choice([0, FSTATES]))
    put(hold, 0x2c, 'I', r.choice([0, FPARTS, FPARTS + 0xa0, FPARTS + 0x1e0, r.getrandbits(32)]))
    return hold, part


def free_shared(r):   # free_holder's shared regions: the model (its records at FRECS for Instance_FreeRecordBlock), its parts, the object model, part states
    n = r.choice([0, 1, 2, 3, 5])
    model = put(put(put(bytearray(r.bytes(0xc0)), 0x44, 'H', n), 0x70, 'I', FPARTS), 0xb0, 'I', r.choice([0, r.getrandbits(32)]))
    put(model, 0x36, 'H', r.choice([0, 1, 3]))
    put(model, 0x54, 'I', FRECS)
    parts = bytearray(r.bytes(0xa0 * 5))
    for k in range(5):
        put(parts, 0xa0 * k, 'I', r.getrandbits(32) & ~0x1000 | r.choice([0, 0x1000]))
    offset = r.choice([0, 0x10, 0x20])
    vt = vtable({0: 1})   # (the deleting destructor: thiscall, the flag)
    put(vt, 4, 'I', offset)
    obj = bytearray(r.bytes(0x80))
    put(obj, 8, 'I', 0)
    put(obj, 0, 'I', FOBJ + 0x80)
    put(obj, offset, 'I', FOBJ + 0x80)
    return {OWNER: r.bytes(0x40), FMODEL: model, FPARTS: parts, FOBJ: bytes(obj) + bytes(vt), FSTATES: r.bytes(0x20), FRECS: r.bytes(0x30)}


INSTS, IHOLDS = RAM + 0xc000, RAM + 0x10000   # (InstFree / FreeAll: instances 0x200 apart, their holders 0x400 apart)


def instance_at(r, base, holder, animator):   # an instance for Instance_Free at base: 0-4 objects (+0xa8) in an array at +0x100 (+0xdc), each
    # pointing into the instance (+0xb0 read there), the object +0xb0 (its +0xc) at +0x180 or none, the animator given or none, blocks or none
    inst = bytearray(r.bytes(0x200))
    put(inst, 8, 'I', holder)
    put(inst, 0xa8, 'I', r.choice([0, 1, 2, 4]))
    put(inst, 0xdc, 'I', base + 0x100)
    for k in range(4):
        put(inst, 0x100 + 4 * k, 'I', base + r.randrange(0, 0x140, 4))
    put(inst, 0xb0, 'I', r.choice([0, base + 0x180]))
    put(inst, 0xb4, 'I', animator)
    for off in (0xac, 0xd8, 0xcc):
        put(inst, off, 'I', r.choice([0, r.getrandbits(32)]))
    put(inst, 0xd0, 'I', 0)
    return inst


def instance_free(r):   # 0x463db0: an instance with its holder (free_holder's) and an animator (animator_parts') or none, kept or not
    hold, part = free_holder(r, RAM)
    anim, apart = animator_parts(r)
    regions = renderer_env(r, [part, apart])
    regions.update({**anim, RAM: hold, **free_shared(r)})
    regions[INSTS] = instance_at(r, INSTS, RAM, r.choice([0, IANIM, IANIM]))
    return Call(0x463db0, [INSTS, r.choice([0, 0, 1, 0x100, 0xffffffff])], regions, conv='c', ret='v')


def free_all(r):   # 0x463ed0: 0-3 types in the table, each with its holder and an instance at +0xd0 (its own holder) or none; one animator at most
    count = r.choice([0, 1, 2, 3, 3])
    anim, apart = animator_parts(r)
    owner = r.randrange(2 * count) if count and r.random() < 0.6 else -1
    regions, parts, table = {}, [apart], bytearray(r.bytes(0x10))
    for k in range(2 * count):   # types at INSTS + 0x200 * k for even k, their instances after them
        if k % 2 and r.random() < 0.4:
            continue
        hold, part = free_holder(r, IHOLDS + 0x400 * k)
        parts.append(part)
        regions[IHOLDS + 0x400 * k] = hold
        regions[INSTS + 0x200 * k] = instance_at(r, INSTS + 0x200 * k, IHOLDS + 0x400 * k, IANIM if k == owner else 0)
        if k % 2:
            put(regions[INSTS + 0x200 * (k - 1)], 0xd0, 'I', INSTS + 0x200 * k)
        else:
            put(table, 4 * (k // 2), 'I', INSTS + 0x200 * k)
    regions.update(renderer_env(r, parts))
    regions.update({**anim, **free_shared(r), 0x7ae6ac: table, 0x7a439c: struct.pack('<I', count)})
    return Call(0x463ed0, [], regions, conv='c', ret='v')


def terrain_free(r):   # 0x450a80: InstFree's instance behind [0x79fcb4] or none, a loaded grid block behind [0x79fc54] or none, GlobalMem_FreeThree's three
    c = instance_free(r)
    regions = dict(c.regions)
    regions[0x79fc54] = struct.pack('<I', r.choice([0, RAM + 0x14000]))
    regions[0x79fcb4] = struct.pack('<I', r.choice([0, INSTS, INSTS]))
    regions[0x74c8b8] = b''.join(struct.pack('<I', r.choice([0, RAM + 0x14100 + 0x10 * k])) for k in range(8))
    return Call(0x450a80, [], regions, conv='c', ret='v')


def rem_rect(r, op):   # 0x45be80 (RemRect) / 0x4254e0 (RemRect14): a holder in the renderer's grid (its part behind a virtual base offset) leaving it by a rectangle
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj}
    width, height = r.choice([(4, 3), (3, 3), (2, 5), (6, 2)])
    off = r.choice([0, 0x10, 0x20])
    put(obj, 0, 'I', r.choice([0, 1, 1, 0xffffffff, r.getrandbits(32)]))
    put(obj, 0x1c, 'I', r.choice([0, RAM + 0x100, RAM + 0x100]))
    put(obj, 0x100, 'I', RAM + 0x300)
    put(obj, 0x304, 'I', off)
    renderer = bytearray(r.bytes(0x100))
    put(renderer, 0x38, 'ii', width, height)
    put(renderer, 0x10, 'I', RAM + 0x1200)
    put(renderer, 4, 'I', 0)
    put(renderer, 0xc, 'I', 7)
    grid, nodes = bytearray(4 * width * height + 16), bytearray(0x100)
    part = RAM + 0x100 + off
    for c in range(width * height):
        if r.random() < 0.6:
            put(nodes, 8 * c, 'II', part if r.random() < 0.7 else r.getrandbits(32), 0)
            put(grid, 4 * c, 'I', RAM + 0x1400 + 8 * c)
    regions.update({0x87a6d0: put(bytearray(r.bytes(0x10)), 0, 'I', RAM + 0x1000), RAM + 0x1000: renderer,
                    RAM + 0x1200: grid, RAM + 0x1400: nodes, 0x8bd500: r.bytes(0x20)})
    pos = lambda: (r.randrange(-1, 7) & 0xffff) << 16 | (r.randrange(-1, 6) & 0xffff)
    if op == 'RemRect14':  # the object whose part at +0x24 is taken out of the grid
        regions[RAM + 0x1800] = put(put(bytearray(r.bytes(0x40)), 0x24, 'I', RAM), 0, 'I', r.choice([0, 0x4000, 0xffffffff, r.getrandbits(32)]))
        return Call(0x4254e0, [RAM + 0x1800, pos(), pos()], regions, ret='a')
    return Call(0x45be80, [RAM, pos(), pos()], regions, ret='a')


def clear0(r):   # 0x424ed0: bit 0 of a dword cleared
    return Call(0x424ed0, [RAM + 0x10], {RAM: r.bytes(0x400)}, ret='a')


CASES = {'Matrices': objmodel.matrices, 'WakeMatrix': wake_matrix, 'AnimWake': animator_wake, 'Sleep': sleep, 'Animator': animator, 'Destroy': destroy, 'Release': release, 'Grid': add_to_grid, 'Dispose': dispose, 'Owner': dispose_owner, 'AnimGrid': animator_grid, 'Children': children, 'Wake': wake, 'Acquire': lambda r: acquire(r, False), 'RideAcquire': lambda r: acquire(r, True), 'AddObj': add_obj, 'SetPart': set_part, 'Unlink': unlink, 'AnimDispose': animator_dispose, 'InstDispose': instance_dispose, 'Free': holder_free, 'InstFree': instance_free, 'FreeAll': free_all, 'TerrainFree': terrain_free,
         'RemRect': lambda r: rem_rect(r, 'RemRect'), 'RemRect14': lambda r: rem_rect(r, 'RemRect14'), 'Clear0': clear0}


def case(r):
    return CASES[r.deal(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
