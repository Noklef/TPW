# The park map's sprites made ready (src/game/world/parkmapwindow: 0x5f3710) against the original: the thing walk, the array's and the sprites' calls recorders.
# usage: parkmapprepare.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, fill, heap, log_region, put, set_return, strings  # noqa: E402

FIRST, NEXT, INIT, ADD, IMAGE, SPRITES = 8, 9, 10, 11, 12, 13
B = Bench([(0x5f3710, 0x5f38d7), (0x5da3c0, 0x5da3c1)],
          hooks={0x516130: _recorder(FIRST, 1), 0x516160: _recorder(NEXT, 2), 0x56b0c0: _recorder(INIT, 2), 0x56b2f0: _recorder(ADD, 0), 0x56b380: _recorder(IMAGE, 1), 0x5f3940: _recorder(SPRITES, 0)})
MAP, THINGS, WORDS, ANSWERS, IMAGES, WORLD = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x2100, RAM + 0x2200, RAM + 0x10000
TAIL = MAP + 0x20060


def case(r):
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: bytes(0x600), HEAP: heap(0x1000, r.choice([0, 0, 0, 1]))}
    # Things 1 to 6, kinds of all sorts; a walk over some of them through the recorders, a chain through +0xc from the world.
    things = bytearray(r.bytes(0x80 * 7))
    slots = bytearray(20 * 7)
    for i in range(1, 7):
        put(slots, 20 * i, 'I', THINGS + 0x80 * i)
        put(things, 0x80 * i + 2, 'B', r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8]))
        put(things, 0x80 * i + 0x32, 'B', r.choice([0, 0x10, 0xef, 0xff]))
    walk = r.sample(range(1, 7), r.randrange(5))
    fill(log, FIRST, 0, struct.pack('<H', walk[0] if walk else 0))
    strings(log, FIRST, 0, 1)
    strings(log, NEXT, 0, 1)
    words = struct.pack('<%dH' % (len(walk) + 1), *(walk + [0])[1:], 0)
    regions[WORDS] = words
    next_table = struct.pack('<%dI' % 40, *[WORDS + 2 * min(i, len(walk)) for i in range(40)])
    regions[ANSWERS] = next_table
    answers(log, NEXT, ANSWERS)
    chain = r.sample(range(1, 7), r.randrange(4))
    for here, then in zip(chain, chain[1:] + [0]):
        put(things, 0x80 * here + 0xc, 'H', then)
    regions[THINGS] = things
    regions[0x7cfb90] = slots
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    regions[WORLD] = r.bytes(0x5000) if r.random() < 0.5 else bytes(0x5000)
    regions[WORLD + 0x1da746] = struct.pack('<H', chain[0] if chain else 0)
    regions[0xf86d48] = r.bytes(0x78)
    regions[IMAGES] = struct.pack('<40I', *[r.getrandbits(32) for _ in range(40)])
    answers(log, IMAGE, IMAGES)
    head = bytearray(r.bytes(0x60))
    put(head, 0x24, '2i', r.choice([0, 1, 2, 3, -1]), r.choice([0, 1, 2, 3, -1]))
    regions[MAP] = head
    regions[TAIL] = r.bytes(0x30)
    set_return(log, INIT, r.getrandbits(32))
    return Call(0x5f3710, [MAP], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
