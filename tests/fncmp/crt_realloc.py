# The C library's small block heap, freeing and resizing (src/game/core/crtrealloc), and realloc against the originals: the Win32 calls COM recorders in their import slots, the locks and _nh_malloc cdecl recorders, free the heap's hash; the regions and pages made here.
# Bench limit: _heap's decommit never ends on a ring where a region without memory is not the first (sic): the region does not advance. Not run.
# usage: crt_realloc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, heap, import_table, log_region, recorder, set_return  # noqa: E402

LOCK, UNLOCK, NEWH, NHMALLOC = 7, 8, 9, 28
VALLOC, VFREE, HALLOC, HFREE, HREALLOC, HSIZE = 14, 15, 16, 17, 18, 19
B = Bench([(0x6818f0, 0x681953),(0x682b40, 0x682b99), (0x682a10, 0x682a6b), (0x682a70, 0x682b3d), (0x682ba0, 0x682bf3), (0x682fc0, 0x683084), (0x681740, 0x6818f0),
           (0x688710, 0x688730), (0x6828a0, 0x682a02), (0x682c00, 0x682e33), (0x682e40, 0x682fb2), (0x67dcd0, 0x67dcf0)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x67ad90: _free})
STATIC, R2 = 0x77eda0, RAM + 0x1000
VAS, HEAPS, NEWHS, FREES, OUT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
PAGES, PAGES2 = RAM + 0x10000, RAM + 0x20000
BIG = HEAP + 0x100
LIMIT = 0xf0
OPS = [n for n in ('find', 'find', 'release', 'decommit', 'decommit', 'decommit', 'free', 'free', 'free', 'resize', 'resize', 'resize', 'resize', 'realloc', 'realloc', 'realloc', 'realloc', 'realloc', 'msize', 'msize')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def page(r, at):
    """The page's bytes, its free paragraphs and its blocks' positions."""
    data = bytearray(r.bytes(0x1000))
    data[8:0xf8] = bytes(LIMIT)
    pos, total, used, bounds = 0, 0, [], []
    while pos < LIMIT:
        bounds.append(pos)
        n = min(r.choice([1, 1, 2, 3, 4, 8, 0x10, 0x20]), LIMIT - pos)
        if r.random() < 0.55:
            data[8 + pos] = n
            used.append(pos)
        else:
            total += n
        pos += n
    data[0xf8] = 0xff
    start = r.choice(bounds)
    run = 0
    while not data[8 + start + run]:
        run += 1
    struct.pack_into('<II', data, 0, at + 8 + start, run)
    return data, total, used or [0]


def groups(r, data, committed, memory_pages):
    """A region's groups, for committed pages with their free counts, the rest uncommitted."""
    for i in range(0x400):
        count, bound = (memory_pages[i][1], r.choice([0xf1, 0xf1, 2])) if i < committed else (0xffffffff, 0xf1)
        struct.pack_into('<II', data, 0x18 + i * 8, count, bound)


def make_region(r, regions, at, memory, committed):
    data = bytearray(0x2020)
    pages = [page(r, memory + i * 0x1000) for i in range(committed)]
    groups(r, data, committed, pages)
    struct.pack_into('<4I', data, 8, at + 0x18, at + 0x18 + 8 * committed, memory, memory + 0x400000)
    regions[memory] = b''.join(bytes(p[0]) for p in pages)
    return data, pages


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000),
               IMPORT_TABLE: import_table(log, {'valloc': (VALLOC, 0x6fd1b4, 4, 0, 0), 'vfree': (VFREE, 0x6fd1b8, 3, 0, 0), 'halloc': (HALLOC, 0x6fd224, 3, 0, 0), 'hfree': (HFREE, 0x6fd210, 3, 0, 0), 'hrealloc': (HREALLOC, 0x6fd258, 4, 0, 0),
                                                         'hsize': (HSIZE, 0x6fd208, 3, 0, 0)})}
    set_return(log, HSIZE, r.getrandbits(32))
    answers(log, VALLOC, VAS)
    answers(log, HALLOC, HEAPS)
    answers(log, HREALLOC, HEAPS)
    answers(log, NEWH, NEWHS)
    answers(log, NHMALLOC, HEAPS)
    answers(log, VFREE, FREES)
    regions[VAS] = bytes(0xa0)
    regions[HEAPS] = struct.pack('<40I', *[r.choice([BIG + 0x2100 * (i % 3), BIG + 0x2100 * (i % 3), 0]) for i in range(40)])
    regions[NEWHS] = struct.pack('<40I', *([1] * r.choice([0, 1, 2]) + [0] * 40)[:40])
    regions[FREES] = struct.pack('<40I', *[0 if r.random() < 0.1 else 1 for _ in range(40)])
    regions[OUT] = r.bytes(8)
    regions[0xfb3cac] = struct.pack('<I', 0x5000)
    regions[0xfb1428] = struct.pack('<I', r.choice([0, 3, 0x1f, 0x1f, 0x20]))
    regions[0xfb1548] = struct.pack('<2I', r.choice([0, 0, 1]), r.choice([0, recorder(NEWH, 1, cdecl=True), recorder(NEWH, 1, cdecl=True)]))
    regions[0x780dc4] = struct.pack('<I', r.choice([0x1e0, 0x1e0, 0x3f0, 0x10, 0]))
    static, pages = make_region(r, regions, STATIC, PAGES, r.randrange(1, 4))
    other, pages2 = make_region(r, regions, R2, PAGES2, r.randrange(1, 3))
    two = r.random() < 0.6
    if two:
        struct.pack_into('<2I', static, 0, R2, R2)
        struct.pack_into('<2I', other, 0, STATIC, STATIC)
    else:
        struct.pack_into('<2I', static, 0, STATIC, STATIC)
    regions[R2] = other
    regions[STATIC] = static
    regions[0x780dc0] = struct.pack('<I', r.choice([R2, STATIC]) if two else STATIC)

    def a_block():
        """A block of a region: its address, region, page and entry."""
        if two and r.random() < 0.3:
            holder, memory, list_ = R2, PAGES2, pages2
        else:
            holder, memory, list_ = STATIC, PAGES, pages
        i = r.randrange(len(list_))
        pos = r.choice(list_[i][2])
        return holder, memory + i * 0x1000, pos

    if name in ('find', 'msize'):
        holder, at, pos = a_block()
        block = at + 0x100 + pos * 0x10
        block = r.choice([block, block, block, block + 1, block + 8, at, at + 0xf0, at + 0xff0, PAGES, PAGES + 0x400000, PAGES + 0x3fffff, PAGES - 1, 0x1000, 0xfffffff0, PAGES2])
        if r.random() < 0.2:   # (a block exactly at a region's end, an end not on a page: not in that region)
            block = at + 0x100 + pos * 0x10
            struct.pack_into('<I', static if holder == STATIC else other, 0x14, block)
        regions[STATIC] = static
        if name == 'msize':
            return Call(0x6818f0, [block], regions, conv='c', ret='a')
        return Call(0x682b40, [block, OUT, OUT + 4], regions, conv='c', ret='a')
    if name == 'release':
        if r.random() < 0.5:
            regions[0x780dc0] = struct.pack('<I', r.choice([R2, STATIC]))
        return Call(0x682a10, [r.choice([STATIC, R2] if two else [STATIC])], regions, conv='c')
    if name == 'decommit':
        scene = r.choice(['scatter', 'scatter', 'all', 'all', 'none'])
        for data, which in ((static, STATIC), (other, R2)):
            for i in range(0x400):
                count = 0xffffffff
                if scene == 'scatter':
                    count = r.choice([0xf0, 0x20, 0xffffffff, 0xffffffff, 0xffffffff, 0])
                struct.pack_into('<I', data, 0x18 + i * 8, count)
            if scene == 'all':
                for i in r.sample(range(0x400), r.choice([1, 2, 3])):
                    struct.pack_into('<I', data, 0x18 + i * 8, 0xf0)
            struct.pack_into('<I', data, 0xc, r.choice([0, which + 0x18, which + 0x2010]))
            if scene == 'all' and r.random() < 0.3:   # (the first group kept, or the last: nothing released)
                struct.pack_into('<I', data, 0x18 + r.choice([0, 0x3ff]) * 8, r.choice([0x20, 0xf0]))
        if not two and r.random() < 0.15:   # (with two regions this never ends, in both: see the header)
            struct.pack_into('<I', static, 0x10, 0xffffffff)
        if r.random() < 0.5:
            regions[FREES] = struct.pack('<40I', *[1] * 40)
        regions[STATIC] = static
        regions[R2] = other
        return Call(0x682a70, [r.choice([1, 2, 3, 0x10, 0x10, 0x7f])], regions, conv='c')
    if name in ('free', 'resize'):
        holder, at, pos = a_block()
        entry = at + 8 + pos
        region_bytes = static if holder == STATIC else other
        group = 0x18 + ((at - (PAGES if holder == STATIC else PAGES2)) >> 12) * 8
        if name == 'free':
            if r.random() < 0.6:
                size = bytes(regions[PAGES if holder == STATIC else PAGES2])[at - (PAGES if holder == STATIC else PAGES2) + 8 + pos]
                struct.pack_into('<I', region_bytes, group, 0xf0 - size)
                if r.random() < 0.3:   # (the free that brings the count to 0x20, with more pages to give back than its 0x10)
                    regions[0xfb1428] = struct.pack('<I', 0x1f)
                    regions[FREES] = struct.pack('<40I', *[1] * 40)
                    for i in range(0x400 - 0x14, 0x400):
                        struct.pack_into('<I', region_bytes, 0x18 + i * 8, 0xf0)
            return Call(0x682ba0, [holder, at, entry], regions, conv='c')
        block_pages = regions[PAGES if holder == STATIC else PAGES2] = bytearray(regions[PAGES if holder == STATIC else PAGES2])
        offset = at - (PAGES if holder == STATIC else PAGES2)
        size = block_pages[offset + 8 + pos]
        if r.random() < 0.3:   # (the whole page after the block free: a grow up to its end)
            block_pages[offset + 8 + pos + size:offset + 0xf8] = bytes(LIMIT - pos - size)
        start = r.choice([pos, pos + size, pos + size + 1, pos - 1 if pos else 0, 0, LIMIT - 1])
        struct.pack_into('<I', block_pages, offset, at + 8 + start)
        return Call(0x682fc0, [holder, at, entry, r.choice([1, 2, max(size - 1, 1), size, size, size + 1, size + 1, size + 2, size + 4, LIMIT - pos, LIMIT - pos + 1, LIMIT])], regions, conv='c', ret='a')
    holder, at, pos = a_block()
    block = at + 0x100 + pos * 0x10
    block = r.choice([block, block, block, block, block, 0, PAGES - 0x10, block + 1])
    sizes = [0, 1, 0xf, 0x10, 0x11, 0x20, 0x40, 0x100, 0x1e0, 0x1e1, 0x3f0, 0x1000, 0xffffffe0, 0xffffffe1, 0xfffffff0]
    return Call(0x681740, [block, r.choice(sizes)], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
