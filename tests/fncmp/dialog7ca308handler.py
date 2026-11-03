# The handler of the window [0x7ca308] (src/game/ui/dialog7ca308: 0x49b650) against the original: every call a recorder numbered as Dialog7ca308HandlerEnv from 10, Window_FindChild answering objects in turn (a checked one 0 now and then) whose slots 7 and 9 are recorders 27 and 28.
# usage: dialog7ca308handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, blob_bytes, fill, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x49b650, 0x49b9f4)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x65d7d0: _recorder(11, 1), 0x5195d0: _recorder(12, 0), 0x6ad810: _recorder(13, 0), 0x6acd60: _recorder(14, 5, cdecl=True), 0x475550: _recorder(15, 1, cdecl=True), 0x486b40: _recorder(16, 1, cdecl=True),
                 0x65ef90: _recorder(17, 2), 0x65f024: _recorder(18, 1), 0x4acc70: _recorder(19, 2, cdecl=True), 0x664c71: _recorder(20, 2), 0x507bd0: _recorder(21, 1), 0x5077f0: _recorder(22, 0), 0x49ba00: _recorder(23, 1, cdecl=True),
                 0x507bf0: _recorder(24, 1), 0x486b00: _recorder(25, 1, cdecl=True), 0x488ba0: _recorder(26, 4, cdecl=True)})
WINDOW, CHILDREN, ENTRIES, VT, OBJECTS, ENTRY = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000, RAM + 0x2000


def case(r):
    log = log_region()
    regions = {LOG: log}
    regions[VT] = put(put(vtable({}, size=0x10), 0x1c, 'I', recorder(27, 1)), 0x24, 'I', recorder(28, 1))
    regions[OBJECTS] = struct.pack('<I', VT) * 40 * 4
    first = ENTRY if r.random() < 0.85 else 0
    second = ENTRY + 0x40 if r.random() < 0.85 else 0
    kids = [OBJECTS + 0x10 * i for i in range(40)]
    if r.random() < 0.1:
        kids[0] = 0   # (the list)
    if r.random() < 0.15:
        kids[1] = 0   # (0x2481)
    if r.random() < 0.15:
        kids[3 if kids[1] and first else 2] = 0   # (0x2485)
    regions[CHILDREN] = struct.pack('<40I', *kids)
    answers(log, 11, CHILDREN)
    regions[ENTRIES] = struct.pack('<40I', first, second, *[0] * 38)
    answers(log, 21, ENTRIES)
    regions[ENTRY] = r.bytes(0x10)
    regions[ENTRY + 0x40] = r.bytes(0x10)
    set_return(log, 12, r.getrandbits(32))
    set_return(log, 13, r.choice([0, 1, 5000, 0xffffffff, 0x80000000, -r.randrange(5000) & 0xffffffff, r.getrandbits(32)]))
    set_return(log, 17, r.choice([0, 0xffff0000, 1, r.getrandbits(32)]))
    set_return(log, 22, r.choice([0, 100, r.randrange(1000), r.getrandbits(32)]))
    set_return(log, 26, r.getrandbits(32))
    blob_bytes(log, 14, 4, 0xfffff)
    strings(log, 20, 0, 1 << 1)
    if r.random() < 0.85:
        fill(log, 20, 1, r.bytes(4))
    struct.pack_into('<I', log, LOG_BYTES + 4 * 19, 1 << 17)   # (Dialog7cc1f8_Open's second argument: a word over the caller's garbage)
    for at in (0x7ca2f0, 0x7ca2fc, 0x7ca300, 0x7ca308, 0x7cf83c, 0x78ba94, 0x7523e4, 0x7523e8, 0x7523ec):
        regions[at] = r.bytes(4)
    regions[0x7ca310] = struct.pack('<I', r.choice([0, r.getrandbits(32)]))
    message = r.deal([5, 0x10, 0x10, 0x10, 0x14, 0x15, 0x100, 0x100, 0x400, 0x400, 0x401, 0x401, 0x401, 0x401, 0x401, 0x406, 0x13, 0x101, 0x402, 0x405, r.getrandbits(32)])
    a = r.choice([0x80080, 0x80080, 0x2495, 0xfffffffe, 0xffffffff, 0x2496, r.getrandbits(32)])
    b = r.choice([0xffffffff, 0, 1, r.randrange(20), r.getrandbits(32)])
    return Call(0x49b650, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
