# Compares window705c58's functions (the GUI library's window subclass 0x705c58) with the originals.
# SetValue (0x66da4d): a window (0x140 random bytes) whose objects +0x98 / +0x9c are recorders of slot 3 (k 3 and 22) or 0, the value small
# or any. Both objects are told twice: once with message 1 (an event whose only word set is +4) and once by Window_Notify80 (message 0x80,
# only the dword +0 set). Each recorder hashes only the bytes set: the word +4 by default (blob_bytes), the dword +0 for message 0x80
# (blob_when on the message argument). The rest of each event is stack garbage and is not compared.
# usage: window705c58.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetValue)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob_bytes, blob_when, log_region, put, recorder, vtable  # noqa: E402

CALLS = {'SetValue': 0x66da4d}
B = Bench([(0x661525, 0x6615a3), (0x66da4d, 0x66dad2)])
WINDOW, OBJ98, OBJ9C, VT98, VT9C = RAM, RAM + 0x200, RAM + 0x240, RAM + 0x280, RAM + 0x300


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VT98: put(vtable({}, size=8), 0xc, 'I', recorder(3, 2)), VT9C: put(vtable({}, size=8), 0xc, 'I', recorder(22, 2)),
               OBJ98: put(bytearray(r.bytes(0x20)), 0, 'I', VT98), OBJ9C: put(bytearray(r.bytes(0x20)), 0, 'I', VT9C)}
    w = bytearray(r.bytes(0x140))
    for off, obj in ((0x98, OBJ98), (0x9c, OBJ9C)):
        put(w, off, 'I', obj if r.random() < 0.75 else 0)
    regions[WINDOW] = w
    for entry, k in enumerate((3, 22)):
        blob_bytes(log, k, 1, 0x30)
        blob_when(log, k, 0, 0x80, 0xf, entry)
    value = r.choice([r.randrange(-3, 300), r.getrandbits(16), r.getrandbits(32), 0, 0xffff, 0x10000, 0x7fffffff, 0x80000000]) & 0xffffffff
    return Call(CALLS[op], [WINDOW, value], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
