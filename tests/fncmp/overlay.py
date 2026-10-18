# The overlay base (src/game/gfx/overlay: 0x476e80) against the original: the shown node picked
# by base + frame from a model's node list (short, ending early, or empty), with flags taking
# either from a descriptor or neither, and the old node apart, null, among the list or the
# overlay itself. The destructors of the overlays (0x476b70, 0x477290, 0x4775c0: the holder disposed of
# or destroyed as holder.py's Dispose has it, or none) and of the bar mesh and its sibling (0x477c50,
# 0x478390: the animator at +0x6c as holder.py's AnimDispose has it), each with its scalar deleting
# destructor (flags freeing it or not), the GUI dialog base's destructor after them.
# usage: overlay.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import holder  # noqa: E402
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

DTORS = [(0x476b50, 0x476bf0), (0x477270, 0x477310), (0x4775a0, 0x477640), (0x477c30, 0x477cc0), (0x478370, 0x478400),
         (0x659886, 0x6598dd), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b), (0x424ed0, 0x424ee1)]
B = Bench([(0x476e80, 0x476f04)] + DTORS + holder.RANGES, hooks=holder.HOOKS)
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


CASES = {'ShowFrame': show_frame,
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
