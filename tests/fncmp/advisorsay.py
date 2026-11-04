# The advisor starting a response (src/game/world/advisorspeech: 0x599050 Advisor_Say, 0x598b20 Advisor_QueueRandom, 0x598bf0 Advisor_QueueAnim) and loading its state (0x599d00 Advisor_Load) against the
# originals, all callees recorders but Stream_Read (a record stream). Say: 0x768fb8's rows dealt, the id in them or not, or the terminator's; sprintf's text filled in and the path's text hashed in the locate's
# log; the file there or not, opened or not; the old times none, static or freed, the static taken or free, sizes either side of 1000; the sound's length 0 at -0xc8. QueueRandom: rand's answers in
# turn, lengths around the table's. QueueAnim: rows of steps, flag 0x400000's times, flags, a row of no length. Load: the magic right or not, the file cut short at each read.
# usage: advisorsay.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, put, record_stream, set_return, strings  # noqa: E402

NOW, PAUSED, SET803, RESIZE, PLAY, FREE = 1, 2, 4, 5, 6, 12
RAND, BASE, SPRINTF, LOCATE, OPEN, SIZE, MALLOC, CLOSE, INFO, CALL, FORWARD = range(13, 24)
CD = {0x51e6f0: (SET803, 1), 0x5ed5a0: (RESIZE, 3), 0x51bfc0: (PLAY, 6), 0x67ad90: (FREE, 1), 0x67b5c0: (RAND, 0), 0x40ef40: (BASE, 0), 0x67a650: (SPRINTF, 3), 0x44a280: (LOCATE, 3),
      0x5c4d50: (OPEN, 2), 0x5c5000: (SIZE, 1), 0x67dcd0: (MALLOC, 1), 0x5f5b80: (CLOSE, 2), 0x51c570: (INFO, 1), 0x429e90: (CALL, 2), 0x51c2c0: (FORWARD, 1)}
HOOKS = {a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()}
HOOKS.update({0x402d80: _recorder(NOW, 0), 0x402df0: _recorder(PAUSED, 0)})
B = Bench([(0x598b20, 0x598ca0), (0x599050, 0x5994d5), (0x599d00, 0x599df8), (0x5f5e20, 0x5f5e50), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
BLOCK, ROWS, STEPS = 0xf79660, 0x768fb8, 0x76dc18
TABLE, TABLE2, STREAM, DATA, TIMES, TEXT, LOCATED = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000, RAM + 0x2000, RAM + 0x2100
OPS = ['Say', 'Say', 'Say', 'Say', 'Random', 'Anim', 'Load']


def block(r):
    b = bytearray(r.bytes(0x560))
    for k in range(11):
        put(b, 0xf796c8 - BLOCK + 4 * k, 'i', r.choice([1, 5, 10, 50, 100, 400, 1000, 0, -3]) if r.random() < 0.9 else 7)
    put(b, 0xf796c4 - BLOCK, 'i', r.choice([0, 3]))
    put(b, 0xf79bb8 - BLOCK, 'I', r.choice([0, 1]))
    put(b, 0xf79768 - BLOCK, 'I', r.choice([0, 3]))
    return b


def queue_rows(r, regions):
    rows = bytearray(0x50 * 4)
    for row in range(4):
        at, n, plain = 0, r.choice([0, 1, 2, 3, 6]), 0
        while n and at < 0x48:
            kind = r.randrange(6)
            if kind == 0:
                put(rows, row * 0x50 + at, 'I', 0x400000 | r.getrandbits(4))
                put(rows, row * 0x50 + at + 4, 'I', r.choice([0, 1, 500, 0x10000]))
                at += 8
            else:
                put(rows, row * 0x50 + at, 'I', r.choice([0xe, r.randrange(1, 0xc)]) | r.choice([0, 0, 0x10000, 0x7f0000, 0xff800000 if r.random() < 0.1 else 0]))
                at += 4
            n -= 1
    regions[STEPS] = bytes(rows)


def rand_answers(r, regions, log):
    regions[TABLE2] = struct.pack('<40i', *[r.choice([r.randrange(0x8000), r.randrange(10), 9]) for _ in range(40)])
    answers(log, RAND, TABLE2)


def lengths(r):
    return r.choice([0, -1, 1, 5, 6, 100, 399, 400, 401, 1500, 5000, 0x7fffffff, r.randrange(3000)])


def say(r, regions, log, b):
    rows = bytearray()
    ids = r.sample(range(1, 40), 4)
    for k in range(4):
        anim = r.choice([-1, -1, 0x10, 0x11, 0x12, 0x13])
        rows += struct.pack('<7I', ids[k] if k < 4 else 0, r.getrandbits(32), r.getrandbits(32), anim & 0xffffffff, r.choice([0, r.randrange(4), r.randrange(1, 4) << 16 | r.randrange(4), 0xffff0003]),
                            r.getrandbits(32), r.getrandbits(32)) + bytes(4)
    rows += struct.pack('<I', 0x270f)
    regions[ROWS] = bytes(rows)
    id_ = r.choice(ids * 4 + [0x270f, 77])
    regions[0x78d90c] = bytes([r.choice([1, 1, 1, 1, 1, 1, 0x80, 0])])
    put(b, 0xc, 'I', 0 if r.random() < 0.95 else 1)
    now = r.getrandbits(31) >> r.choice([0, 12])
    set_return(log, NOW, now)
    set_return(log, PAUSED, r.choice([0, 0, 1]))
    put(b, 0xf7977c - BLOCK, 'I', r.choice([0, 1]))
    put(b, 0xf79780 - BLOCK, 'i', now - r.randrange(500))
    put(b, 0xf79784 - BLOCK, 'i', r.randrange(1000))
    # the file
    strings(log, SPRINTF, 0b010, out=0b001)
    fill(log, SPRINTF, 0, b'\\Speech\\lips\\sp_' + b'%d.lip\0' % r.randrange(100))
    regions[TEXT] = r.choice([b'', b'c:\\tpw\\', b'Data\\Local']) + b'\0'
    set_return(log, BASE, TEXT)
    strings(log, LOCATE, 0b001)
    regions[TABLE] = struct.pack('<40I', *([r.choice([1, 1, 1, 1, 0]), LOCATED] + [0] * 38))
    answers(log, LOCATE, TABLE)
    regions[LOCATED] = b'located\0'
    set_return(log, OPEN, r.choice([STREAM, STREAM, STREAM, 0]))
    size = r.choice([0, 4, 8, 0x40, 999, 1000, 1001, 2000, -4])
    set_return(log, SIZE, size)
    set_return(log, MALLOC, r.choice([TIMES, TIMES, TIMES, 0x10]) if size < 2000 else TIMES)
    data = struct.pack('<16i', *[r.choice([-1, 0, 999, 1000, 1001, -1000, -1001, r.randrange(1 << 20)]) for _ in range(16)])
    regions[DATA] = data
    regions[STREAM] = record_stream(STREAM, DATA, r.choice([0, 4, 0x10, 0x40]))
    regions[TIMES] = struct.pack('<16i', *[r.choice([-1, 1234, 40000]) for _ in range(16)])
    strings(log, CLOSE, 0, out=0b001)
    put(b, 0xf797a0 - BLOCK, 'I', r.choice([0, 0xf797d0, TIMES + 0x100]))
    put(b, 0xf79798 - BLOCK, 'I', r.choice([0, 1, 3]))
    # the sound
    regions[0x803a34] = r.bytes(0x10)
    set_return(log, PLAY, r.choice([0, r.getrandbits(32)]))
    set_return(log, INFO, r.choice([-0xc8, 0, 100, 400, 2000, -0x12c - 0xc8, r.randrange(4000)]))
    rand_answers(r, regions, log)
    queue_rows(r, regions)
    return id_


def case(r):
    log = log_region()
    b = block(r)
    regions = {LOG: log}
    op = r.deal(OPS)
    if op == 'Say':
        id_ = say(r, regions, log, b)
        regions[BLOCK] = b
        return Call(0x599050, [id_], regions, ret='a')
    if op == 'Random':
        rand_answers(r, regions, log)
        length = lengths(r)
        if r.random() < 0.5:   # (the rest after the first gesture just what a row plays: the ties of the two compares)
            rows = [r.choice([5, 50, 100]) for _ in range(11)]
            for k, value in enumerate(rows):
                put(b, 0xf796c8 - BLOCK + 4 * k, 'i', value)
            put(b, 0xf796fc - BLOCK, 'i', 0)
            length = rows[r.randrange(10)]
        regions[BLOCK] = b
        return Call(0x598b20, [length], regions, ret='a')
    if op == 'Anim':
        queue_rows(r, regions)
        regions[BLOCK] = b
        return Call(0x598bf0, [r.randrange(4), r.choice([0, 0, 1, lengths(r)])], regions, ret='a')
    n = r.choice([0, 0x40, 0x168, 0x168, 0x168])
    times = struct.pack('<16i', *[r.randrange(1 << 20) for _ in range(16)])
    count = r.choice([0, 4, 0x40, 999, 1000, 0x7fff, -4])
    magic = 0x5f564441 if r.random() < 0.9 else r.getrandbits(32)
    data = struct.pack('<II', magic, n) + r.bytes(n) + struct.pack('<i', count) + times
    regions[DATA] = data
    regions[STREAM] = record_stream(STREAM, DATA, r.choice([len(data), len(data), len(data), 2, 4, 6, 8, 8 + n // 2, 8 + n, 8 + n + 2, 8 + n + 4]))
    set_return(log, MALLOC, TIMES)
    regions[TIMES] = r.bytes(0x40)
    put(b, 0xf797a0 - BLOCK, 'I', r.choice([0, 0xf797d0, TIMES + 0x100]))
    regions[BLOCK] = b
    return Call(0x599d00, [STREAM], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
