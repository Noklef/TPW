# The user slots' record accessors (src/game/files/userslots: 0x5c7bb0, 0x5c7d00, 0x5c7d20) against the originals: four 24-byte records
# whose dword at +4 (the name's length) is 0, 1, the top bit alone or anything, and each record asked for.
# Part (0x5cadf0): its object's constructor on random bytes; its allocator bytes (the stack's garbage) left out. Ctor (0x5c7420): the whole object's, the same.
# usage: userslots.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Count,IfUsed,Field14,Part,Ctor)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x5c7bb0, 0x5c7bc7), (0x5c7d00, 0x5c7d16), (0x5c7d20, 0x5c7d2e), (0x5cadf0, 0x5caf20), (0x5cde50, 0x5cde80), (0x45b590, 0x45b5f0), (0x67acd0, 0x67ada0),
           (0x5c7420, 0x5c749c), (0x5cd400, 0x5cd430), (0x5f6a20, 0x5f6a60), (0x5f5fc0, 0x5f6000)])
CALLS = {'Count': 0x5c7bb0, 'IfUsed': 0x5c7d00, 'Field14': 0x5c7d20}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS) + ',Part,Ctor').split(',') if k]
PART_BYTES = (0, 0x10, 0x20, 0x30, 0x3c, 0x58, 0x64, 0x70, 0x80)


def case(r):
    op = OPS[r.deal(range(len(OPS)))]
    if op == 'Part':
        ignore = [(RAM + k, 1) for k in PART_BYTES]
        return Call(0x5cadf0, [RAM], {RAM: r.bytes(0xa0), HEAP: heap()}, conv='t', ret='a', ignore=ignore)
    if op == 'Ctor':   # (Part's allocator bytes, and the list head's byte +0x100 as ListHead_Ctor's own: the top byte of the object's address)
        ignore = [(RAM + 0x64 + k, 1) for k in PART_BYTES]
        return Call(0x5c7420, [RAM], {RAM: r.bytes(0x110), HEAP: heap()}, conv='t', ret='a', ignore=ignore)
    obj = bytearray(r.bytes(0x110))
    for i in range(4):
        put(obj, 4 + 24 * i, 'I', r.choice([0, 0, 1, 0x80000000, r.getrandbits(32)]))
    args = [RAM] if op == 'Count' else [RAM, r.randrange(4)]
    return Call(CALLS[op], args, {RAM: obj}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
