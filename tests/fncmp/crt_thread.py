# The C library's thread data and thread start (src/game/core/crt: _getptd, _errno, _beginthread, _mtinit, _freeptd) against the originals. Their imports (GetLastError, TlsGetValue, TlsSetValue,
# GetCurrentThreadId, SetLastError, CreateThread, ResumeThread, TlsAlloc, InitializeCriticalSection) are COM recorders in their slots, calloc, _initptd, _amsg_exit and _dosmaperr cdecl recorders, the data blocks a case's region.
# usage: crt_thread.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _free, _recorder, heap, log_region, recorder, set_return  # noqa: E402

CALLOC, INITPTD, AMSG, DOSMAP = 24, 25, 26, 27
LASTERR, TLSGET, TLSSET, THREADID, SETERR, CREATE, RESUME, TLSALLOC, INITCS = 32, 33, 34, 35, 36, 37, 38, 39, 40
PTD = RAM
NHMALLOC = 28
B = Bench([(0x682460, 0x6824e0), (0x67e050, 0x67e0d0), (0x67dcd0, 0x67dcf0), (0x6823e0, 0x68243c), (0x6826f0, 0x68271b), (0x6824e0, 0x682590)],
          hooks={0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x67fcb0: _recorder(CALLOC, 2, cdecl=True), 0x682440: _recorder(INITPTD, 1, cdecl=True), 0x67efc0: _recorder(AMSG, 1, cdecl=True), 0x686980: _recorder(DOSMAP, 1, cdecl=True),
                 0x67ad90: _free})


def slots(r, log):
    set_return(log, LASTERR, r.choice([0, 0, 5, r.getrandbits(32)]))
    set_return(log, THREADID, r.getrandbits(32))
    return {0x6fd138: struct.pack('<I', recorder(LASTERR, 0, True)), 0x6fd1c4: struct.pack('<I', recorder(TLSGET, 1, True)), 0x6fd230: struct.pack('<I', recorder(TLSSET, 2, True)),
            0x6fd248: struct.pack('<I', recorder(THREADID, 0, True)), 0x6fd1c8: struct.pack('<I', recorder(SETERR, 1, True)), 0x6fd22c: struct.pack('<I', recorder(CREATE, 6, True)),
            0x6fd228: struct.pack('<I', recorder(RESUME, 1, True)), 0x6fd178: struct.pack('<I', recorder(TLSALLOC, 0, True)), 0x6fd144: struct.pack('<I', recorder(INITCS, 1, True))}


def mtinit(r):   # (_mtinitlocks runs too: the lock table's four sections initialised)
    log = log_region()
    regions = slots(r, log)
    set_return(log, TLSALLOC, r.choice([5, 5, 0xffffffff, r.getrandbits(32)]))
    set_return(log, CALLOC, r.choice([PTD, PTD, 0]))
    set_return(log, TLSSET, r.choice([1, 1, 0]))
    regions.update({LOG: log, 0x77ecd0: r.bytes(4), 0x77ece0: r.bytes(0xc0), PTD: r.bytes(0x80)})
    return Call(0x6823e0, [], regions, conv='c', ret='a')


def freeptd(r):
    log = log_region()
    regions = slots(r, log)
    set_return(log, TLSGET, r.choice([PTD, PTD, 0]))
    data = bytearray(r.bytes(0x74))
    for off in (0x24, 0x28, 0x30, 0x38, 0x40, 0x44, 0x50):
        struct.pack_into('<I', data, off, r.choice([0, r.getrandbits(32), 0x781490 if off == 0x50 else 0]))
    regions.update({LOG: log, HEAP: heap(0x40), 0x77ecd0: struct.pack('<I', r.choice([5, 5, 5, 0xffffffff])), PTD: data})
    return Call(0x6824e0, [r.choice([PTD, 0])], regions, conv='c', ret='v')


def getptd(r):
    log = log_region()
    regions = slots(r, log)
    set_return(log, TLSGET, r.choice([PTD, 0, 0, 0]))
    set_return(log, CALLOC, r.choice([PTD, PTD, 0]))
    set_return(log, TLSSET, r.choice([1, 1, 0]))
    regions.update({LOG: log, 0x77ecd0: r.bytes(4), PTD: r.bytes(0x20)})
    return Call(0x682460, [], regions, conv='c', ret='a')


def beginthread(r):
    log = log_region()
    regions = slots(r, log)
    set_return(log, CALLOC, r.choice([PTD, PTD, PTD, 0]))
    set_return(log, CREATE, r.choice([0x1234, r.getrandbits(32) | 1, 0]))
    set_return(log, RESUME, r.choice([0, 1, 0xffffffff]))
    regions.update({LOG: log, HEAP: heap(0x40), PTD: r.bytes(0x60)})
    return Call(0x67e050, [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='c', ret='a')


def malloc(r):
    log = log_region()
    set_return(log, NHMALLOC, r.getrandbits(32))
    return Call(0x67dcd0, [r.getrandbits(32)], {LOG: log, 0xfb1548: r.bytes(4)}, conv='c', ret='a')


KINDS = {'getptd': getptd, 'beginthread': beginthread, 'malloc': malloc, 'mtinit': mtinit, 'freeptd': freeptd}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    return KINDS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
