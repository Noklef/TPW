# The park's gates (src/game/world/parkgates: the step 0x4cf3e0, the arrival's size 0x4cf5b0, the visitor brought 0x4cf720) against the original. Recorders as driver.cpp's kParkGatesEnv. Visitor_Ctor's cell is
# logged by its low word: the original pushes a dword whose high word is uninitialised stack, and the constructor reads the low word. The out pointers of 0x51a2f0 and Map_BusStopPos are stack addresses, logged as 0.
# usage: parkgates.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, echo, fill, heap, log_region, put, set_return, strings  # noqa: E402

SINCE, STAMP, TABLE, RECORD, TAKE, BLOCKED, ARRIVAL, NOTSIZE1, BUSSTOP, VISITOR, COUNT, UPKEEP, READMODE, ATEXIT = range(10, 24)


def _low_word(k):
    rec = _recorder(k, 1)

    def hook(e):
        sp = e.r['ESP'] + (4 if e.tail else 0)
        e.write(sp, struct.pack('<I', e.u32(sp) & 0xffff))
        return rec(e)
    return hook


B = Bench([(0x4cf3e0, 0x4cf5b0), (0x4cf5b0, 0x4cf71d), (0x4cf720, 0x4cf7bd), (0x5da3c0, 0x5da3c1)],
          hooks={0x41a990: _recorder(SINCE, 0), 0x41a960: _recorder(STAMP, 0), 0x519510: _recorder(TABLE, 0), 0x519590: _recorder(RECORD, 0), 0x51a690: _recorder(TAKE, 0), 0x51a9d0: _recorder(BLOCKED, 0),
                 0x51a2f0: _recorder(ARRIVAL, 2), 0x51aad0: _recorder(NOTSIZE1, 0), 0x4d8650: _recorder(BUSSTOP, 2), 0x4faec0: _low_word(VISITOR), 0x4c7fa0: _recorder(COUNT, 1), 0x4c8240: _recorder(UPKEEP, 0),
                 0x550ca0: _recorder(READMODE, 0), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True)})
GATES, WORLD, ANALYSER, REC, TAKES = RAM + 0x1000, RAM + 0x2000, RAM + 0x10000, RAM + 0x3000, RAM + 0x4000


def case(r):
    log = log_region()
    op = r.deal([0x4cf3e0, 0x4cf3e0, 0x4cf3e0, 0x4cf5b0, 0x4cf720])
    wait = r.deal([0, 1, 0x100, r.getrandbits(32)])
    balance = bytearray(0x14)
    put(balance, 0, 'i', r.deal([0, 1, 40, 300, 700, r.getrandbits(32) - (1 << 31)]))                          # 0x785310, the least
    put(balance, 4, 'I', wait)                                                                                  # 0x785314
    put(balance, 0xc, 'i', r.deal([0, 10, 300, 1000, 2000, r.getrandbits(32) - (1 << 31)]))                     # 0x78531c
    put(balance, 0x10, 'i', r.deal([-1, 0, 1, 2, 3, 10, r.getrandbits(32) - (1 << 31)]))                        # 0x785320
    gates = bytearray(r.bytes(0x20))
    gates[0x10] = r.deal([0, 0, 1, 1, 1, 2])
    put(gates, 0xc, 'i', r.deal([0, 0, 1, 2, 0x23, 0x24, 0x3c, 0x3d, -1, r.getrandbits(32) - (1 << 31)]))
    size = r.deal([1, 2, 5])
    ring = bytearray(r.bytes(0x60))
    put(ring, 0x18, 'i', r.deal([-1, 0, 1, size - 1, size]))
    put(ring, 0x1c, 'i', size)
    takes = bytes(b for _ in range(10) for b in struct.pack('<i', r.choice([-1, -1, 0, 0, 1, 2, 2, 2, 3, 4, 4, 5, 6, 6])))
    regions = {LOG: log, HEAP: heap(0x400, r.deal([0, 0, 0, 1])), GATES: gates, 0x80239c: struct.pack('<I', WORLD), WORLD + 0x1da738: struct.pack('<I', r.deal([0, 1, 3, 4, 4, 5, 8][:6])),
               0x8023a0: struct.pack('<I', RAM + 0x5000), 0x785310: balance, ANALYSER + 0x20cc0: r.bytes(4), ANALYSER + 0x216d8: ring, REC: put(bytearray(r.bytes(0x40)), 0x30, 'i', r.deal([-1, 0, 1, 2])),
               TAKES: takes + bytes(0x100), 0xfb34c2: bytes([r.deal([0, 1, 0xfe, 0xff])]), 0xfb3b7c: struct.pack('<I', r.deal([0, 1, 1, 2]))}
    set_return(log, SINCE, r.deal([0, wait - 1, wait, wait + 1, r.getrandbits(32)]))
    set_return(log, STAMP, r.getrandbits(32))
    set_return(log, TABLE, ANALYSER)
    set_return(log, RECORD, REC)
    answers(log, TAKE, TAKES)
    set_return(log, BLOCKED, r.deal([0, 1, 0x100, r.getrandbits(32)]))
    set_return(log, NOTSIZE1, r.deal([0, 1, 0x10000]))
    set_return(log, COUNT, r.deal([0, 10, 400, 490, 500, 1400, 1490, 1500, r.getrandbits(32)]))
    set_return(log, UPKEEP, r.deal([0, 10, 1000, 5000, r.getrandbits(32)]))
    strings(log, ARRIVAL, 0, out=1)
    strings(log, BUSSTOP, 0, out=1)
    fill(log, BUSSTOP, 0, struct.pack('<H', r.deal([0, 0x80, 0x100, 0x101, 0xffff, r.getrandbits(16)])))
    echo(log, BUSSTOP, 0)
    return Call(op, [GATES], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
