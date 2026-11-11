# Panel7cc328_Refresh (src/game/ui/panel7cc328, 0x4b4a50) against the original. Every callee a recorder (driver.cpp's kPanel7cc328RefreshCalls), the panel's
# slot 8 a vtable recorder. Window_FindChild answers one window for every id; Thing_GetHandle's out argument (a stack address) logged as 0, its answer a
# word in RAM; the GUI window's name hashed. The group count small, negative or any; the tab [0x7cc358] 0-4 mostly, else past 4.
# usage: panel7cc328refresh.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, strings, vtable  # noqa: E402

# (thiscall 10-14, 16-18, 21, 23-25; cdecl 15, 19, 20, 22: driver.cpp's kPanel7cc328RefreshCalls)
COUNT, FIND, SHOWN, BUILD, BOTTOM, BY_NAME, HIGHLIGHT, HANDLER, SORT, WIDE, BUTTON, HANDLE, SWEEP, REFRESH, RADIO, BIND = range(10, 26)
B = Bench([(0x4b4a50, 0x4b4c68)],
          hooks={0x48ca00: _recorder(COUNT, 1), 0x65d7d0: _recorder(FIND, 1), 0x65d9dd: _recorder(SHOWN, 1), 0x6639cb: _recorder(BUILD, 2), 0x6636b2: _recorder(BOTTOM, 2),
                 0x659bc9: _recorder(BY_NAME, 1, cdecl=True), 0x662eee: _recorder(HIGHLIGHT, 1), 0x65e526: _recorder(HANDLER, 1), 0x6659af: _recorder(SORT, 2),
                 0x485b00: _recorder(WIDE, 1, cdecl=True), 0x486660: _recorder(BUTTON, 7, cdecl=True), 0x50b350: _recorder(HANDLE, 1), 0x481bc0: _recorder(SWEEP, 2, cdecl=True),
                 0x663324: _recorder(REFRESH, 0), 0x66a295: _recorder(RADIO, 1), 0x48d290: _recorder(BIND, 1)})
SELF, VT, WORD = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    set_return(log, COUNT, r.choice([0, 1, 1, 2, 2, 3, 0xffffffff, 0x80000000, r.getrandbits(32)]))
    set_return(log, FIND, r.getrandbits(32))
    set_return(log, BY_NAME, r.getrandbits(32))
    set_return(log, WIDE, r.getrandbits(32))
    set_return(log, HANDLE, WORD)
    strings(log, HANDLE, 0, out=1)
    strings(log, BY_NAME, 1)
    globs = bytearray(r.bytes(0x50))   # 0x7cc310..0x7cc360: the area, the list, the group, the tab +0x48
    put(globs, 0x48, 'I', r.choice(list(range(5)) * 3 + [5, 0xffffffff, r.getrandbits(32)]))
    regions = {LOG: log, SELF: put(bytearray(r.bytes(0x40)), 0, 'I', VT), VT: vtable({8: 0}), WORD: r.bytes(4), 0x7cc310: globs, 0x7c2658: r.bytes(4),
               0x756f3c: r.bytes(8), 0x74fa84: bytes(r.choice(b'abcDGlo') for _ in range(r.randrange(8))).ljust(8, b'\0')}
    return Call(0x4b4a50, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
