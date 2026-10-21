# The GUI library's kind 13 window (vtable 0x705b98) against the originals (src/game/ui/window705b98: 0x66c85a).
# SetBits: the old bits +0x138 and the new mask each few bits (of the low five, the top one, any), equal now and then; the children
# (+0x5c, next +0x54) 0-6 windows whose ids (+0x8c) are 0x11100000 | a bit number (0-31, or another id now and then), each with its
# objects +0x98 / +0x9c recorders (slot 3: k 4 and 23) or 0, the window's own others (k 3 and 22). Each recorder logs its event as the
# hash of the bytes set: the children's (Window_Notify1) the word +4, the window's (Window_Notify80) the dword +0 (the rest is stack
# garbage, not compared).
# usage: window705b98.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetBits)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob_bytes, log_region, put, recorder, vtable  # noqa: E402

CALLS = {'SetBits': 0x66c85a}
B = Bench([(0x65d3a3, 0x65d40e), (0x65d7bc, 0x65d832), (0x661525, 0x6615a6), (0x66c85a, 0x66c922)])
WINDOW, OBJ98, OBJ9C, VT98, VT9C, KIDS = RAM, RAM + 0x200, RAM + 0x240, RAM + 0x280, RAM + 0x300, RAM + 0x400
KOBJ98, KOBJ9C, KVT98, KVT9C = RAM + 0x1000, RAM + 0x1040, RAM + 0x1080, RAM + 0x1100
KID = 0xa0


def bits(r):
    return r.choice([r.getrandbits(5), r.getrandbits(5), 1 << r.randrange(32), r.getrandbits(32), 0, 0x80000000, 0x7fffffff,
                     r.getrandbits(5) | r.getrandbits(2) << 29])


def objects(r, buf, which=(OBJ98, OBJ9C)):
    for off, obj in zip((0x98, 0x9c), which):
        put(buf, off, 'I', obj if r.random() < 0.7 else 0)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    vt9c = vtable({}, size=8)
    put(vt9c, 0xc, 'I', recorder(22, 2))
    regions = {LOG: log, VT98: vtable({3: 2}, size=8), VT9C: vt9c, OBJ98: put(bytearray(r.bytes(0x20)), 0, 'I', VT98),
               OBJ9C: put(bytearray(r.bytes(0x20)), 0, 'I', VT9C)}
    for vt, obj, k in ((KVT98, KOBJ98, 4), (KVT9C, KOBJ9C, 23)):
        regions[vt] = put(vtable({}, size=8), 0xc, 'I', recorder(k, 2))
        regions[obj] = put(bytearray(r.bytes(0x20)), 0, 'I', vt)
    w = bytearray(r.bytes(0x140))
    old = bits(r)
    put(w, 0x138, 'I', old)
    objects(r, w)
    n = r.choice([0, 1, 2, 3, 4, 6])
    put(w, 0x5c, 'I', KIDS if n else 0)
    for i in range(n):
        kid = bytearray(r.bytes(KID))
        put(kid, 0x54, 'I', KIDS + KID * (i + 1) if i + 1 < n else 0)
        put(kid, 0x8c, 'I', r.choice([0x11100000 | r.randrange(5)] * 3 + [0x11100000 | r.randrange(32), r.getrandbits(32)]))
        objects(r, kid, (KOBJ98, KOBJ9C))
        regions[KIDS + KID * i] = kid
    regions[WINDOW] = w
    blob_bytes(log, 3, 1, 0xf)
    blob_bytes(log, 22, 1, 0xf)
    blob_bytes(log, 4, 1, 0x30)
    blob_bytes(log, 23, 1, 0x30)
    new = old if r.random() < 0.1 else old ^ r.getrandbits(5) if r.random() < 0.5 else bits(r)
    return Call(CALLS[op], [WINDOW, new], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
