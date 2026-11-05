# The game owner shut down with its mouse mode (src/game/core/gameowner: 0x409150 GameOwner_ShutdownAll) against the original: GameOwner_Shutdown (k 20),
# MouseMode_Set (k 21) and Mem_Free (k 19) recorders; the mode at [0x7b05e8] or none.
# usage: gameownerall.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

SHUTDOWN, SET_MODE, FREE = 20, 21, 19
B = Bench([(0x409150, 0x40917e)], hooks={0x409180: _recorder(SHUTDOWN, 0), 0x46c350: _recorder(SET_MODE, 1), 0x67acb0: _recorder(FREE, 1, cdecl=True)})
SELF, MODE = RAM, RAM + 0x100


def case(r):
    regions = {LOG: log_region(), SELF: r.bytes(0x40), MODE: r.bytes(0x20), 0x7b05e8: struct.pack('<I', r.choice([0, MODE, MODE, MODE]))}
    return Call(0x409150, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
