# The in-memory stream source (src/game/files/memsource) against the originals: sources of 0-4 chunks (sizes 1-0x40, built by the case; the bench's Mem_Free
# only records what it is given), their size anywhere in the last chunk, the position and its chunk / cursor consistent or (now and then) at a chunk's end
# with no chunk current, as the originals leave them; reads and writes of 0 to past the end, seeks from each origin (and an unknown one) to before the
# start, inside, onto chunk ends and past the size. CreateSemaphoreA / CloseHandle are import-slot recorders; chunks come from the bench heap.
# usage: memsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Write,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, heap, import_table, log_region, set_return  # noqa: E402

B = Bench([(0x618c00, 0x619260), (0x5f8000, 0x5f9000), (0x6307c0, 0x630800), (0x405000, 0x406000), (0x67a400, 0x690000)])
THIS, RESULT, CHUNKS, OUT, DATA = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x1000, RAM + 0x1400
IMPORTS = {'CreateSemaphoreA': (40, 0x6fd16c, 4, 8, 0), 'CloseHandle': (41, 0x6fd13c, 1, 0, 0)}
VTABLE = 0x703868
# name: (address, stack arguments, returns)
OPS = {
    'Ctor': (0x618df0, 'z', 'a'), 'DeletingDtor': (0x618e60, 'f', 'a'), 'Dtor': (0x618e80, '', 'v'), 'Teardown': (0x618ed0, '', 'v'),
    'Read': (0x618ee0, 'On', 'a'), 'Write': (0x618fc0, 'Dn', 'a'), 'Seek': (0x6190d0, 'so', 'a'), 'Size': (0x619150, '', 'a'),
    'Tell': (0x619160, '', 'a'), 'AddChunk': (0x619170, 'R', 'a'), 'Clear': (0x619200, '', 'v'), 'GetLock': (0x619240, '', 'a'),
}


def case(r):
    op = r.choice(PICK)
    address, args, ret = OPS[op]
    size = r.choice([1, 3, 4, 7, 0x10, 0x40])
    count = r.choice([0, 1, 1, 2, 3, 4])
    chunks = bytearray(r.bytes(0xf00))
    at = [CHUNKS + i * 0x60 for i in range(count)]
    for i, c in enumerate(at):
        struct.pack_into('<5I', chunks, c - CHUNKS, at[i - 1] if i else 0, at[i + 1] if i + 1 < count else 0, c + 0x14, i * size, (i + 1) * size)
    total = (count - 1) * size + r.randrange(size + 1) if count else 0
    if r.random() < 0.1:
        total = r.choice([0, count * size, total + 1])
    position = r.randrange(total + 1) if r.random() < 0.9 else r.choice([0, total, count * size])
    chunk = next((c for i, c in enumerate(at) if (i + 1) * size > position), 0)
    if r.random() < 0.15 and position and position % size == 0:
        chunk = 0   # (a read to a chunk's end leaves none current)
    cursor = chunk + 0x14 + position - at.index(chunk) * size if chunk else 0
    if r.random() < 0.05:
        chunk, cursor = r.choice(at + [0]), 0
    this = bytearray(r.bytes(0x40))
    struct.pack_into('<3I7I', this, 0, VTABLE, r.choice([0, 0x80000000]), r.randrange(4), at[0] if at else 0, at[-1] if at else 0, chunk, cursor, position,
                     total, size)
    if r.random() < 0.05:   # (the tail's size field, now and then inconsistent with the chunks)
        struct.pack_into('<I', this, 0x24, r.choice([0, size + 1, 0x7fffffff]))
    log = log_region()
    set_return(log, 40, r.choice([0, 0x44, r.getrandbits(32)]))
    stack = []
    for x in args:
        if x == 'z':
            stack.append(r.choice([0, 1, 4, 0x10, 0x100, 0x1000]))
        elif x == 'f':
            stack.append(r.choice([0, 1, 2, 3]))
        elif x == 'O':
            stack.append(OUT)
        elif x == 'D':
            stack.append(DATA)
        elif x == 'n':
            stack.append(r.choice([0, 1, 2, size - 1, size, size + 1, total - position, total - position + 1, 3 * size, 0x100]) & 0xffffffff)
        elif x == 's':
            stack.append(r.choice([0, 1, -1, size, -size, total, -total, total - position, position, 0x7fffffff, -0x80000000, r.randrange(-100, 100)]) & 0xffffffff)
        elif x == 'o':
            stack.append(r.choice([0, 1, 2, 0, 1, 2, 3, 0xffffffff]))
        elif x == 'R':
            stack.append(RESULT)
    regions = {HEAP: heap(0x4000), THIS: this, RESULT: r.bytes(0x10), CHUNKS: chunks, OUT: r.bytes(0x400), DATA: r.bytes(0x400), LOG: log,
               IMPORT_TABLE: import_table(log, IMPORTS)}
    return Call(address, [THIS] + stack, regions, conv='t', ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
