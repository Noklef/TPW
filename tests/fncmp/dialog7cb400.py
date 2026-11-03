# Dialog7cb400_Open (src/game/ui/dialog7cb400, 0x4a6e40) against the original. Window_FindChild, GuiRect, Window_GetSavedRect, Language_Font, Fn_66781a and Window_SetDwordA0 real, the heap heap()'s (one allocation failing now and then), the other calls recorders
# (kDialog7cb400Env, numbered as there from 10; the children's slot 9 is 29): dialog7cdac4.py's dialog over windows of random bytes with their vtable, the children 0x70a, 0x70c (0x70d, 0x70e among decoys), 0x709 among decoys; [0x7cb3f8] 0 now and then.
# usage: dialog7cb400.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x4a6e40, 0x4a71b0), (0x65d7bc, 0x65d840), (0x659d00, 0x659dc0), (0x65a9d0, 0x65ab00), (0x65d4f6, 0x65d520), (0x485a70, 0x485ab0), (0x66781a, 0x667833), (0x65e6d5, 0x65e705), (0x66bff7, 0x66c01b)],
          hooks={0x5d6110: _recorder(10, 2), 0x47ed80: _recorder(11, 3, cdecl=True), 0x6662b7: _recorder(12, 2), 0x491170: _recorder(13, 1), 0x48f6a0: _recorder(14, 1), 0x491920: _recorder(15, 1), 0x65d1c8: _recorder(16, 1), 0x65c388: _recorder(17, 1),
                 0x65c5d5: _recorder(18, 4), 0x65c4b9: _recorder(19, 2), 0x65c428: _recorder(20, 2), 0x65f5b7: _recorder(21, 1), 0x659bc9: _recorder(22, 1, cdecl=True), 0x66656c: _recorder(23, 1), 0x485b00: _recorder(24, 1, cdecl=True),
                 0x65e59b: _recorder(25, 1), 0x6677ae: _recorder(26, 0), 0x66a295: _recorder(27, 1), 0x492d20: _recorder(28, 1, cdecl=True)})
DIALOG, SCENE, VT, KIDS = RAM + 0x100, RAM + 0x300, RAM + 0x400, RAM + 0x1000


def kids(r, regions, at, ids):
    ids = list(ids) + [r.getrandbits(32) & 0xfffff000 | 1 for _ in range(r.randrange(3))]
    r.shuffle(ids)
    out = {}
    for k, ident in enumerate(ids):
        kid = bytearray(r.bytes(0x200))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', ident)
        put(kid, 0x54, 'I', 0 if k == len(ids) - 1 else at + 0x200 * (k + 1))
        put(kid, 0x5c, 'I', 0)
        regions[at + 0x200 * k] = kid
        out[ident] = at + 0x200 * k
    return at, out


def case(r):
    log = log_region()
    regions = {LOG: log}
    first, top = kids(r, regions, KIDS, (0x70a, 0x70c, 0x709))
    inner, _ = kids(r, regions, KIDS + 0x1000, (0x70d, 0x70e))
    put(regions[top[0x70c]], 0x5c, 'I', inner)
    dialog = bytearray(r.bytes(0x200))
    put(dialog, 0x5c, 'I', first)
    regions[DIALOG] = dialog
    regions[VT] = put(vtable({}, size=0x10), 0x24, 'I', recorder(29, 1))
    regions[SCENE] = r.bytes(0x20)
    regions[0xf85614] = struct.pack('<I', SCENE)
    regions[0x7cb3f8] = struct.pack('<I', r.choice([1, 1, 1, 1, 1, 1, 1, 0, r.getrandbits(32) | 1]))
    regions[0x7cb400] = r.bytes(4)
    regions[0x78b534] = struct.pack('<I', r.randrange(4))
    regions[HEAP] = heap(fail=r.choice([0] * 7 + [1, 2, 3, 4]))
    set_return(log, 11, DIALOG)
    for k in (13, 14, 15, 22, 24):
        set_return(log, k, r.getrandbits(32))
    return Call(0x4a6e40, [r.getrandbits(32)], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
