# A mesh's textures told to the texture cache (src/game/render/meshtextures: 0x57a420) against the original, the calls recorders: records empty or without a set, the texture found or not, by the word or 0, kept or not, unlinked or not, the extra texture of bit 0x2000000, the level over 0, 0, under 0 or NaN.
# usage: meshtextures.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fb, log_region, put  # noqa: E402

TEXTURE, KEEP, UNLINK, TOUCH = 1, 2, 3, 4
B = Bench([(0x57a420, 0x57a52e)], hooks={0x575f90: _recorder(TEXTURE, 1), 0x590f90: _recorder(KEEP, 0), 0x58dc50: _recorder(UNLINK, 1), 0x58c9f0: _recorder(TOUCH, 2)})
MESH, LIST, RECORDS, HOLDER, SCENE, TEXTURES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x380
OPS = ['plain', 'plain', 'none', 'kept', 'extra', 'no holder', 'no extra set', 'no textures']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    count = 0 if op == 'none' else r.randrange(1, 9)
    records = bytearray(r.bytes(0x10 * 8))
    pointers = bytearray(r.bytes(0x10 * 8))
    for i in range(8):
        put(records, 0x10 * i, 'H', r.choice([0, 0x80, 0x400, 0x480, 0x480, 0xffff, r.getrandbits(16)]))
        put(records, 0x10 * i + 4, 'I', r.choice([0, RAM + 0x4000 + i * 16, RAM + 0x5000 + i * 16, RAM + 0x6000 + i * 16]))
        put(pointers, 0x10 * i, 'I', r.choice([0, RECORDS + 0x10 * i, RECORDS + 0x10 * i, RECORDS + 0x10 * i]))
    answers(log, TEXTURE, TEXTURES)
    textures = [0 if op == 'no textures' else r.choice([0, RAM + 0x7000, RAM + 0x7100, RAM + 0x7200]) for _ in range(40)]
    flags = r.getrandbits(32) & ~0x20c0000
    if op == 'kept':
        flags |= r.deal([0x40000, 0x80000, 0xc0000])
    if op in ('extra', 'no holder', 'no extra set'):
        flags |= 0x2000000
    level = r.deal([fb(r.uniform(0.01, 50)), 0, 0x80000000, fb(-1.5), 0x7fc00000, fb(r.uniform(0.01, 50))])
    mesh = bytearray(r.bytes(0xa0))
    put(mesh, 0, 'I', flags)
    put(mesh, 0x5a, 'H', count)
    put(mesh, 0x6c, 'I', LIST)
    put(mesh, 0x98, 'I', 0 if op == 'no holder' else HOLDER)
    regions = {LOG: log, MESH: mesh, LIST: pointers, RECORDS: records, HOLDER: put(bytearray(r.bytes(8)), 4, 'I', 0 if op == 'no extra set' else RAM + 0x8000), SCENE: put(bytearray(r.bytes(8)), 4, 'I', level), TEXTURES: struct.pack('<40I', *textures),
               0x879a20: struct.pack('<I', SCENE), 0x8bd550: struct.pack('<I', r.deal([0, 0x40, 0xffffffbf, 0xffffffff])), 0x8c4b44: r.bytes(4)}
    return Call(0x57a420, [MESH], regions, conv='s', ret='v')


if __name__ == '__main__':
    B.main(case)
