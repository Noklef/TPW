# The GUI library's kind 9 window (vtable 0x705c28) against the originals (src/game/ui/window705c28: 0x66d885). SetValue: the value +0x140
# 0-0x400 or any, the new one equal to it now and then, else near the ends or any; the objects +0x98 / +0x9c recorders (slot 3: k 3 and
# 22) or 0, each logging its event as the hash of its dword +0 (the rest is stack garbage, not compared).
# usage: window705c28.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetValue)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob_bytes, log_region, put, recorder, vtable  # noqa: E402

CALLS = {'SetValue': 0x66d885}
B = Bench([(0x66d885, 0x66d950)])
WINDOW, OBJ98, OBJ9C, VT98, VT9C = RAM, RAM + 0x200, RAM + 0x240, RAM + 0x280, RAM + 0x300


def value(r):
    return r.choice([0, 1, -1, 0x3ff, 0x400, 0x401, r.randrange(0, 0x401), 0x7fffffff, -0x80000000, r.getrandbits(32)]) & 0xffffffff


def case(r):
    op = r.deal(OPS)
    log = log_region()
    w = bytearray(r.bytes(0x160))
    old = value(r)
    put(w, 0x140, 'I', old)
    vt9c = vtable({}, size=8)
    put(vt9c, 0xc, 'I', recorder(22, 2))
    regions = {LOG: log, WINDOW: w, VT98: vtable({3: 2}, size=8), VT9C: vt9c,
               OBJ98: put(bytearray(r.bytes(0x20)), 0, 'I', VT98), OBJ9C: put(bytearray(r.bytes(0x20)), 0, 'I', VT9C)}
    for off, obj in ((0x98, OBJ98), (0x9c, OBJ9C)):
        put(w, off, 'I', obj if r.random() < 0.8 else 0)
    blob_bytes(log, 3, 1, 0xf)
    blob_bytes(log, 22, 1, 0xf)
    return Call(CALLS[op], [WINDOW, old if r.random() < 0.15 else value(r)], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
