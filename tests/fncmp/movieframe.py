# The video library's frame buffers and block frames (src/game/gfx/movieframe: MovieFrame_Allocate 0x6777d0, MovieFrame_Decode 0x677310) against the original. Allocate: the settings' sizes small, the old buffers set or not, the heap failing one of its four allocations now and then, RefPack_DecodeIf a recorder (cdecl, k 10). Decode: the
# heap as fncmp's; UnpackFields, ExpandBlocks, UnpackIndices and DrawBlocks run as they are, the settings small multiples of 4, the row table's dwords small, the frames padded each way for the cells' offsets and the indices' reach (of up to 8 bits; below the cells the offsets, above them the blocks' bytes in the
# padded heap or the padded old block buffer). The word 0 of bits leaves the index buffer as it was: small random dwords.
# usage: movieframe.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Allocate,Decode)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _alloc, _free, _recorder, heap, log_region, put  # noqa: E402

CALLS = {'Allocate': 0x6777d0, 'Decode': 0x677310}
B = Bench([(0x6777d0, 0x67792a), (0x677310, 0x67763c), (0x677640, 0x6776b8), (0x6776c0, 0x6777ce), (0x67862f, 0x6786d8), (0x6786d8, 0x67878c)],
          hooks={0x67dcd0: _alloc, 0x67ad90: _free, 0x67850c: _recorder(10, 3, cdecl=True)})
PLAYER, SETTINGS, DATA, TABLE, A, BB, INDICES, OLD65C, OLD660, OLD658 = RAM, RAM + 0x800, RAM + 0x1000, RAM + 0x4000, RAM + 0x8000, RAM + 0xa000, RAM + 0xc000, RAM + 0xd000, RAM + 0xe000, RAM + 0xf000
OLD = [RAM + 0x10000 + 0x40 * i for i in range(4)]


def allocate_case(r):
    log = log_region()
    player = bytearray(r.bytes(0x680))
    put(player, 0, 'I', SETTINGS)
    put(player, 0x62c, 'I', r.choice([0, 1, 4, r.randrange(0x400)]))
    for i, offset in enumerate((0x648, 0x64c, 0x654, 0x650)):
        put(player, offset, 'I', OLD[i] if r.random() < 0.7 else 0)
    settings = bytearray(r.bytes(0x80))
    a, b = r.choice([0, 4, 8, 16, 32, r.randrange(0x40)]), r.choice([0, 4, 8, 16, 32, r.randrange(0x40)])
    put(settings, 0x60, 'II', a, b)
    regions = {LOG: log, PLAYER: player, SETTINGS: settings, HEAP: heap(0x8000, r.choice([0] * 8 + [1, 2, 3, 4]))}
    return Call(CALLS['Allocate'], [PLAYER, DATA], regions, conv='f', ret='a')


def decode_case(r):
    player = bytearray(r.bytes(0x680))
    put(player, 0, 'I', SETTINGS)
    settings = bytearray(r.bytes(0x80))
    a, b = r.choice([4, 8, 8, 16, 24]), r.choice([4, 8, 8, 16, 24])
    put(settings, 0x60, 'II', a, b)
    cells, plain, expanded = r.choice([0, 1, 2, 3, 8, r.randrange(0x40)]), r.choice([0, 1, 2, 8, r.randrange(0x20)]), r.choice([0, 1, 2, 8, r.randrange(0x20)])
    bits = r.choice([0, 1, 2, 3, 4, 5, 6, 8])
    data = bytearray(r.bytes(0x1000))
    put(data, 4, 'HHHH', cells, plain, expanded, bits)
    capacity = r.choice([0, cells, cells + 1, max(cells - 1, 0), 0x40])
    blocks = plain + expanded
    room = r.choice([0, blocks, blocks + 1, max(blocks - 1, 0), 0x40])
    put(player, 0x640, 'II', capacity, room)
    put(player, 0x648, 'III', A, BB, INDICES)
    put(player, 0x654, 'I', TABLE + 0x800)
    put(player, 0x658, 'III', OLD658 if r.random() < 0.7 else 0, OLD65C if r.random() < 0.7 else 0, OLD660 if r.random() < 0.7 else 0)
    regions = {PLAYER: player, SETTINGS: settings, DATA: data, HEAP: heap(0x8000, r.choice([0] * 12 + [1, 2, 3])),
               TABLE: struct.pack('<%di' % 0x440, *[4 * r.randrange(-0x40, 0x41) for _ in range(0x440)]),
               A - 0x1000: r.bytes(0x2000), BB - 0x1000: r.bytes(0x2000),
               INDICES: struct.pack('<64I', *[r.randrange(0x20) for _ in range(64)]),
               OLD65C: r.bytes(0x200), OLD660: r.bytes(0x100), OLD658: r.bytes(0x1000)}
    return Call(CALLS['Decode'], [PLAYER, DATA + 4], regions, conv='f', ret='a')


def case(r):
    return allocate_case(r) if r.deal(OPS) == 'Allocate' else decode_case(r)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
