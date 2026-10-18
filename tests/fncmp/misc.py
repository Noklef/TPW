# The small functions of src/game/core/misc against the originals: any object bytes, any positions.
# usage: misc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear0,Pixel,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x424ed0, 0x424ee9), (0x40f300, 0x40f329), (0x59b550, 0x59b590), (0x458910, 0x458935), (0x458940, 0x458965),
           (0x5836b0, 0x5836c2), (0x56b620, 0x56b628), (0x633dc0, 0x633e00), (0x6486e0, 0x648720), (0x5d36c0, 0x5d36d0),
           (0x633120, 0x63313a), (0x6338b0, 0x6338c0), (0x5f6a20, 0x5f6a60), (0x45be80, 0x45becd), (0x4254e0, 0x425510),
           (0x5766e0, 0x57681a), (0x409260, 0x409296), (0x576540, 0x5765b8), (0x4583d0, 0x4583f6), (0x458310, 0x45834d),
           (0x402ef0, 0x402f10), (0x4032b0, 0x4032ca), (0x402f30, 0x402f50), (0x4030c0, 0x4030c4), (0x47f940, 0x47f95a),
           (0x657d30, 0x657d6c)])
CALLS = ['Ctor3ca0', 'MaxW', 'Ctor3c18', 'RemRect', 'Flags', 'RemRect14', 'SetVec', 'CtorVirtual', 'CtorPart', 'MeshClear', 'MeshSet', 'Advance', 'CtorOne', 'CtorValue', 'Clear0']


def case(r):
    op = r.choice(OPS)
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj}
    if op == 'Ctor3ca0':
        return Call(0x633dc0, [RAM], regions, conv='t', ret='a')
    if op == 'MaxW':
        n = r.choice([0, 1, 3, 6, -1])
        for i in range(6):
            put(obj, 0x20 * i + 0x18, 'i', r.choice([-1, 0, 1, 5]))
            put(obj, 0x20 * i + 0x1c, 'i', r.choice([-3, 0, 1, 5, 5, 9, 0x7fffffff]))
        return Call(0x6486e0, [RAM, n & 0xffffffff], regions, ret='a')
    if op == 'Ctor3c18':
        return Call(0x633120, [RAM], regions, conv='t', ret='a')
    if op in ('RemRect', 'RemRect14'):
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
    if op == 'Flags':
        put(obj, 0x18, 'I', r.choice([0, 0, 1, 5]))
        regions.update({0x7a1a10: r.bytes(8), 0x7477e0: r.bytes(8), 0x785970: r.bytes(0x60)})
        return Call(0x409260, [RAM], regions, conv='t', ret='a')
    if op == 'SetVec':
        regions.update({0x8bd500: r.bytes(0x20), 0x8bcc40: r.bytes(8)})
        return Call(0x4583d0, [RAM, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    if op == 'CtorVirtual':
        off = r.choice([0x60, 0x60, 0x64, 0x70, 0x80, 0x68])
        put(obj, 0, 'I', RAM + 0x300)  # the vtable it starts with (when not most derived)
        put(obj, 0x304, 'I', off)
        return Call(0x458310, [RAM, r.choice([0, 1, 1])], regions, conv='t', ret='a')
    if op == 'CtorPart':
        return Call(0x40f300, [RAM, RAM + 0x100 + r.choice([0, 4, 0x20])], regions, conv='t', ret='a')
    if op in ('MeshClear', 'MeshSet'):
        put(obj, 0x100, 'I', RAM + 0x200)  # the reference: -> the object, whose vtable gives the part's offset
        put(obj, 0x200, 'I', RAM + 0x300)
        put(obj, 0x308, 'I', r.choice([0x20, 0x40, 0x10, 0x60]))
        regions.update({0x8bd500: r.bytes(0x20), 0x8bcc40: r.bytes(8)})
        return Call(0x458910 if op == 'MeshClear' else 0x458940, [RAM + 0x100], regions)
    if op == 'Advance':
        put(obj, 0x30, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        return Call(0x402ef0, [RAM], regions, conv='t', ret='a')
    if op == 'CtorOne':
        return Call(0x4032b0, [RAM], regions, conv='t', ret='a')
    if op == 'CtorValue':
        return Call(0x47f940, [RAM, r.getrandbits(32)], regions, conv='t', ret='a')
    if op == 'Clear0':
        return Call(0x424ed0, [RAM + 0x10], regions, ret='a')
    raise ValueError(op)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
