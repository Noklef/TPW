# libjpeg 6a's jmemmgr.c, the memory manager (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a, the module through its
# wrapper unit third_party/wrap/jpeg-6a/jmemmgr.c) against the exe's 6a copies; jpeg6b_memmgr.py at 6a's addresses and layout. cinfo
# at CINFO (err +0 at ERR: error_exit a cdecl recorder, slot 0; mem +4 at MEM: my_memory_mgr, 0x50 bytes: 6a's jpeg_memory_mgr has no
# max_alloc_chunk, so small_list[2] is at +0x30, large_list[2] +0x38, virt_sarray_list +0x40, virt_barray_list +0x44,
# total_space_allocated +0x48, last_rowsperchunk +0x4c). Pool blocks (a 16-byte header: next, bytes_used, bytes_left) are laid out in
# POOLS and chained from small_list / large_list; new ones come from jpeg_get_small / large (0x650210, 0x650232, the library's
# jmemansi.c on the exe's malloc), whose malloc (0x67dcd0) and free (0x67ad90) are fncmp's heap in both, as in jpeg6a_memansi.py.
# Virtual arrays' control blocks (0x80 bytes, 6a's int booleans: mem_buffer, rows_in_array +4, samples / blocksperrow +8,
# maxaccess +0xc, rows_in_mem +0x10, rowsperchunk +0x14, cur_start_row +0x18, first_undef_row +0x1c, pre_zero +0x20, dirty +0x24,
# b_s_open +0x28, next +0x2c, b_s_info +0x30: read, write and close_backing_store cdecl recorders, slots 3, 4, 5) in VIRT, their row
# pointers in ROWS into SAMPLES.
# What the module calls outside the library is the exe's in the game (src/game/lib/jpeg6a.h's Jpeg6aCrtEnv): getenv (0x67fbf0),
# sscanf (0x67f620) and 6a's jpeg_open_backing_store (0x650265) are cdecl recorders 24, 25 and 26 in both (the driver's env, these
# hooks). The getenv name and the sscanf strings are logged as string hashes (the library's literals are not the exe's), sscanf's out
# arguments as 0 (the stack) and filled: the value and the character it would scan.
# Edges dealt with care, as the source would meet them (jpeg6b_memmgr.py's): maxaccess 0 divides by zero (realize_virt_arrays);
# rowsperchunk 0 with rows to move loops forever (do_sarray_io), so does alloc_sarray's JERR_WIDTH_OVERFLOW (rowsperchunk 0) with rows
# to allocate: the overflow is dealt with numrows 0; alloc_small's pool ids past 1 read first_pool_slop / extra_pool_slop past their
# ends (the exe's neighbours and the library's differ), so a bad pool id there (and in request_virt_sarray / barray, which go on to
# alloc_small of it) is dealt only where the pool it reads (large_list[0] for pool 2) has room. jpeg_mem_init and jpeg_mem_term are
# the exe's hand ports in the exe (10000000, as the library's DEFAULT_MAX_MEM gives) and the library's own in the library.
# usage: jpeg6a_memmgr.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=jinit,oom,small,large,sarray,barray,vsarray,vbarray,realize,asarray,abarray,sio,bio,free,destroy)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _alloc, _free, _recorder, fill, heap, log_region, recorder, set_return, strings  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); jmemansi.c's (with jpeg_mem_init / term) and jutils.c's jzero_far
FUNCS = {'jinit_memory_mgr': (0x64e020, 0x64e21d), 'alloc_small': (0x64e21d, 0x64e3e3), 'out_of_memory': (0x64e3e3, 0x64e40d),
         'alloc_large': (0x64e40d, 0x64e4ff), 'alloc_sarray': (0x64e4ff, 0x64e600), 'alloc_barray': (0x64e600, 0x64e715),
         'request_virt_sarray': (0x64e715, 0x64e7ba), 'request_virt_barray': (0x64e7ba, 0x64e85f),
         'realize_virt_arrays': (0x64e85f, 0x64eb3a), 'access_virt_sarray': (0x64eb3a, 0x64ed29), 'do_sarray_io': (0x64ed29, 0x64ee6f),
         'access_virt_barray': (0x64ee6f, 0x64f061), 'do_barray_io': (0x64f061, 0x64f1aa), 'free_pool': (0x64f1aa, 0x64f360),
         'self_destruct': (0x64f360, 0x64f3c0), 'jmemansi': (0x650210, 0x650265), 'jpeg_mem_init': (0x6503c1, 0x6503d0),
         'jzero_far': (0x6504a2, 0x6504bf)}
K_ERROR, K_READ, K_WRITE, K_CLOSE = 0, 3, 4, 5
K_GETENV, K_SSCANF, K_OPEN = 24, 25, 26  # (driver.cpp's kJpeg6aCrtEnv)
B = Bench(sorted(FUNCS.values()), hooks={0x67dcd0: _alloc, 0x67ad90: _free, 0x67fbf0: _recorder(K_GETENV, 1, cdecl=True),
                                         0x67f620: _recorder(K_SSCANF, 4, cdecl=True), 0x650265: _recorder(K_OPEN, 3, cdecl=True)})

CINFO, ERR, MEM, POOLS, VIRT, ROWS, SAMPLES, STR = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0xc00, RAM + 0xe00, RAM + 0x1000, RAM + 0x1800
POOL_AT = [0, 0x100, 0x200, 0x400, 0x600]  # (blocks in POOLS: 0x100, 0x100, 0x200, 0x200, 0x200 bytes)
POOL_SIZE = [0x100, 0x100, 0x200, 0x200, 0x200]
CTRL = 0x80
SMALL, LARGE, VSARRAY, VBARRAY, TOTAL = 0x30, 0x38, 0x40, 0x44, 0x48


def common(r, fail=None):
    log = log_region()
    cinfo = bytearray(r.bytes(0x10))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    err = struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x7c)
    if fail is None:
        fail = r.choice([0] * 12 + [1, 2])
    return {CINFO: cinfo, ERR: err, LOG: log, HEAP: heap(0x400, fail)}


def pools(r, regs, mem, room=0):
    """Pool chains over POOLS' five blocks: small_list[0..1] and large_list[0..1], each a chain of some of them (each block in one
    chain), bytes_used + bytes_left within the block; room: large_list[0]'s first block has at least that much left."""
    buf = bytearray(r.bytes(0x800))
    blocks = list(range(5))
    r.shuffle(blocks)
    for slot in (LARGE, SMALL, SMALL + 4, LARGE + 4):  # (large_list[0] first: with room it gets a block)
        chain = [blocks.pop() for _ in range(min(len(blocks), max(1 if room and slot == LARGE else 0, r.choice([0, 1, 1, 2]))))]
        nxt = 0
        for b in reversed(chain):
            space = POOL_SIZE[b] - 16
            used = r.randrange(0, space + 1, 8) if r.random() < 0.9 else r.randrange(space + 1)
            left = space - used if r.random() < 0.8 else r.randrange(space - used + 1)
            struct.pack_into('<III', buf, POOL_AT[b], nxt, used, left)
            nxt = POOLS + POOL_AT[b]
        struct.pack_into('<I', mem, slot, nxt)
        if room and slot == LARGE:
            struct.pack_into('<II', buf, POOL_AT[chain[0]] + 4, 0, max(room, POOL_SIZE[chain[0]] - 16))
    regs[POOLS] = buf


def memmgr(r, regs, room=0):
    mem = bytearray(r.bytes(0x50))
    struct.pack_into('<i', mem, 0x2c, r.choice([0, 10000000, 10000, r.randrange(0x10000), r.getrandbits(31)]))
    struct.pack_into('<I', mem, TOTAL, r.choice([0x50, r.randrange(0x10000), r.getrandbits(31)]))
    pools(r, regs, mem, room)
    regs[MEM] = mem
    return mem


def size(r):
    return r.choice([0, 1, 7, 8, 9, 15, 16, 24, 100, r.randrange(0x40), r.randrange(0x200)]) if r.random() < 0.98 else \
        r.choice([0x3b9ac9f0, 0x3b9ac9f1])


def pool_id(r):
    return r.choice([0, 1, 1]) if r.random() < 0.95 else 2


def jinit(r):
    regs = common(r, r.choice([0] * 30 + [1]))
    log = regs[LOG]
    strings(log, K_GETENV, 1)
    strings(log, K_SSCANF, 3, out=0xc)
    env = r.random() < 0.6
    set_return(log, K_GETENV, STR if env else 0)
    regs[STR] = r.choice([b'123', b'4m', b'77M', b'x', b'', b' 9k']).ljust(0x10, b'\0')
    set_return(log, K_SSCANF, r.choice([-1, 0, 1, 2, 2, 2]))
    fill(log, K_SSCANF, 2, struct.pack('<i', r.choice([0, 1, 100, 4000, -3, r.randrange(1 << 20), r.getrandbits(32) - (1 << 31)])))
    fill(log, K_SSCANF, 3, bytes([r.choice([ord('m'), ord('M'), ord('M'), ord('x'), ord('k'), 0, r.randrange(256)])]), which=1)
    regs[MEM] = r.bytes(0x10)
    return Call(0x64e020, [CINFO], regs)


def oom(r):
    regs = common(r)
    memmgr(r, regs)
    return Call(0x64e3e3, [CINFO, r.choice([0, 1, 2, 3, 4, r.getrandbits(32)])], regs)


def small(r):
    regs = common(r)
    pid = pool_id(r)
    n = size(r)
    mem = memmgr(r, regs, room=(n + 7) & ~7 if pid == 2 else 0)
    head = struct.unpack_from('<I', mem, SMALL + 4 * pid)[0]
    if head and n < 0x200 and r.random() < 0.3:  # (a pool with exactly the room asked for)
        struct.pack_into('<I', regs[POOLS], head - POOLS + 8, (n + 7) & ~7)
    return Call(0x64e21d, [CINFO, pid, n], regs, ret='a')


def large(r):
    regs = common(r, r.choice([0] * 30 + [1]))
    memmgr(r, regs)
    return Call(0x64e40d, [CINFO, pool_id(r), size(r) if r.random() < 0.99 else 0x3b9ac9f1], regs, ret='a')


def array(r, addr, unit):
    regs = common(r, r.choice([0] * 30 + [1, 2]))
    memmgr(r, regs)
    width = r.choice([1, 2, 3, 8, 16, r.randrange(1, 0x40)]) if r.random() < 0.97 else r.choice([0x40000000 // unit, 0x7fffffff])
    rows = r.choice([0, 1, 2, 3, 4, 8]) if width < 0x100000 else 0
    if r.random() < 0.01:
        width = 0  # (divides by zero in both)
    return Call(addr, [CINFO, r.choice([0, 1, 1]), width, rows], regs, ret='a')


def sarray(r):
    return array(r, 0x64e4ff, 1)


def barray(r):
    return array(r, 0x64e600, 128)


def request(r, addr):
    regs = common(r)
    pid = r.choice([1, 1, 1, 0, 2])
    memmgr(r, regs, room=CTRL if pid == 2 else 0)  # (the control block comes from alloc_small of pool_id even when it is bad)
    return Call(addr, [CINFO, pid, r.choice([0, 1]), r.randrange(1, 0x40), r.randrange(1, 0x40), r.randrange(1, 0x10)], regs, ret='a')


def vsarray(r):
    return request(r, 0x64e715)


def vbarray(r):
    return request(r, 0x64e7ba)


def methods(c):
    struct.pack_into('<III', c, 0x30, recorder(K_READ, 5, cdecl=True), recorder(K_WRITE, 5, cdecl=True), recorder(K_CLOSE, 2, cdecl=True))


def flags(r, c, open_weight=(0, 1)):
    struct.pack_into('<III', c, 0x20, r.choice([0, 1]), r.choice([0, 1]), r.choice(open_weight))


def control(r, unit, nxt, realized):
    """A control block: rows_in_array 1..16, width 1..32 samples or 1..2 blocks, maxaccess 1..rows."""
    c = bytearray(r.bytes(CTRL))
    rows = r.randrange(1, 17)
    width = r.randrange(1, 33) if unit == 1 else r.randrange(1, 3)
    struct.pack_into('<IIII', c, 0, ROWS if realized else 0, rows, width, r.randrange(1, rows + 1))
    struct.pack_into('<I', c, 0x2c, nxt)
    flags(r, c)
    methods(c)
    return c


def virt_lists(r, regs, mem):
    """Up to four control blocks in VIRT, on the sarray and barray lists."""
    virt = bytearray()
    heads = {VSARRAY: 0, VBARRAY: 0}
    for i in range(r.choice([0, 1, 2, 3, 4])):
        slot = r.choice([VSARRAY, VBARRAY])
        virt += control(r, 1 if slot == VSARRAY else 128, heads[slot], r.random() < 0.3)
        heads[slot] = VIRT + CTRL * i
    for slot, head in heads.items():
        struct.pack_into('<I', mem, slot, head)
    regs[VIRT] = bytes(virt) or b'\0' * 4


def realize(r):
    regs = common(r, r.choice([0] * 30 + [1, 2]))
    mem = memmgr(r, regs)
    virt_lists(r, regs, mem)
    total = r.choice([0, 0x50, r.randrange(4000)])
    # maximum_space over the unrealized arrays (mem_buffer NULL), so jpeg_mem_available can return it exactly
    virt, slots = regs[VIRT], {VSARRAY: 1, VBARRAY: 128}
    space = 0
    for slot, unit in slots.items():
        head = struct.unpack_from('<I', mem, slot)[0]
        while head:
            buf, rows, width = struct.unpack_from('<3I', virt, head - VIRT)
            space += 0 if buf else rows * width * unit
            head = struct.unpack_from('<I', virt, head - VIRT + 0x2c)[0]
    exact = (total + space + r.choice([-1, 0, 0, 1])) & 0xffffffff
    struct.pack_into('<I', mem, 0x2c, r.choice([0, 100, 1000, 4000, 10000000, r.randrange(5000), exact, exact]))
    struct.pack_into('<I', mem, TOTAL, total)
    return Call(0x64e85f, [CINFO], regs)


def buffer(r, regs, unit):
    """A realized control block at VIRT: rows_in_mem 1..6 row pointers in ROWS into SAMPLES, its state as the manager leaves it."""
    c = bytearray(r.bytes(CTRL))
    width = r.randrange(1, 33) if unit == 1 else r.randrange(1, 3)
    in_mem = r.randrange(1, 7)
    rows = r.choice([in_mem, in_mem, r.randrange(in_mem, 20)])
    maxaccess = r.randrange(1, in_mem + 1) if r.random() < 0.97 else in_mem + 1
    cur = r.randrange(0, max(1, rows - in_mem + 1))
    undef = r.choice([0, cur, cur + r.randrange(in_mem + 1), rows, r.randrange(rows + 1)])
    struct.pack_into('<8I', c, 0, ROWS if r.random() < 0.98 else 0, rows, width, maxaccess, in_mem,
                     r.choice([1, 1, 2, in_mem, r.randrange(1, in_mem + 1)]), cur, undef)
    flags(r, c, (0, 1, 1))
    struct.pack_into('<I', c, 0x2c, 0)
    methods(c)
    regs[VIRT] = c
    regs[ROWS] = struct.pack('<10I', *[SAMPLES + i * width * unit for i in range(10)])
    regs[SAMPLES] = r.bytes(10 * width * unit)
    return rows, maxaccess


def access(r, addr, unit):
    regs = common(r)
    memmgr(r, regs)
    rows, maxaccess = buffer(r, regs, unit)
    start = r.randrange(0, rows + 1) if r.random() < 0.95 else rows + 1
    num = r.randrange(1, maxaccess + 1) if r.random() < 0.95 else r.choice([0, maxaccess + 1])
    num = min(num, max(0, rows - start)) if r.random() < 0.95 else num
    return Call(addr, [CINFO, VIRT, start, num, r.choice([0, 1])], regs, ret='a')


def asarray(r):
    return access(r, 0x64eb3a, 1)


def abarray(r):
    return access(r, 0x64ee6f, 128)


def io(r, addr, unit):
    regs = common(r)
    memmgr(r, regs)
    buffer(r, regs, unit)
    return Call(addr, [CINFO, VIRT, r.choice([0, 1])], regs)


def sio(r):
    return io(r, 0x64ed29, 1)


def bio(r):
    return io(r, 0x64f061, 128)


def free(r):
    regs = common(r)
    mem = memmgr(r, regs)
    virt_lists(r, regs, mem)
    return Call(0x64f1aa, [CINFO, pool_id(r)], regs)


def destroy(r):
    regs = common(r)
    mem = memmgr(r, regs)
    virt_lists(r, regs, mem)
    return Call(0x64f360, [CINFO], regs)


OPS = [f for f in (jinit, oom, small, large, sarray, barray, vsarray, vbarray, realize, asarray, abarray, sio, bio, free, destroy)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
