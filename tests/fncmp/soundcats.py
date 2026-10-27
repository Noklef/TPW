# The sound categories' registration (src/game/core/soundset: 0x51e8f0 Lobby, 0x51eae0 Global, 0x51ec50 Theme, 0x51e890 LobbySfx) against the originals: the named
# objects registry [0x786b9c] (named_lists.py's layout: a list of 0 to 4 folder objects, nodes {item, next}, each item's name a String at +4)
# whose items are named after the keys (the global Strings 0x803768, 0x8039c0, 0x802e68 with the lobby's name or the theme appended; 0x8030d8,
# 0x803558; case varied) or not; each item a folder list as soundfile.py's (class6fd9d8.py's FindFirstMatch: +0x2c a list of folder objects
# or none, their slot 1 answering alike) for Sound_QueryFound; the sound globals on or off with its manager's recorder (&folders, the found
# text, the name, &name); the categories 0x803a20-0x803a44 random beforehand; Lobby's array [0x803a4c] of [0x803a50] pairs (none, or 0-3 in a
# block of their own), grown from the heap (GlobalAlloc / GlobalFree served by fncmp's stubs).
# usage: soundcats.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Lobby,Global,Theme,LobbySfx)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import class6fd9d8 as c6  # noqa: E402
import gamestring  # noqa: E402
from fncmp import HEAP, LOG, Bench, Call, RAM, echo, fill, global_heap, heap, log_region, put, recorder, strings, vtable  # noqa: E402


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


CALLS = {'Lobby': 0x51e8f0, 'Global': 0x51eae0, 'Theme': 0x51ec50, 'LobbySfx': 0x51e890}
B = Bench(merged(c6.RANGES + [(0x41e8e0, 0x41eab0), (0x421770, 0x4217f0), (0x424cf0, 0x424d20), (0x51b4a0, 0x51b500), (0x51b530, 0x51b630),
                              (0x51e890, 0x51e8e7), (0x51e8f0, 0x51ea60), (0x51eae0, 0x51edb0), (0x5f5660, 0x5f56a0), (0x5f6040, 0x5f6160), (0x5f61a0, 0x5f6210),
                              (0x5f76c0, 0x5f7740), (0x67ad90, 0x67af10), (0x67dcd0, 0x67ddb0), (0x67df40, 0x67e020), (0x67efc0, 0x67f1b0),
                              (0x67faf0, 0x67fc00), (0x680430, 0x6804e0), (0x682720, 0x6827d0), (0x682a70, 0x682c10), (0x688710, 0x688740),
                              (0x688f50, 0x6891c0), (0x68a170, 0x68a450), (0x68a5d0, 0x68a720), (0x68efb0, 0x68f050), (0x5da3c0, 0x5da3c1)]))
MANAGER, MVT = RAM + 0x7100, RAM + 0x7200
REG, LIST, NODES, ITEMS, TEXTS = RAM + 0x8000, RAM + 0x8100, RAM + 0x8200, RAM + 0x9000, RAM + 0x9800
NAME, THEME, OLD, GDATA = RAM + 0xa000, RAM + 0xa100, RAM + 0xa200, RAM + 0xb000
FINDER, SOUND = 1, 7   # the recorders: the folder objects' slot 1, the manager's slot 1
GLOBALS = [0x803768, 0x8039c0, 0x802e68, 0x8030d8, 0x803558]
WORDS = [b'snd\\', b'Sound\\', b'x', b'', b'amb', b'lobby_', b'Theme1']


def varied(r, text):
    return bytes(c ^ 0x20 if 0x61 <= (c | 0x20) <= 0x7a and r.random() < 0.3 else c for c in text)


def folders(r, regions, log):
    """soundfile.py's folder list (class6fd9d8.py's objects) and sound globals; returns the list's head (c6.LST) or 0 now and then."""
    strings(log, FINDER, 2, 5)
    fill(log, FINDER, 0, struct.pack('<i', r.choice([0, 1, 1, 0x7fffffff, -1])), 0)
    echo(log, FINDER, 0)
    strings(log, 4, 0, 3)
    table = vtable({4: 3})
    put(table, 4, 'I', recorder(FINDER, 3))
    regions[c6.VT] = table
    n = r.choice([0, 1, 1, 2, 3])
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'I', c6.NODES if n else 0)
    nodes = bytearray(r.bytes(0x60))
    for k in range(n):
        held = c6.OBJS + 0x40 * k
        put(nodes, 0x10 * k, 'II', held, c6.NODES + 0x10 * (k + 1) if k + 1 < n else 0)
        regions[held] = struct.pack('<I', c6.VT) + r.bytes(0x20)
    regions[c6.LST], regions[c6.NODES] = lst, nodes
    gamestring.string(r, 0, c6.STAR, RAM + 0x6200, regions, b'*')
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


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {c6.OBJ: bytearray(r.bytes(0x60)), LOG: log, HEAP: heap(), **global_heap()}
    folders(r, regions, log)
    texts = {}
    for k, g in enumerate(GLOBALS):
        texts[g] = r.choice(WORDS)
        gamestring.string(r, 0, g, GDATA + 0x80 * k, regions, texts[g])
    suffix = r.choice(WORDS[:3] + [b'Jungle', b'a\\b'])
    if name == 'Lobby':
        regions[NAME] = suffix + bytes(4)
        keys = [texts[0x803768] + suffix]
    elif name == 'Theme':
        gamestring.string(r, 0, THEME, GDATA + 0x400, regions, suffix)
        keys = [texts[g] + suffix for g in GLOBALS[:3]]
    elif name == 'LobbySfx':
        keys = [texts[0x8030d8]]
    else:
        keys = [texts[0x8030d8], texts[0x803558]]
    # the registry: 0-4 items, each named one of the keys (case varied) or not, its folder list the shared one or none
    n = r.choice([0, 1, 2, 3, 4, 4])
    lst = bytearray(r.bytes(0x10))
    put(lst, 4, 'I', NODES if n else 0)
    nodes = bytearray(r.bytes(0x10 * 5))
    for k in range(n):
        put(nodes, 0x10 * k, 'II', ITEMS + 0x80 * k, NODES + 0x10 * (k + 1) if k + 1 < n else 0)
        item = bytearray(r.bytes(0x60))
        put(item, 0xc, 'I', TEXTS + 0x40 * k)
        put(item, 0x2c, 'I', c6.LST if r.random() < 0.9 else 0)
        regions[ITEMS + 0x80 * k] = item
        regions[TEXTS + 0x40 * k] = varied(r, r.choice(keys) if r.random() < 0.7 else r.choice(WORDS + [suffix])) + bytes(0x10)
    regions[REG] = struct.pack('<I', LIST if r.random() < 0.95 else 0) + r.bytes(0xc)
    regions[LIST], regions[NODES] = lst, nodes
    regions[0x786b9c] = struct.pack('<I', REG)
    # the categories and Lobby's array
    cats = bytearray(r.bytes(0x38))
    pairs = r.choice([0, 0, 1, 2, 3])
    put(cats, 0x2c, 'I', OLD if pairs else 0)
    put(cats, 0x30, 'I', pairs)
    regions[0x803a20] = cats
    regions[OLD] = r.bytes(8 * 4)
    if name == 'Lobby':
        return Call(CALLS[name], [NAME], regions, ret='a')
    if name == 'Theme':
        return Call(CALLS[name], [THEME], regions, ret='v')
    return Call(CALLS[name], [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
