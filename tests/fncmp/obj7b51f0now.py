# Obj7b51f0_ShowNow (src/game/ui/dialog7cb2b8, 0x4815e0) against the original. Every call a recorder (their own benches have them; driver.cpp's kObj7b51f0NowEnv): Globals7c264c_Clear 10, Dialog74fb20_Open 11, KeyMaps_EnableAllAndClearFlag 12, FunnyTime_Now 13
# (its out on the stack; it fills the year, the month and the day), Dialog7cb2b8_ShowDate 14. The game's mode 4 or others.
# usage: obj7b51f0now.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, strings  # noqa: E402

B = Bench([(0x4815e0, 0x48167b)], hooks={0x48c980: _recorder(10, 0, cdecl=True), 0x48adb0: _recorder(11, 0, cdecl=True), 0x486b60: _recorder(12, 0, cdecl=True), 0x4f8690: _recorder(13, 9), 0x4a0e30: _recorder(14, 3, cdecl=True)})
GAME = RAM + 0x20000   # (the game object [0x7cf83c]: its mode +0x1da738 alone)


def case(r):
    log = log_region()
    strings(log, 13, 0, 0b1)
    fill(log, 13, 1, struct.pack('<I', r.choice([1999, 2000, 0x76c, 0, r.getrandbits(32)])))
    fill(log, 13, 2, struct.pack('<I', r.choice([1, 12, 0, r.getrandbits(32)])), which=1)
    fill(log, 13, 3, struct.pack('<I', r.getrandbits(32)), which=2)
    regions = {LOG: log, 0x7cf83c: struct.pack('<I', GAME), GAME + 0x1da738: struct.pack('<I', r.choice([4, 4, 0, 1, 3, 5, 0x104, r.getrandbits(32)])), 0x7b51f8: r.bytes(0x1c)}
    return Call(0x4815e0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
