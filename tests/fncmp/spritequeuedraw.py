# The sprite queue's drawing (src/game/render/spritequeue: 0x58a770 SpriteQueue_Draw) against the original: recorders for the frames' draws (k 1 SpriteFrame_DrawQuad, 2 SpriteFrame_DrawLightQuads, 3 SpriteFrame_DrawQuadInPlace, 4 SpriteFrame_DrawLightQuadsInPlace) and
# k 5 Batch_Close, answering 0 now and then; the sort shows in the calls' order and the frames' +0x40. Texture_FormatBits' answer is dropped, so it is a hook without a log.
# usage: spritequeuedraw.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put  # noqa: E402

B = Bench([(0x58a770, 0x58ab60), (0x67a400, 0x690000)],
          hooks={0x589f90: _recorder(1, 0), 0x589770: _recorder(2, 0), 0x589fd0: _recorder(3, 0), 0x589990: _recorder(4, 1), 0x5658c0: _recorder(5, 1), 0x590c40: lambda e: None})
QUEUE, FRAMES, TEXTURES, CELLS, TABLES, BATCH = RAM, RAM + 0x3000, RAM + 0x4000, RAM + 0x4800, RAM + 0x5000, RAM + 0x6000
FRAME = 0x44
DEPTHS = [struct.unpack('<I', struct.pack('<f', f))[0] for f in (0.0, 0.5, 1.0, 2.0, -1.0, 100.0)] + [0x7fc00000]
BITS = (4, 0x2000, 0x200000, 0x20000, 0x4000, 0x10000)


def case(r):
    log = log_region()
    for k in range(1, 6):
        answers(log, k, TABLES + k * 0x100)
    fails = r.choice([0, 0, 0, 0.05, 0.2])
    tables = bytearray(0x600)
    for i in range(0x180):
        put(tables, i * 4, 'I', 0 if r.random() < fails else r.choice([1, 1, 2, 0x80000000]))

    count = r.choice([0, 1, 2, 3, 5, 8, 12])
    values = [r.choice([0, 1, 2, 7, 0x80000000]) for _ in range(3)] + [0xffffffff if r.random() < 0.1 else r.getrandbits(32)]
    textures, cells = bytearray(r.bytes(0x100)), bytearray(0x10)
    for i in range(4):
        put(cells, i * 4, 'I', TEXTURES + i * 0x30)
        put(textures, i * 0x30 + 8, 'I', r.getrandbits(32) & ~0x1000 | (0x1000 if r.random() < 0.8 else 0))
        put(textures, i * 0x30 + 0x1c, 'I', values[i])

    queue = bytearray(0x2410)
    put(queue, 0x2400, 'I', r.choice([0, TEXTURES, r.getrandbits(32)]))
    put(queue, 0x2404, 'I', count)
    frames = bytearray(r.bytes(FRAME * 12))
    few = r.random() < 0.5
    for i in range(count):
        put(queue, i * 4, 'I', FRAMES + i * FRAME)
        flags = r.getrandbits(32)
        for bit in BITS:
            flags = flags & ~bit | (bit if r.random() < (0.1 if few else 0.4) else 0)
        put(frames, i * FRAME + 0x10, 'I', flags)
        put(frames, i * FRAME + 8, 'I', r.choice(DEPTHS))
        put(frames, i * FRAME + 0x28, 'I', CELLS + r.randrange(2 if few else 4) * 4)

    render = r.getrandbits(32) & ~0x60080
    render |= 0x80 if r.random() < 0.2 else 0
    render |= r.choice([0, 0x20000, 0x40000, 0x40000, 0x60000])
    regions = {LOG: log, QUEUE: queue, FRAMES: frames, TEXTURES: textures, CELLS: cells, TABLES: tables, BATCH: r.bytes(0x40),
               0x879ab0: struct.pack('<I', BATCH), 0x8bcbc8: struct.pack('<I', render), 0x8bd4fc: r.bytes(4)}
    return Call(0x58a770, [QUEUE], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
