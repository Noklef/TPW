# Dialog7ca098_Create (src/game/ui/dialogs, 0x48d8b0) against the original. Its calls recorders (their own benches have them; driver.cpp's kDialog7ca098CreateEnv): WindowScript_Build thiscall 10, Window_Unlink 11, Window_SetDwordA0 12,
# Window_SetShown 13, GameHeap_Alloc 14, TextPart_Ctor 15, Window_SetImpl 16, Language_Font 17, Window_Set4c 18, Window_Notify3000 19, Window_NotifyC00 20; Window_FindChild runs. The child there or not, the heap empty or not.
# usage: dialog7ca098create.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

B = Bench([(0x48d8b0, 0x48d98c), (0x65d7bc, 0x65d7d0), (0x65d7d0, 0x65d840)],
          hooks={0x65fd0e: _recorder(10, 2), 0x65f538: _recorder(11, 0), 0x65e6d5: _recorder(12, 2), 0x65d9dd: _recorder(13, 1), 0x67acc0: _recorder(14, 1, cdecl=True), 0x48f6a0: _recorder(15, 1),
                 0x65d1c8: _recorder(16, 1), 0x485a70: _recorder(17, 1, cdecl=True), 0x65c388: _recorder(18, 1), 0x65c4b9: _recorder(19, 2), 0x65c428: _recorder(20, 2)})
WINDOW = RAM + 0x100
KIDS = RAM + 0x400
ID = 0x10d32


def case(r):
    log = log_region()
    set_return(log, 10, WINDOW)
    set_return(log, 14, r.choice([RAM + 0x1000, RAM + 0x1000, RAM + 0x1000, 0]))
    set_return(log, 15, r.getrandbits(32))
    set_return(log, 17, r.getrandbits(32))
    # The children: a few others, then the one or not (a last one's next is 0: the search ends).
    count = r.choice([1, 2, 3])
    found = r.choice([True, True, True, False])
    kids = bytearray()
    for k in range(count):
        kid = bytearray(r.bytes(0x100))
        last = k == count - 1
        put(kid, 0x8c, 'I', ID if last and found else r.choice([0, ID - 1, ID + 1, ID + 0x10000, r.getrandbits(32) & 0xfffffff0]))
        put(kid, 0x54, 'I', 0 if last else KIDS + 0x100 * (k + 1))
        kids += kid
    regions = {LOG: log, KIDS: bytes(kids), WINDOW: bytes(put(bytearray(r.bytes(0x100)), 0x5c, 'I', r.choice([KIDS, KIDS, KIDS, KIDS, 0]))), 0xfaa5d8: r.bytes(4), 0x7ca098: r.bytes(4)}
    return Call(0x48d8b0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
