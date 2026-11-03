# The dialog [0x7cb3f8]'s user slots shown (src/game/ui/dialog7cb3f8slots: Dialog7cb3f8_RefreshSlots 0x4a62b0) against the original. Window_FindChild real on nested children among decoys; recorders (driver.cpp's kDialog7cb3f8SlotsEnv): UserSlots_Instance 10, Records24_IfUsed 11 (its answers in turn, some none), Records24_Field14 12, Language_WideText 13, Window_SetShown 14; slot 9 of the windows 15.
# usage: dialog7cb3f8slots.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import VT, kids  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put, recorder, set_return, vtable  # noqa: E402

C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4a62b0, 0x4a657d), (0x65d7bc, 0x65d840)], hooks={0x48bcd0: C(10, 0), 0x5c7d00: _recorder(11, 1), 0x5c7d20: _recorder(12, 1), 0x485b00: C(13, 1), 0x65d9dd: _recorder(14, 1)})
DIALOG, TABLE, USED, KIDS = RAM + 0x100, RAM + 0x300, RAM + 0x400, RAM + 0x4000
SLOTS = (0x7a14, 0x7a19, 0x7a1a, 0x7a1b)


def case(r):
    log = log_region()
    regions = {LOG: log, 0x7cb3f8: struct.pack('<I', r.choice([DIALOG] * 12 + [0])), USED: r.bytes(0x40), TABLE: struct.pack('<40I', *[r.choice([USED, USED, 0]) for _ in range(40)])}
    regions[VT] = put(vtable({}, size=0x10), 0x24, 'I', recorder(15, 1))
    first, top = kids(r, regions, KIDS + 0x10000, SLOTS)
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    for g, slot in enumerate(SLOTS):
        at = KIDS + 0x4000 * g
        inner, group = kids(r, regions, at, (0x7a15, 0x7a17))
        put(regions[top[slot]], 0x5c, 'I', inner)
        below, _ = kids(r, regions, at + 0x1000, (0x7a16,))
        put(regions[group[0x7a15]], 0x5c, 'I', below)
    answers(log, 11, TABLE)
    set_return(log, 10, RAM + 0x10)
    set_return(log, 12, r.choice([0, 1, 0x10000, r.getrandbits(32)]))
    set_return(log, 13, r.getrandbits(32))
    return Call(0x4a62b0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
