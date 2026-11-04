# A list of models' groups put in drawing order and drawn (src/game/render/modellist: 0x581db0) against the original: the calls recorders answering in turn from tables (the textures among a few with keys, the orders among a few, a batch or null, an add or close that fails now and then); models with 0-3 groups, null materials and sets.
# usage: modellist.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, put, strings  # noqa: E402

TEXTURE, ORDER, BEGIN, ADD, CLOSE = 1, 2, 3, 4, 5
B = Bench([(0x581db0, 0x582170), (0x67b110, 0x67b13f)],
          hooks={0x575f90: _recorder(TEXTURE, 1), 0x581790: _recorder(ORDER, 2), 0x581880: _recorder(BEGIN, 2), 0x580530: _recorder(ADD, 3), 0x5658c0: _recorder(CLOSE, 1)})
LIST, RENDERER, TEXTURES, MODELS, ARRAYS, MATERIALS, TABLES = (RAM + 0x400 * k for k in range(7))


def table(r, choices):
    return struct.pack('<40I', *[r.choice(choices) for _ in range(40)])


def case(r):
    log = log_region()
    count = r.choice([0, 1, 1, 2, 3, 4, 6])
    entries = bytearray(struct.pack('<I', count) + r.bytes(0x14 * 6))
    models = bytearray(r.bytes(0x70 * 6))
    arrays = bytearray(r.bytes(0x10 * 4 * 6))
    materials = bytearray(r.bytes(8 * 8))
    for i in range(8):
        put(materials, i * 8 + 2, 'H', r.choice([0, 1, 7]))
        put(materials, i * 8 + 4, 'I', r.choice([0, 0x1000 + i, 0x2000 + i, 0x3000 + i]))
    for i in range(6):
        put(entries, 4 + i * 0x14 + 4, 'I', MODELS + i * 0x70)
        put(entries, 4 + i * 0x14 + 8, 'I', r.getrandbits(32) & ~0x2000000 | r.choice([0, 0, 0, 0x2000000]))
        groups = r.choice([0, 1, 2, 3])
        put(models, i * 0x70 + 0x5a, 'H', groups)
        put(models, i * 0x70 + 0x6c, 'I', ARRAYS + i * 0x40)
        for g in range(4):
            put(arrays, i * 0x40 + g * 0x10, 'I', r.choice([0] + [MATERIALS + k * 8 for k in range(8)]))
    textures = bytearray(r.bytes(0x20 * 4))
    for i in range(4):
        put(textures, i * 0x20 + 8, 'I', r.getrandbits(32) & ~0x1000 | r.choice([0, 0x1000, 0x1000]))
        put(textures, i * 0x20 + 0x1c, 'I', r.choice([0, 1, 2, 0xfffffff0]))
    tables = (table(r, [TEXTURES + i * 0x20 for i in range(4)] + [0]) + table(r, [0, 1, 2, 3, 3, 0xfffffff0, 0x1256])
              + table(r, [0x77777] * 15 + [0]) + table(r, [1] * 15 + [0]) + table(r, [1] * 15 + [0]))
    for i, k in enumerate((TEXTURE, ORDER, BEGIN, ADD, CLOSE)):
        answers(log, k, TABLES + 0xa0 * i)
    strings(log, BEGIN, 0, out=1 << 1)   # (the size's a stack address)
    fill(log, BEGIN, 1, struct.pack('<4I', 3, 5, 0, 0x100), records=4)
    regions = {LOG: log, LIST: entries, RENDERER: put(bytearray(r.bytes(0x10)), 0xc, 'I', r.choice([0, 1, 3, 0xffffffff])), TEXTURES: textures, MODELS: models, ARRAYS: arrays, MATERIALS: materials, TABLES: tables,
               0x879ab0: struct.pack('<I', RENDERER), 0x8bd550: r.bytes(1)}
    return Call(0x581db0, [LIST], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
