# A tick message sent down the window tree (src/game/ui/windowtick: 0x658f6c, 0x658f97) against the original. The send a recorder as driver.cpp's kWindowTickEnv; the accessors real. A tree
# of 0-15 windows, each new one a child of an earlier one (the list's last).
# usage: windowtick.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_fold, log_region, put  # noqa: E402

SEND = 10
B = Bench([(0x658f6c, 0x658fe9), (0x65d547, 0x65d55a), (0x65d56b, 0x65d57c)], hooks={0x658d9f: _recorder(SEND, 4, cdecl=True)})
WINDOWS, NODE = RAM + 0x1000, 0x80


def case(r):
    log = log_region()
    log_fold(log)
    n = r.randrange(1, 16)
    windows = bytearray(r.bytes(NODE * n))
    last = {}
    for i in range(n):
        put(windows, NODE * i + 0x54, 'I', 0)
        put(windows, NODE * i + 0x5c, 'I', 0)
    for i in range(1, n):
        parent = r.randrange(0, i)
        if parent in last:
            put(windows, NODE * last[parent] + 0x54, 'I', WINDOWS + NODE * i)
        else:
            put(windows, NODE * parent + 0x5c, 'I', WINDOWS + NODE * i)
        last[parent] = i
    regions = {LOG: log, WINDOWS: windows, 0xfaa60c: r.bytes(4), 0xfaa5d8: struct.pack('<I', r.choice([WINDOWS, WINDOWS, 0]))}
    if r.randrange(3) == 0:
        return Call(0x658f97, [WINDOWS], regions, ret='v')
    return Call(0x658f6c, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
