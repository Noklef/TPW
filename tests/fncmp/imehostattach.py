# The IME host taken to a text window and off it (src/game/ui/imehostattach: ImeHost_Attach 0x480ec0, ImeHost_Detach 0x480f50) against the originals. Recorders (driver.cpp's kImeHostAttachEnv): 0x47a3a0 10 (its result the stack's, filled), 0x47a570 11, Window705a50_TextLength 12, Ime_SetLimit 13, Fn_668564 14, Window_SendMessage 15.
# usage: imehostattach.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x480ec0, 0x480f49), (0x480f50, 0x480f8b)], hooks={0x47a3a0: _recorder(10, 2), 0x47a570: _recorder(11, 0), 0x66852b: _recorder(12, 0), 0x47a980: _recorder(13, 1), 0x668564: _recorder(14, 1), 0x658d9f: _recorder(15, 4, cdecl=True)})
HOST, WINDOW = RAM, RAM + 0x400


def case(r):
    log = log_region()
    strings(log, 10, 0, out=1)
    fill(log, 10, 0, struct.pack('<i', r.choice([0, 1, -1, 0x7fffffff, -0x80000000, r.getrandbits(31)])))
    set_return(log, 12, r.choice([0, 1, 5, 0x7fff, 0x8000, 0xffff, r.getrandbits(32)]))
    host = bytearray(r.bytes(0x1b0))
    put(host, 0, 'I', r.choice([0, r.getrandbits(32)]))
    put(host, 4, 'I', r.choice([0, r.getrandbits(32)]))
    w = bytearray(r.bytes(0x140))
    put(w, 0x13c, 'H', r.choice([0, 1, 100, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
    regions = {LOG: log, HOST: host, WINDOW: w, 0x7a130c: r.bytes(4)}
    if r.random() < 0.6:
        return Call(0x480ec0, [HOST, WINDOW], regions, conv='t', ret='v')
    return Call(0x480f50, [HOST], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
