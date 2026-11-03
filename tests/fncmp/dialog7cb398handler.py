# The handler of the window [0x7cb398] (src/game/ui/dialog7cb398: 0x4a4830) against the original: every call a recorder numbered as Dialog7cb398Env from 10, Window_FindChild answering objects in turn whose slot 7 is recorder 27.
# usage: dialog7cb398handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x4a4830, 0x4a4aa8)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x65d7d0: _recorder(11, 1), 0x519510: _recorder(12, 0), 0x4c9130: _recorder(13, 0, cdecl=True), 0x4c94c0: _recorder(14, 0), 0x4c9750: _recorder(15, 1), 0x4c9670: _recorder(16, 1),
                 0x4a4ab0: _recorder(17, 0, cdecl=True), 0x485b00: _recorder(18, 1, cdecl=True), 0x485d20: _recorder(19, 3, cdecl=True), 0x486b40: _recorder(20, 1, cdecl=True), 0x496620: _recorder(21, 2, cdecl=True), 0x495aa0: _recorder(22, 2, cdecl=True),
                 0x493530: _recorder(23, 1, cdecl=True), 0x5195d0: _recorder(24, 0), 0x4d05d0: _recorder(25, 1), 0x488ba0: _recorder(26, 4, cdecl=True)})
WINDOW, CHILDREN, VT, OBJECTS = RAM, RAM + 0x100, RAM + 0x300, RAM + 0x1000


def case(r):
    log = log_region()
    regions = {LOG: log}
    regions[VT] = put(vtable({}, size=0x10), 0x1c, 'I', recorder(27, 1))
    regions[OBJECTS] = struct.pack('<I', VT) * 40 * 4
    regions[CHILDREN] = struct.pack('<40I', *[OBJECTS + 0x10 * i for i in range(40)])
    answers(log, 11, CHILDREN)
    people, total = r.choice([0, 5, r.getrandbits(32)]), r.choice([0, 1000, r.getrandbits(32)])
    set_return(log, 13, people)
    set_return(log, 14, total)
    set_return(log, 15, r.choice([0, 33, 100, r.randrange(200), r.getrandbits(32)]))
    set_return(log, 16, r.choice([0, 33, 100, r.randrange(200), r.getrandbits(32)]))
    for k in (12, 18, 24, 26):
        set_return(log, k, r.getrandbits(32))
    regions[0x7cb360] = struct.pack('<I', r.choice([people, r.getrandbits(32)]))
    regions[0x7cb378] = struct.pack('<I', r.choice([total, r.getrandbits(32)]))
    regions[0x7cb398] = struct.pack('<I', r.choice([0, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]))
    regions[0x7cf83c] = r.bytes(4)
    message = r.deal([5, 0x10, 0x10, 0x10, 0x10, 0x10, 0x14, 0x15, 0x100, 0x100, 0x100, 0x100, 0x100, 0x800, 0x13, 0x101, 0x801, r.getrandbits(32)])
    if message == 0x10:
        a = r.choice([0x80083, 0x80083, 0x80083, 0x80082, r.getrandbits(32)])
    else:
        a = r.choice([0xffffffff, 0xfffffffe, 0x4a68c5, 0x4a68c6, 0x4a68c7, 0x4a68c8, 0x4a68c9, r.getrandbits(32)])
    return Call(0x4a4830, [WINDOW, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
