# Panel7c2534_AddRow (src/game/ui/panel7c2534, 0x492f60) against the original. Every call a recorder (their own benches have them; driver.cpp's kPanel7c2534Env): GuiText_Measure 10 (the rectangle's dwords hashed, the box out, the font and the alignments
# their low bytes), Window_GetRect 11 (the rectangle out; its words +4, +8 filled), Window_CreateChild 12, Window_SetHandler 13, TextPool_Dup 14, Window_SetDwordA0 15, GameHeap_Alloc 16, TextPart6ff530_Ctor 17, Window_SetImpl 18, Language_Font 19,
# Window_Set4c 20, Window_NotifyC00 21, the child's slot 0x24 22, Window_Notify40000 23. No child, an empty heap; the screen words [0xfaa5c0] and [0xfaa5c4] not 0.
# usage: panel7c2534.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob, fill, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x492f60, 0x493126), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b)],
          hooks={0x4929f0: _recorder(10, 7, cdecl=True), 0x65d42e: _recorder(11, 2), 0x65dd7b: _recorder(12, 7), 0x65e526: _recorder(13, 1), 0x6696b4: _recorder(14, 1, cdecl=True), 0x65e6d5: _recorder(15, 2),
                 0x67acc0: _recorder(16, 1, cdecl=True), 0x491920: _recorder(17, 1), 0x65d1c8: _recorder(18, 1), 0x485a70: _recorder(19, 1, cdecl=True), 0x65c388: _recorder(20, 1), 0x65c428: _recorder(21, 2), 0x65c5d5: _recorder(23, 4)})
SELF, CHILD, VT, MEM = RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x600


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 10, 0b11100)
    blob(log, 10, 1, 0b1111)
    strings(log, 10, 0, 0b1000000)
    strings(log, 11, 0, 0b1)
    span = r.choice([0, 1, 20, 300, 0x100000, r.randrange(-1000, 1000)])
    x0, y0 = r.choice([0, 1, 5, r.randrange(-1000, 1000)]), r.choice([0, 2, 7, r.randrange(-1000, 1000)])
    fill(log, 10, 6, struct.pack('<iiii', x0, y0, x0 + span, y0 + r.choice([0, 1, 16, 100, r.randrange(-100, 100)])), which=0)
    fill(log, 11, 0, struct.pack('<hhhh', r.choice([0, 10, -10, r.randrange(-0x8000, 0x8000)]), r.getrandbits(15), r.choice([640, 800, -1, r.randrange(-0x8000, 0x8000)]), r.getrandbits(15)), which=1, offset=4)
    set_return(log, 12, r.choice([CHILD, CHILD, CHILD, CHILD, 0]))
    set_return(log, 14, r.getrandbits(32))
    set_return(log, 16, r.choice([MEM, MEM, MEM, 0]))
    set_return(log, 17, r.getrandbits(32))
    set_return(log, 19, r.getrandbits(32))
    screen = bytearray(r.bytes(8))
    put(screen, 0, 'h', r.choice([480, 600, 768, 1, -1, -480, r.randrange(1, 0x8000)]))
    put(screen, 4, 'h', r.choice([640, 800, 1024, 1, -1, -640, r.randrange(1, 0x8000)]))
    regions = {LOG: log, SELF: bytes(r.bytes(0x40)), CHILD: struct.pack('<I', VT) + r.bytes(0xfc), VT: put(vtable({}, size=0x10), 0x24, 'I', recorder(22, 1)), 0xfaa5c0: screen}
    args = [SELF, r.choice([0, 10, 100, 0x180, -5 & 0xffffffff, r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32), r.choice([6, 7, 0x10, 0x107, r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]
    return Call(0x492f60, args, regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
