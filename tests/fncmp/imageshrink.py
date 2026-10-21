# An image's box-average shrink (src/game/gfx/imageops: 0x5d4260) against the original: sources of 1 to 40 x 1 to 30 pixels of random bytes (many extremes),
# new sizes from 1 to the source's (now and then larger: a count of 0 divides by 0 in both), formats 1 and 2 (and others, which write nothing), and the
# heap failing one of the three scratch blocks now and then. Panel (0x5d4970) shrinks a panel's image (+0x50) the same way; now and then its pixels are
# null, so the panel reset runs first (its six source images empty): through the null pointer for an image that is not empty (a crash in both).
# Surface (0x5d3ac0): an image made 320 x 240 from a surface description (see surface()). Blocks (0x5d48a0): the same on the seven-block
# object's image (+0x60) with the description passed by address; mostly with the bit counts that do no more than the set up (fast).
# Surface and Blocks take some 19 s a case at full size, so the ops are dealt from a weighted deck (WEIGHTS: each of them 1 in 25, the others
# several times), and Surface's bit counts and mask layouts from a deck of their own (LAYOUTS), so that the first 400 cases reach each of them.
# ToImage (0x5e80d0): Shrink's source shrunk into a destination image of the new size (its width now and then 0 or -1).
# usage: imageshrink.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Shrink,Panel,Surface,Blocks,ToImage)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x5d4260, 0x5d45b0), (0x67acb0, 0x67acb1), (0x5d4970, 0x5d4a2d), (0x5d48d0, 0x5d4970), (0x5d4170, 0x5d4260), (0x5d3ac0, 0x5d4162), (0x5d48a0, 0x5d48c3), (0x5e80d0, 0x5e840f)])
PIXELS, DEST = RAM + 0x1000, RAM + 0x8000
OPS = [k for k in os.environ.get('OPS', 'Shrink,Panel,Surface,Blocks,ToImage').split(',') if k]
SURFACE = RAM + 0x10000
MASKS16 = [(0xf800, 0x07e0, 0x001f), (0x7c00, 0x03e0, 0x001f), (0x0f00, 0x00f0, 0x000f), (0x001f, 0x07e0, 0xf800), (0xfff0, 0x000c, 0x0003),
           (0x0505, 0x8000, 0x0a0a), (0xffffffff, 0x10, 0x80000000)]
MASKS32 = [(0xff0000, 0xff00, 0xff), (0xff, 0xff00, 0xff0000), (0xff000000, 0xff0000, 0xffff), (0x3ff00000, 0xffc00, 0x3ff)]
# (bits, masks): each layout of each count, odd masks ('odd': three random ones) of each, and the counts that do nothing (24 returns 0, 8 and 0 1)
LAYOUTS = [(16, m) for m in MASKS16] + [(32, m) for m in MASKS32] + [(16, 'odd'), (32, 'odd'), (24, None), (8, None), (0, None)]
WEIGHTS = {'Shrink': 8, 'Panel': 8, 'ToImage': 7, 'Surface': 1, 'Blocks': 1}   # (a deck of 25: 400 cases deal Surface 16 times, LAYOUTS' size)
DECK = [k for k in OPS for _ in range(WEIGHTS.get(k, 1))]


def surface(r, dealt=True):
    """0x5d3ac0: a surface of 320 to 327 x 240 to 245 pixels (smaller ones divide by 0 in both; even these take some 6-9 million steps), 16 or 32 bits
    (now and then 24, 8 or 0: nothing done) of random bytes (many extremes), masks of the usual layouts or odd ones (gaps, more than 8 bits),
    the pitch the row's bytes or more (odd too); the image's old pixels or none; the heap failing one of the four blocks now and then (the
    image's: written through null in both). dealt: the count and masks dealt from LAYOUTS, else drawn."""
    w, h = r.choice([320, 320, 321, 327]), r.choice([240, 240, 241, 245])
    bits, masks = r.deal(LAYOUTS) if dealt else r.choice(LAYOUTS)
    pitch = w * max(bits, 8) // 8 + r.choice([0, 0, 1, 2, 6, 64])
    if masks == 'odd' or masks is None:
        masks = [r.randrange(1, 1 << 32) for _ in range(3)]
    desc = bytearray(r.bytes(0x7c))
    put(desc, 8, 'IIII', h, w, pitch, r.getrandbits(32))
    put(desc, 0x24, 'I', SURFACE)
    put(desc, 0x54, 'IIII', bits, *masks)
    data = bytearray(r.bytes(pitch * h + 0x40))
    for _ in range(len(data) // 8):
        data[r.randrange(len(data))] = r.choice([0, 0xff, 0x80, 0xf0, 0x0f, 1])
    img = put(bytearray(r.bytes(0x10)), 0xc, 'I', r.choice([0, 0, RAM + 0x800]))
    fail = r.choice([0] * 16 + [1, 2, 3, 4])
    regions = {RAM: img, SURFACE: data, HEAP: heap(0x60000, fail=fail)}
    return Call(0x5d3ac0, [RAM] + list(struct.unpack('<31I', desc)), regions, conv='t', ret='b', steps=12_000_000)


def blocks(r):
    """0x5d48a0: surface()'s case on an object whose image is at +0x60, the description at RAM + 0x200; three in four with 24, 8, 0 or 15
    bits (the set up alone: the blocks, the counts, the result)."""
    c = surface(r, dealt=False)
    desc = list(c.args[1:])
    if r.random() < 0.75:
        desc[0x54 // 4] = r.choice([24, 8, 0, 15])
    obj = bytearray(r.bytes(0x60)) + c.regions.pop(RAM)
    c.regions[RAM] = obj
    c.regions[RAM + 0x200] = struct.pack('<31I', *desc)
    return Call(0x5d48a0, [RAM, RAM + 0x200], c.regions, conv='t', ret='b', steps=12_000_000)


def size(r, n):
    return r.choice([1, n, n, max(1, n // 2), max(1, n // 3), r.randrange(1, n + 1), r.randrange(1, n + 1)] + ([n + 1] if r.random() < 0.05 else []))


def case(r):
    op = r.deal(DECK)
    if op == 'Surface':
        return surface(r)
    if op == 'Blocks':
        return blocks(r)
    w, h = r.choice([1, 2, 3, 4, 8, r.randrange(1, 41)]), r.choice([1, 2, 3, 5, r.randrange(1, 31)])
    img = put(bytearray(r.bytes(0x10)), 0, 'II', w, h)
    put(img, 0xc, 'I', PIXELS)
    data = bytearray(r.bytes(w * h * 4 + 0x40))
    for i in range(len(data)):
        if r.random() < 0.3:
            data[i] = r.choice([0, 0xff, 0x80, 0xf0, 0x0f, 1])
    nw, nh = size(r, w), size(r, h)
    fmt = r.choice([1, 1, 2, 2, 0, 3])
    fail = r.choice([0] * 12 + [1, 2, 3])
    regions = {PIXELS: data, DEST: r.bytes(nw * nh * 4 + 0x20), HEAP: heap(fail=fail)}
    if op == 'ToImage':
        dst = put(bytearray(r.bytes(0x10)), 0, 'II', nw if r.random() < 0.95 else r.choice([0, 0xffffffff]), nh)
        put(dst, 0xc, 'I', DEST)
        regions[RAM] = img
        regions[RAM + 0x100] = dst
        return Call(0x5e80d0, [RAM, RAM + 0x100], regions, conv='t', ret='b')
    if op == 'Panel':
        panel = bytearray(r.bytes(0x80))
        for k in range(7):   # (the six images the reset blends: empty)
            put(panel, 0x10 * k, 'IIII', 0, r.choice([0, 1, 2]), 4, PIXELS)
        if r.random() < 0.15:
            put(img, 0xc, 'I', 0)
            if r.random() < 0.5:
                put(img, 0, 'I', 0)
        panel[0x50:0x60] = img
        regions[RAM] = panel
        return Call(0x5d4970, [RAM, DEST, nw, nh, fmt], regions, conv='t', ret='b')
    regions[RAM] = img
    return Call(0x5d4260, [RAM, DEST, nw, nh, fmt], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
