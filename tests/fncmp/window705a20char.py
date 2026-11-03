# A character typed into the edit window 0x705a20 (src/game/ui/window705a20char: 0x667833, 0x6687e0) against the original. NotifyText and SetSelection recorders as driver.cpp's
# kWindow705a20CharEnv; wcslen and wcschr real. A text of 0-20 characters in a buffer with room around it, its capacity about the length, the cursor and the anchor in it or a little out.
# usage: window705a20char.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

NOTIFY, SELECT = 10, 11
B = Bench([(0x667833, 0x667a72), (0x6687e0, 0x6687f6), (0x67acd0, 0x67acf0), (0x67c2f0, 0x67c31e)], hooks={0x66671d: _recorder(NOTIFY, 1), 0x6668e9: _recorder(SELECT, 3)})
WINDOW, BUFFER, ALLOW, DENY = RAM + 0x1000, RAM + 0x2100, RAM + 0x3000, RAM + 0x3100
KEEP = 0x100  # the room below the buffer's start, for a place a little below 0


def chars(r, n):
    return [r.randrange(0x41, 0x48) | (r.choice([0, 0, 0, 1]) << 8) for _ in range(n)]


def wlist(values):
    buf = bytearray(2 * (len(values) + 1))
    for i, v in enumerate(values):
        put(buf, 2 * i, 'H', v)
    return buf


def case(r):
    log = log_region()
    regions = {LOG: log}
    allow, deny = chars(r, r.randrange(0, 5)), chars(r, r.randrange(0, 5))
    regions[ALLOW], regions[DENY] = wlist(allow), wlist(deny)
    typed = r.choice(chars(r, 1) + chars(r, 1) + chars(r, 1) + allow + deny + [0xd, 9, 8, 0x1b, 0, 0x10d])
    high = r.getrandbits(16)
    if r.randrange(4) == 0:
        return Call(0x6687e0, [ALLOW if r.randrange(2) else DENY, typed | (high << 16)], regions, ret='a')
    length = r.randrange(0, 21)
    buffer = bytearray(r.bytes(0x400))
    put(buffer, KEEP, '%dH' % (length + 1), *(chars(r, length) + [0]))
    regions[BUFFER - KEEP] = buffer
    window = bytearray(r.bytes(0x170))
    put(window, 0x138, 'I', r.choice([BUFFER] * 15 + [0]))
    put(window, 0x13c, 'I', max(0, length + r.randrange(-3, 12)))
    put(window, 0x140, 'I', r.choice([0, 0, 0, ALLOW]))
    put(window, 0x144, 'I', r.choice([0, 0, 0, DENY]))
    put(window, 0x14a, 'h', r.choice([r.randrange(0, length + 1)] * 8 + [r.randrange(-3, 0), length + r.randrange(0, 4)]))
    put(window, 0x14c, 'h', r.choice([-1, -1, -1, r.randrange(0, length + 1), r.randrange(0, length + 1), r.randrange(0, length + 1), r.randrange(-3, 0), length + r.randrange(0, 4)]))
    regions[WINDOW] = window
    return Call(0x667833, [WINDOW, typed | (high << 16)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
