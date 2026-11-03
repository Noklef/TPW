# A thing's two names as text (src/game/world/thingname) against the originals: snwprintf 0x67c830 a recorder (what it is given is compared; the static it would fill stays as it is). The text +0x3c8 empty,
# short, ending in one or two dashes; its length past the static's size now and then (the word cleared is not bounded).
# usage: thingname.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return, strings  # noqa: E402

B = Bench([(0x4e2860, 0x4e291d), (0x4e2920, 0x4e2941), (0x67acd0, 0x67acee)], hooks={0x67c830: _recorder(1, 4, cdecl=True)})
THING, TO = RAM, RAM + 0x1000
STATIC = 0x7cdc00   # (the static 0x7cdc58 and the words about it)


def case(r):
    log = log_region()
    set_return(log, 1, r.getrandbits(32))
    length = r.choice([0, 1, 2, 3, 0x20, 0x21, 0x30, r.randrange(0, 0x30)])
    tail = r.choice([[], [0x2d], [0x2d, 0x2d], [0x2d, 0x2d, 0x2d], [0x2c, 0x2d], [0x2d, 0x2e]])[:length]
    units = [r.randrange(1, 0x10000) for _ in range(length - len(tail))] + tail
    units = [0x41 if u == 0x2d and i < length - 2 else u for i, u in enumerate(units)]
    thing = bytearray(r.bytes(0x500))
    struct.pack_into('<%dH' % (length + 1), thing, 0x3c8, *units, 0)
    struct.pack_into('<H', thing, 0x3c6, r.choice([0x2d, r.getrandbits(16)]))   # (one unit: the word before the text must stay unread)
    regions = {LOG: log, THING: thing, TO: r.bytes(0x10), STATIC: r.bytes(0x100)}
    address = 0x4e2920 if r.random() < 0.2 else 0x4e2860
    if address == 0x4e2860 and not length:
        strings(log, 1, 0, out=8)   # (three arguments here: the recorder's fourth is the stack's)
    return Call(address, [THING, TO, r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
