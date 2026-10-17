# The list leaving destructor and the queue room test of src/game/core/small51 (0x5f4400, 0x4dda20) against the originals: the global list of 0 to 4 nodes (the object in
# it or not, first / middle / last), the pointer array at 0x879ce0 holding the object's pointer or not; objects.py's person queues for the room test.
# usage: small51.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dtor,Room)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectmsg  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

CALLS = {'Dtor': 0x5f4400, 'Room': 0x4dda20}
B = Bench([(0x5f4400, 0x5f4490), (0x4dda20, 0x4dda50), (0x56b5c0, 0x56b620), (0x4ddf50, 0x4de040), (0x502430, 0x502460), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1), (0x67acb0, 0x67acb1)])
LIST, NODES, ARR, ITEMS = RAM + 0x400, RAM + 0x500, RAM + 0x800, RAM + 0x900


def case(r):
    name = r.choice(OPS)
    if name == 'Room':
        saved = objectmsg.case
        c = objectmsg.queue(r, 'Pos')
        obj = bytearray(c.regions[objectmsg.OBJ])
        put(obj, 0x40, 'I', r.choice([0, 1, 2, 3, 5, 0x40000000]))
        c.regions[objectmsg.OBJ] = obj
        return Call(CALLS[name], [objectmsg.OBJ], c.regions, conv='t', ret='b')
    obj = bytearray(r.bytes(0x60))
    n = r.choice([0, 1, 2, 4])
    nodes = bytearray(0x10 * 5)
    holder = r.randrange(n) if n and r.random() < 0.8 else -1
    for k in range(n):
        put(nodes, 0x10 * k, 'III', RAM if k == holder else RAM + 0x1000 + 0x40 * k, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
    lst = bytearray(r.bytes(0x10))
    put(lst, 4, 'III', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0, n)
    items = [r.choice([0, 1, 0x30004000 + 0x10 * i]) for i in range(3)]
    item = r.choice(items + [0x30004000])
    put(obj, 0x28, 'I', item)
    arr = bytearray(r.bytes(0x20))
    holder_obj = bytearray(r.bytes(0x40))
    for i, v in enumerate(items):
        put(arr, 4 * i, 'I', v)
    put(holder_obj, 0x1c, 'i', r.choice([3, 3, 2, 0]))
    put(holder_obj, 8, 'I', ITEMS)
    put(holder_obj, 4, 'I', RAM + 0xa00)
    regions = {RAM: obj, LIST: lst, NODES: nodes, 0xfa6e38: put(bytearray(8), 0, 'I', LIST), 0x879ce0: holder_obj, ITEMS: arr, RAM + 0xa00: r.bytes(0x20), HEAP: heap()}
    return Call(CALLS[name], [RAM, r.choice([0, 1, 2, 0xffffffff])], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
