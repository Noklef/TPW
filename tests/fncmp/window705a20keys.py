# A key pressed in the edit window 0x705a20 (src/game/ui/window705a20char: 0x667a72) against the original. NotifyText, SetSelection, Window_SendMessage and TypeChar recorders as driver.cpp's
# kWindow705a20CharEnv; wcslen real. SetSelection's cursor and anchor logged as words (the original leaves other halves in the registers it pushes). The text and selection as window705a20char.py.
# usage: window705a20keys.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

NOTIFY, SELECT, SEND, TYPE = 10, 11, 12, 13
B = Bench([(0x667a72, 0x667fee), (0x67acd0, 0x67acf0)],
          hooks={0x66671d: _recorder(NOTIFY, 1), 0x6668e9: _recorder(SELECT, 3), 0x658d9f: _recorder(SEND, 4, cdecl=True), 0x667833: _recorder(TYPE, 1)})
WINDOW, BUFFER = RAM + 0x1000, RAM + 0x2100
KEEP = 0x100
KEYS = [0x2700, 0x2500, 0x2300, 0x2400, 0x2600, 0x2800, 0x2e00, 0x2e00, 0x2e00, 8, 8, 8, 8, 0xd, 0x41, 0, 0x2701, 0x3000, 0xa700]


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * SELECT, 5 << 16)
    regions = {LOG: log, 0xfaa56c: bytes([r.choice([0, 0, 1, 0x80])])}
    length = r.randrange(0, 21)
    buffer = bytearray(r.bytes(0x400))
    put(buffer, KEEP, '%dH' % (length + 1), *([r.randrange(0x41, 0x5b) for _ in range(length)] + [0]))
    regions[BUFFER - KEEP] = buffer
    window = bytearray(r.bytes(0x170))
    put(window, 0x138, 'I', r.choice([BUFFER] * 15 + [0]))
    put(window, 0x13c, 'I', max(0, length + r.randrange(-3, 12)))
    put(window, 0x14a, 'h', r.choice([r.randrange(0, length + 1)] * 8 + [r.randrange(-3, 0), length + r.randrange(0, 4)]))
    put(window, 0x14c, 'h', r.choice([-1, -1, -1, r.randrange(0, length + 1), r.randrange(0, length + 1), r.randrange(0, length + 1), r.randrange(-3, 0), length + r.randrange(0, 4)]))
    regions[WINDOW] = window
    key = r.choice(KEYS) | (r.getrandbits(16) << 16)
    b = r.choice([3 << 16, 3 << 16, 0, 0x103 << 16, r.getrandbits(32)])
    return Call(0x667a72, [WINDOW, key, b], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
