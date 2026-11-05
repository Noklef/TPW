# A mesh tree drawn (src/game/model/objmodel: 0x57a530 ObjModel_DrawMeshTree) against the original, its recursion real: the transformer's slot 6 (a cdecl recorder, k 6, filling each output with its own dwords), the band test (k 9), the clip codes (k 10), the lighting
# (k 11), the draw's add (k 12) and the textures touched (k 13) recorders by their addresses. 0-9 meshes as siblings and children, flags with each bit that counts (hidden, wanted, banded, the rectangle's bounds about the four bytes of 0x8bd4e8, no children); the answers failing now and then.
# usage: objmodeltree.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, put, strings, vtable  # noqa: E402

XFORM, BAND, CLIP, LIGHT, ADD, TOUCH = 6, 9, 10, 11, 12, 13
B = Bench([(0x57a530, 0x57a747)], hooks={0x56ee90: _recorder(BAND, 1), 0x561a30: _recorder(CLIP, 2), 0x57aa10: _recorder(LIGHT, 4), 0x581cd0: _recorder(ADD, 5), 0x57a420: _recorder(TOUCH, 1)})
SELF, MATA, MATB, XF, XFVT, MESHES, TABLES = RAM, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x400, RAM + 0x1000, RAM + 0x2000
SIZE = 0xa0


def table(k):
    return TABLES + 0x100 * k


def answer_table(r, k, choices):
    return table(k), struct.pack('<40I', *[r.choice(choices) for _ in range(40)])


def mesh(r, sibling, child, bounds):
    m = bytearray(r.bytes(SIZE))
    flags = r.choice([0, 0, 0, 0x8000, 0x200, 0x10]) | r.choice([0x40000, 0x80000, 0x40000, 0]) | r.choice([0, 0x2000, 0x2000 | 0x4000000]) | r.choice([0, 0, 0x20])
    put(m, 0, 'I', flags | r.getrandbits(32) & ~0x040ca230)
    put(m, 8, 'I', sibling)
    put(m, 0xc, 'I', child)
    if r.random() < 0.8:                                                    # (the rectangle's bytes +0x90..+0x93 about the four of 0x8bd4e8, at their limits or past by one)
        near = [bounds[1], bounds[3], bounds[0], bounds[2]]
        m[0x90:0x94] = struct.pack('<4b', *[b + r.choice([0, 0, 0, 1, -1, 5, -5]) for b in near])
    return m


def case(r):
    log = log_region()
    count = r.randrange(0, 10)
    sibling, child = [0] * count, [0] * count
    for i in range(1, count):                                               # (each new mesh hangs on an earlier one that has that place free)
        free = [(j, s) for j in range(i) for s, links in enumerate((sibling, child)) if not links[j]]
        j, s = r.choice(free)
        (sibling, child)[s][j] = MESHES + SIZE * i
    bounds = [r.randrange(-100, 100) for _ in range(4)]
    regions = {LOG: log, SELF: r.bytes(0x150), MATA: r.bytes(0x40), MATB: r.bytes(0x40), XF: struct.pack('<I', XFVT), XFVT: vtable({}, 8, cdecl={XFORM: 4}),
               0x879d60: r.bytes(4), 0x879a20: r.bytes(4), 0x8bcbb8: r.bytes(4), 0x8bb624: r.bytes(4), 0x87b1d8: struct.pack('<I', XF), 0x8bd4e8: struct.pack('<4b', *bounds),
               0x8bd508: struct.pack('<I', r.choice([0, 0, 0x40000000, 0x80000000, r.getrandbits(32)]))}
    for i in range(count):
        regions[MESHES + SIZE * i] = mesh(r, sibling[i], child[i], bounds)
    for k, choices in ((BAND, [0, 1, 1, 0x100]), (CLIP, [0x40, 0x40, 0x41, 0x7f, 0x40000040, 0, 0x3f, r.getrandbits(32)]), (LIGHT, [1] * 19 + [0]), (ADD, [1] * 19 + [0])):
        at, data = answer_table(r, k, choices)
        answers(log, k, at)
        regions[at] = data
    blob(log, XFORM, 2, 0xffff)                                             # (the matrices given: the caller's, or a parent's on the stack; the identity on the stack shows under the fill's first dwords)
    strings(log, XFORM, 0, out=1 << 3)
    fill(log, XFORM, 3, r.bytes(0x70), records=7)
    blob(log, CLIP, 1, 0xffff)
    blob(log, LIGHT, 1, 0xffff)
    blob(log, ADD, 0, 0xffff)
    strings(log, ADD, 0, out=1 << 1)
    return Call(0x57a530, [SELF, MESHES if count else 0, MATA, MATB], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
