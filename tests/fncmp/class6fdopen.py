# The folders-and-archives lists' opens and finds (src/game/core/class6fd9d8: 0x41d3d0, 0x41ef40, 0x41d570, 0x41e740) against the originals, old skips: lists of 0-4 objects (or no list, or a list with no nodes), recorders for their slot 1 (find) and slot 3 (open),
# each result any sign and logged with the relative path's or the path's text, the status pointers as 0; the root strings narrow or (as the code reads them) with a wide table, the names under them or not, the real strings and Path_StripBase; the found file's member at +0x3c there or not, its
# new copy's block failing now and then. 0x41d570's record's first 0x20 bytes are the stack's: left out of the comparison; 0x41e740's own record logged by the text its cursor points to.
# usage: class6fdopen.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, blob_bytes, echo, fill, heap, log_region, put, recorder, strings  # noqa: E402
from gamestring import LOCALE, string  # noqa: E402
import vfs  # noqa: E402

FIND, OPEN, DELETE = 1, 3, 4
CALLS = {'Open': (0x41d3d0, 0x41d56d), 'OpenInner': (0x41ef40, 0x41f0cd), 'Find': (0x41d570, 0x41d701), 'FindOpen': (0x41e740, 0x41e8dc)}
B = Bench(vfs.RANGES + list(CALLS.values()))
OBJECT, LIST, NODES, VT, MEMBER, MEMBERVT, HOLDER, INFO, PATH, PATHDATA, OBJS, ROOTS = (RAM + 0x4000, RAM + 0x4100, RAM + 0x4140, RAM + 0x4200, RAM + 0x4240, RAM + 0x4260, RAM + 0x42a0, RAM + 0x42c0,
                                                                                         RAM + 0x4400, RAM + 0x4440, RAM + 0x4800, RAM + 0x5000)
OPS = list(CALLS)


def root_at(r, me, off, data, regions):
    """A root at me + off, as mounts() in vfs.py makes the source's: returns its text."""
    root = vfs.folder(r) or b'data'
    if r.random() < 0.3:
        root += b'\\'
    part = {}
    wide = r.random() < 0.2
    string(r, 0, RAM + 0x10, data, part, root if not wide else b''.join(bytes([c, 0]) for c in root))
    head = bytearray(part[RAM + 0x10][:0x14])
    if wide:
        put(head, 0, 'II', 0x7030c8, len(root))
    me[off:off + 0x14] = head
    regions[data] = part[data] + r.bytes(0x20)
    return root


def results(r, log, k, sure):
    strings(log, k, *sure)
    echo(log, k, 0)
    fill(log, k, 0, struct.pack('<5i', *[r.choice([0, 1, 0x7fffffff, -1, -1, -1, -0x80000000]) for _ in range(5)]), which=0 if k == FIND else 1, records=5)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 1]) if op == 'Find' else 0), LOCALE: bytes(4)}
    vfs.globals_(r, regions)
    me = bytearray(r.bytes(0x60))
    regions[OBJECT] = me
    table = bytearray(r.bytes(0x20))
    put(table, 1 * 4, 'I', recorder(FIND, 3))
    put(table, 3 * 4, 'I', recorder(OPEN, 5))
    regions[VT] = table
    n = r.choice([0, 1, 1, 2, 2, 3, 4])
    nodes = bytearray(r.bytes(0x50))
    root = None
    for k in range(n):
        held = OBJS + 0x80 * k
        put(nodes, 0x10 * k, 'II', held, NODES + 0x10 * (k + 1) if k + 1 < n else 0)
        obj = bytearray(r.bytes(0x60))
        put(obj, 0, 'I', VT)
        if op == 'OpenInner':
            put(obj, 0x2c, 'I', VT)
            root = root_at(r, obj, 0x18, ROOTS + 0x100 * (k + 1), regions)
        regions[held] = obj
    regions[NODES] = nodes
    lst = bytearray(r.bytes(0x10))
    put(lst, 4, 'I', NODES if n else 0)
    regions[LIST] = lst
    put(me, 0x4c if op == 'OpenInner' else 0x2c, 'I', LIST if r.random() < 0.9 else 0)
    if op == 'Open':
        root = root_at(r, me, 0x18, ROOTS, regions)
        put(me, 0x30, 'I', HOLDER)
        regions[HOLDER] = r.bytes(0x10)
    text = vfs.name_under(r, root) if root else vfs.folder(r) + r.choice([b'', b'.wct'])
    string(r, 0, PATH, PATHDATA, regions, text)
    results(r, log, OPEN, (1 << 2, 1))
    stream = r.getrandbits(32)
    if op in ('Open', 'OpenInner'):
        return Call(CALLS[op][0], [OBJECT, stream, PATH, r.getrandbits(32)], regions, conv='t', ret='a')
    if op == 'Find':
        results(r, log, FIND, (1 << 1, 1))
        info = bytearray(r.bytes(0x130))
        regions[INFO] = info
        put(me, 0x3c, 'I', r.choice([0, MEMBER]))
        regions[MEMBER] = struct.pack('<I', MEMBERVT) + r.bytes(0x10)
        member = bytearray(r.bytes(0x20))
        put(member, 3 * 4, 'I', recorder(DELETE, 1))
        regions[MEMBERVT] = member
        return Call(CALLS[op][0], [OBJECT, PATH, INFO], regions, conv='t', ret='a', ignore=[(INFO, 0x20)])
    results(r, log, FIND, (1 << 1, 1 | 1 << 2))
    blob_bytes(log, FIND, 2, 0, inner=(0x20, 1))
    return Call(CALLS[op][0], [OBJECT, stream, PATH], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
