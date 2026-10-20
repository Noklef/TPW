# The coaster's mesh finished and begun again (src/game/coaster/lists: 0x43f150) against the original. The coaster at 0x792218 with its
# build (+4: the sizes, the animator to clone, 0-4 lists, now and then more) and its animator (+0x38): none, a bare one (no matrix, its
# holder without a part, the renderer's pool empty so the wake adds nothing), or holder.py's AnimWake case. The lists (0x1c bytes at
# [0x7925dc]) with or without a slot, their items chained (0-3, the last one now and then running on into the next list's), the bounds
# (0x7925a8..) any floats (NaNs and beyond the int range among them, for __ftol), the counts any, the track's float any. Keep 0 (Reset):
# animatormesh.py's Clone case as the build's animator, flags 0x21ef (the animator then bare or none).
# usage: coastermesh.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402
import animatormesh  # noqa: E402
import clone  # noqa: E402
import holder  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged([(0x43f150, 0x43f4db), (0x45bef0, 0x45bf20), (0x455690, 0x4557bf), (0x454a10, 0x455689), (0x44b140, 0x44b220)]
                 + animatormesh.RANGES + holder.RANGES), hooks={**holder.HOOKS, **animatormesh.HOOKS})
COASTER, BUILD, SIZES, COUNT, LISTS, ITEMS, SLOTS, PAIRS, ANIM, HOLD, MESH, PART, RENDER, TRACK = (RAM + 0x70000 + 0x400 * k for k in range(14))


def bound(r):
    k = r.random()
    return r.f() if k < 0.6 else fb(r.uniform(-3e4, 3e4)) if k < 0.85 else r.choice([0x7fc00000, 0x4f800000, 0xcf800000, 0x49742400, 0xc9742400])


def finish(r, animator):
    """The regions of a case, the animator 'wake' (holder.py's AnimWake case), 'bare' or None (the build's animator to clone left to
    the caller)."""
    regions = {}
    count = r.choice([0, 1, 2, 3, 4])
    if animator == 'wake':
        c = holder.animator_wake(r)
        regions.update(c.regions)
        anim = c.args[0]
        hold = struct.unpack_from('<I', regions[anim], 0x24)[0]
        regions[hold] = put(bytearray(regions[hold]), 4, 'I', MESH)
        regions[anim] = put(bytearray(regions[anim]), 0x1c, 'I', PAIRS)
    elif animator == 'bare':
        anim = ANIM
        regions[ANIM] = put(put(put(bytearray(r.bytes(0x40)), 0xc, 'I', 0), 0x1c, 'I', PAIRS), 0x24, 'I', HOLD)
        regions[HOLD] = put(put(bytearray(r.bytes(0x20)), 4, 'I', MESH), 0x1c, 'I', 0)
        regions[0x87a6d0] = struct.pack('<I', RENDER)
        regions[RENDER] = put(bytearray(r.bytes(0x10)), 4, 'I', 0)
    else:
        anim = 0
    regions[MESH] = put(bytearray(r.bytes(0xb0)), 0x78, 'I', PART)
    regions[PART] = r.bytes(0x90)
    regions[PAIRS] = r.bytes(0x100)
    regions[COASTER] = put(put(bytearray(r.bytes(0x40)), 4, 'I', BUILD), 0x38, 'I', anim)
    regions[BUILD] = put(put(bytearray(r.bytes(0x20)), 0x14, 'I', SIZES), 0x1c, 'I', COUNT)
    regions[SIZES] = r.bytes(0x10)
    regions[COUNT] = put(bytearray(r.bytes(8)), 4, 'H', count)
    lists = bytearray(r.bytes(0x1c * 5))
    items = bytearray(r.bytes(0xc * 16))
    for k in range(16):
        put(items, 0xc * k + 8, 'I', 0)
    used = 0
    for i in range(5):
        n = r.choice([0, 1, 2, 3])
        head = ITEMS + 0xc * used if n else 0
        for j in range(n):
            nxt = ITEMS + 0xc * (used + j + 1) if j + 1 < n else 0
            put(items, 0xc * (used + j) + 8, 'I', nxt)
        if n and r.random() < 0.1:   # the last item runs on into the next list's (or a spare one)
            put(items, 0xc * (used + n - 1) + 8, 'I', ITEMS + 0xc * (used + n))
        used += n
        put(lists, 0x1c * i + 4, 'I', r.choice([0, 1, 4, r.getrandbits(32)]))
        put(lists, 0x1c * i + 8, 'I', head)
        put(lists, 0x1c * i + 0x10, 'I', r.choice([SLOTS + 0x10 * i, SLOTS + 0x10 * i, 0]))
    regions[LISTS] = lists
    regions[ITEMS] = items
    regions[SLOTS] = r.bytes(0x50)
    regions[TRACK] = put(bytearray(r.bytes(0x40)), 0x38, 'I', bound(r))
    regions[0x792218] = struct.pack('<I', COASTER)
    bounds = bytearray(r.bytes(0x38))
    for k in range(0, 0x1c, 4):
        put(bounds, k, 'I', bound(r))
    put(bounds, 0x34, 'I', LISTS)
    regions[0x7925a8] = bounds
    regions[0x7923e0] = r.bytes(0xc)
    regions[0x792408] = r.bytes(0xc)
    regions[0x79c600] = r.bytes(8)
    regions[0x790fe4] = struct.pack('<I', TRACK)
    return regions


def finished(r):
    regions = finish(r, r.choice(['wake', 'wake', 'bare', 'bare', None]))
    return Call(0x43f150, [r.choice([1, 1, 0x100, r.getrandbits(32) | 1])], regions, ret='v')


def reset(r):
    c = animatormesh.clone_case(r)
    regions = finish(r, r.choice(['bare', 'bare', None]))   # (holder.py's AnimWake case shares RAM's low addresses with the clone's)
    regions.update(c.regions)
    if r.random() < 0.8 and not struct.unpack_from('<I', regions[clone.MODEL], 0x78)[0]:   # (mostly a part to place)
        regions[clone.MODEL] = put(bytearray(regions[clone.MODEL]), 0x78, 'I', clone.ROOT)
    put(regions[BUILD], 0x18, 'I', c.args[0])
    return Call(0x43f150, [0], regions, ret='v')


CASES = {'Finish': finished, 'Reset': reset}
OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
