# A frame's sprites by group (src/game/render/spritegroups: 0x57c620) against the original, with Texture_FormatBits, Mat4_Transform and Quad_TurnXY running as they are. Batch_DrawQuad is a recorder (k 1; its spec compared
# with its points 0x38 on, not dword 1, a stack address, 5, unset, or 11 to 13, not the spec's); the object's list (k 2), release (k 3) and count (k 4) are cdecl recorders. Their answers go in turn.
# usage: spritegroups.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fb, log_region, put, recorder, set_return  # noqa: E402

B = Bench([(0x57c620, 0x57c9e1), (0x590c40, 0x590ce8), (0x578d20, 0x579004), (0x591430, 0x591510)], hooks={0x565f90: _recorder(1, 2)})
OBJECT, QUADS, MATRIX, HOLDER, HOLDER2, EMPTY, TEX, QANSWERS, LISTS, COUNTS = (RAM + 0x100 * k for k in range(10))
SPRITES = RAM + 0x1000
SINES = 0x8c63d8
SPEC = 0x3fffc7dd & 0xffffffff
NANS = [0x7fc12345, 0x7f812345]


def place(r, lo, hi, nan=0.0):
    return r.choice(NANS) if r.random() < nan else fb(r.uniform(lo, hi))


def case(r):
    log = log_region()
    nan = r.choice([0, 0, 0, 0.05])
    frame = r.getrandbits(32), r.getrandbits(32)
    obj = bytearray(r.bytes(0xa0))
    same = r.deal([0] * 7 + [1, 2, 3])
    put(obj, 4, 'II', frame[0] if same in (1, 2) else frame[0] ^ 1, frame[1] if same in (1, 3) else frame[1] ^ 4)
    put(obj, 0x94, 'III', recorder(2, 1, cdecl=True), recorder(3, 1, cdecl=True), recorder(4, 1, cdecl=True))
    for k in range(0x78):
        obj[0xc + k] = r.getrandbits(8) & ~0x40 & 0xff
    # Groups: most empty; the count's answers go by group in turn, so that they are set for those at the front.
    groups = r.deal([1, 1, 2, 3])
    chosen = r.sample(range(0x78), groups)
    counts = [0] * 120
    lists = []
    total = 0
    for g in chosen:
        n = r.deal([1, 1, 2, 3, 5, -1, 0])
        counts[g] = n
        if r.random() < 0.8:
            obj[0xc + g] |= 0x40
        if r.random() < 0.3:
            obj[0xc + r.randrange(0x78)] |= 0x40
        if n > 0 and obj[0xc + g] & 0x40:
            lists.append(0 if r.random() < 0.1 else SPRITES + total * 0x1c)
            total += max(n, 0)
    sprites = bytearray(r.bytes(max(total, 1) * 0x1c))
    for k in range(total):
        at = k * 0x1c
        put(sprites, at, 'iii', r.randrange(-1 << 18, 1 << 18), r.randrange(-1 << 18, 1 << 18), r.randrange(-1 << 18, 1 << 18))
        put(sprites, at + 0xc, 'I', r.deal([0, 0x80000000, 0xef000000, 0xf0000000, 0xff000000]) | r.getrandbits(24))
        put(sprites, at + 0x10, 'I', r.deal([0xffffffff, HOLDER, HOLDER2]))
        put(sprites, at + 0x14, 'HH', r.deal([0, 1, 1024, 65535]) if r.random() < 0.3 else r.getrandbits(16), r.deal([0, 0, 1, 0x8000, 0xffff]) if r.random() < 0.3 else r.getrandbits(16))
        flags = r.getrandbits(32)
        put(sprites, at + 0x18, 'I', flags)
    tex = bytearray(r.bytes(0x40))
    put(tex, 8, 'I', r.getrandbits(32) & ~0x101000 | r.deal([0, 0x1000]) | r.deal([0, 0, 0x100000]))
    put(tex, 0x20, 'IIII', place(r, 0, 2, nan), place(r, 0, 2, nan), place(r, -1, 1, nan), place(r, -1, 1, nan))

    quads = [1] * 64
    if r.random() < 0.2:
        quads[r.randrange(0, 6)] = 0
    answers(log, 1, QANSWERS)
    answers(log, 2, LISTS)
    answers(log, 4, COUNTS)
    blob(log, 1, 0, SPEC)
    set_return(log, 3, r.getrandbits(32))
    regions = {LOG: log, OBJECT: obj, QUADS: r.bytes(0x40), EMPTY: bytes(4), HOLDER: struct.pack('<II', TEX, r.getrandbits(32)) + struct.pack('<I', place(r, 0, 3, nan)), HOLDER2: bytes(4) + r.bytes(4) + struct.pack('<I', place(r, 0, 3, nan)),
               TEX: tex, QANSWERS: struct.pack('<64I', *quads), LISTS: struct.pack('<40I', *(lists + [0] * (40 - len(lists)))), COUNTS: struct.pack('<120i', *counts), SPRITES: sprites,
               MATRIX: b''.join(struct.pack('<I', place(r, -2, 2)) for _ in range(16)), 0x87b088: struct.pack('<I', MATRIX), 0x879ab0: struct.pack('<I', QUADS),
               0x8bcc40: struct.pack('<II', *frame), 0x8bcbcc: struct.pack('<I', place(r, 0, 2, nan)), 0x8bd524: struct.pack('<I', r.deal([4, 1, 0, 2])),
               SINES: b''.join(struct.pack('<f', r.uniform(-1, 1)) for _ in range(256))}
    return Call(0x57c620, [OBJECT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
