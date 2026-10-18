# The renderer's text parts (src/game/render/textparts: 0x48f6a0-0x491a00, twelve classes' constructors, destructors and deleting
# destructors) against the originals: the RendererPart's regions as rendererpart.py makes them (the live objects list, the tracked blocks,
# the heap, for the destructors the sprite set's table and blocks), the part grown to 0x1a0 bytes with its text layout at +0xe4: for the
# destructors the layout's font 0 or an object with a recorder vtable and its three vectors' blocks heap blocks, random or 0; the deleting
# destructors' flags 0, 1 or random.
# usage: textparts.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor6ff320,Dtor6ff320,Del6ff320,... to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rendererpart  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, log_region, put, vtable  # noqa: E402

# (ctor, deleting dtor, dtor) by vtable
CLASSES = {0x6ff320: (0x48f6a0, 0x48f740, 0x48f760), 0x6ff350: (0x48fde0, 0x48fe80, 0x48fea0), 0x6ff380: (0x490040, 0x4900e0, 0x490100),
           0x6ff3b0: (0x490250, 0x4902f0, 0x490310), 0x6ff3e0: (0x4904c0, 0x490560, 0x490580), 0x6ff410: (0x4907b0, 0x490850, 0x490870),
           0x6ff440: (0x4909f0, 0x490a90, 0x490ab0), 0x6ff470: (0x490c30, 0x490cd0, 0x490cf0), 0x6ff4a0: (0x490e60, 0x490f00, 0x490f20),
           0x6ff4d0: (0x491170, 0x491210, 0x491230), 0x6ff500: (0x491710, 0x4917b0, 0x4917d0), 0x6ff530: (0x491920, 0x4919c0, 0x4919e0)}
CALLS = {}
for v, (ctor, dele, dtor) in CLASSES.items():
    CALLS['Ctor%x' % v], CALLS['Del%x' % v], CALLS['Dtor%x' % v] = ctor, dele, dtor
B = Bench([(0x48f150, 0x48f290), (0x48f2e0, 0x48f3d0), (0x5775d0, 0x577660), (0x579b40, 0x579c30), (0x588320, 0x588420), (0x588d70, 0x588e50),
           (0x58a4f0, 0x58a560), (0x58dc50, 0x58dd70), (0x590b90, 0x590c50), (0x590a70, 0x590a75), (0x588ca0, 0x588d6e), (0x6597b0, 0x6598ed),
           (0x659994, 0x6599bc), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b), (0x67afa0, 0x67b0d0),
           (0x48f6a0, 0x491aac),(0x5fa680, 0x5fa6b0), (0x6ad5d0, 0x6ad670), (0x6ade00, 0x6adf30),
                                   (0x6b0230, 0x6b0290)])
PART = rendererpart.PART
FONT, VTABLE = RAM + 0x400, RAM + 0x480


def case(r):
    op = r.deal(OPS)
    kind = op[:4] if op.startswith('Ctor') or op.startswith('Dtor') else 'Del'
    rendererpart.OPS = ['Ctor' if kind == 'Ctor' else 'Dtor']
    base = rendererpart.case(r)
    regions = dict(base.regions)
    part = bytearray(regions[PART]) + r.bytes(0xa0)
    if kind != 'Ctor':
        put(part, 0xe4 + 0xc, 'I', r.choice([0, FONT, FONT]))   # the layout's style's font
        for off in (0x60, 0x74, 0x88):
            put(part, 0xe4 + off, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
    regions[PART] = part
    regions[FONT] = struct.pack('<I', VTABLE) + r.bytes(0x1c)
    regions[VTABLE] = vtable({})
    regions[LOG] = log_region()
    if kind == 'Ctor':
        return Call(CALLS[op], [PART, r.getrandbits(32)], regions, conv='t', ret='a')
    if kind == 'Dtor':
        return Call(CALLS[op], [PART], regions, conv='t')
    return Call(CALLS[op], [PART, r.choice([0, 1, 1, 2, 3, 0xffffffff])], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
