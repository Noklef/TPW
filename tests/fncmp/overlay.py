# The overlay base (src/game/gfx/overlay: 0x476e80) against the original: the shown node picked
# by base + frame from a model's node list (short, ending early, or empty), with flags taking
# either from a descriptor or neither, and the old node apart, null, among the list or the
# overlay itself. The destructors of the overlays (0x476b70, 0x477290, 0x4775c0: the holder disposed of
# or destroyed as holder.py's Dispose has it, or none) and of the bar mesh and its sibling (0x477c50,
# 0x478390: the animator at +0x6c as holder.py's AnimDispose has it), each with its scalar deleting
# destructor (flags freeing it or not), the GUI dialog base's destructor after them. Apply (0x476f10): see apply(). Show (0x476bf0): see show().
# The resized overlays' Applies (0x477310, 0x477640): apply()'s case with each node's quad (+0x60) of vertices.
# usage: overlay.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import holder  # noqa: E402
import objmodel  # noqa: E402
from fncmp import HEAP, Bench, Call, RAM, _cdecl_log, heap, put  # noqa: E402

DTORS = [(0x476b50, 0x476bf0), (0x477270, 0x477310), (0x4775a0, 0x477640), (0x477c30, 0x477cc0), (0x478370, 0x478400),
         (0x659886, 0x6598dd), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b), (0x424ed0, 0x424ee1)]
B = Bench([(0x476e80, 0x476f04), (0x476f10, 0x477190), (0x476bf0, 0x476c4c), (0x476c50, 0x476e80), (0x46f150, 0x46f180), (0x4700f0, 0x4702d0), (0x46fc00, 0x46fcb0),
           (0x57a260, 0x57a41c), (0x477310, 0x4774be), (0x477640, 0x4777a1)] + DTORS + holder.RANGES, hooks={**holder.HOOKS, 0x67a7d0: _cdecl_log}, sines=4096)
OV, DESC, HOLDER, MODEL, NODES = (RAM + k * 0x1000 for k in range(5))


def show_frame(r):
    count = r.choice([0, 1, r.randrange(2, 12)])
    nodes = bytearray(r.bytes(12 * 0x10))
    for i in range(count):
        put(nodes, i * 0x10, 'I', r.choice([r.getrandbits(32), 0, 0x8010, 0xffffffff]))
        put(nodes, i * 0x10 + 0xc, 'I', NODES + (i + 1) * 0x10 if i + 1 < count else 0)
    model = put(bytearray(r.bytes(0x80)), 0x78, 'I', NODES if count else 0)
    holder = put(bytearray(r.bytes(0x10)), 4, 'I', MODEL)
    ov = bytearray(r.bytes(0x80))
    put(ov, 4, 'I', HOLDER)
    small = lambda: r.choice([r.randrange(4), r.randrange(count + 2), r.getrandbits(16)])  # noqa: E731
    put(ov, 0xa, '2H', small(), small())
    old = r.choice([0, NODES + 0x10 * r.randrange(max(count, 1)), OV, RAM + 0x5000])
    put(ov, 0x7c, 'I', old)
    desc = bytearray(r.bytes(0x10))
    put(desc, 4, 'H', small())
    put(desc, 0xa, 'H', small())
    flags = r.choice([0, 1, 0x80000, 0x80001, r.getrandbits(32), r.getrandbits(32) & ~0x80001])
    regs = {OV: ov, DESC: desc, HOLDER: holder, MODEL: model, NODES: nodes, RAM + 0x5000: r.bytes(4)}
    return Call(0x476e80, [OV, flags, DESC], regs, conv='t', ret='a')


DOBJ = RAM + 0x10000
ANODES, PARENTS, RECT, AOBJ, SOV, SNODE = RAM + 0x20000, RAM + 0x21000, RAM + 0x22000, RAM + 0x23000, RAM + 0x30000, RAM + 0x30100
QUADS = RAM + 0x24000   # the resized overlays' quads: 0xc0 bytes a node (4 rows of x[4], y[4], z[4])


def matrix(r):   # an affine one (row vectors, the translation in row 3) of moderate entries, now and then any floats
    m = [r.uniform(-2, 2) for _ in range(16)]
    m[3], m[7], m[11], m[15] = 0.0, 0.0, 0.0, 1.0
    m[12], m[13], m[14] = (r.uniform(-100, 100) for _ in range(3))
    out = bytearray(struct.pack('<16f', *m))
    if r.random() < 0.05:
        put(out, 4 * r.randrange(16), 'I', r.f())
    return out


def apply(r, address=0x476f10):
    """0x476f10 (thiscall (flags, desc), pops 8): show_frame()'s overlay and node list, the nodes 0x100 apart (+0x10 a matrix, +4 a chain
    of 0-2 parents with theirs, +0x78 / +0x84 the bounds, low and high or the other way round, a NaN now and then); the overlay's +0x74
    any float, +0x80 a rate (0 mostly, small, or any); the holder's +0x1c an object for objmodel.py's matrices_env(); the descriptor's
    rectangle (+0x14) and words; flags of bits 0x1, 0x2, 0x4, 0x10, 0x40, 0x80000 or any; the holder or the descriptor now and then 0.
    For 0x477310 / 0x477640 (address) each node's +0x60 is a quad of vertices around 0 (now and then exactly 0, -0 or a NaN)."""
    count = r.choice([1, 2, r.randrange(1, 5), 0])
    nodes, parents = bytearray(r.bytes(4 * 0x100)), bytearray(r.bytes(4 * 0x50))
    for i in range(count):
        b = i * 0x100
        put(nodes, b, 'I', r.choice([r.getrandbits(32), 0, 0x8010]))
        put(nodes, b + 0xc, 'I', ANODES + (i + 1) * 0x100 if i + 1 < count else 0)
        nodes[b + 0x10:b + 0x50] = matrix(r)
        put(nodes, b + 4, 'I', r.choice([0, 0, PARENTS, PARENTS + 0x50 * r.randrange(1, 4)]))
        bounds = [r.uniform(-60, 60) for _ in range(6)]
        if r.random() < 0.6:
            bounds = [min(bounds[k], bounds[k + 3]) for k in range(3)] + [max(bounds[k], bounds[k + 3]) for k in range(3)]
        put(nodes, b + 0x78, '6f', *bounds)
        if address != 0x476f10:
            put(nodes, b + 0x60, 'I', QUADS + 0xc0 * i)
        if r.random() < 0.05:
            put(nodes, b + 0x78 + 4 * r.randrange(6), 'I', r.choice([0x7fc00000, 0x7f800001, 0xffc00000]))
    for k in range(4):   # (each parent's own parent the next, or none)
        parents[0x50 * k + 0x10:0x50 * k + 0x50] = matrix(r)
        put(parents, 0x50 * k + 4, 'I', PARENTS + 0x50 * (k + 1) if k < 3 and r.random() < 0.4 else 0)
    model = put(bytearray(r.bytes(0x80)), 0x78, 'I', ANODES if count else 0)
    hold = put(bytearray(r.bytes(0x20)), 4, 'I', MODEL)
    put(hold, 0x1c, 'I', AOBJ)
    ov = bytearray(r.bytes(0x90))
    put(ov, 4, 'I', HOLDER if r.random() < 0.95 else 0)
    small = lambda: r.choice([r.randrange(3), r.randrange(count + 2), r.getrandbits(16)])  # noqa: E731
    put(ov, 0xa, '2H', small(), small())
    put(ov, 0x7c, 'I', r.choice([0, ANODES, OV]))
    put(ov, 0x74, 'f', r.uniform(-2, 2))
    put(ov, 0x80, 'I', r.choice([0, 0, 1, 40, r.randrange(-100, 1000), r.getrandbits(32)]) & 0xffffffff)
    desc = bytearray(r.bytes(0x18))
    put(desc, 4, 'H', small())
    put(desc, 0xa, 'H', small())
    put(desc, 0x14, 'I', RECT)
    rect = bytearray(r.bytes(0x10))
    for k in range(4):
        put(rect, 4 + 2 * k, 'h', r.choice([r.randrange(0, 2048), r.randrange(-100, 100), r.getrandbits(15)]))
    bits = [0x1, 0x2, 0x4, 0x10, 0x40, 0x80000]
    flags = r.choice([sum(b for b in bits if r.random() < 0.5), 0x11, 0x13, 0x80055, 0x57, r.getrandbits(32), 0])
    regions = objmodel.matrices_env(r, {OV: ov, DESC: desc, HOLDER: hold, MODEL: model, ANODES: nodes, PARENTS: parents, RECT: rect,
                                        AOBJ: r.bytes(0x100), 0x8bd508: r.bytes(4)})
    if address != 0x476f10:
        quads = bytearray(struct.pack('<%df' % (0x30 * 4), *[r.uniform(-80, 80) for _ in range(0x30 * 4)]))
        for _ in range(r.choice([0, 0, 2, 6])):
            put(quads, 4 * r.randrange(0x30 * 4), 'I', r.choice([0, 0x80000000, 0x7fc00000, 0xffc00000, 0x7f800001]))
        regions[QUADS] = quads
    return Call(address, [OV, flags, DESC if r.random() < 0.95 else 0], regions, conv='t')


def overlay_dtor(r, address):   # holder.py's Dispose: the holder at RAM (its part in the renderer), disposed of (word +0x10 clear) or destroyed
    regions = holder.dispose(r).regions
    ov = bytearray(r.bytes(0x80))
    put(ov, 4, 'I', r.choice([RAM, RAM, RAM, 0]))
    put(ov, 0x10, 'H', r.choice([0, 0, 1, 0x100, r.getrandbits(16)]))   # (0x100: the word, not its low byte)
    regions[DOBJ] = ov
    return dtor_call(r, address, DOBJ, regions)


def mesh_dtor(r, address):   # holder.py's AnimDispose: the animator embedded at +0x6c (IANIM), its holder's part in the renderer
    regions = holder.animator_env(r)
    mesh = bytearray(r.bytes(0xc0))
    mesh[0x6c:0xac] = regions.pop(holder.IANIM)
    put(mesh, 4, 'I', r.choice([1, RAM, r.getrandbits(32), 0]))
    regions[holder.IANIM - 0x6c] = mesh
    return dtor_call(r, address, holder.IANIM - 0x6c, regions)


def dtor_call(r, address, obj, regions):   # a scalar deleting destructor (at a 0x10-aligned address) takes flags and returns the object
    if address & 0xf:
        return Call(address, [obj], regions, conv='t')
    regions.setdefault(HEAP, heap())
    return Call(address, [obj, r.choice([0, 1, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')


def show(r):
    """0x476bf0 (thiscall (shown, a word), pops 4): an overlay (its matrix at +0x3c any) whose holder is holder.py's WakeMatrix one (shown
    not 0) or its Sleep one (shown 0), or none; the word +8 the shown one now and then; the shown node (+0x7c) one or none."""
    wake = r.random() < 0.5
    c = holder.wake_matrix(r) if wake else holder.sleep(r)
    regions = dict(c.regions)
    shown = r.choice([1, 1, 0x100, r.getrandbits(16) | 1]) if wake else r.choice([0, 0, 0, 0x10000])
    ov = bytearray(r.bytes(0x90))
    put(ov, 4, 'I', c.args[0] if r.random() < 0.95 else 0)
    put(ov, 8, 'H', shown & 0xffff if r.random() < 0.1 else r.getrandbits(16))
    put(ov, 0x7c, 'I', r.choice([SNODE, SNODE, 0]))
    regions[SOV], regions[SNODE] = ov, r.bytes(0x10)
    return Call(0x476bf0, [SOV, shown], regions, conv='t', ret='v')


CASES = {'ShowFrame': show_frame, 'Apply': apply, 'Show': show,
         'Resized': lambda r: apply(r, 0x477310), 'ResizedHeight': lambda r: apply(r, 0x477640),
         'Dtor': lambda r: overlay_dtor(r, 0x476b70), 'ScalarDtor': lambda r: overlay_dtor(r, 0x476b50),
         'Dtor38': lambda r: overlay_dtor(r, 0x477290), 'ScalarDtor38': lambda r: overlay_dtor(r, 0x477270),
         'Dtor60': lambda r: overlay_dtor(r, 0x4775c0), 'ScalarDtor60': lambda r: overlay_dtor(r, 0x4775a0),
         'BarDtor': lambda r: mesh_dtor(r, 0x477c50), 'BarScalarDtor': lambda r: mesh_dtor(r, 0x477c30),
         'MeshDtor': lambda r: mesh_dtor(r, 0x478390), 'MeshScalarDtor': lambda r: mesh_dtor(r, 0x478370)}


def case(r):
    return CASES[r.deal(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
