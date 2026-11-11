# The C++ runtime's terminate (src/game/core/crtterminate: 0x682590) against the original: _getptd a cdecl recorder answering a thread block whose terminate
# handler +0x60 is a cdecl recorder or none; abort a cdecl recorder that returns.
# Bench limit: abort never returns in the game; the emulator is given 0x682620's epilogue (the same frame) after it, as the port returns.
# usage: crtterminate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, recorder, set_return  # noqa: E402

GETPTD, HANDLER, ABORT = 1, 2, 3
B = Bench([(0x682590, 0x6825f6), (0x682607, 0x68260c), (0x68267d, 0x68268e)],
          hooks={0x682460: _recorder(GETPTD, 0, cdecl=True), 0x68cc30: _recorder(ABORT, 0, cdecl=True)})
B.emu.code.update({0x68260c: ('ADD', 'ESP,0x4'), 0x68260f: ('JMP', '0x0068267d')})
B.emu.order = sorted(set(B.emu.order) | {0x68260c, 0x68260f})
B.emu.next = dict(zip(B.emu.order, B.emu.order[1:]))
PTD = RAM


def case(r):
    log = log_region()
    set_return(log, GETPTD, PTD)
    ptd = bytearray(r.bytes(0x74))
    struct.pack_into('<I', ptd, 0x60, r.deal([0, recorder(HANDLER, 0, cdecl=True), recorder(HANDLER, 0, cdecl=True)]))
    return Call(0x682590, [], {LOG: log, PTD: ptd}, ret='v')


if __name__ == '__main__':
    B.main(case)
