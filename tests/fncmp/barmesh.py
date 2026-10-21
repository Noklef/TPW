# The bar mesh (src/game/gfx/barmesh: 0x477c00, 0x4786f0, 0x477cc0, 0x477d20) against the
# original: a vertex and a texture coordinate set, and the top edge's vertices and texture
# coordinates at the bar's level, with the arrays apart from the bar or overlapping it (level,
# pointers).
# Construct / Overlay (0x477870, 0x477ff0): a random bar built, its name a text (animatormesh.py's environment: global_heap(), the
# texture directories and holder logged). Shown (0x477d70): the bar's animator (+0x6c) holder.py's AnimWake case (a word other than
# 0) or its Animator case (0), the word +8 the same or not.
# Apply / OverlayApply (0x477db0, 0x478400): see apply().
# usage: barmesh.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, _cdecl_log, _cdecl_log2, global_heap, heap, log_region, put, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import animatormesh  # noqa: E402
import holder  # noqa: E402
import objmodel  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged([(0x477c00, 0x477c30), (0x477cc0, 0x477d19), (0x477d20, 0x477d70), (0x4786f0, 0x478715), (0x477870, 0x477bf6), (0x477ff0, 0x478369),
                  (0x477d70, 0x477dae), (0x6597b0, 0x659896), (0x659994, 0x6599bc), (0x659d00, 0x659d53), (0x65a9d0, 0x65aa00), (0x424ec0, 0x424ec6),
                  (0x477db0, 0x477fef), (0x478400, 0x4786e8), (0x476c50, 0x476e80), (0x46f150, 0x46f180), (0x4700f0, 0x4702d0), (0x46fc00, 0x46fcb0)]
                 + animatormesh.RANGES + holder.RANGES), hooks={**holder.HOOKS, **animatormesh.HOOKS}, sines=4096)
BAR, ARR = RAM, RAM + 0x1000
CALLS = ('Vertex', 'TexCoord', 'TopVerts', 'TopTex', 'Construct', 'Overlay', 'Shown', 'Apply', 'OverlayApply')
NAME = RAM + 0x800



def construct(r, address):
    """0x477870 / 0x477ff0 (thiscall; name): a random bar, the name a text (or now and then none: the record's name copied faults in both)."""
    regions = {BAR: r.bytes(0x130), NAME: animatormesh.text(r), HEAP: heap(0x800), LOG: log_region()}
    regions.update(global_heap())
    return Call(address, [BAR, NAME if r.random() < 0.97 else 0], regions, conv='t', ret='a')


def shown(r):
    """0x477d70 (thiscall; word): the bar's animator holder.py's AnimWake case (for a word other than 0) or Animator case (0); the word +8
    the same or not."""
    word = r.choice([0, 0, 1, 0x10000, r.getrandbits(32)])
    c = holder.animator_wake(r) if word & 0xffff else holder.animator(r)
    regions = dict(c.regions)
    anim = c.args[0]
    bar = put(bytearray(r.bytes(0x6c)), 8, 'H', word & 0xffff if r.random() < 0.2 else r.getrandbits(16)) + regions.pop(anim)
    regions[anim - 0x6c] = bar
    return Call(0x477d70, [anim - 0x6c, word], regions, conv='t', ret='v')


MESH, QUADS, HOLD, AOBJ, DESC, RECT, VT = (RAM + 0x50000 + k * 0x1000 for k in range(7))


def apply(r, address):
    """0x477db0 / 0x478400 (thiscall (flags, desc), pops 8): a mesh (0x180 bytes) with a holder (+4, now and then 0), its vtable recorders
    (slots 9 and 10 the bar's top edge), the bounds +0x4c / +0x58 low and high or the other way round (a NaN now and then), the rectangle's
    words, the scale +0x114 any, the level +0x120 the one the desc gives now and then (or a NaN); its vertices (+0x7c) and coordinates (+0x84)
    apart (QUADS) or now and then inside the mesh; its holder (+0x90) one for objmodel.py's matrices_env() (+0x1c). The desc's dword (the
    level / 100 * 1024: about 0..1024, past either end, or any), words +6 and +8 and rectangle (+0x14); flags of bits 1, 2, 4, 8, 0x10 or
    any; the desc now and then 0."""
    mesh = bytearray(r.bytes(0x180))   # (room for the quads put inside it)
    put(mesh, 0, 'I', VT)
    put(mesh, 4, 'I', r.choice([1, HOLD, r.getrandbits(32) | 1, 1, 0 if r.random() < 0.3 else 1]))
    bounds = [r.uniform(-60, 60) for _ in range(6)]
    if r.random() < 0.6:
        bounds = [min(bounds[k], bounds[k + 3]) for k in range(3)] + [max(bounds[k], bounds[k + 3]) for k in range(3)]
    put(mesh, 0x4c, '6f', *bounds)
    if r.random() < 0.05:
        put(mesh, 0x4c + 4 * r.randrange(6), 'I', r.choice([0x7fc00000, 0x7f800001, 0xffc00000]))
    elif r.random() < 0.1:   # (a NaN low or high bound of x or y, now and then both of one axis)
        for k in r.choice([[0], [1], [3], [4], [0, 3], [1, 4]]):
            put(mesh, 0x4c + 4 * k, 'I', r.choice([0x7fc00000, 0x7f800001, 0xffc00000, 0x7fc12345, 0xff812345]))
    for k in range(4):
        put(mesh, 0x18 + 2 * k, 'h', r.choice([r.randrange(0, 2048), r.randrange(-100, 100), r.getrandbits(15)]))
    for off in (0x7c, 0x84):
        put(mesh, off, 'I', r.choice([QUADS, QUADS, QUADS, QUADS + 0x80, MESH + 0xe0 + 4 * r.randrange(0x10)]))
    put(mesh, 0x90, 'I', HOLD)
    put(mesh, 0x114, 'f', r.uniform(-2, 2))
    raw = r.choice([r.randrange(0, 1025), r.randrange(-50, 1100), 0, 1024, r.getrandbits(32)])
    put(mesh, 0x120, 'f', r.choice([max(0.0, min(100.0, raw / 1024 * 100)), r.uniform(0, 100), float(r.randrange(101)), 0.0, 100.0]))
    if r.random() < 0.05:
        put(mesh, 0x120, 'I', r.choice([0x7fc00000, 0xffc00000]))
    desc = bytearray(r.bytes(0x18))
    put(desc, 0, 'I', raw & 0xffffffff)
    put(desc, 0x14, 'I', RECT)
    rect = bytearray(r.bytes(0x10))
    for k in range(4):
        put(rect, 4 + 2 * k, 'h', r.choice([r.randrange(0, 2048), r.randrange(-100, 100), r.getrandbits(15)]))
    hold = put(bytearray(r.bytes(0x20)), 0x1c, 'I', AOBJ)
    bits = [0x1, 0x2, 0x4, 0x8, 0x10]
    flags = r.choice([sum(b for b in bits if r.random() < 0.5), 8, 8, 0x18, 0xa, 0x14, r.getrandbits(32), 0])
    regions = objmodel.matrices_env(r, {MESH: mesh, QUADS: r.bytes(0x100), HOLD: hold, AOBJ: r.bytes(0x100), DESC: desc, RECT: rect,
                                        VT: vtable({}), 0x8bd508: r.bytes(4)})
    return Call(address, [MESH, flags, DESC if r.random() < 0.97 else 0], regions, conv='t')


def case(r):
    op = r.deal(OPS)
    if op == 'Apply':
        return apply(r, 0x477db0)
    if op == 'OverlayApply':
        return apply(r, 0x478400)
    if op == 'Construct':
        return construct(r, 0x477870)
    if op == 'Overlay':
        return construct(r, 0x477ff0)
    if op == 'Shown':
        return shown(r)
    if op == 'Vertex':  # vertex i of ARR (blocks of 0x30)
        i = r.choice([r.randrange(8), r.randrange(0x40)])
        return Call(0x477c00, [ARR, i, r.f(), r.f(), r.f()], {ARR: r.bytes(0x400)})
    if op == 'TexCoord':  # coordinate i of ARR (blocks of 0x20)
        i = r.choice([r.randrange(8), r.randrange(0x40)])
        return Call(0x4786f0, [ARR, i, r.f(), r.f()], {ARR: r.bytes(0x400)})
    bar = bytearray(r.bytes(0x200))
    struct.pack_into('<I', bar, 0x120, r.f())
    for off in (0x7c, 0x84):  # apart, or inside the bar (over the level or the pointers)
        struct.pack_into('<I', bar, off, r.choice([ARR, ARR, BAR + 4 * r.randrange(0x60)]))
    regs = {BAR: bar, ARR: r.bytes(0x100)}
    return Call(0x477cc0 if op == 'TopVerts' else 0x477d20, [BAR], regs, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
