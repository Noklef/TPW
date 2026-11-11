# The advisor scoring's criteria with their callees recorded (src/game/core/advisorscoring, AdvisorScoring_SetCalls's table): the staff counts, patrols, research definitions and money each a recorder
# answering in turn from a small table of its own, the two game pointers' table entries distinct, the scoring's dwords small and its shares' bounds about 0-150, so each test lands on either side.
# usage: advisorscoringcalls.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, echo, fill, log_region, put, strings  # noqa: E402

ELAPSED, RED = 1, 26
# the recorders: address -> (k, stack arguments), as driver.cpp's kAdvisorRingCalls
CALLEES = {0x4f8800: (ELAPSED, 1), 0x4c9960: (6, 1), 0x4c81b0: (7, 1), 0x4c9b40: (8, 1), 0x4cd500: (9, 2), 0x4c90d0: (10, 0), 0x4c9070: (11, 0), 0x4c9ed0: (12, 0), 0x4c9e40: (13, 1),
           0x4c9e60: (14, 0), 0x4ca8e0: (15, 2), 0x4ca7b0: (16, 2), 0x4ca450: (17, 1), 0x4ca4c0: (18, 1), 0x4ca510: (19, 1), 0x4ca570: (20, 1), 0x4ca5d0: (21, 1), 0x503980: (22, 1),
           0x6ad810: (23, 0), 0x4d0690: (24, 0), 0x4d0810: (25, 0), 0x4d0260: (RED, 1),
           0x4c81e0: (2, 0), 0x4c9050: (3, 0), 0x4c9ee0: (4, 0), 0x4c9290: (27, 1), 0x4c9130: (28, 0), 0x4d0a00: (29, 0), 0x4d09a0: (30, 0), 0x4d0970: (31, 1)}
NEAR = {'Half7', 'Ratio1', 'Ratio2', 'Stat1ac', 'Loan'}
STAT = 27   # (0x4c9290's argument logged by its low byte: 0x59e380 pushes EDX after MOV DL, the high bytes whatever EDX held)
CALLS = {'Half7': 0x59ccb0, 'Share4': 0x59d0c0, 'Ratio1': 0x59de80, 'Ratio2': 0x59e110, 'Stat1ac': 0x59e380, 'Loan': 0x59e910,
'Obj4': 0x59cac0, 'Busy5': 0x59cc20, 'Busy7': 0x59cd90, 'Chain8': 0x59cfe0, 'Cover4': 0x59d180, 'Share5': 0x59d250, 'Cover5': 0x59d310, 'Share7': 0x59d400, 'Share6': 0x59d4c0, 'Cover6': 0x59d580,
         'Res0': 0x59d990, 'None0': 0x59dad0, 'Idle8': 0x59db40, 'Res1': 0x59dca0, 'Wait0': 0x59dde0, 'Res2': 0x59df30, 'Wait1': 0x59e070, 'Res3': 0x59e1c0, 'Wait2': 0x59e2e0, 'Money': 0x59e8d0,
         'Red3': 0x59e9f0, 'Red34': 0x59ea70, 'Red5': 0x59eaf0, 'Days1': 0x5a06c0, 'Days2': 0x5a0720}
DAYS = {'Days1': 0x5dc, 'Days2': 0x5e8}   # (the first bound; the second 4 on, the result 8 on)


def _low_byte(k):
    rec = _recorder(k, 1)

    def hook(e):
        sp = e.r['ESP'] + (4 if e.tail else 0)
        e.write(sp, struct.pack('<I', e.u32(sp) & 0xff))
        return rec(e)
    return hook


B = Bench([(0x59cac0, 0x59eb70), (0x5a06c0, 0x5a0776), (0x519510, 0x519590), (0x5195d0, 0x51960d), (0x51a280, 0x51a287), (0x67c320, 0x67c3e0), (0x5da3c0, 0x5da3c1)],
          hooks={a: _low_byte(k) if k == STAT else _recorder(k, n) for a, (k, n) in CALLEES.items()})
SCORING, POINTER, GAME1, GAME2, ANSWERS = RAM, RAM + 0x800, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000
PARK1, LAB, MONEY1, PARK2, MONEY2 = RAM + 0x4000, RAM + 0x4010, RAM + 0x4020, RAM + 0x4030, RAM + 0x4040
MONTH, DAY = 0x1792f8648000, 0xc92a69c000
SHARES = {0x124, 0x130, 0x2bc, 0x13c, 0x148, 0x154, 0x168, 0x180, 0x18c, 0x220, 0x258, 0x27c, 0x2a0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def small(r):
    return r.choice([0, 1, 1, 2, 2, 3, 3, 5, -1])


def time(r, months):
    return struct.pack('<q', months * MONTH + r.choice([0, 1, MONTH - 1] if months >= 0 else [0, -1, 1 - MONTH]))


def game(words):
    return bytes(0x10) + struct.pack('<8H', *words, *([0] * (8 - len(words))))


def case(r):
    op = r.deal(OPS)
    log = log_region()
    table = bytearray()
    for i, (k, _n) in enumerate(CALLEES.values()):
        if k in (ELAPSED, RED):
            table += bytes(0x10)   # (so slot i's answers are the i-th 0x10 bytes)
            continue
        answers(log, k, ANSWERS + 0x10 * i)
        table += struct.pack('<4i', *(r.choice([0, 25, 50, 100, 101, -1, r.randrange(0, 200)]) if k == 9 else
                                      r.choice([0, 0, 1, 1, 2, 2, 3] if op in NEAR else [0, 1, 1, 2, 2, 3, 3, 5, -1, 10]) for _ in range(4)))
    if op == 'Stat1ac' and r.random() < 0.8:   # (mostly past its two tests: 0x4c9050's low byte set, 0x4c9ee0 0)
        slot = [k for k, _n in CALLEES.values()].index
        put(table, 0x10 * slot(3), '4I', *(r.choice([1, 0x101, 0xff]) for _ in range(4)))
        put(table, 0x10 * slot(4), '4I', 0, 0, 0, 0)
    for k in (ELAPSED, RED):
        strings(log, k, 0, 1)
        echo(log, k, 0)
    days = r.choice([0, 1, 2, 5, 30, -1, -2])
    fill(log, ELAPSED, 0, struct.pack('<q', days * DAY + r.choice([0, 1, DAY - 1] if days >= 0 else [0, -1, 1 - DAY])) if op in DAYS else time(r, r.choice([0, 1, 2, 3, 3, 5, -1, 40])), 0)
    fill(log, RED, 0, time(r, r.choice([0, 2, 3, 4, 5, 5, 6, -1, -3])), 1)
    s = bytearray(r.bytes(0x640))
    for off in range(0xc0, 0x640, 4):
        put(s, off, 'i', small(r))
    for off in SHARES:
        if r.random() < 0.85:
            put(s, off, 'i', r.choice([0, 33, 50, 66, 100, 101, 150, 300, -1]))
    for off in (0xcc, 0xe8, 0x104, 0x128, 0x14c, 0x16c, 0x184, 0x228, 0x230, 0x234, 0x23c, 0x260, 0x268, 0x26c, 0x284, 0x28c, 0x290, 0x2a8, 0x2b0, 0x2b4, 0x334, 0x344, 0x348, 0x34c):
        if r.random() < 0.8:
            put(s, off, 'I', r.choice([1, r.getrandbits(31) | 1, r.getrandbits(31) | 1]))   # (1: a score that can equal a floor)
    for off in (0x270, 0x294, 0x338):   # (the ratios' and the loan's floats)
        put(s, off, 'f', r.choice([0.0, 0.5, 1.0, 1.0, 1.5, 2.0, 3.0, -1.0, float('nan')]))
    if op in DAYS:
        put(s, DAYS[op], 'i', days + r.choice([-2, -1, -1, 0, 1]))
        put(s, DAYS[op] + 4, 'i', days + r.choice([2, 1, 1, 0, -1]))
        put(s, DAYS[op] + 8, 'I', r.getrandbits(31) | 1)
    if op == 'Money':
        put(s, 0x330, 'i', r.choice([0, 1, 2, 3]))
    if op in NEAR:   # (the answers 0-3, so the bounds are met exactly now and then)
        for off in (0x630, 0x634, 0x274, 0x298):
            put(s, off, 'i', r.choice([0, 1, 2, 3]))
        put(s, 0x2bc, 'i', r.choice([0, 33, 50, 66, 100, 150]))
        for off in (0xf4, 0x278, 0x29c, 0x2c0, 0x33c):   # (the results not 0)
            put(s, off, 'I', r.getrandbits(31) | 1)
    regions = {LOG: log, SCORING: s, 0x7cf6ec: struct.pack('<I', POINTER), POINTER: struct.pack('<I', GAME1), 0x7cf83c: struct.pack('<I', GAME2), ANSWERS: table,
               GAME1 + 0x1da710: struct.pack('<I', r.choice([0, 0, 0, 0, 1, r.getrandbits(32)])) + game([1, 2, 0, 3])[4:], GAME2 + 0x1da710: bytes(4) + game([4, 0, 0, 5])[4:],
               0x7cfb90: struct.pack('<30I', *[v for t in (0, PARK1, LAB, MONEY1, PARK2, MONEY2) for v in (t, 0, 0, 0, 0)])}
    return Call(CALLS[op], [SCORING], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
