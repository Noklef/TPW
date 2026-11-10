# The C library's rand, srand, exit table, free, fclose and exception destructor (src/game/core/crt) against the originals. The routines under them (the thread's data, the locks,
# msize, realloc, the small block heap, _fclose_lk: CrtInnerEnv) are cdecl recorders at their game addresses; the small block heap's out arguments are stack slots, logged as
# 0 and filled, so what it writes is what the free call that follows logs. HeapFree is a COM recorder in its import slot. Also _ValidateRead / Write / Execute and
# __CxxUnhandledExceptionFilter over their IsBad*Ptr imports and the filter before it (COM recorders).
# usage: crt_runtime.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, log_region, recorder, set_return, strings  # noqa: E402

GETPTD, LOCKEXIT, UNLOCKEXIT, MSIZE, REALLOC, LOCK, UNLOCK, FIND, FREEBLOCK, LOCKFILE, UNLOCKFILE, FCLOSE = range(2, 14)
HEAPFREE, SETFILTER, AMSG, NHMALLOC = 20, 21, 26, 28
BADREAD, BADWRITE, BADCODE, PREV = 14, 15, 16, 17
B = Bench([(0x67d380, 0x67d39e), (0x67d230, 0x67d24e), (0x67d350, 0x67d37f), (0x682360, 0x682385), (0x68f990, 0x68f9cf), (0x67fa40, 0x67fa60), (0x686a00, 0x686a10), (0x67b5c0, 0x67b5f0), (0x67b5b0, 0x67b5bd), (0x67a740, 0x67a7f0), (0x67ad90, 0x67ae00), (0x67b620, 0x67b660), (0x67d320, 0x67d340),
           (0x67a7f0, 0x67a826), (0x687c20, 0x687c31), (0x687c40, 0x687c4d), (0x67dcd0, 0x67dcf0),
           (0x68cbd0, 0x68cbf0), (0x68cbf0, 0x68cc10), (0x68cc10, 0x68cc2b), (0x687bc0, 0x687c11)],
          hooks={0x682460: _recorder(GETPTD, 0, cdecl=True), 0x67fa20: _recorder(LOCKEXIT, 0, cdecl=True), 0x67fa30: _recorder(UNLOCKEXIT, 0, cdecl=True),
                 0x6818f0: _recorder(MSIZE, 1, cdecl=True), 0x681740: _recorder(REALLOC, 2, cdecl=True), 0x682720: _recorder(LOCK, 1, cdecl=True),
                 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682b40: _recorder(FIND, 3, cdecl=True), 0x682ba0: _recorder(FREEBLOCK, 3, cdecl=True),
                 0x6827c0: _recorder(LOCKFILE, 1, cdecl=True), 0x682830: _recorder(UNLOCKFILE, 1, cdecl=True), 0x67b660: _recorder(FCLOSE, 1, cdecl=True),
                 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x67efc0: _recorder(AMSG, 1, cdecl=True)})
del B.emu.hooks[0x67b5c0]   # (the bench's stand-in of rand: here the original runs)
TABLE, NEW, DATA = RAM, RAM + 0x400, RAM + 0x800
END = 0xfb3cb0


def rand(r):
    log = log_region()
    set_return(log, GETPTD, DATA)
    return Call(0x67b5c0, [], {LOG: log, DATA: r.bytes(0x20)}, conv='c', ret='a')


def srand(r):
    log = log_region()
    set_return(log, GETPTD, DATA)
    return Call(0x67b5b0, [r.getrandbits(32)], {LOG: log, DATA: r.bytes(0x20)}, conv='c', ret='v')


def exit_table(r, address):
    log = log_region()
    used = r.choice([0, 1, 4, 9, 20])
    # (msize: room for one more, exactly none, too little; a used table of odd bytes now and then: the difference's low bits)
    extra = r.choice([0, 1, 3]) if r.random() < 0.15 else 0
    set_return(log, MSIZE, used * 4 + extra + r.choice([4, 5, 0x20, 3, 0, 0]))
    set_return(log, REALLOC, r.choice([NEW, NEW, NEW, 0]))
    regions = {LOG: log, END: struct.pack('<II', TABLE + used * 4 + extra, TABLE), TABLE: r.bytes(0x100), NEW: r.bytes(0x100)}
    return Call(address, [r.choice([r.getrandbits(32) | 1, 0])], regions, conv='c', ret='a')


def onexit(r):
    return exit_table(r, 0x67a740)


def atexit(r):
    return exit_table(r, 0x67a7d0)


def onexitinit(r):   # (no block: _amsg_exit returns and the first entry is written through 0, a crash in both)
    log = log_region()
    set_return(log, NHMALLOC, TABLE if r.random() < 0.9 else 0)
    return Call(0x67a7f0, [], {LOG: log, END: r.bytes(8), TABLE: r.bytes(8), 0xfb1548: r.bytes(4)}, conv='c', ret='v')


def filter_(r):   # (SetUnhandledExceptionFilter a COM recorder in its slot)
    log = log_region()
    set_return(log, SETFILTER, r.getrandbits(32))
    regions = {LOG: log, 0xfb1454: r.bytes(4), 0x6fd1a4: struct.pack('<I', recorder(SETFILTER, 1, True))}
    return Call(r.choice([0x687c20, 0x687c40]), [], regions, conv='c', ret='v')


def validate(r):   # (IsBadReadPtr, IsBadWritePtr, IsBadCodePtr COM recorders in their slots)
    log = log_region()
    for k in (BADREAD, BADWRITE, BADCODE):
        set_return(log, k, r.choice([0, 0, 1, r.getrandbits(32)]))
    regions = {LOG: log, 0x6fd1d8: struct.pack('<I', recorder(BADREAD, 2, True)), 0x6fd25c: struct.pack('<I', recorder(BADWRITE, 2, True)), 0x6fd280: struct.pack('<I', recorder(BADCODE, 1, True))}
    address = r.choice([0x68cbd0, 0x68cbf0, 0x68cc10])
    return Call(address, [r.getrandbits(32) for _ in range(1 if address == 0x68cc10 else 2)], regions, conv='c', ret='a')


def unhandled(r):   # (a C++ exception, all three tests passed, goes to terminate 0x682590, the EH runtime: not benched, one test always fails here)
    log = log_region()
    record = [r.choice([0xe06d7363, 0xe06d7363, 0xc0000005]), 0, 0, 0, r.choice([3, 3, 2]), r.choice([0x19930520, 0x19930520, 0x19930521])]
    if record[0] == 0xe06d7363 and record[4] == 3 and record[5] == 0x19930520:
        record[r.choice([0, 4, 5])] ^= 1
    set_return(log, BADCODE, r.choice([0, 0, 1, r.getrandbits(32)]))
    set_return(log, PREV, r.getrandbits(32))
    regions = {LOG: log, TABLE: struct.pack('<2I', DATA, NEW), DATA: struct.pack('<6I', *record) + r.bytes(0x10), 0xfb1454: struct.pack('<I', r.choice([0, recorder(PREV, 1, True)])),
               0x6fd280: struct.pack('<I', recorder(BADCODE, 1, True))}
    return Call(0x687bc0, [TABLE], regions, conv='s', ret='a')


def free(r):
    log = log_region()
    strings(log, FIND, 0, out=6)
    hit = r.random() < 0.6
    set_return(log, FIND, r.getrandbits(32) | 1 if hit else 0)
    fill(log, FIND, 1, r.bytes(4), which=0)
    fill(log, FIND, 2, r.bytes(4), which=1)
    regions = {LOG: log, 0xfb3cac: r.bytes(4), 0x6fd210: struct.pack('<I', recorder(HEAPFREE, 3, True))}
    return Call(0x67ad90, [r.choice([r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1, 0])], regions, conv='c', ret='v')


def fclose(r):
    log = log_region()
    set_return(log, FCLOSE, r.getrandbits(32))
    file = bytearray(r.bytes(0x20))
    if r.random() < 0.6:
        file[0xc] &= ~0x40 & 0xff
    return Call(0x67b620, [DATA], {LOG: log, DATA: file}, conv='c', ret='a')


def exception_dtor(r):
    h = heap()
    obj = bytearray(r.bytes(0xc))
    struct.pack_into('<II', obj, 4, HEAP + 0x100, r.choice([0, 1, r.getrandbits(32)]))
    return Call(0x67d320, [DATA], {HEAP: h, DATA: obj}, conv='t', ret='v')


def errno(r):
    log = log_region()
    set_return(log, GETPTD, r.getrandbits(32))
    return Call(0x686a00, [], {LOG: log}, conv='c', ret='a')


def type_info_dtor(r):
    log = log_region()
    strings(log, FIND, 0, out=6)
    set_return(log, FIND, r.choice([0, r.getrandbits(32) | 1]))
    fill(log, FIND, 1, r.bytes(4), which=0)
    fill(log, FIND, 2, r.bytes(4), which=1)
    regions = {LOG: log, 0xfb3cac: r.bytes(4), 0x6fd210: struct.pack('<I', recorder(HEAPFREE, 3, True)), DATA: struct.pack('<2I', r.getrandbits(32), r.choice([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))}
    if r.random() < 0.4:
        return Call(0x67d380, [DATA, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    return Call(0x67d350, [DATA], regions, conv='t', ret='v')


def adjust_pointer(r):
    pdisp = r.choice([-1, -2, -0x80000000, 0, 4, 8, 0x10, 0x20])
    vdisp = r.choice([0, 4, 8, 0x10])
    obj = bytearray(r.bytes(0x40))
    if pdisp >= 0:
        struct.pack_into('<I', obj, pdisp, NEW)
    return Call(0x682360, [DATA, TABLE], {DATA: obj, NEW: r.bytes(0x20), TABLE: struct.pack('<iii', r.choice([0, 4, 8, -4, r.getrandbits(31)]), pdisp, vdisp)}, conv='c', ret='a')


def siglookup(r):
    signals = [r.choice([2, 4, 8, 0xb, 0x16, r.getrandbits(32)]) for _ in range(0x20)]
    table = b''.join(struct.pack('<3I', r.getrandbits(32), s, r.getrandbits(32)) for s in signals)
    signal = r.choice([2, 4, 8, 0xb, 0x16, 3, signals[0], signals[1], signals[2]])
    count = r.choice([0, 1, 2, 3, 0xa, 0x20, 0x15555556, 0x15555555])   # (the last two: the table's size wraps at 32 bits)
    return Call(0x68f990, [signal, TABLE], {TABLE: table, 0x781514: struct.pack('<I', count)}, conv='c', ret='a')


def initterm(r):
    log = log_region()
    calls = [r.choice([0, recorder(21, 0, True), recorder(22, 0, True)]) for _ in range(8)]
    begin = TABLE + 4 * r.choice([0, 0, 1])
    end = TABLE + 4 * r.choice([0, 1, 2, 5, 8])
    return Call(0x67fa40, [begin, end], {LOG: log, TABLE: struct.pack('<8I', *calls)}, conv='c', ret='v')


def exception_delete(r):
    h = heap()
    h[0x200:0x20c] = struct.pack('<3I', r.getrandbits(32), HEAP + 0x100, r.choice([0, 1, r.getrandbits(32)]))
    return Call(0x67d230, [HEAP + 0x200, r.choice([0, 1, 2, 3, r.getrandbits(32)])], {HEAP: h}, conv='t', ret='a')


KINDS = {'excdelete': exception_delete, 'typeinfo': type_info_dtor, 'adjust': adjust_pointer, 'siglookup': siglookup, 'initterm': initterm, 'errno': errno, 'rand': rand, 'srand': srand, 'onexitinit': onexitinit, 'filter': filter_, 'onexit': onexit, 'atexit': atexit, 'free': free, 'fclose': fclose, 'exception_dtor': exception_dtor,
         'validate': validate, 'unhandled': unhandled}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    return KINDS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
