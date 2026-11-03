# The handler of the chat window [0x7cc8bc] (src/game/ui/window7cc8bchandler: 0x4be820) against the original. Recorders as driver.cpp's kWindow7cc8bcHandlerEnv; Window_FindChild real.
# usage: window7cc8bchandler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return  # noqa: E402

B = Bench([(0x4be820, 0x4be970), (0x65d7bc, 0x65d840)],
          hooks={0x46cf10: _recorder(10, 0, cdecl=True), 0x4a9380: _recorder(11, 0, cdecl=True), 0x4eb300: _recorder(12, 0), 0x4ebee0: _recorder(13, 0), 0x4ebef0: _recorder(14, 0),
                 0x4ee8d0: _recorder(15, 0), 0x4ee9e0: _recorder(16, 0), 0x6649d5: _recorder(17, 0), 0x65f66e: _recorder(18, 3)})
TOP, MID, LIST, WINDOW, CHILD, SLOTS = (RAM + 0x200 * k for k in range(6))


def node(r, ident, kid=0):
    out = bytearray(r.bytes(0x100))
    put(out, 0x54, 'I', 0)
    put(out, 0x5c, 'I', kid)
    put(out, 0x8c, 'I', ident)
    return out


def case(r):
    log = log_region()
    set_return(log, 10, r.getrandbits(32))
    set_return(log, 18, r.getrandbits(32))
    child = node(r, 0xc3129)
    put(child, 0, 'I', SLOTS)
    slots = bytearray(0x40)
    put(slots, 5 * 4, 'I', recorder(20, 1))
    regions = {LOG: log, TOP: node(r, 1, MID), MID: node(r, 0xc3123, LIST), LIST: node(r, 0xc3124), WINDOW: node(r, 2, CHILD), CHILD: child, SLOTS: slots,
               0x7cc8bc: TOP.to_bytes(4, 'little'), 0x7cc8d0: r.bytes(4)}
    message = r.choice([0x100, 0x100, 0x100, 0x101, 0x101, 0xff, 0x102, r.getrandbits(32)])
    a = r.choice([0xc3126, 0xc3127, 0xc3128, 0xc3128, 0xc3129, 0xc3129, 0xc312a, 0xc312b, r.getrandbits(32)])
    b = r.choice([1, 1, 0, 2, 0x101, r.getrandbits(32)])
    return Call(0x4be820, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
