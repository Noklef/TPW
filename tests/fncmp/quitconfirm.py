# The answer of the quit message box (src/game/ui/quitconfirm: QuitConfirm_Answered, 0x4c0830) against the original. Recorders (driver.cpp's kQuitConfirmEnv): Setting_ReadMode 10, atexit 11, Setting_SetMode 12.
# usage: quitconfirm.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

B = Bench([(0x4c0830, 0x4c087c)], hooks={0x550ca0: _recorder(10, 0), 0x67a7d0: _recorder(11, 1, cdecl=True), 0x550d80: _recorder(12, 1)})
GAME = RAM


def case(r):
    regions = {LOG: log_region(), GAME: r.bytes(0x20), 0xf82884: struct.pack('<I', GAME), 0xfb34bc: bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])])}
    return Call(0x4c0830, [r.choice([0, 0, 0, 1, r.getrandbits(32)])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
