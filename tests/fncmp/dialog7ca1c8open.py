# The dialog 0x7ca1c8 opened for a thing (src/game/ui/dialog7ca1c8: Dialog7ca1c8_OpenFor, 0x4988c0) against the original (dialog7ca1c8.py benches the module's others). Up [0x7ca1cc] with the same thing [0x7ca1d0] or the
# byte [0x7ca1d4] set now and then; the flag [0x785a2c]; every call a recorder (their own benches have them; driver.cpp's kDialog7ca1c8OpenEnv): FocusCell_Update 10, Dialog7cb2b8_ResetChild21 11, Dialog7cb2b8_Child21Post 12,
# Window_SetFocus thiscall 13, Object_LinkOpen 14 (0 or not), MouseMode_EnterTile15 15, CoasterEdit_EditRide 16, MouseMode_Ensure6feaa0 17, GameCursor_Set 18, CoasterEdit_Tools 19, Dialog7ca1c8_SetChecks 20,
# CoasterEdit_StartChosenTool 21, Dialog7ca1c8_SelectButton 22, Dialog7ca1c8_UpdateLoopButton 23. The tools' two out words (the stack's) logged as 0 and filled. The dialog closing under the link check is the thing
# being 0x7ca1cc itself with Object_LinkOpen's recorder writing 0 through its argument.
# usage: dialog7ca1c8open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, set_return, strings  # noqa: E402

B = Bench([(0x4988c0, 0x4989cf), (0x48c940, 0x48c94a)],
          hooks={0x5276d0: _recorder(10, 1, cdecl=True), 0x4a25f0: _recorder(11, 1, cdecl=True), 0x4a2590: _recorder(12, 1, cdecl=True), 0x65e59b: _recorder(13, 1),
                 0x441910: _recorder(14, 1, cdecl=True), 0x497d20: _recorder(15, 0, cdecl=True), 0x4474d0: _recorder(16, 2, cdecl=True), 0x497bc0: _recorder(17, 0, cdecl=True),
                 0x4a2aa0: _recorder(18, 1, cdecl=True), 0x445910: _recorder(19, 1, cdecl=True), 0x498090: _recorder(20, 1, cdecl=True), 0x4455c0: _recorder(21, 1, cdecl=True),
                 0x4981d0: _recorder(22, 1, cdecl=True), 0x4987f0: _recorder(23, 0, cdecl=True)})
THING = RAM + 0x100


def case(r):
    log = log_region()
    thing = r.choice([THING] * 9 + [0x7ca1cc])
    if thing == 0x7ca1cc:
        fill(log, 14, 0, bytes(4))
    set_return(log, 14, r.choice([0, 0, 1, r.getrandbits(32)]))
    for k, which in ((19, 1), (21, 2)):
        strings(log, k, 0, 0b1)
        fill(log, k, 0, r.bytes(4), which=which)
    regions = {LOG: log}
    regions[0x7ca1c8] = struct.pack('<III', r.getrandbits(32), r.choice([0, 0, 0, 1, r.getrandbits(32)]), r.choice([thing, r.getrandbits(32), r.getrandbits(32)])) + bytes([r.choice([0] * 7 + [1, r.getrandbits(8)])]) + r.bytes(3)
    regions[0x785a2c] = struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[0x7c2654] = r.bytes(4)
    return Call(0x4988c0, [thing], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
