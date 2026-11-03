# The online chat's queue of events: ChatAvatar_ShowResult 0x4eec40, ChatAvatar_LobbyEvent 0x4e6790 and ChatAvatar_PumpEvents 0x4e6560 (src/game/online/chatevents)
# on chatevents.py's world. Lobby: every code, the room's session (chatcommand.py's vote world, the texts of its object and item added), one of the
# strings told to the Chat's slot 0x6fd4c4 hashed at random; what the lobby calls of the others is recorders. Pump: the deque at the avatar's +0xec8 of 0-5
# vectors with 0-3 tokens of narrow and wide strings, first words 0x2711 (the result, real), lobby codes (real) or any (HandleEvent, a recorder, when +0x28 is
# set); it starts near its block's end so the next block and the map are walked, or the count ends in the block.
# usage: chatqueue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Result,Lobby,Pump)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import IMPORT_TABLE, RAM, Bench, Call, _recorder, blob_bytes, put, recorder, strings  # noqa: E402
import chatcommand as cc  # noqa: E402
from chatcommand import AVATAR, ITEMS, OBJ, TOKENS, TOKVEC  # noqa: E402
import chatevents as ce  # noqa: E402
import onlinechat  # noqa: E402

TEXTS, DEQ_MAP, BLOCKS, VECS, STRS = RAM + 0x40000, RAM + 0x44000, RAM + 0x48000, RAM + 0x42000, onlinechat.STRINGS


def merged(ranges):
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(ce.RANGES + [(0x4e6560, 0x4e6785), (0x4e6790, 0x4e6e0c), (0x4eec40, 0x4eefd4), (0x5c8b90, 0x5c8be0), (0x46cf10, 0x46cf7d), (0x4e6130, 0x4e6150),
                              (0x5b64c0, 0x5b64c4), (0x4f7170, 0x4f7178), (0x4e51a0, 0x4e5220), (0x67a400, 0x6902c0), (0x45b590, 0x45b690)]),
          hooks={**ce.HOOKS, 0x47f410: _recorder(16, 2, cdecl=True), 0x4bce80: _recorder(17, 0, cdecl=True), 0x4ee6d0: _recorder(18, 2), 0x516700: _recorder(19, 0),
                 0x4ef3b0: _recorder(20, 1), 0x4efc10: _recorder(4, 1), 0x4e6e80: _recorder(6, 1)})
RESULTS = [x & 0xffffffff for x in (0, -1, -2, -3, -4, -5, -6, -0x1e, -0x1f, -0x20, -0x1d, 1, 0x7fffffff, -0x80000000)]
CODES = [1, 7, 0x2c, 0x34, 0x34, 0x34, 0x34, 0x31, 0x36, 0x28, 0x29, 0x2b, 0x2d, 0x30, 0x32, 0x33, 0, 2, 0x2a, 0x2e, 0x2f, 0x35, 0x37, 0xffffffff]
PUMPED = [1, 7, 0x2c, 0x31, 0x2a, 0x32, 0x37, 0x12345]
OTHER_CHAT = RAM + 0x43000


def chat_imports(regions, log):
    """The critical sections' and the two lobby methods' import slots, on chatevents.py's table."""
    table = regions[IMPORT_TABLE]
    put(table, 0x6fd140 - IMPORT_TABLE, 'I', recorder(0, 1, com=True))
    put(table, 0x6fd130 - IMPORT_TABLE, 'I', recorder(5, 1, com=True))
    put(table, 0x6fd480 - IMPORT_TABLE, 'I', recorder(11, 2))
    put(table, 0x6fd4c4 - IMPORT_TABLE, 'I', recorder(14, 6))
    strings(log, 25, 0, out=0x70)   # (ShowResult's format has none of the text arguments)


def world(r):
    regions, avatar, pool, log, _, _ = ce.world(r)
    chat_imports(regions, log)
    regions[0x7cddec] = struct.pack('<I', AVATAR if r.random() < 0.7 else OTHER_CHAT)
    if struct.unpack('<I', regions[0x7cddec])[0] == OTHER_CHAT:
        regions[OTHER_CHAT] = r.bytes(0xf0c)
    return regions, avatar, log


def wide(r, data, at, n):
    """The wide string of n characters at offset at of the texts; its next free offset."""
    data[at:at + 2 * n + 2] = struct.pack('<%dH' % (n + 1), *(r.randrange(1, 0x10000) for _ in range(n)), 0)
    return at + 2 * n + 2


def lobby(r):
    regions, avatar, log = world(r)
    code = r.deal(CODES)
    toks = put(bytearray(r.bytes(0x48)), 0x20, 'I', code)
    regions[TOKENS] = toks
    regions[TOKVEC] = put(bytearray(r.bytes(0x10)), 4, '3I', TOKENS, TOKENS + 0x48, TOKENS + 0x48)
    if code == 0x34:
        cc.vote(r, regions, log)
        texts = bytearray(0x200)
        put(regions[OBJ], 0x14, 'I', TEXTS)
        off = wide(r, texts, 0, r.choice([0, 1, 5, 31, 40]))
        items = regions[ITEMS]
        for i in range(3):
            put(items, 0x40 + 0x60 * i + 0x14, 'I', TEXTS + off)
            off = wide(r, texts, off, r.choice([0, 1, 5, 31, 40]))
        regions[TEXTS] = texts
        g = bytearray(0x60)
        wide(r, g, 0, r.choice([0, 1, 5, 40]))
        regions[0x78a374] = g
        told = r.randrange(4)
        blob_bytes(log, 14, told, 0xfff0, inner=(4, 0x7fff))
        strings(log, 14, 0, out=sum(1 << a for a in range(4) if a != told))
    return Call(0x4e6790, [AVATAR, TOKVEC], regions, conv='t', ret='v')


def pump(r):
    regions, avatar, log = world(r)
    avatar[0x28] = r.choice([0, 0, 1, 0xff])
    regions[0x7cddec] = struct.pack('<I', AVATAR) if r.random() < 0.7 else regions[0x7cddec]
    n = r.choice([0, 1, 1, 2, 3, 5])
    off = r.choice([0xff, 0xfe, 0xfd, 0xfc, 0, 5, r.randrange(0x100)])
    node = r.randrange(3)
    slots = 4 + node
    m = bytearray(r.bytes(4 * slots))
    place = r.sample(range(3), 3)
    for i in range(3):
        put(m, 4 * (node + i), 'I', BLOCKS + 0x1000 * place[i])
    pool, used = bytearray(r.bytes(0x400)), 0
    vectors = bytearray(r.bytes(0x24 * 16))
    at = 0
    blocks = {}
    for i in range(n):
        q = off + i
        b = BLOCKS + 0x1000 * place[q // 0x100]
        count = r.choice([0, 1, 1, 2, 3])
        first = VECS + at
        for t in range(count):
            for field, width in ((0, 1), (0x10, 2)):
                ptr, length, res, used = onlinechat.text(r, pool, used, width)
                put(vectors, at + field + 4, '3I', ptr, length, res)
            put(vectors, at + 0x20, 'I', r.getrandbits(32))
            at += 0x24
        entry = bytearray(r.bytes(0x10))
        empty = not count and (avatar[0x28] != 0 and r.random() < 0.5)   # (no array: the lobby event would read through null)
        put(entry, 4, '3I', 0 if empty else first, 0 if empty else VECS + at, r.getrandbits(32))
        if count:
            kind = r.random()
            word = 0x2711 if kind < 0.3 else r.choice(PUMPED) if kind < 0.8 else r.getrandbits(32)
            put(vectors, first - VECS + 0x20, 'I', word)
            if word == 0x2711 and count > 1:
                put(vectors, first - VECS + 0x44, 'I', r.choice(RESULTS))
        blocks.setdefault(b, {})[q % 0x100] = entry
    for b, entries in blocks.items():
        lo, hi = min(entries), max(entries)
        data = bytearray(r.bytes(0x10 * (hi - lo + 1)))
        for q, entry in entries.items():
            data[0x10 * (q - lo):0x10 * (q - lo) + 0x10] = entry
        regions[b + 0x10 * lo] = data
    base = BLOCKS + 0x1000 * place[off // 0x100]
    put(avatar, 0xecc, '4I', base, base + 0x1000, base + 0x10 * (off % 0x100), DEQ_MAP + 4 * node)
    put(avatar, 0xeec, 'I', DEQ_MAP)
    put(avatar, 0xef4, 'I', n)
    regions.update({DEQ_MAP: m, VECS: vectors, STRS: pool})
    return Call(0x4e6560, [AVATAR], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Lobby':
        return lobby(r)
    if op == 'Pump':
        return pump(r)
    regions, avatar, log = world(r)
    return Call(0x4eec40, [AVATAR, r.deal(RESULTS)], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Result,Lobby,Lobby,Pump,Pump').split(',') if k]

if __name__ == '__main__':
    B.main(case)
