# The thing panel 0x7ca228's handler and follow button (src/game/ui/panel7ca228handler: 0x499bb0, 0x499b90) against the original. Every callee a recorder as
# driver.cpp's kPanel7ca228HandlerEnv; Window_FindChild real over a child list of 0x1586 (or not) and 0x1584 with the recording vtable.
# usage: panel7ca228handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

CLOSE, BACK, STEP, GOTO, MOVE, DEF, REFUND, ANALYSER, WORD18, FOLLOW, SETTIMER, STARTTIMER, SWING, CANCEL, OPEN, DEFAULT = range(12, 28)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x499b90, 0x499bb0), (0x499bb0, 0x499d78), (0x65d7bc, 0x65d832), (0x5da3c0, 0x5da3c1)],
          hooks={0x48cf70: _recorder(CLOSE, 0), 0x48cbe0: _recorder(BACK, 0), 0x48caf0: _recorder(STEP, 0), 0x48cd10: _recorder(GOTO, 0), 0x48cfa0: _recorder(MOVE, 0),
                 0x4dd4e0: _recorder(DEF, 0), 0x4e2400: _recorder(REFUND, 0), 0x519510: _recorder(ANALYSER, 0), 0x4ca8c0: _recorder(WORD18, 1), 0x4e15b0: _recorder(FOLLOW, 1),
                 0x65ef90: _recorder(SETTIMER, 2), 0x65f024: _recorder(STARTTIMER, 1), 0x4864f0: C(SWING, 0), 0x486b40: C(CANCEL, 1), 0x495aa0: C(OPEN, 2), 0x48d200: C(DEFAULT, 4)})
PANEL, KVTABLE, THING, WINDOW, WORD, REFUNDW, DECOY, NODE = 0x7ca228, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3200, RAM + 0x3400, RAM + 0x3600, 0x180


def node(r, ident, nxt):
    buf = bytearray(r.bytes(NODE))
    put(buf, 0, 'I', KVTABLE)
    put(buf, 0x54, 'I', nxt)
    put(buf, 0x5c, 'I', 0)
    put(buf, 0x8c, 'I', ident)
    return buf


def case(r):
    log = log_region()
    regions = {LOG: log, KVTABLE: vtable({7: 1, 11: 0})}
    panel = bytearray(r.bytes(0x20))
    put(panel, 0, 'I', KVTABLE)
    put(panel, 0x14, 'I', r.choice([WINDOW, WINDOW, WINDOW, 0]))
    regions[PANEL] = panel
    regions[WINDOW] = put(node(r, 1, 0), 0x5c, 'I', DECOY)
    regions[DECOY] = node(r, 0x1585, WORD if r.random() < 0.7 else REFUNDW)
    regions[WORD] = node(r, 0x1586, REFUNDW)
    regions[REFUNDW] = node(r, 0x1584, 0)
    regions[THING] = r.bytes(0x20)
    regions[0x7c2658] = struct.pack('<I', THING)
    regions[0x7cf83c] = r.bytes(4)
    set_return(log, ANALYSER, RAM + 0x4000)
    set_return(log, SETTIMER, r.choice([0, 1, 0x10000, r.getrandbits(32)]))
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, CLOSE, r.getrandbits(32))
    if r.random() < 0.05:
        return Call(0x499b90, [PANEL], regions, conv='t', ret='v')
    message = r.choice([5, 0x10, 0x10, 0x14, 0x15, 0x1e, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, r.randrange(0, 0x110), r.getrandbits(32)])
    a = r.choice([0x80080, 0xffffffff, 0x157b, 0x157a, 0x157c, 0x157d, 0x157e, 0x157f, 0x1580, 0x1581, 0x1582, 0x1583, r.getrandbits(32)])
    return Call(0x499bb0, [RAM + 0x5000, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
