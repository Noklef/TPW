# The bar mesh (src/game/gfx/barmesh: 0x477c00, 0x4786f0, 0x477cc0, 0x477d20) against the
# original: a vertex and a texture coordinate set, and the top edge's vertices and texture
# coordinates at the bar's level, with the arrays apart from the bar or overlapping it (level,
# pointers).
# Construct / Overlay (0x477870, 0x477ff0): a random bar built, its name a text (animatormesh.py's environment: global_heap(), the
# texture directories and holder logged). Shown (0x477d70): the bar's animator (+0x6c) holder.py's AnimWake case (a word other than
# 0) or its Animator case (0), the word +8 the same or not.
# usage: barmesh.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, _cdecl_log, _cdecl_log2, global_heap, heap, log_region, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import animatormesh  # noqa: E402
import holder  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged([(0x477c00, 0x477c30), (0x477cc0, 0x477d19), (0x477d20, 0x477d70), (0x4786f0, 0x478715), (0x477870, 0x477bf6), (0x477ff0, 0x478369),
                  (0x477d70, 0x477dae), (0x6597b0, 0x659896), (0x659994, 0x6599bc), (0x659d00, 0x659d53), (0x65a9d0, 0x65aa00), (0x424ec0, 0x424ec6)]
                 + animatormesh.RANGES + holder.RANGES), hooks={**holder.HOOKS, **animatormesh.HOOKS})
BAR, ARR = RAM, RAM + 0x1000
CALLS = ('Vertex', 'TexCoord', 'TopVerts', 'TopTex', 'Construct', 'Overlay', 'Shown')
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


def case(r):
    op = r.deal(OPS)
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
