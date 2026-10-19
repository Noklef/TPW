# The binary semaphore and its scoped lock (src/game/core/semaphore: 0x6307c0-0x630876) against the originals: the four KERNEL32 imports
# (CreateSemaphoreA, CloseHandle, WaitForSingleObject, ReleaseSemaphore) are recorders in their import slots, so each call's arguments are
# compared; the wait answers signalled (0), timed out (0x102), failed (-1) or its hash. A lock held or not, kept or not.
# usage: semaphore.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor,LockCtor,LockDtor,Acquire,Release)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, log_region, recorder, set_return  # noqa: E402

B = Bench([(0x6307c0, 0x630877)])
SEM, LOCK = RAM, RAM + 0x10
# import: (recorder slot, import table address, stack arguments)
IMPORTS = {'CreateSemaphoreA': (51, 0x6fd16c, 4), 'CloseHandle': (50, 0x6fd13c, 1), 'WaitForSingleObject': (52, 0x6fd128, 2),
           'ReleaseSemaphore': (53, 0x6fd170, 3)}
CALLS = {'Ctor': (0x6307c0, 0, 'a'), 'Dtor': (0x6307e0, 0, 'a'), 'LockCtor': (0x6307f0, 2, 'a'), 'LockDtor': (0x630810, 0, 'v'),
         'Acquire': (0x630820, 1, 'a'), 'Release': (0x630850, 0, 'v')}


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    for k, slot, n in IMPORTS.values():
        regions[slot] = struct.pack('<I', recorder(k, n, True))
    if r.random() < 0.8:
        set_return(log, IMPORTS['WaitForSingleObject'][0], r.choice([0, 0, 0x102, 0xffffffff, 0x80]))
    lock = bytearray(r.bytes(0x10))
    struct.pack_into('<III', lock, 0, r.getrandbits(32), r.choice([0, 1, 1, r.getrandbits(32)]), r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[SEM] = bytearray(r.bytes(0x10))
    regions[LOCK] = lock
    address, nargs, ret = CALLS[op]
    if op == 'LockCtor':
        args = [LOCK, SEM, r.choice([0, 1, r.getrandbits(32)])]
    elif op == 'Acquire':
        args = [LOCK, r.choice([0, 1, r.getrandbits(32)])]
    else:
        args = [SEM if op in ('Ctor', 'Dtor') else LOCK]
    return Call(address, args, regions, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
