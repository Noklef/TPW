# The GUI's activation told to the window tree (src/game/ui/windowbroadcast: 0x659333, 0x659364, 0x6593c5) against the originals: a tree of up to 13 windows (first child +0x5c, next +0x54),
# Fn_65d547 and Fn_65d56b real, Window_PostMessage a recorder as driver.cpp's kWindowBroadcastEnv.
# usage: windowbroadcast.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

POST = 10
B = Bench([(0x659333, 0x659364), (0x659364, 0x6593c5), (0x6593c5, 0x6593ff), (0x65d547, 0x65d56b), (0x65d56b, 0x65d57c)], hooks={0x658ccc: _recorder(POST, 4, cdecl=True)})
SIZE = 0x80


def tree(r):
    """The windows at RAM: the root first."""
    windows = [bytearray(r.bytes(SIZE))]
    for w in windows:
        put(w, 0x5c, 'I', 0)
        put(w, 0x54, 'I', 0)
    open_ = [(0, 0)]
    while open_ and len(windows) < 13:
        at, depth = open_.pop(0)
        last = None
        for _ in range(r.deal([0, 0, 1, 2, 3]) if depth < 3 else 0):
            w = bytearray(r.bytes(SIZE))
            put(w, 0x5c, 'I', 0)
            put(w, 0x54, 'I', 0)
            windows.append(w)
            k = len(windows) - 1
            if last is None:
                put(windows[at], 0x5c, 'I', RAM + SIZE * k)
            else:
                put(windows[last], 0x54, 'I', RAM + SIZE * k)
            last = k
            open_.append((k, depth + 1))
    return b''.join(windows)


def case(r):
    regions = {LOG: log_region(), RAM: tree(r)}
    make = r.deal([0, 1, 2])
    if make == 0:
        return Call(0x659364, [RAM, r.deal([0, 1, 2, r.getrandbits(32)])], regions, ret='v')
    regions[0xfaa5d8] = struct.pack('<I', r.deal([RAM, RAM, RAM, 0]))
    regions[0x77c488] = struct.pack('<I', r.deal([0, 0, 1, r.getrandbits(32)]))
    regions[0xfaa5bc] = r.bytes(4)
    return Call(0x659333 if make == 1 else 0x6593c5, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
