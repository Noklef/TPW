# The C library's rand, exit table, free, fclose and exception destructor (src/game/core/crt) against the originals. The routines under them (the thread's data, the locks,
# msize, realloc, the small block heap, _fclose_lk: CrtInnerEnv) are cdecl recorders at their game addresses; the small block heap's out arguments are stack slots, logged as
# 0 and filled, so what it writes is what the free call that follows logs. HeapFree is a COM recorder in its import slot.
# usage: crt_runtime.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, log_region, recorder, set_return, strings  # noqa: E402

GETPTD, LOCKEXIT, UNLOCKEXIT, MSIZE, REALLOC, LOCK, UNLOCK, FIND, FREEBLOCK, LOCKFILE, UNLOCKFILE, FCLOSE = range(2, 14)
HEAPFREE = 20
B = Bench([(0x686a00, 0x686a10), (0x67b5c0, 0x67b5f0), (0x67a740, 0x67a7f0), (0x67ad90, 0x67ae00), (0x67b620, 0x67b660), (0x67d320, 0x67d340)],
          hooks={0x682460: _recorder(GETPTD, 0, cdecl=True), 0x67fa20: _recorder(LOCKEXIT, 0, cdecl=True), 0x67fa30: _recorder(UNLOCKEXIT, 0, cdecl=True),
                 0x6818f0: _recorder(MSIZE, 1, cdecl=True), 0x681740: _recorder(REALLOC, 2, cdecl=True), 0x682720: _recorder(LOCK, 1, cdecl=True),
                 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682b40: _recorder(FIND, 3, cdecl=True), 0x682ba0: _recorder(FREEBLOCK, 3, cdecl=True),
                 0x6827c0: _recorder(LOCKFILE, 1, cdecl=True), 0x682830: _recorder(UNLOCKFILE, 1, cdecl=True), 0x67b660: _recorder(FCLOSE, 1, cdecl=True)})
del B.emu.hooks[0x67b5c0]   # (the bench's stand-in of rand: here the original runs)
TABLE, NEW, DATA = RAM, RAM + 0x400, RAM + 0x800
END = 0xfb3cb0


def rand(r):
    log = log_region()
    set_return(log, GETPTD, DATA)
    return Call(0x67b5c0, [], {LOG: log, DATA: r.bytes(0x20)}, conv='c', ret='a')


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


KINDS = {'errno': errno, 'rand': rand,'onexit': onexit, 'atexit': atexit, 'free': free, 'fclose': fclose, 'exception_dtor': exception_dtor}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    return KINDS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
