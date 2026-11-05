# The software rasteriser's drawing methods (src/game/render/rasterdraw: 0x56a030, 0x56a180, 0x56a230, 0x56a2f0, 0x56a540) against the originals: the routines recorders, the vertices on the stack hashed where set (one array a case: the other logged as 0), FixedVerts_PackRounded real.
# usage: rasterdraw.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fb, log_region, put, recorder, set_return, strings  # noqa: E402

TRI, TRI2, MAPPED, FAN, FANRECT, BASE8, BASEC, TEX8 = range(1, 9)
B = Bench([(0x56a030, 0x56a173), (0x56a180, 0x56a223), (0x56a230, 0x56a2e4), (0x56a2f0, 0x56a430), (0x56a540, 0x56a644), (0x563130, 0x563220), (0x67a830, 0x67a860)],
          hooks={0x6ef7f0: _recorder(TRI, 3, cdecl=True), 0x6f02e0: _recorder(TRI2, 3, cdecl=True), 0x6f0a50: _recorder(MAPPED, 3, cdecl=True), 0x6792f0: _recorder(FAN, 7, cdecl=True), 0x679350: _recorder(FANRECT, 7, cdecl=True)})
DEVICE, TABLE, BASE, BASE_VT, PRIM, VERTS, INDICES, TEXTURE, TEX_VT = RAM + 0x10, RAM + 0x40, RAM + 0x60, RAM + 0x80, RAM + 0xc0, RAM + 0x100, RAM + 0x400, RAM + 0x480, RAM + 0x4c0
ALL = 0xffffff
SET = sum(0xf << (i * 6) for i in range(4))


def globals_(r):
    return {0xfbe008: r.bytes(4), 0xfbe044: r.bytes(4), 0x8bd550: r.bytes(4), 0x8bd508: r.bytes(4), 0x6f19b0: r.bytes(4), 0x6f19cc: r.bytes(4), 0x6f1a10: r.bytes(4),
            0x7013b0: struct.pack('<f', r.choice([16.0, 8.0, r.uniform(1, 32)])), 0x7013b4: struct.pack('<f', r.choice([-1.0, 1.0, r.uniform(-4, 4)])), 0x7013c4: struct.pack('<f', r.choice([128.0, 256.0, r.uniform(1, 512)]))}


def primitive(r):
    data = bytearray(r.bytes(0x40))
    flags = r.getrandbits(32) & ~0x4000
    if r.random() < 0.1:
        flags |= 0x4000
    if r.random() < 0.5:
        flags = flags & ~0x100 | 0x10000000
    n = r.choice([0, 1, 2, 3, 3, 4, 5, 6, 9, 12])
    put(data, 4, 'I', flags)
    put(data, 0x24, 'i', n)
    put(data, 0x28, '2I', VERTS, INDICES)
    return data, struct.pack('<16H', *[r.randrange(0x20) for _ in range(16)])


def device(r):
    """self at DEVICE + 8: the table at self - 8 (its +4 the base's offset)."""
    return {DEVICE: struct.pack('<I', TABLE), TABLE: struct.pack('<2I', 0, BASE - DEVICE), BASE: struct.pack('<I', BASE_VT),
            BASE_VT: struct.pack('<4I', 0, 0, recorder(BASE8, 1), recorder(BASEC, 2))}


def case(r):
    log = log_region()
    regions = {LOG: log, **globals_(r)}
    which = r.choice(['prim', 'prim', 'prim', 'outline', 'outline', 'quad', 'rect', 'rect', 'rect', 'mapped', 'mapped'])
    if which in ('prim', 'outline'):
        data, indices = primitive(r)
        regions.update({PRIM: data, INDICES: indices})
        if which == 'outline':
            set_return(log, FAN, r.getrandbits(32))
            return Call(0x56a180, [PRIM], regions, conv='s', ret='a')
        regions.update(device(r))
        return Call(0x56a030, [DEVICE + 8, PRIM], regions, conv='t', ret='a')
    if which == 'quad':
        set_return(log, FAN, r.getrandbits(32))
        if r.random() < 0.5:
            blob(log, FAN, 2, SET)
            strings(log, FAN, 0, out=1 << 4)
        else:
            blob(log, FAN, 4, 7)
            strings(log, FAN, 0, out=1 << 2)
        return Call(0x56a230, [], regions, conv='c', ret='a')
    if which == 'rect':
        texture = r.choice([0, TEXTURE, TEXTURE])
        regions.update(device(r))
        regions.update({TEXTURE: struct.pack('<4I', TEX_VT, 0, r.randrange(0x400), r.randrange(0x400)), TEX_VT: struct.pack('<4I', 0, 0, recorder(TEX8, 0), 0)})
        set_return(log, TEX8, r.getrandbits(32))
        if r.random() < 0.7:
            blob(log, FANRECT, 2, ALL if texture else SET)
            strings(log, FANRECT, 0, out=1 << 4)
        else:
            blob(log, FANRECT, 4, 7)
            strings(log, FANRECT, 0, out=1 << 2)
        return Call(0x56a2f0, [DEVICE + 8, fb(r.uniform(-20, 700)), fb(r.uniform(-20, 500)), fb(r.uniform(-20, 700)), fb(r.uniform(-20, 500)), r.getrandbits(32), texture], regions, conv='t', ret='a')
    blob(log, MAPPED, 0, 0x3ffff)
    strings(log, MAPPED, 0, out=0b110)
    return Call(0x56a540, [fb(r.choice([256.0, 512.0, r.uniform(1, 1000)])), r.getrandbits(32), r.getrandbits(32), fb(r.uniform(0, 256)), fb(r.uniform(0, 256)), fb(r.uniform(0, 256)), fb(r.uniform(0, 256))], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)
