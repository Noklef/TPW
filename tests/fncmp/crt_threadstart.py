# The C library's thread entries and ends (src/game/core/crt: _beginthreadex, _threadstart(ex), _endthread(ex)) against the originals. _getptd, calloc, _initptd, _amsg_exit and _dosmaperr
# are cdecl recorders (as the driver's CrtInnerEnv), _freeptd runs; the imports (TlsSetValue, GetCurrentThreadId, CreateThread, GetLastError, CloseHandle, ExitThread) COM recorders in their
# slots. ExitThread returns: the originals' tails (5e c3, past the CALL taken for no-return) are given to the emulator. The exception path (_XcptFilter, _exit) is not benched.
# usage: crt_threadstart.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _free, _recorder, heap, log_region, recorder, set_return  # noqa: E402

GETPTD, CALLOC, INITPTD, AMSG, DOSMAP = 2, 24, 25, 26, 27
PROC, TERM, FPINIT = 14, 15, 16
TLSGET, TLSSET, THREADID, LASTERR, CREATE, CLOSE, EXIT = 33, 34, 35, 32, 37, 41, 42
PTD = RAM
B = Bench([(0x6824e0, 0x682590), (0x67e0d0, 0x67e177), (0x67e180, 0x67e1c3), (0x67f780, 0x67f7fb), (0x67f800, 0x67f8ad), (0x67f8b0, 0x67f8e7)],
          hooks={0x682460: _recorder(GETPTD, 0, cdecl=True), 0x67fcb0: _recorder(CALLOC, 2, cdecl=True), 0x682440: _recorder(INITPTD, 1, cdecl=True), 0x67efc0: _recorder(AMSG, 1, cdecl=True),
                 0x686980: _recorder(DOSMAP, 1, cdecl=True), 0x67ad90: _free})
B.emu.code.update({0x67e1c1: ('POP', 'ESI'), 0x67e1c2: ('RET', ''), 0x67f8e5: ('POP', 'ESI'), 0x67f8e6: ('RET', '')})
B.emu.next.update({0x67e1bb: 0x67e1c1, 0x67e1c1: 0x67e1c2, 0x67e1c2: 0x67e1c3, 0x67f8df: 0x67f8e5, 0x67f8e5: 0x67f8e6, 0x67f8e6: 0x67f8e7})
KINDS = {'beginthreadex': 2, 'threadstart': 2, 'threadstartex': 2, 'endthread': 1, 'endthreadex': 1}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k for _ in range(KINDS[k])]


def slots(r, log):
    set_return(log, LASTERR, r.choice([0, 0, 5, r.getrandbits(32)]))
    set_return(log, THREADID, r.getrandbits(32))
    set_return(log, TLSGET, r.choice([PTD, PTD, 0]))
    set_return(log, TLSSET, r.choice([1, 1, 1, 0]))
    return {0x6fd138: struct.pack('<I', recorder(LASTERR, 0, True)), 0x6fd1c4: struct.pack('<I', recorder(TLSGET, 1, True)), 0x6fd230: struct.pack('<I', recorder(TLSSET, 2, True)),
            0x6fd248: struct.pack('<I', recorder(THREADID, 0, True)), 0x6fd22c: struct.pack('<I', recorder(CREATE, 6, True)), 0x6fd13c: struct.pack('<I', recorder(CLOSE, 1, True)),
            0x6fd234: struct.pack('<I', recorder(EXIT, 1, True))}


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = slots(r, log)
    regions.update({LOG: log, HEAP: heap(0x40), 0x77ecd0: struct.pack('<I', r.choice([5, 5, 5, 0xffffffff]))})
    if name == 'beginthreadex':
        set_return(log, CALLOC, r.choice([PTD, PTD, PTD, 0]))
        set_return(log, CREATE, r.choice([0x1234, r.getrandbits(32) | 1, 0]))
        regions[PTD] = r.bytes(0x80)
        return Call(0x67f780, [r.getrandbits(32) for _ in range(6)], regions, conv='c', ret='a')
    set_return(log, GETPTD, r.choice([PTD, PTD, PTD, 0]) if name.startswith('end') else PTD)
    set_return(log, PROC, r.getrandbits(32))
    data = bytearray(r.bytes(0x74))   # (_freeptd's buffers null or the default locale's; the handle -1 or not)
    for off in (0x24, 0x28, 0x30, 0x38, 0x40, 0x44, 0x50):
        struct.pack_into('<I', data, off, r.choice([0, 0, 0x781490 if off == 0x50 else 0]))
    struct.pack_into('<I', data, 4, r.choice([0xffffffff, 0x1234, r.getrandbits(32)]))
    struct.pack_into('<II', data, 0x48, recorder(PROC, 1, com=name == 'threadstartex', cdecl=name == 'threadstart'), r.getrandbits(32))
    regions[PTD] = data
    regions[0x77e71c] = struct.pack('<II', r.choice([0, recorder(FPINIT, 0, cdecl=True)]), r.choice([0, recorder(TERM, 0, cdecl=True)]))
    if name == 'endthread':
        return Call(0x67e180, [], regions, conv='c', ret='v')
    if name == 'endthreadex':
        return Call(0x67f8b0, [r.getrandbits(32)], regions, conv='c', ret='v')
    return Call(0x67e0d0 if name == 'threadstart' else 0x67f800, [PTD], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)
