# The window on a stream (src/game/files/substream) against the originals. The constructors: a stream at STREAM whose source is a
# RefObject (StreamBase_CopyCtor's AddRef: InterlockedIncrement a recorder), CreateSemaphoreA a recorder answering a handle. The read,
# seek and rewind: the object at SELF (its vtable's slot 2 a recorder, k 20), position and start small or any, its stream a recorder
# vtable (k 2 read answering any count, 4 seek any position, 6 tell answering start + position or not), the semaphore's
# WaitForSingleObject and ReleaseSemaphore recorders. The destructor (0x6c7960): its stream's source REF (a RefObject: InterlockedDecrement
# answering 0 or more, its slots 0 and 9 recorders) or none, CloseHandle a recorder. The voice stream's window read (0x6c8070, the voice
# stream sample's slot 1 of its window at +0x40): the window as the read's inside a sample at OBJ, its length (+0x18) about the position.
# usage: substream.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, recorder, set_return  # noqa: E402

B = Bench([(0x6d4d10, 0x6d4d72), (0x6d4d80, 0x6d4de2), (0x6d4df0, 0x6d4e90), (0x6d4e90, 0x6d4eb8), (0x6d4ec0, 0x6d4ed9), (0x5f71b0, 0x5f71d1),
           (0x618d20, 0x618d2b), (0x6307c0, 0x630877), (0x6c7960, 0x6c79ae), (0x6c8070, 0x6c80bf), (0x5f71e0, 0x5f7208), (0x618d30, 0x618d72)])
OBJ, REF_VT = RAM + 0x700, RAM + 0x600
SELF, STREAM, REF, SELF_VT, INNER_VT, BUFFER = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500
# import: (recorder slot, import table address, stack arguments)
IMPORTS = {'CreateSemaphoreA': (51, 0x6fd16c, 4), 'CloseHandle': (50, 0x6fd13c, 1), 'WaitForSingleObject': (52, 0x6fd128, 2),
           'ReleaseSemaphore': (53, 0x6fd170, 3), 'InterlockedIncrement': (54, 0x6fd0c0, 1), 'InterlockedDecrement': (55, 0x6fd12c, 1)}
CALLS = {'ctor': 0x6d4d10, 'ctorat': 0x6d4d80, 'read': 0x6d4df0, 'seek': 0x6d4e90, 'rewind': 0x6d4ec0, 'dtor': 0x6c7960, 'vread': 0x6c8070}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def table(slots):
    return struct.pack('<32I', *[slots.get(i, 0) for i in range(32)])


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, SELF_VT: table({2: recorder(20, 1)}), INNER_VT: table({2: recorder(2, 2), 4: recorder(4, 2), 6: recorder(6, 0)}),
               REF: struct.pack('<I', RAM + 0x7f0) + r.bytes(0x1c), BUFFER: r.bytes(0x40)}
    for k, slot, n in IMPORTS.values():
        regions[slot] = struct.pack('<I', recorder(k, n, True))
    set_return(log, 51, r.getrandbits(32))
    set_return(log, 52, r.choice([0, 0, 0x102, 0xffffffff]))
    me = bytearray(r.bytes(0x20))
    regions[SELF] = me
    if name in ('ctor', 'ctorat'):
        regions[STREAM] = struct.pack('<II', r.getrandbits(32), REF) + r.bytes(8)
        args = [SELF, STREAM] + ([r.getrandbits(32)] if name == 'ctorat' else [])
        return Call(CALLS[name], args, regions, conv='t', ret='a')
    position, start = (r.choice([0, r.randrange(0x100), r.getrandbits(32)]) for _ in range(2))
    struct.pack_into('<IIIII', me, 0, SELF_VT, position, start, INNER_VT, REF)
    set_return(log, 6, (start + position) & 0xffffffff if r.random() < 0.5 else r.choice([0, r.getrandbits(32)]))
    set_return(log, 2, r.choice([0, 4, r.randrange(0x40), 0xffffffff, r.getrandbits(32)]))
    set_return(log, 4, r.choice([0, start, (start + r.randrange(0x100)) & 0xffffffff, 0xffffffff, r.getrandbits(32)]))
    if name == 'dtor':
        struct.pack_into('<I', me, 0x10, r.choice([REF, REF, REF, 0]))
        regions[REF] = struct.pack('<II', REF_VT, 0) + r.bytes(0x28)   # (+4 the link: none)
        regions[REF_VT] = table({0: recorder(13, 1), 9: recorder(12, 0)})
        set_return(log, 55, r.choice([0, 0, 1, 2, 0xffffffff]))
        return Call(0x6c7960, [SELF], regions, conv='t')
    if name == 'vread':
        obj = bytearray(r.bytes(0x40)) + me
        length = r.choice([position, (position + r.randrange(1, 0x40)) & 0xffffffff, (position - 1) & 0xffffffff, r.getrandbits(32)])
        struct.pack_into('<I', obj, 0x18, length)
        regions[OBJ] = obj
        del regions[SELF]
        return Call(0x6c8070, [OBJ + 0x40, BUFFER, r.choice([0, 4, r.randrange(0x40), r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'read':
        return Call(0x6d4df0, [SELF, BUFFER, r.choice([0, 4, r.randrange(0x40), r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'seek':
        return Call(0x6d4e90, [SELF, r.choice([0, r.randrange(0x100), r.getrandbits(32)])], regions, conv='t', ret='a')
    return Call(0x6d4ec0, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
