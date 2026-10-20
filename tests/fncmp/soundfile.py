# The sound query by file name (src/game/core/soundset: 0x51b530) against the original: a name (a C string of 0 to 12 letters, now and then with a '*'), a folder list
# as class6fd9d8.py's FindFirstMatch bench builds it (0 to 4 objects, or no list at all; here every object's slot 1 answers the same: all find or none does), the
# found String that the match's slot 4 is handed left as String_Ctor made it; the sound globals (on 0x802bc8, the manager 0x802bcc, started 0x802bd4) on or off,
# the manager's slot 1 a recorder (&folders, the found text, the name, &name; the stack addresses logged as 0) that now and then writes the result through &name.
# usage: soundfile.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import class6fd9d8 as c6  # noqa: E402
import gamestring  # noqa: E402
from fncmp import HEAP, LOG, Bench, Call, RAM, echo, fill, heap, log_region, put, recorder, strings, vtable  # noqa: E402

B = Bench(c6.RANGES + [(0x51b530, 0x51b619), (0x5da3c0, 0x5da3c1)])
NAME, MANAGER, MVT = RAM + 0x7000, RAM + 0x7100, RAM + 0x7200
FINDER, SOUND = 1, 7   # the recorders: the folder objects' slot 1, the manager's slot 1


def case(r):
    log = log_region()
    regions = {c6.OBJ: bytearray(r.bytes(0x60)), LOG: log, HEAP: heap()}
    strings(log, FINDER, 2, 5)
    fill(log, FINDER, 0, struct.pack('<i', r.choice([0, 1, 0x7fffffff, -1, -0x80000000])), 0)
    echo(log, FINDER, 0)
    strings(log, 4, 0, 3)   # (the match's slot 4: &result and the found String are stack addresses)
    table = vtable({4: 3})
    put(table, 4, 'I', recorder(FINDER, 3))
    regions[c6.VT] = table
    n = r.choice([0, 1, 1, 2, 3, 4])
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'I', c6.NODES if n else 0)
    nodes = bytearray(r.bytes(0x60))
    for k in range(n):
        held = c6.OBJS + 0x40 * k
        put(nodes, 0x10 * k, 'II', held, c6.NODES + 0x10 * (k + 1) if k + 1 < n else 0)
        regions[held] = struct.pack('<I', c6.VT) + r.bytes(0x20)
    regions[c6.LST], regions[c6.NODES] = lst, nodes
    put(regions[c6.OBJ], 0x2c, 'I', c6.LST if r.random() < 0.9 else 0)
    gamestring.string(r, 0, c6.STAR, RAM + 0x6200, regions, b'*')
    text = bytes(r.choice(b'abcdefgz_.') for _ in range(r.randrange(0, 13)))
    if r.random() < 0.15:
        text += b'*'
    regions[NAME] = text + bytes(4)
    g = bytearray(r.bytes(0x20))   # (the sound globals from 0x802bc0)
    put(g, 8, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    regions[0x802bc0] = g
    regions[MANAGER] = struct.pack('<I', MVT) + r.bytes(0xc)
    mvt = vtable({})
    put(mvt, 4, 'I', recorder(SOUND, 4))
    regions[MVT] = mvt
    strings(log, SOUND, 6, 9)
    if r.random() < 0.6:
        fill(log, SOUND, 3, r.bytes(4), 1)
    return Call(0x51b530, [NAME, c6.OBJ], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
