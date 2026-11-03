# The unlocks' step (src/game/world/objectunlock: ObjectUnlock_Step, 0x4d4a00) and the set levels' count 0x4d4ac0 against the originals. Recorders as driver.cpp's kObjectUnlockStepEnv; GlobalSave_Instance and GlobalSave_CountSet4
# the originals' on the info [0xf7c58c]. The original's debug print (the empty 0x5da3c0) is left out by the port.
# usage: objectunlockstep.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

READ, ATEXIT, THEMES, SCORES, STORE, BYTE26, PERCENT, MARK, ADVISE = range(10, 19)
B = Bench([(0x4d4a00, 0x4d4ab6), (0x4d4ac0, 0x4d4acd), (0x409410, 0x409490), (0x5af4a0, 0x5af530), (0x5da3c0, 0x5da3c1)],
          hooks={0x550ca0: _recorder(READ, 0), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x4d4bc0: _recorder(THEMES, 0), 0x4d4e50: _recorder(SCORES, 0), 0x519510: _recorder(STORE, 0),
                 0x5af800: _recorder(BYTE26, 1), 0x4cd720: _recorder(PERCENT, 0, cdecl=True), 0x5afb00: _recorder(MARK, 1), 0x4d5870: _recorder(ADVISE, 2)})
SELF, GAME, INFO, THEMES_MAP = RAM + 0x100, RAM + 0x1000, RAM + 0x2000, RAM + 0x2100


def case(r):
    log = log_region()
    set_return(log, STORE, r.getrandbits(32))
    set_return(log, BYTE26, r.deal([0, 0, 0, 0, 1, 2, 0x80, 0x100, 0x12300]))
    set_return(log, PERCENT, r.deal([100, 100, 100, 99, 0, 0x164]))
    set_return(log, MARK, r.deal([0, 1, 5, 0x10000, r.getrandbits(32)]))
    info = put(bytearray(r.bytes(0x60)), 0x3c, 'I', THEMES_MAP)   # (the theme map empty: its head its own first node)
    regions = {LOG: log, 0x80239c: struct.pack('<I', GAME), 0xf7c58c: struct.pack('<I', INFO), INFO: info, THEMES_MAP: struct.pack('<I', THEMES_MAP), 0xfb34ba: bytes([r.deal([0, 1, 1, 1, 2, 0xfe, 0xff])]), 0xfb3b7c: struct.pack('<I', r.deal([0, 0, 0, 1, 2, 0x10000]))}
    if r.deal([0, 0, 0, 0, 1]):
        return Call(0x4d4ac0, [], regions, ret='a')
    return Call(0x4d4a00, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
