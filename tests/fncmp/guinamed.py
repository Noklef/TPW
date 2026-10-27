# The GUI's named objects (src/game/ui/guinamed: 0x661a93, 0x661b3c, 0x661bab) against the originals. The list [0xfaa624]: 0-5 nodes
# (0x14 bytes: the object, the name's 12 characters, the next node), their names short, empty, or 12 long (not terminated: the next
# pointer's bytes read after them), from a small alphabet so names repeat; or no list (a lone node of random bytes now and then). Add: an
# object (0 now and then) and a name (empty now and then, 1-20 long, one of the list's now and then), the heap (Mem_Alloc) failing now and
# then. FreeAll: the nodes' objects 0 or objects whose vtable's slot 0 is a recorder (the deleting destructor, 1). Find: a name among the
# list's mostly (one short of it or longer now and then), or another.
# usage: guinamed.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Add,FreeAll,Find)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, vtable  # noqa: E402

CALLS = {'Add': 0x661a93, 'FreeAll': 0x661b3c, 'Find': 0x661bab}
B = Bench([(0x661a93, 0x661b33), (0x661b3c, 0x661bab), (0x661bab, 0x661c0b), (0x67ff70, 0x680000), (0x67ae00, 0x67af10),
           (0x680060, 0x6800e7)])
NODES, OBJECTS, OVT, NAME = RAM, RAM + 0x200, RAM + 0x400, RAM + 0x600
ALPHABET = b'ab!'


def name(r, length):
    return bytes(r.choice(ALPHABET) for _ in range(length))


def nodes(r, regions, objects=False):
    """the list (above); returns the nodes' names"""
    n = r.choice([0, 1, 1, 2, 3, 3, 4, 5])
    names = []
    buf = bytearray(r.bytes(0x20 * 5))
    for i in range(n):
        text = name(r, r.choice([1, 2, 3, 4, 5, 11, 12, 0, r.randrange(1, 13)]))
        names.append(text)
        obj = r.choice([OBJECTS + 0x20 * i] * 6 + [0]) if objects else r.getrandbits(32)
        put(buf, 0x20 * i, 'I12sI', obj, text, NODES + 0x20 * (i + 1) if i + 1 < n else 0)
    head = NODES if n or (not objects and r.random() < 0.25) else 0
    if head and not n:   # (a lone node of random bytes: ended)
        put(buf, 0x10, 'I', 0)
        names.append(bytes(buf[4:0x10]).split(b'\0')[0])
    regions[NODES] = buf
    regions[0xfaa624] = struct.pack('<I', head)
    if objects:
        regions[OBJECTS] = b''.join(struct.pack('<I', OVT) + r.bytes(0x1c) for _ in range(5))
        regions[OVT] = vtable({0: 1}, size=4)
    return names


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    if op == 'FreeAll':
        nodes(r, regions, objects=True)
        regions[HEAP] = heap(0x100)
        return Call(CALLS[op], [], regions, ret='v')
    names = nodes(r, regions)
    if op == 'Add':
        text = r.choice(names) if names and r.random() < 0.2 else name(r, r.choice([0, 1, 3, 11, 12, 13, 20, r.randrange(1, 21)]))
        regions[NAME] = text + b'\0' + r.bytes(3)
        regions[HEAP] = heap(0x100, r.choice([0] * 8 + [1]))
        obj = r.choice([r.getrandbits(32)] * 12 + [0])
        return Call(CALLS[op], [obj, NAME], regions, ret='w')
    if names and r.random() < 0.8:
        text = r.choice(names)
        text = r.choice([text] * 6 + [text[:-1], text + name(r, 1)])
    else:
        text = name(r, r.choice([0, 1, 2, 4, 12, 13]))
    regions[NAME] = text + b'\0' + r.bytes(3)
    return Call(CALLS[op], [NAME], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
