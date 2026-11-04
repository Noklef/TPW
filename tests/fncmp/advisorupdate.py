# The advisor's update and its held message (src/game/world/advisor: 0x59a550 Advisor_Update, 0x59ab50 Advisor_PostHeld) against the originals, every callee a recorder (driver.cpp's kAdvisorCalls): the scoring built already,
# its floor about the scores; the pick's score, the clock about +0xd8 + +0xdc, the eight slots' flags and scores, their lines given or not, each id's last line about the count, the kinds 0-3 mostly and above now and then.
# A kind above 3 and kind 0 at the last line leave the original on an uninitialised local (0 on the emulator's stack, 0 in the port).
# usage: advisorupdate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Update,Held)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, fill, log_region, mask_this, put, set_return, strings  # noqa: E402

PICK, SCORE, INIT, POST, NOW, F28, F2C, RAND, SEND, SCORED, STAMP, MINUS = range(1, 13)
B = Bench([(0x59a550, 0x59a923), (0x59ab50, 0x59abb3), (0x409cd0, 0x409cdd)],
          hooks={0x59c680: _recorder(PICK, 1), 0x59bf20: _recorder(SCORE, 1), 0x59b590: _recorder(INIT, 6), 0x59a940: _recorder(POST, 1), 0x402d80: _recorder(NOW, 0), 0x59c2b0: _recorder(F28, 1),
                 0x59c490: _recorder(F2C, 1), 0x516330: _recorder(RAND, 0), 0x59b870: _recorder(SEND, 2), 0x59b620: _recorder(SCORED, 2), 0x41a960: _recorder(STAMP, 0), 0x41a990: _recorder(MINUS, 0)})
SELF, TIMES, COUNTS = RAM, RAM + 0x2000, RAM + 0x2100
MESSAGE = 0x12345678   # (what the message's build answers: the post must get it)
OPS = [k for k in os.environ.get('OPS', 'Update,Held').split(',') if k]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    self = bytearray(r.bytes(0x1700))
    floor = r.choice([0, 1, 50, -1, 1000])
    regions = {LOG: log, SELF: self, 0xfb2766: bytes([r.getrandbits(8) | 1]), 0xfb3560: struct.pack('<i', floor), 0xfb386c: r.bytes(4), 0x80239c: r.bytes(4)}
    set_return(log, POST, 0)
    if op == 'Held':
        self[0x16e4] = r.choice([0, 1, 1, 0x80])
        return Call(0x59ab50, [SELF], regions, conv='t', ret='b')
    set_return(log, PICK, r.randrange(0x160))
    set_return(log, SCORE, (floor + r.choice([-1, 0, 1, 1, 100])) & 0xffffffff)
    set_return(log, INIT, MESSAGE)
    mask_this(log, INIT)
    log_mask = struct.unpack_from('<I', log, LOG_BYTES + 4 * INIT)[0] | 1 << 17   # (the word built by 0x409cd0: its high half the stack's)
    struct.pack_into('<I', log, LOG_BYTES + 4 * INIT, log_mask)
    now = r.getrandbits(32)
    start, length = r.getrandbits(32), r.choice([0, 1, 1000, r.getrandbits(32)])
    if r.random() < 0.8:   # (the time up, by little or much, or just short)
        length = r.choice([0, 1, 1000, 50000])
        start = (now - length - r.choice([0, 0, 1, 1000, -1])) & 0xffffffff
    put(self, 0xd8, 'II', start, length)
    self[0x16e8] = r.choice([0, 0, 0, 0, 1, 2])
    n = r.choice([1, 2, 3, 5, 5, 0x7fffffff, 0x80000000])
    regions[TIMES] = struct.pack('<40I', *([now, r.getrandbits(32)] * 20))
    regions[COUNTS] = struct.pack('<40I', *([n, n if r.random() < 0.8 else r.choice([1, 2, 3])] * 20))
    answers(log, NOW, TIMES)
    answers(log, F28, COUNTS)
    set_return(log, F2C, r.choice([0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 0xffffffff]))
    set_return(log, RAND, r.getrandbits(32))
    for k in (SEND, SCORED):
        strings(log, k, 0, 1)
        set_return(log, k, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
        fill(log, k, 0, r.bytes(4), k - SEND)
    any_slot = r.random() < 0.9
    for k in range(8):
        slot = 0x14 + 0x18 * k
        self[slot + 0x14] = r.choice([0, 0, 1, 0x80]) if any_slot else 0
        put(self, slot + 0x10, 'i', floor + r.choice([-1, 0, 1, 2, 3, 100, -100]))
        put(self, slot + 8, 'i', r.choice([-1, -1, -1, -2, 0, 1, 7]))
        id = r.randrange(0x160)
        put(self, slot + 0xc, 'I', id)
        self[slot + 0x16] = r.choice([0, 1, 0x80])
        put(self, 0xe4 + (id << 4), 'I', r.choice([0, n - 1, n - 1, n - 2, n, n + 1, 0xffffffff, 0xfffffffe, r.randrange(8)]) & 0xffffffff)
    return Call(0x59a550, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
