# The tracked global blocks of src/game/core/gameheap (0x579b40 alloc, 0x579b80 free, the dword arrays' 0x58a4f0 / 0x58a520, the
# holders' 0x586e20 / 0x587110) against the originals: the count at 0x8bb214 small,
# at the 0x8000 limit or past it (and 0, -1, 0x80000000 for the alloc), the pairs at 0x87b1f8 holding the block asked for (as it is or 4
# below), another or none, GlobalAlloc failing now and then (the heap region serves GlobalAlloc / GlobalFree).
# The arrays: counts whose size (times 4) is small, wraps included, the block at +0x14 (a holder's at +0) tracked, another or none.
# Their users: a texture set's shared list (src/game/gfx/texset: 0x575c00) over any bytes with any array; the renderer's frame buffer
# (render/renderer: 0x576310) with an old one tracked, another or none, sizes up to 24 by 24 (and wrapping ones), the render state stack's
# count -1 to 5 holding the state's address or not; a sprite image's pixels (gfx/spriteframe: 0x588940), the format 0-3 indexing pixel sizes
# 2, 4, 3 and 1 at 0x701fe0, sizes with any high bits, and a renderer part's (render/renderer: 0x48f3d0) for its image at +0x9c
# of its s16 sizes (small, negative or wide), done (+0x44) or not, its frame (+0x54) holding no image or the part's own with no textures; a texture's pixels (gfx/texture: 0x590a80) for sides around the level steps and
# wrapping ones (0x8000, 0x10000; a block too big for the bench's heap fails), any flags, the float at +0x30 around 0..1.5 or any bits. (The allocations' errors go onto the engine's list at 0x879ad0, its count 0 to 0x40.)
# usage: trackedglobal.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Alloc,Free,ArrayAlloc,ArrayFree,Holder,HolderFree,TexSetShared,FrameBuffer,ImagePixels,PartPixels)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, fb, global_heap, heap, put  # noqa: E402

CALLS = {'Alloc': 0x579b40, 'Free': 0x579b80, 'ArrayAlloc': 0x58a4f0, 'ArrayFree': 0x58a520, 'Holder': 0x586e20, 'HolderFree': 0x587110,
         'TexSetShared': 0x575c00, 'FrameBuffer': 0x576310, 'ImagePixels': 0x588940, 'TextureAlloc': 0x590a80,
         'PartPixels': 0x48f3d0}
B = Bench([(0x579b40, 0x579c13), (0x58a4f0, 0x58a548), (0x586e20, 0x586e3a), (0x587110, 0x58712f), (0x575c00, 0x575c5b), (0x576310, 0x5763e1),
           (0x588940, 0x5889a1), (0x56af20, 0x56af9c), (0x590a80, 0x590b88), (0x48f3d0, 0x48f409), (0x588e40, 0x588ee8)])
COUNT, TABLE, LOW = 0x8bb214, 0x87b1f8, 0x87b1e0   # (the pairs from index -3, for the counts of -1 and below)
BLOCKS = [HEAP + 0x40, HEAP + 0x44, HEAP + 0x80, HEAP + 0x84, HEAP + 0x100, 0, 4]


def user(r, name, regions):
    """the tracked blocks' users: an error list at 0x879ad0 for their failures."""
    errors = bytearray(r.bytes(0x204))
    put(errors, 0x200, 'i', r.choice([r.randrange(0x40), 0, 0x3f, 0x40]))
    regions[0x879ad0] = errors
    regions[0x8bd508] = struct.pack('<I', r.getrandbits(32) & ~0x1000000)
    if name == 'TexSetShared':
        regions[RAM] = r.bytes(0x10)
        return Call(CALLS[name], [RAM, r.getrandbits(32)], regions, conv='t', ret='a')
    if name == 'FrameBuffer':
        obj = bytearray(r.bytes(0x80))
        put(obj, 0x10, 'I', r.choice(BLOCKS + [r.getrandbits(32) | 0x80000000]))
        count = r.choice([-1, 0, 1, 2, 3, 3, 4, 5, r.randrange(4)])
        put(obj, 0x70, 'i', count)
        for k in range(4):
            if r.random() < 0.3:
                put(obj, 0x60 + 4 * k, 'I', RAM + 0x48)
        regions[RAM] = obj
        regions[0x87afe8] = r.bytes(4)
        w, h = r.choice([(r.randrange(25), r.randrange(25)), (0, 5), (1, 1), (0x40000001, 3), (0x80000002, 0xc0000001), (-1, -2), (3, 0x80000000)])
        return Call(CALLS[name], [RAM, w & 0xffffffff, h & 0xffffffff], regions, conv='t', ret='a')
    if name == 'TextureAlloc':
        tex = bytearray(r.bytes(0x60))
        put(tex, 0x30, 'I', r.choice([fb(r.uniform(0, 1.5)), fb(r.choice([0.0, 1.5, 1.0, -1.0, 3.0, 1e9])), r.getrandbits(32)]))
        mode = r.randrange(4)
        side = r.choice([0, 1, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, r.randrange(40), 0x8000, 0x10000, 0x18000])
        need = [2, 2, 2, 4][mode] * side * side & 0xffffffff   # (the pixel sizes at 0x7024e0)
        regions[HEAP] = heap(need + 0x200, r.choice([0] * 7 + [1])) if need < 0x80000 else heap(0x400, 1)   # (a too-big block fails, as it would)
        regions[0x8bd554] = struct.pack('<I', mode)
        return Call(CALLS[name], [RAM, side, r.getrandbits(32) & r.choice([0xffffffff, 0x2100, 0])], {**regions, RAM: tex}, conv='t', ret='a')
    regions[RAM] = r.bytes(0x18)
    if name == 'PartPixels':
        part = bytearray(r.bytes(0xb4))
        w, h = (r.choice([0, 1, 5, 16, r.randrange(40), -1, -0x8000, 0x7fff]) for _ in range(2))
        if (w & 0xffff) * (h & 0xffff) * 4 > 0xf000:   # (the block, of the sizes taken as words, zeroed within the heap's 64 KB)
            h = r.choice([0, r.randrange(40) if (w & 0xffff) < 0x180 else 0])
        put(part, 0x3c, '2h', w, h)
        put(part, 0x44, 'H', r.choice([0, 0, 0, 1, r.getrandbits(16)]))
        put(part, 0x7c, 'I', r.choice([0, RAM + 0x9c]))
        put(part, 0xb2, 'H', 0)
        regions[RAM] = part
    regions[0x701fe0] = struct.pack('<4I', 2, 4, 3, 1)
    regions[0x8bd554] = struct.pack('<I', r.randrange(4))
    w, h = (r.randrange(40) | r.choice([0, 0, 0x10000, 0xffff0000]) for _ in range(2))
    if name == 'PartPixels':
        return Call(CALLS[name], [RAM, r.getrandbits(32)], regions, conv='t')
    return Call(CALLS[name], [RAM, w, h], regions, conv='t', ret='a')


def case(r):
    name = r.deal(OPS)
    regions = {HEAP: heap(fail=r.choice([0] * 7 + [1])), **global_heap()}
    big = r.random() < 0.1
    if big:
        count = r.choice([0x7fff, 0x8000, 0x8001] if name in ('Free', 'ArrayFree', 'Holder', 'HolderFree') else [0x7fff, 0x8000, 0x8001, 0x80000000])
        table = bytearray(0x18 + 0x8002 * 8)
        for _ in range(r.randrange(0, 6)):   # a few pairs anywhere
            k = r.randrange(0, 0x8002)
            struct.pack_into('<2I', table, 0x18 + 8 * k, r.choice(BLOCKS), r.choice(BLOCKS))
    else:
        count = r.choice([0, 1, 2, 3, 4, 5, 6, 0xffffffff, 0xfffffffe] + ([0x80000000] if name in ('Alloc', 'ArrayAlloc') else []))
        table = bytearray(0x18 + 8 * 8)
        for k in range(len(table) // 4):
            struct.pack_into('<I', table, 4 * k, r.choice(BLOCKS + [r.getrandbits(32)]))
    regions[LOW] = table
    regions[COUNT] = struct.pack('<I', count) + r.bytes(4)
    if name in USERS:
        return user(r, name, regions)
    if name.startswith('Holder'):
        regions[RAM] = put(bytearray(r.bytes(8)), 0, 'I', r.choice(BLOCKS + [r.getrandbits(32)]))
        return Call(CALLS[name], [RAM], regions, conv='t', ret='a' if name == 'HolderFree' else 'v')
    if name.startswith('Array'):
        regions[RAM] = put(bytearray(r.bytes(0x20)), 0x14, 'I', r.choice(BLOCKS + [r.getrandbits(32)]))
        args = [r.choice([0, 1, 4, 0x40, 0x40000001, 0x80000010])] if name == 'ArrayAlloc' else []
        return Call(CALLS[name], [RAM] + args, regions, conv='t', ret='a')
    arg = r.choice([0, 1, 0x10, 0x100]) if name == 'Alloc' else r.choice(BLOCKS + [r.getrandbits(32)])
    return Call(CALLS[name], [arg], regions, conv='c', ret='a')


USERS = ('TexSetShared', 'FrameBuffer', 'ImagePixels', 'TextureAlloc', 'PartPixels')
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
