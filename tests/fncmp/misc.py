# The small functions of src/game/core/misc against the originals: any object bytes; the streambuf
# pointers pointing into the object or a buffer (now and then at each other), any positions.
# usage: misc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear0,Setg,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x424ed0, 0x424ee9), (0x4f6020, 0x4f6080), (0x40f300, 0x40f329), (0x59b550, 0x59b590), (0x458910, 0x458935), (0x458940, 0x458965),
           (0x5836b0, 0x5836c2), (0x52fbf0, 0x52fc21), (0x5f4490, 0x5f44d0), (0x56b620, 0x56b628), (0x633dc0, 0x633e00), (0x6486e0, 0x648720), (0x5c9040, 0x5c9062), (0x5d36c0, 0x5d36d0), (0x5d03b0, 0x5d03cf), (0x5dd3d0, 0x5dd3fd), (0x5ddac0, 0x5ddae0), (0x5dd810, 0x5dd822), (0x5e2750, 0x5e2763), (0x633120, 0x63313a), (0x6338b0, 0x6338c0), (0x5c7490, 0x5c749c), (0x5f6a20, 0x5f6a60), (0x5c5230, 0x5c524b), (0x5c7d20, 0x5c7d2e), (0x5c7d00, 0x5c7d16), (0x5c7bb0, 0x5c7bc7), (0x5b0d40, 0x5b0d54), (0x5d5ee0, 0x5d5eec), (0x5d1440, 0x5d1458), (0x54dcc0, 0x54dcf0), (0x556a80, 0x556aac), (0x569f30, 0x569f58), (0x45be80, 0x45becd), (0x4254e0, 0x425510), (0x5766e0, 0x57681a), (0x557110, 0x55715b), (0x409260, 0x409296), (0x4f36c0, 0x4f36e1), (0x52fc30, 0x52fc79), (0x5997b0, 0x5997f8), (0x56e780, 0x56e7de), (0x54b290, 0x54b2f0), (0x56b340, 0x56b37d), (0x576540, 0x5765b8), (0x583360, 0x5833ba), (0x5646c0, 0x564708), (0x564ba0, 0x564bf0), (0x56b570, 0x56b5b2), (0x4583d0, 0x4583f6), (0x5651c0, 0x56520b), (0x458310, 0x45834d), (0x402ef0, 0x402f10), (0x4032b0, 0x4032ca), (0x402f30, 0x402f50), (0x4030c0, 0x4030c4),
           (0x47f940, 0x47f95a), (0x657d30, 0x657d6c), (0x4f6820, 0x4f6850),
           (0x4f6850, 0x4f6880), (0x56a510, 0x56a540), (0x56b630, 0x56b650), (0x56b830, 0x56b850)])
CALLS = ['Shown', 'Ctor3ca0', 'MaxW', 'Ctor295c', 'Swap', 'Choose', 'Pending', 'Pos40', 'SetOrKeep', 'Ctor3c18', 'WCtor', 'DirOk', 'Rec14', 'RecUsed', 'RecCount', 'Tab73', 'FlagsAnd', 'Span', 'Flag54', 'GetVec', 'Setting', 'RemRect', 'TakeDone', 'Flags', 'Size36', 'Commit', 'Lookup', 'Extents', 'FreeAll', 'RemRect14', 'ReleaseR', 'TopBits', 'Virt5646', 'Virt564b', 'FindSet', 'SetVec', 'Ctor1260', 'CtorVirtual', 'PairAdd', 'CtorPart', 'MeshClear', 'MeshSet', 'Advance', 'CtorOne', 'CtorValue', 'InitBuf', 'Clear0', 'Setg', 'Setp', 'Pixel', 'Fill', 'Set3034']
BUF = RAM + 0x1000


def case(r):
    op = r.choice(OPS)
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj}
    if op == 'Ctor295c':
        return Call(0x5c9040, [RAM], regions, conv='t', ret='a')
    if op == 'Swap':
        return Call(0x5d03b0, [RAM + 0x40, RAM + r.choice([0x40, 0x80, 0x44])], regions, conv='t')
    if op == 'Choose':
        put(obj, 0x58, 'I', r.choice([0, 0, 1, 7]))
        return Call(0x5dd3d0, [RAM], regions, conv='t')
    if op == 'Pending':
        put(obj, 0x60, 'I', r.choice([0, 0, 1]))
        put(obj, 0x40, 'i', r.choice([0, 1, 5, -1, -5, 0x7fffffff, -0x80000000]))
        put(obj, 0x64, 'i', r.choice([0, 1, 5, -1, 6, 0x7fffffff, -0x80000000]))
        return Call(0x5ddac0, [RAM], regions, conv='s', ret='a')
    if op == 'Shown':
        put(obj, 0x28, 'I', RAM + 0x100)
        put(obj, 0x2c, 'I', r.choice([0, 1, 5]))
        return Call(0x5f4490, [RAM, r.choice([0, 1, 5, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 'Ctor3ca0':
        return Call(0x633dc0, [RAM], regions, conv='t', ret='a')
    if op == 'MaxW':
        n = r.choice([0, 1, 3, 6, -1])
        for i in range(6):
            put(obj, 0x20 * i + 0x18, 'i', r.choice([-1, 0, 1, 5]))
            put(obj, 0x20 * i + 0x1c, 'i', r.choice([-3, 0, 1, 5, 5, 9, 0x7fffffff]))
        return Call(0x6486e0, [RAM, n & 0xffffffff], regions, ret='a')
    if op == 'Pos40':
        put(obj, 0x40, 'I', r.choice([0, 1, 0xffffffff, 0x80000000, 0x7fffffff, r.getrandbits(32)]))
        return Call(0x5dd810, [RAM], regions, conv='s', ret='a')
    if op == 'SetOrKeep':
        return Call(0x5e2750, [RAM, r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='t')
    if op == 'Ctor3c18':
        return Call(0x633120, [RAM], regions, conv='t', ret='a')
    if op == 'WCtor':
        return Call(0x5c7490, [RAM], regions, conv='t', ret='a')
    if op == 'DirOk':
        put(obj, 0x20, 'I', RAM + 0x100)
        obj[4] = r.choice([0, 0x10, 0x10, 0x30, 0x20])
        obj[0x100] = r.choice([0x2e, 0x2e, 0x41, 0])
        return Call(0x5c5230, [RAM], regions, ret='a')
    if op in ('Rec14', 'RecUsed'):
        for i in range(4):
            put(obj, 4 + 24 * i, 'I', r.choice([0, 0, 1, 0x80000000, r.getrandbits(32)]))
        i = r.randrange(4)
        return Call(0x5c7d20 if op == 'Rec14' else 0x5c7d00, [RAM, i], regions, conv='t', ret='a')
    if op == 'RecCount':
        for i in range(4):
            put(obj, 4 + 24 * i, 'I', r.choice([0, 0, 1, 0x80000000, r.getrandbits(32)]))
        return Call(0x5c7bb0, [RAM], regions, conv='t', ret='a')
    if op == 'Tab73':
        return Call(0x5b0d40, [r.choice([0, 1, 5, 100, 0xffffffff, r.getrandbits(32)])], {}, conv='s', ret='a')
    if op == 'FlagsAnd':
        return Call(0x5d5ee0, [RAM, r.choice([0, 1, 0xff, 0xffffffff, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 'Span':
        v = lambda: r.choice([0, 1, 5, 100, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])
        return Call(0x5d1440, [v(), v(), v(), v()], {}, conv='s', ret='a')
    if op == 'Flag54':
        regions = {0x78d960: put(bytearray(r.bytes(0x10)), 8, 'I', r.choice([0, 0, 1])), 0x785870: put(bytearray(r.bytes(0x10)), 4, 'I', r.choice([0, 1, 2, 5, 0x100])),
                   0x79d860: put(bytearray(r.bytes(0x10)), 0xc, 'I', r.choice([0, 0x800, 0x7ff, 0xffffffff, r.getrandbits(32)]))}
        return Call(0x54dcc0, [], regions, ret='b')
    if op == 'GetVec':
        put(obj, 8, 'I', RAM + 0x100)
        put(obj, 0x104, 'I', RAM + 0x200)
        put(obj, 0x278, 'I', RAM + 0x300)
        return Call(0x556a80, [RAM, RAM + 0x3f0, RAM + 0x3f4, RAM + 0x3f8], regions, ret='a')
    if op == 'Setting':
        regions = {0xfbe030: r.bytes(0x20), 0x6f1a10: r.bytes(8)}
        return Call(0x569f30, [r.getrandbits(32), r.choice([0, 0, 1, 7, r.getrandbits(32)])], regions, conv='s', ret='a')
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
    if op == 'TakeDone':
        count = r.choice([0, 1, 3, 5, -1])
        put(obj, 0x7c, 'i', count)
        put(obj, 0x2c, 'I', RAM + 0x200)
        for i in range(6):
            put(obj, 0x200 + 0x20 * i + 0x18, 'H', r.choice([0, 4, 4, 1]))
            put(obj, 0x200 + 0x20 * i + 0x10, 'I', r.getrandbits(32))
        return Call(0x557110, [RAM], regions, ret='a')
    if op == 'Flags':
        put(obj, 0x18, 'I', r.choice([0, 0, 1, 5]))
        regions.update({0x7a1a10: r.bytes(8), 0x7477e0: r.bytes(8), 0x785970: r.bytes(0x60)})
        return Call(0x409260, [RAM], regions, conv='t', ret='a')
    if op == 'Size36':
        put(obj, 4, 'I', r.choice([0, 0x1000, 0x7fffff00, 0xfffffff0]))
        put(obj, 8, 'I', r.choice([0, 0x1000 + 36 * r.randrange(100), 0x1000 + r.randrange(5000), r.getrandbits(32)]))
        return Call(0x4f36c0, [RAM], regions, conv='t', ret='a')
    if op == 'Commit':
        count = r.choice([0, 1, 5, 0x400, 0x401, 0xffffffff, 0xfffffffb])
        regions = {0x81b740: r.bytes(0x2010), 0x818d90: r.bytes(0x2010), 0x820a80: put(bytearray(r.bytes(0x20)), 0xc, 'I', count)}
        return Call(0x52fc30, [], regions, ret='a')
    if op == 'Lookup':
        table = bytearray(r.bytes(0x20 * 8))
        end = r.randrange(0, 8)
        ids = [r.choice([1, 2, 3, 100, 0x270e]) for _ in range(8)]
        for i in range(8):
            put(table, 0x20 * i, 'I', 0x270f if i == end else ids[i])
        wanted = r.choice(ids + [0x270f, 0, 4])
        return Call(0x5997b0, [wanted], {0x768fb8: table}, ret='a')
    if op == 'Extents':
        arg = lambda: r.choice([0, 1, 5, 9, -1, -5, 100, 0x7f, 0x80, -0x80, 0x1ff, -0x1ff, r.randrange(-300, 300)]) & 0xffffffff
        first = r.randrange(0, 10)
        last = first + r.randrange(0, 6) - (2 if r.random() < 0.1 else 0)
        put(obj, 0x20, 'i', first + r.choice([-1, 0, 1, r.randrange(-20, 20)]))  # the stored extents about the new ones
        put(obj, 0x24, 'i', last + r.choice([-1, 0, 1, r.randrange(-20, 20)]))
        return Call(0x56e780, [RAM, arg(), first, arg(), last], regions, conv='t')
    if op == 'FreeAll':
        # chunks at RAM + 0x100 + 0x40 * i (list head at the owner's +0xbc), their nodes at RAM + 0x400 + 0x20 * j
        owner = RAM + 0x20
        obj = bytearray(obj) + bytes(0x200)
        nodes = [RAM + 0x400 + 0x20 * j for j in range(6)]
        cs = [RAM + 0x100 + 0x40 * i for i in range(3)]
        put(obj, 0x20 + 0xbc, 'I', cs[0] if r.random() < 0.9 else 0)
        for i, c in enumerate(cs):
            put(obj, 0x100 + 0x40 * i + 0x24, 'I', cs[i + 1] if i + 1 < len(cs) and r.random() < 0.8 else 0)
            put(obj, 0x100 + 0x40 * i + 0x1c, 'I', r.choice([0, nodes[2 * i]]))
            put(obj, 0x100 + 0x40 * i, 'I', r.getrandbits(32))
        for j in range(6):
            put(obj, 0x400 + 0x20 * j + 0x14, 'I', nodes[j + 1] if j % 2 == 0 and r.random() < 0.7 else 0)
            put(obj, 0x400 + 0x20 * j, 'I', r.getrandbits(32))
        regions[RAM] = obj
        regions[0x877b70] = r.bytes(0x20)
        return Call(0x54b290, [owner], regions)
    if op == 'ReleaseR':
        put(obj, 0, 'I', r.choice([0, 1, 1, 0xffffffff, r.getrandbits(32)]))
        put(obj, 4, 'I', r.choice([0, RAM + 0x100]))
        put(obj, 0x100, 'I', RAM + 0x300)  # the part's vtable: +4 = the virtual base offset
        offset = r.choice([0, 0x10, 0x20])
        put(obj, 0x304, 'I', offset)
        renderer = bytearray(r.bytes(0x100))
        put(renderer, 0x28, 'I', r.choice([0, RAM + 0x1100, RAM + 0x1100]))  # the tag 0x60 list: one node, the part's or not
        put(renderer, 4, 'I', 0)
        put(renderer, 0xc, 'I', 5)
        regions[RAM + 0x1100] = put(put(bytearray(8), 0, 'I', RAM + 0x100 + offset if r.random() < 0.6 else r.getrandbits(32)), 4, 'I', 0)
        regions.update({0x87a6d0: put(bytearray(r.bytes(0x10)), 0, 'I', r.choice([RAM + 0x1000, RAM + 0x1000, 0])),
                        RAM + 0x1000: renderer, 0x8bd500: r.bytes(0x20)})
        return Call(0x56b340, [RAM], regions, conv='t', ret='a')
    if op == 'TopBits':
        obj = bytearray(obj) + bytes(0x600)
        masks = [0xff0000, 0xff00, 0xff, 0xf800, 0x7e0, 0x1f, 0, 0x80000000, 1, r.getrandbits(32)]
        for off in (0x8fc, 0x904, 0x90c):
            put(obj, off, 'I', r.choice(masks))
        regions[RAM] = obj
        return Call(0x583360, [RAM], regions, conv='t', ret='a')
    if op in ('Virt5646', 'Virt564b'):
        off = r.choice([0x18, 0x18, 0x1c, 0x20] if op == 'Virt5646' else [0x2c, 0x2c, 0x30, 0x34])
        put(obj, 0, 'I', RAM + 0x300)
        put(obj, 0x304, 'I', off)
        return Call(0x5646c0 if op == 'Virt5646' else 0x564ba0, [RAM, r.choice([0, 1, 1])], regions, conv='t', ret='a')
    if op == 'FindSet':
        n = r.choice([0, 1, 2, 3, 6])
        keys = [RAM + 0x100 + 0x40 * i for i in range(6)]
        put(obj, 0x1c, 'I', r.choice([n, n, 0xffffffff]))
        put(obj, 8, 'I', RAM + 0x300)
        for i in range(6):
            put(obj, 0x300 + 4 * i, 'I', r.choice(keys[:4]))
        key = r.choice(keys[:5] + [0x1234])
        if key == 0x1234:
            key = RAM + 0x3c0
        return Call(0x56b570, [RAM, key, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    if op == 'SetVec':
        regions.update({0x8bd500: r.bytes(0x20), 0x8bcc40: r.bytes(8)})
        return Call(0x4583d0, [RAM, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    if op == 'Ctor1260':
        regions.update({0x879a20: put(bytearray(r.bytes(0x20)), 4, 'I', RAM + 0x1000), 0x879ab0: r.bytes(0x10),
                        0x8bd520: r.bytes(4) + struct.pack('<I', r.choice([1, 1, 0, 4]))})
        regions[RAM + 0x1000] = put(bytearray(r.bytes(0x500)), 0x428, 'I', r.choice([0x10de, 0x10de, 0x1002, 0]))
        return Call(0x5651c0, [RAM], regions, conv='t', ret='a')
    if op == 'CtorVirtual':
        off = r.choice([0x60, 0x60, 0x64, 0x70, 0x80, 0x68])
        put(obj, 0, 'I', RAM + 0x300)  # the vtable it starts with (when not most derived)
        put(obj, 0x304, 'I', off)
        return Call(0x458310, [RAM, r.choice([0, 1, 1])], regions, conv='t', ret='a')
    if op == 'PairAdd':
        count = r.choice([0, 1, 5, 0x3ff, 0x400, 0x401, 0x402, 0xffffffff, r.randrange(0x400)])
        regions = {0x81b740: r.bytes(0x2010), 0x820a80: put(bytearray(r.bytes(0x20)), 0xc, 'I', count)}
        return Call(0x52fbf0, [r.getrandbits(32), r.getrandbits(32)], regions, ret='a')
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
    if op == 'InitBuf':
        return Call(0x4f6020, [RAM + r.choice([0, 0x20])], regions, conv='t', ret='a')
    if op == 'Clear0':
        return Call(0x424ed0, [RAM + 0x10], regions, ret='a')
    if op in ('Setg', 'Setp'):
        slots = (0xc, 0x1c, 0x2c) if op == 'Setg' else (0x10, 0x20, 0x30)
        for i, off in enumerate(slots):  # the pointers go to their own words, or all to one, or the object itself
            put(obj, off, 'I', r.choice([RAM + 0x200 + 4 * i, RAM + 0x200 + 4 * i, RAM + 0x200, RAM + off]))
        a = r.choice([0, 1, 100, 0x7fffffff, r.getrandbits(32)])
        b = r.choice([0, 2, 100, 0x80000000, r.getrandbits(32)])
        c = r.choice([0, 8, 0x1000, r.getrandbits(32)])
        if op == 'Setg':
            return Call(0x4f6820, [RAM, a, b, c], regions, conv='t', ret='a')
        return Call(0x4f6850, [RAM, a, b], regions, conv='t', ret='a')
    if op == 'Pixel':
        width = r.choice([1, 4, 16, 64])
        x, y = r.randrange(width), r.randrange(8)
        regions = {RAM: obj, BUF: r.bytes(0x400), 0x879da8: struct.pack('<IIII', 0, RAM + 0x800, 0, 0),
                   0x8bcbd0: struct.pack('<II', width, 0)}
        regions[RAM + 0x800] = bytearray(obj)  # the renderer object: +8 = the buffer
        put(regions[RAM + 0x800], 8, 'I', BUF)
        return Call(0x56a510, [x, y, RAM + 0x200], regions, conv='s', ret='a')
    if op == 'Fill':
        return Call(0x56b630, [], {0x879d00: r.bytes(0x60)}, ret='a')
    return Call(0x56b830, [RAM, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
