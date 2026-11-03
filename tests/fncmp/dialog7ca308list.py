# Dialog7ca308_ListHandler (src/game/ui/dialog7ca308, 0x49bc80) against the original. Dialog7ca308_FillList 17, HoverLabel_Attach 21, SpriteScript_Kill 22, Language_WideText 23 recorders (cdecl; kDialog7ca308Env): their own benches have them. The window random bytes with its default handler +0x118 a
# cdecl recorder answering at random (Window_CallDefault real). The message 0x101 and value 1 mostly, the id each of the buttons 0x2490-0x2494 and its neighbours; the script [0x7ca310] set or not, the globals random.
# usage: dialog7ca308list.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return  # noqa: E402

B = Bench([(0x49bc80, 0x49bdbc), (0x65f66e, 0x65f696)],
          hooks={0x49b5b0: _recorder(17, 2, cdecl=True), 0x485d20: _recorder(21, 3, cdecl=True), 0x475550: _recorder(22, 1, cdecl=True), 0x485b00: _recorder(23, 1, cdecl=True)})
WINDOW = RAM


def case(r):
    log = log_region()
    regions = {LOG: log}
    window = bytearray(r.bytes(0x200))
    put(window, 0x118, 'I', recorder(24, 4, cdecl=True))
    regions[WINDOW] = window
    regions[0x7ca308] = struct.pack('<I', r.getrandbits(32))
    regions[0x7ca310] = struct.pack('<I', r.choice([0, r.getrandbits(32) | 1]))
    regions[0x7523e8] = r.bytes(8)
    set_return(log, 23, r.getrandbits(32))
    set_return(log, 24, r.getrandbits(32))
    id_ = r.deal([0x2490, 0x2491, 0x2492, 0x2493, 0x2494])
    id_ = r.choice([id_] * 8 + [0x248f, 0x2495, 0, r.getrandbits(32)])
    message = r.choice([0x101] * 8 + [0x100, 0x102, r.getrandbits(32)])
    value = r.choice([1] * 8 + [0, 2, r.getrandbits(32)])
    return Call(0x49bc80, [WINDOW, message, id_, value], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
