# The advisor's speech from frame to frame (src/game/world/advisorspeech: 0x599880) against the original: the clock and all calls recorders; the speech's globals set around the clock's time, so each time falls either side; the lip times in the static or an allocated array.
# usage: advisorupdatespeech.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, RAND_SEED, Bench, Call, _recorder, fill, log_region, put, set_return, strings  # noqa: E402

NOW, PAUSED, POSITION, SET803, RESIZE, PLAY, SPAWN, DIALOG, HOLDER, QUEUED, ANIMATE, FREE = range(1, 13)
B = Bench([(0x599880, 0x599c30)],
          hooks={0x402d80: _recorder(NOW, 0), 0x402df0: _recorder(PAUSED, 0), 0x51c270: _recorder(POSITION, 4, cdecl=True), 0x51e6f0: _recorder(SET803, 1, cdecl=True), 0x5ed5a0: _recorder(RESIZE, 3, cdecl=True), 0x51bfc0: _recorder(PLAY, 6, cdecl=True),
                 0x521e60: _recorder(SPAWN, 4, cdecl=True), 0x4b9340: _recorder(DIALOG, 0, cdecl=True), 0x429fe0: _recorder(HOLDER, 1, cdecl=True), 0x473ff0: _recorder(QUEUED, 4, cdecl=True), 0x4732a0: _recorder(ANIMATE, 6, cdecl=True), 0x67ad90: _recorder(FREE, 1, cdecl=True)})
BLOCK = 0xf79660
HOLDING, PARTS, MODEL, FACES, TIMES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x1000


def case(r):
    log = log_region()
    clock = r.getrandbits(31) >> r.choice([0, 12, 20])
    block = bytearray(r.bytes(0x190))

    def g(address, value):
        put(block, address - BLOCK, 'I', value & 0xffffffff)

    def time(now):
        return now + r.choice([-300, -100, -1, 0, 1, 100, 300])

    paused = r.choice([0, 0, 1])
    pausedAt = time(clock)
    off = r.choice([0, 0, 1, 1000])
    now = (pausedAt if paused and r.random() < 0.7 else clock) - off
    g(0xf7977c, r.choice([0, 0, 1]))
    g(0xf79780, pausedAt)
    g(0xf79784, off)
    g(0xf7966c, r.choice([1] * 7 + [0]))
    g(0xf79680, r.choice([0, 0, 0x5000]))
    g(0xf79668, time(now))
    g(0xf79694, r.choice([0, time(now)]))
    g(0xf79678, r.choice([0, 1]))
    g(0xf79778, r.choice([0, time(now)]))
    g(0xf79690, r.choice([0, 0, 0, 1]))
    index = r.randrange(6)
    g(0xf79768, index)
    for i in range(8):
        g(0xf79718 + i * 4, r.choice([0, 0x7f0000 & r.getrandbits(32), r.getrandbits(32) & 0xff]) | r.choice([0, r.randrange(1, 100)]))
    slot = r.randrange(4)
    g(0xf79674, slot)
    for i in range(30):
        g(0xf7969c + i * 4, r.randrange(8))
    g(0xf7979c, r.choice([0, 1, 1]))
    g(0xf79774, time(now))
    g(0xf79670, r.choice([0, 1]))
    g(0xf79790, time(now))
    g(0xf79770, r.choice([1, 2, 3, 4]))
    g(0xf79788, r.choice([0, 1, 2, 3]))
    g(0xf7978c, r.choice([-1, 0, 1, 2, 3, 4]))
    g(0xf79660, r.getrandbits(32) & ~0x10000 | r.choice([0, 0, 0x10000]))
    g(0xf79798, r.randrange(4))
    g(0xf79664, r.getrandbits(16))
    times = r.choice([0, 0xf797d0, TIMES, TIMES])
    g(0xf797a0, times)
    values = [r.choice([-1, -1000, -999, 0, 999, 1000, 54321, -54321, 2147483647]) for _ in range(8)]
    array = struct.pack('<8i', *values)
    put(block, 0xf797d0 - BLOCK, '8i', *values)

    faces = bytearray(r.bytes(0xa0 * 8))
    parts = put(bytearray(r.bytes(8)), 4, 'I', MODEL)
    regions = {LOG: log, BLOCK: block, 0xf79bb8: r.bytes(4), 0x803a2c: r.bytes(0x18), RAND_SEED: r.bytes(4),
               HOLDING: put(bytearray(r.bytes(0x10)), 8, 'I', PARTS), PARTS: parts, MODEL: put(bytearray(r.bytes(0x80)), 0x70, 'I', FACES), FACES: faces, TIMES: array}
    set_return(log, NOW, clock)
    set_return(log, PAUSED, paused)
    set_return(log, HOLDER, HOLDING)
    set_return(log, POSITION, r.choice([0, 0x5000, 0x5000]))
    set_return(log, PLAY, r.getrandbits(32))
    strings(log, QUEUED, 0, out=1 << 1 | 1 << 2)
    fill(log, QUEUED, 1, struct.pack('<I', r.choice([0xc, 0xc, 0xc, 0xb, 0x10000000 + 0xc])))
    return Call(0x599880, [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
