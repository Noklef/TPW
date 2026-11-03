# Dialog7cdac4_Open (src/game/ui/dialog7cdac4, 0x4c4670) against the original. Window_FindChild, Obj_Self and the local WString real, the heap heap()'s (its 1st or 3rd allocation failing), the other calls recorders (their own benches have them; driver.cpp's
# kDialog7cdac4Env): Window7c24c8_Close 10, Dialog_Create 11, Window_SetDepth 12 (a word: ECX's upper half is left over), TextPart6ff4d0_Ctor 13, Window_SetImpl 14, Language_Font 15, Window_Set4c 16, Window705a20_SetBuffer 17, GuiWindows_FindByName 18,
# Window705a20_SetCaret 19, NamedObjects_SetThemeLabelAnsi 20 (the local string logged as 0; its data pointer replaced in some cases: the slot 9 call after it reads that), Window_Notify3000 21, Window_NotifyC00 22, Language_WideText 23, HoverLabel_Attach 24, Class702da0_Push 25, the
# children's slot 9 26.
# usage: dialog7cdac4.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, fill, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x4c4670, 0x4c4871), (0x65d7bc, 0x65d7d0), (0x65d7d0, 0x65d840), (0x5c5520, 0x5c5523), (0x5f6a20, 0x5f6a60), (0x5f6a60, 0x5f6a73)],
          hooks={0x485b40: _recorder(10, 0, cdecl=True), 0x47ed80: _recorder(11, 3, cdecl=True), 0x65f16b: _recorder(12, 1), 0x491170: _recorder(13, 1), 0x65d1c8: _recorder(14, 1), 0x485a70: _recorder(15, 1, cdecl=True), 0x65c388: _recorder(16, 1),
                 0x6662b7: _recorder(17, 2), 0x659bc9: _recorder(18, 1, cdecl=True), 0x66656c: _recorder(19, 1), 0x4099b0: _recorder(20, 2), 0x65c4b9: _recorder(21, 2), 0x65c428: _recorder(22, 2), 0x485b00: _recorder(23, 1, cdecl=True),
                 0x485d20: _recorder(24, 3, cdecl=True), 0x5d6110: _recorder(25, 2)})
DIALOG, SCENE, VT, KIDS = RAM + 0x100, RAM + 0x300, RAM + 0x400, RAM + 0x1000


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 12, 1 << 16)
    set_return(log, 11, DIALOG)
    set_return(log, 13, r.getrandbits(32))
    set_return(log, 15, r.getrandbits(32))
    set_return(log, 18, r.getrandbits(32))
    set_return(log, 23, r.getrandbits(32))
    strings(log, 20, 0, 0b10)
    if r.random() < 0.5:
        fill(log, 20, 1, struct.pack('<I', r.getrandbits(32)), offset=8)
    ids = [0x11e4, 0x11e3] + [r.getrandbits(32) & 0xfffffff0 for _ in range(r.randrange(3))]
    r.shuffle(ids)
    kids = bytearray()
    for k, kid_id in enumerate(ids):
        kid = bytearray(r.bytes(0x100))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', kid_id)
        put(kid, 0x54, 'I', 0 if k == len(ids) - 1 else KIDS + 0x100 * (k + 1))
        kids += kid
    dialog = bytearray(r.bytes(0x100))
    put(dialog, 0x5c, 'I', KIDS)
    put(dialog, 0xe0, 'H', r.choice([0, 1, 0xfffc, 0xffff, 0x7ffe, r.getrandbits(16)]))
    regions = {LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 0, 1, 3])), KIDS: kids, DIALOG: dialog, VT: put(vtable({}, size=0x10), 0x24, 'I', recorder(26, 1)), SCENE: r.bytes(0x20), 0xf85614: struct.pack('<I', SCENE), 0x7cdac4: r.bytes(4)}
    return Call(0x4c4670, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
