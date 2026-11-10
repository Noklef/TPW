# The C library's small block heap, allocating (src/game/core/crtsbh), and calloc against the originals: the Win32 calls COM recorders in their import slots, the locks cdecl recorders, the regions and pages made here.
# Pages are a walk of blocks and free runs up to the sentinel; a group's count and bound are the page's or less, as stale ones are. "commit", "deep" and "fresh" reach the code past the search; "plain" fails it all.
# usage: crt_sbh.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, import_table, log_region, recorder, set_return  # noqa: E402

LOCK, UNLOCK, NEWH = 7, 8, 9
VALLOC, VFREE, HALLOC, HFREE, HCREATE, HDESTROY = 14, 15, 16, 17, 20, 21
B = Bench([(0x67dcf0, 0x67dd34), (0x67dd40, 0x67dd95), (0x6826b0, 0x6826e7),(0x67fcb0, 0x67fd5b), (0x688710, 0x688730), (0x6828a0, 0x682a02), (0x682c00, 0x682e33), (0x682e40, 0x682fb2)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True)})
STATIC, R2, R3 = 0x77eda0, RAM + 0x1000, RAM + 0x4000
VAS, HEAPS, NEWHS = RAM, RAM + 0x100, RAM + 0x200
PAGES, PAGES2, FRESH = RAM + 0x10000, RAM + 0x30000, RAM + 0x50000
LIMIT = 0xf0
PARAS = [1, 1, 1, 2, 3, 4, 8, 0x10, 0x20, 0x40, 0x80, 0xf0, 0xf1]
BYTES = [0, 1, 2, 0xf, 0x10, 0x11, 0x20, 0x100, 0x1e0, 0x1e1, 0x3f0, 0x1000, 0xffffffe0, 0xffffffe1, 0xfffffff0]
OPS = [n for n in ('page', 'page', 'block', 'block', 'block', 'new', 'calloc', 'calloc', 'newh', 'heap_init', 'heapalloc', 'heapalloc', 'nhmalloc', 'nhmalloc') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def page(r, at):
    """The page's bytes and its free paragraphs."""
    data = bytearray(b'\xcd') * 0x1000
    data[8:0xf8] = bytes(LIMIT)
    pos, total, bounds = 0, 0, []
    while pos < LIMIT:
        bounds.append(pos)
        n = min(r.choice([1, 1, 2, 3, 4, 8, 0x10, 0x20, 0x40, LIMIT]), LIMIT - pos)
        if r.random() < 0.5:
            data[8 + pos] = n
        else:
            total += n
        pos += n
    data[0xf8] = 0xff
    start = r.choice(bounds)
    run = 0
    while not data[8 + start + run]:
        run += 1
    other = r.choice(bounds)   # (a stale count, past the free run: to another block's start)
    struct.pack_into('<II', data, 0, at + 8 + start, r.choice([run, run, run // 2, 0, other - start if other > start else run]))
    return data, total


def fitting(at, n, start_at_end):
    """A page whose only free run, of n paragraphs, ends at the sentinel, for the fits that leave no room: its start there or before it, nothing counted free."""
    data = bytearray(b'\xcd') * 0x1000
    data[8:0xf8] = bytes(LIMIT)
    data[8] = LIMIT - n
    data[0xf8] = 0xff
    struct.pack_into('<II', data, 0, at + 8 + (LIMIT - n if start_at_end else 0), n if start_at_end else 0)
    return data


def region(r, regions, at, memory, committed, uncommitted):
    """The region and its pages: committed pages, then uncommitted ones (their memory zero, -1 in their groups), sometimes another uncommitted group past a gap."""
    data = bytearray(0x2020)
    pages = bytearray()
    for i in range(0x400):
        count, bound = 0, 0xf1
        if i < committed:
            bytes_, total = page(r, memory + i * 0x1000)
            pages += bytes_
            count, bound = r.choice([total, total, total // 2, 0xf0]), r.choice([0xf1, 0xf1, 0xf1, 2, 1])
        elif i < committed + uncommitted:
            count, bound = 0xffffffff, r.choice([0xf1, 0, 7])
        struct.pack_into('<II', data, 0x18 + i * 8, count, bound)
    if committed + uncommitted + 2 < 0x400 and r.random() < 0.3:
        struct.pack_into('<I', data, 0x18 + (committed + uncommitted + 2) * 8, 0xffffffff)
    pages += bytes(0x1000 * uncommitted)
    struct.pack_into('<4I', data, 8, at + 0x18 + 8 * r.randrange(max(committed, 1)), 0, memory, memory + 0x400000)
    if pages:
        regions[memory] = pages
    return data


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'valloc': (VALLOC, 0x6fd1b4, 4, 0, 0), 'vfree': (VFREE, 0x6fd1b8, 3, 0, 0), 'halloc': (HALLOC, 0x6fd224, 3, 0, 0), 'hfree': (HFREE, 0x6fd210, 3, 0, 0),
                                                         'hcreate': (HCREATE, 0x6fd1bc, 3, 0, 0), 'hdestroy': (HDESTROY, 0x6fd1c0, 1, 0, 0)})}
    answers(log, VALLOC, VAS)
    answers(log, HALLOC, HEAPS)
    answers(log, NEWH, NEWHS)
    regions[0xfb3cac] = struct.pack('<I', 0x5000)
    regions[0xfb1548] = struct.pack('<2I', r.choice([0, 1]), r.choice([0, recorder(NEWH, 1, cdecl=True), recorder(NEWH, 1, cdecl=True)]))
    regions[0x780dc4] = struct.pack('<I', r.choice([0x1e0, 0x1e0, 0x3f0, 0x10, 0]))
    regions[NEWHS] = struct.pack('<40I', *([r.choice([1, 2])] * r.choice([0, 1, 2]) + [0] * 40)[:40])
    if name == 'newh':
        return Call(0x688710, [r.getrandbits(32)], regions, conv='c', ret='a')
    scene = 'fresh' if name in ('new', 'heap_init') else 'plain' if name == 'page' else r.choice(['plain', 'plain', 'plain', 'commit', 'commit', 'fresh', 'deep'])
    committed = 1 if scene == 'deep' else r.randrange(1, 4)
    uncommitted = 17 if scene == 'deep' else r.randrange(1, 4) if scene == 'commit' else 0
    static = region(r, regions, STATIC, PAGES, committed, uncommitted)
    va = [0] * 40
    heaps = [r.choice([R3, 0x70000, 0]), r.choice([0x70000, 0x70000, 0]), r.choice([0x71000, 0]), 0x72000] + [0] * 36
    two = r.random() < 0.4
    if two:
        other = region(r, regions, R2, PAGES2, r.randrange(1, 3), 0)
        struct.pack_into('<2I', static, 0, R2, R2)
        struct.pack_into('<2I', other, 0, STATIC, STATIC)
        regions[R2] = other
    else:
        struct.pack_into('<2I', static, 0, STATIC, STATIC)
    regions[0x780dc0] = struct.pack('<I', R2 if two and r.random() < 0.5 else STATIC)
    if scene in ('commit', 'deep'):
        index = committed if r.random() < 0.9 else 0
        struct.pack_into('<I', static, 0xc, STATIC + 0x18 + 8 * index)
        va[0] = PAGES + index * 0x1000 if r.random() < 0.85 else r.choice([0, PAGES + 0x1000 * 0x40])
    elif scene == 'fresh':
        regions[FRESH] = bytes(0x10000)
        regions[R3] = r.bytes(0x2020)
        va[0] = r.choice([FRESH, FRESH, FRESH, 0])
        va[1] = r.choice([FRESH, FRESH, FRESH, 0])
        heaps[0] = r.choice([R3, R3, R3, 0])
        if r.random() < 0.5:   # (no memory yet: the static region is the new one, its links zero or its own)
            struct.pack_into('<I', static, 0x10, 0xffffffff)
            struct.pack_into('<2I', static, 0, STATIC, STATIC)
            struct.pack_into('<I', static, 0xc, r.choice([0, STATIC + 0x18]))   # (a first uncommitted group, of no use without memory)
            regions[0x780dc0] = struct.pack('<I', STATIC)
            if name == 'new':
                struct.pack_into('<2I', static, 0, *r.choice([(0, 0), (0, STATIC), (STATIC, 0)]))
    regions[STATIC] = static
    regions[VAS] = struct.pack('<40I', *va)
    regions[HEAPS] = struct.pack('<40I', *heaps)
    if name == 'page':
        if r.random() < 0.3:
            n = r.choice([1, 2, 3, 8, 0x20])
            regions[PAGES] = fitting(PAGES, n, r.random() < 0.5)
            return Call(0x682e40, [PAGES, 0xf0, n], regions, conv='c', ret='a')
        return Call(0x682e40, [PAGES + 0x1000 * r.randrange(committed), r.choice([0, 1, 4, 0x10, 0x40, 0xf0]), r.choice(PARAS)], regions, conv='c', ret='a')
    if name == 'block':
        return Call(0x682c00, [r.choice(PARAS)], regions, conv='c', ret='a')
    if name == 'new':
        return Call(0x6828a0, [], regions, conv='c', ret='a')
    if name == 'heapalloc':
        return Call(0x67dd40, [r.choice(BYTES)], regions, conv='c', ret='a')
    if name == 'nhmalloc':   # (with the mode, _callnewh's answers end the retries)
        return Call(0x67dcf0, [r.choice(BYTES), r.choice([0, 1, 1, 2])], regions, conv='c', ret='a')
    if name == 'heap_init':   # (HeapCreate's heap, or none; then the new region as 'new' has it)
        set_return(log, HCREATE, r.choice([0x5000, 0x5000, 0x5000, 0]))
        regions[0xfb3cac] = r.bytes(4)
        return Call(0x6826b0, [], regions, conv='c', ret='a')
    return Call(0x67fcb0, [r.choice(BYTES), 1] if r.random() < 0.7 else [r.choice([2, 3, 0x10, 0x10000]), r.choice([1, 0x10, 0x18, 0x10000])], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
