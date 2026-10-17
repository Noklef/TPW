# The buffered source (src/game/files/bufsource) against the originals: a BufferedSource at THIS whose own vtable is recorders (+0x10, the size Seek asks
# for, returns a chosen value) and whose wrapped source (LINK, often unset) is recorders too: +4 read (filling the buffer it is given with up to 0x70
# bytes), +8 write, +0xc seek (returning the position asked for, or a miss), +0x10 size, +0x14 tell, +0x28 the attach hand-over. The window, position
# and wrapped position are consistent (inside the window, at its ends, just outside it) or now and then not; the buffer (BUF) holds the window's bytes.
# Mem_Alloc / Mem_Free are the bench heap's (its next block 0 now and then: an allocation that fails).
# usage: bufsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Read,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, HEAP, RAM, fill, heap, log_region, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x638ef0, 0x639230), (0x618cd0, 0x618df0), (0x67a400, 0x690000)])
THIS, RESULT, LINK, SOURCE = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xc0
VT_THIS, VT_LINK = RAM + 0x100, RAM + 0x180
BUF, OUT, DATA = RAM + 0x400, RAM + 0x800, RAM + 0xc00
# The wrapped source's slots: 1 read, 2 write, 3 seek (2 stack arguments), 4 size, 5 tell, 10 the hand-over (1); the source's own size is slot 20.
K_READ, K_WRITE, K_SEEK, K_SIZE, K_TELL, K_OWN_SIZE = 1, 2, 3, 4, 5, 20
# name: (address, stack arguments, returns)
OPS = {
    'Ctor': (0x638ef0, '', 'a'), 'Open': (0x638f20, 'Rc', 'a'), 'Free': (0x638f70, '', 'v'), 'Read': (0x638f80, 'On', 'a'),
    'Seek': (0x639000, 'so', 'a'), 'Write': (0x639050, 'Dn', 'a'), 'Tell': (0x6390f0, '', 'a'), 'Attach': (0x639110, 'L', 'a'),
    'ReadDirect': (0x639130, 'On', 'a'), 'CopyWindow': (0x639150, 'On', 'a'), 'Fill': (0x6391b0, '', 'a'), 'ReadRaw': (0x6391e0, 'On', 'a'),
}


def case(r):
    op = r.choice(PICK)
    address, args, ret = OPS[op]
    capacity = r.choice([1, 4, 0x10, 0x40, 0x100, 0x100])
    start = r.choice([0, 0, 1, 0x10, 0x100, 0x1000, r.getrandbits(32)])
    length = r.choice([0, 1, capacity, capacity, r.randrange(capacity + 1)])
    end = (start + length) & 0xffffffff
    offset = r.choice([0, 0, 1, 0x40])   # (the window's start in the buffer, after a write moved it)
    position = r.choice([start, end, (end - 1) & 0xffffffff, (start - 1) & 0xffffffff, end + 1, (start + r.randrange(length + 1)) & 0xffffffff,
                         0, r.getrandbits(32)]) & 0xffffffff
    source_pos = position if r.random() < 0.4 else r.choice([end, start, (position + 1) & 0xffffffff, (position - 1) & 0xffffffff, 0, r.getrandbits(32)])
    link = 0 if r.random() < 0.12 else LINK
    this = bytearray(r.bytes(0x40))
    struct.pack_into('<3I7I', this, 0, VT_THIS, link, r.randrange(4), BUF, BUF + offset, position, start, end, source_pos, capacity)
    log = log_region()
    count = r.choice([0, 1, 2, 3, capacity - 1, capacity, capacity + 1, length, (end - position) & 0xffffffff, 0x80, 0x200]) & 0xffffffff
    if count > 0x200:
        count = r.choice([0x200, 0xffffffff, 0x80000000])
    # The wrapped seek hits (the position it is asked for: where the copy ends, or the position) or misses.
    set_return(log, K_SEEK, r.choice([position, position, end, (position + count) & 0xffffffff, (position + 1) & 0xffffffff, 0, 0xffffffff]))
    set_return(log, K_READ, r.choice([0, 1, capacity, capacity, min(count, 0x200), r.randrange(capacity + 1), min(count, 0x40)]))
    set_return(log, K_WRITE, r.choice([0, 1, count, count, length, r.randrange(0x100)]))
    size = r.choice([0, position, end, (position + 1) & 0xffffffff, 0x100, 0x1000, 0x7fffffff, 0xffffffff])
    set_return(log, K_SIZE, size)
    set_return(log, K_OWN_SIZE, size if r.random() < 0.8 else r.choice([0, 0x100, 0xffffffff]))
    set_return(log, K_TELL, r.choice([0, 1, position, 0x100, 0xffffffff]))
    fill(log, K_READ, 0, r.bytes(r.choice([0, 1, 0x10, 0x70])))
    vt_this = bytearray(r.bytes(0x80))
    struct.pack_into('<I', vt_this, 0x10, recorder(K_OWN_SIZE, 0))
    vt_link = vtable({K_READ: 2, K_WRITE: 2, K_SEEK: 2, 10: 1})
    # The source Attach is handed: one with a recorder vtable (often LINK itself), or none.
    source = bytearray(struct.pack('<3I', VT_LINK, r.choice([0, LINK]), 1))
    heap_region = heap(0x4000)
    if r.random() < 0.1:
        struct.pack_into('<I', heap_region, 0, 0)   # (the next allocation answers 0)
    stack = []
    for x in args:
        if x == 'R':
            stack.append(RESULT)
        elif x == 'c':
            stack.append(r.choice([0, 1, 0x10, 0x100, 0x1000]))
        elif x == 'O':
            stack.append(OUT)
        elif x == 'D':
            stack.append(DATA)
        elif x == 'n':
            stack.append(count)
        elif x == 's':
            stack.append(r.choice([0, 1, -1, capacity, -length, position, -position, size, size + 1, 0x7fffffff, -0x80000000, r.randrange(-0x100, 0x100)])
                         & 0xffffffff)
        elif x == 'o':
            stack.append(r.choice([0, 1, 2, 0, 1, 2, 3, 0xffffffff]))
        elif x == 'L':
            stack.append(r.choice([SOURCE, SOURCE, LINK, 0]))
    link_obj = bytearray(struct.pack('<3I', VT_LINK, 0, 1))
    regions = {HEAP: heap_region, THIS: this, RESULT: r.bytes(0x10), LINK: link_obj + r.bytes(0x34), SOURCE: source + r.bytes(0x34),
               VT_THIS: vt_this, VT_LINK: vt_link, BUF: r.bytes(0x400), OUT: r.bytes(0x400), DATA: r.bytes(0x400), LOG: log}
    return Call(address, [THIS] + stack, regions, conv='t', ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
