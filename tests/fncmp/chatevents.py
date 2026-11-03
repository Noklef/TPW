# The online chat's events: ChatAvatar_HandleEvent 0x4e6e80 and ChatAvatar_SetBuddy 0x4ee710, ChatBuddySet_EraseKey 0x4f2700, WStdString_CtorCopyC 0x4ea0e0.
# chatcommand.py's avatar (its names, the room tree, the buddy set with its two bytes at +0x1c and its colour at +0x20, the ignored set), six tokens
# (the code in the first, the rest of the avatar's names or not, words of every kind), thingenv.py's park with chat players named as the avatar's
# names, and recorders for what the events call of the others (ChatEventEnv). Each case makes one format call at most: its string arguments
# are stack structures of which one, picked at random, is hashed.
# usage: chatevents.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Event,SetBuddy,EraseBuddy,CopyC)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob, blob_bytes, fill, heap, log_region, put, recorder, set_return, strings  # noqa: E402
import chatcommand as cc  # noqa: E402
from chatcommand import AVATAR, CHAT, OBJ, OUT, POOL, SESSION, TOKENS, TOKVEC, TREES, Pool, flip, fold, formats, name, save, tree  # noqa: E402
import thingenv  # noqa: E402

NAMEW, NARROW, DIGITS, ME, THINGS = RAM + 0x1400, RAM + 0x7000, RAM + 0x7100, RAM + 0x7200, RAM + 0x10000
B = Bench(cc.RANGES + [(0x4e6e80, 0x4e9ec0), (0x4ee710, 0x4ee8c3), (0x4f2700, 0x4f2767), (0x4ea0e0, 0x4ea215), (0x4f0170, 0x4f0240), (0x4f2590, 0x4f2700),
                       (0x4f3b10, 0x4f4050), (0x5ce530, 0x5ce540), (0x4f0370, 0x4f0390), (0x4f5430, 0x4f54e0), (0x67a520, 0x67a610), (0x67a480, 0x67a520),
                       (0x516130, 0x5161b0), (0x4cd300, 0x4cd720), (0x5da3c0, 0x5da3d0), (0x4f7d70, 0x4f7da0), (0x67c290, 0x67d340), (0x50b350, 0x50b360),
                       (0x4f04c0, 0x4f04d0), (0x4f0240, 0x4f0250), (0x4ea220, 0x4ea240), (0x4d8690, 0x4d86d0), (0x4ebf00, 0x4ebfb0), (0x4f2d50, 0x4f3290)],
          hooks={**cc.HOOKS, 0x6acd60: _recorder(25, 7, cdecl=True), 0x481850: _recorder(29, 2, com=True), 0x4818a0: _recorder(30, 2, com=True),
                 0x481840: _recorder(31, 1, com=True), 0x481890: _recorder(32, 1, com=True), 0x4f7b20: _recorder(3, 4), 0x50b780: _recorder(4, 0),
                 0x4c4a80: _recorder(5, 1), 0x5b87e0: _recorder(6, 3), 0x4f7de0: _recorder(7, 2), 0x4f7e20: _recorder(8, 2), 0x4f9ed0: _recorder(10, 0),
                 0x4f9e60: _recorder(11, 5), 0x4755d0: _recorder(14, 3, cdecl=True), 0x4ebfb0: _recorder(13, 2), 0x4bd400: _recorder(15, 1, cdecl=True)})
# The Chat's methods the events call (slot: k, stack arguments with the context)
METHODS = {0x6fd4bc: (1, 1), 0x6fd484: (9, 2), 0x6fd488: (12, 2), 0x6fd4cc: (2, 4)}
# The format's string arguments by code (the codes of no format: 0)
ARGS = {**{c: 0 for c in (0xb, 0xc, 0xf, 0x10, 0x44, 0x47)}, **{c: 2 for c in (0x11, 0x12, 0x13, 0x16, 0x18, 0x1a, 0x1c, 0x3f, 0x40)}, 0x3d: 3,
        **{c: 1 for c in (0x1e, 0x1f, 0x23, 0x24, 0x2a, 0x2e, 0x2f, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x41, 0x42, 0x45, 0x51)}}
WORLDLY = [0x22, 0x25, 0x26, 0x27, 0x4f, 0x52]
# A new buddy's node (the first block, or the one past the strings before it): its key's allocator byte +0xc is stack garbage, copied from a local nothing sets
NEW_BUDDY = [(HEAP + 0x1c, 1), (HEAP + 0xbc, 1)]
CODES = sorted(ARGS) + WORLDLY * 4 + [0x23, 0x24, 0x38, 0x39, 0x3a, 0x3b] * 2 + [0xf, 0x10] * 3 + [0, 0xa, 0xd, 0x14, 0x53, 0xffffffff]


def imports(regions, log):
    table = bytearray(0x604)
    for slot, (k, n) in METHODS.items():
        put(table, slot - IMPORT_TABLE, 'I', recorder(k, n))
    put(table, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(23, 1, com=True))   # OutputDebugStringA
    strings(log, 23, 1)
    for k in (9, 12, 21):   # (a std::wstring argument: its pointer, length, reserve and first characters)
        blob_bytes(log, k, 0, 0xfff0, inner=(4, 0x7fff))
    blob_bytes(log, 3, 2, 0xfff0, inner=(4, 0x7fff))   # the player's name
    for k in (24, 27, 28, 29, 30, 31, 32):   # (the window lists' texts: the first 32 characters)
        blob(log, k, 0, 0xffff)
    put(log, LOG_BYTES + 4 * 3, 'I', 1 << 16)   # (the player's cell: a WORD, in a stack slot of other uses)
    for k in (7, 8, 13):   # (the cells and coordinates: WORDs)
        put(log, LOG_BYTES + 4 * k, 'I', 0x30000)
    put(log, LOG_BYTES + 4 * 15, 'I', 1)   # (SetC3128's value: a BYTE)
    regions.update({IMPORT_TABLE: table, LOG: log})


def buddies(r, regions, avatar, names, pool):
    """The buddy set: tree()'s, the colours moved to +0x20 and two bytes at +0x1c."""
    nil = tree(r, regions, avatar, 'B', names, pool)
    nodes = regions[TREES['B'][0]]
    for i in range(len(names)):
        put(nodes, 0x40 * i + 0x20, 'I', struct.unpack_from('<I', nodes, 0x40 * i + 0x1c)[0])
        nodes[0x40 * i + 0x1c:0x40 * i + 0x1e] = bytes(r.choice([0, 0, 1, 0xff]) for _ in range(2))
    put(regions[nil], 0x20, 'I', 1)
    return nil


def world(r):
    """(regions, avatar, pool, log, names to pick from, the chat players' names): the avatar and its trees, and the park."""
    pool = Pool(r)
    avatar = bytearray(r.bytes(0xf10))
    put(avatar, 0, 'I', CHAT)
    people = [name(r, r.choice([1, 2, 3, 4])) for _ in range(5)]
    own = r.choice(people + [()])
    pool.string(avatar, 8, own)
    for off in (0xe60, 0xe70, 0xefc):
        pool.string(avatar, off, r.choice(people) if r.random() < 0.6 else name(r))
    regions = {}
    sets = {}
    for which in 'DBI':
        chosen = {}
        for _ in range(r.choice([0, 1, 2, 3, 5])):
            s = flip(r.choice(people), 0.3, r) if r.random() < 0.8 else name(r)
            chosen.setdefault(s if which == 'D' else fold(s), s)
        sets[which] = [chosen[k] for k in sorted(chosen)]
    nil_d = tree(r, regions, avatar, 'D', sets['D'], pool)
    nil_b = buddies(r, regions, avatar, sets['B'], pool)
    nil_i = tree(r, regions, avatar, 'I', sets['I'], pool)
    avatar[0xef8] = r.choice([0, 0, 0, 1, 2])
    parked, _, things = thingenv.park(r, defs=False, pool=THINGS)
    buf = bytearray(parked[THINGS])
    players = []
    for t in things:
        s = flip(r.choice(people), 0.2, r) if r.random() < 0.8 else name(r)
        if r.random() < 0.7:
            buf[t - THINGS + 2] = 0x12
            players.append(s)
        put(buf, t - THINGS + 0x19c, '%dH' % (len(s) + 1), *s, 0)
    parked[THINGS] = buf
    regions.update(parked)
    put(avatar, 4, 'I', r.choice(things) if things and r.random() < 0.7 else ME)
    candidates = people + sets['D'] + sets['B'] + sets['I'] + [own]
    log = log_region()
    regions.update({AVATAR: avatar, CHAT: r.bytes(0x20), POOL: pool.buf, HEAP: heap(0x4000), ME: r.bytes(0x40),
                    0x7ced24: struct.pack('<5I', nil_d, TREES['D'][0] + 0x800, nil_b, TREES['B'][0] + 0x800, nil_i),
                    0xfb1440: bytes(4), 0x78bac8: struct.pack('<I', r.getrandbits(32))})
    save(r, regions)
    imports(regions, log)
    formats(r, log)
    set_return(log, 21, r.getrandbits(32))
    return regions, avatar, pool, log, candidates, players


def session(r, regions, log):
    room = r.choice([0, 5, 12, 0x1234])
    obj = put(bytearray(r.bytes(0x50)), 8, 'I', room)
    sess = put(bytearray(r.bytes(0x400)), 0x3a4, 'I', OBJ)
    regions.update({OBJ: obj, SESSION: sess})
    if r.random() < 0.3:
        regions[0xf7d88c] = bytes(4)
        set_return(log, 22, SESSION)
    else:
        regions[0xf7d88c] = struct.pack('<I', SESSION)
    # The token's narrow string: its digits, that room or not, or none
    digits = b'%d\0' % (room if r.random() < 0.7 else r.choice([0, 5, 12]))
    regions[DIGITS] = digits
    regions[NARROW] = struct.pack('<4I', 0, 0 if r.random() < 0.15 else DIGITS, len(digits) - 1, len(digits) - 1)
    set_return(log, 5, NARROW)
    fill(log, 5, 0, bytes(0x10))
    strings(log, 5, 0, out=1)
    set_return(log, 13, r.choice([0, 1, 1, 0x100, 0x101]))


def event(r):
    regions, avatar, pool, log, candidates, players = world(r)
    code = r.deal(CODES)
    toks = bytearray(r.bytes(0xd8))
    for i in range(6):
        s = flip(r.choice(candidates), 0.4, r) if r.random() < 0.75 else name(r)
        if i and r.random() < 0.08:
            s = ()
        pool.string(toks, 0x24 * i + 0x10, s, null=0.5)
        put(toks, 0x24 * i + 0x20, 'I', code if i == 0 else r.choice([0, 1, 2, 3, 0xff, 0x100, 0x1234, 0xffffffff, r.getrandbits(32) | 0x80000000, r.getrandbits(16)]))
    if code in (0x22, 0x26, 0x52) and players and r.random() < 0.7:
        pool.string(toks, 0x24 + 0x10, r.choice(players), null=0.5)
    if code == 0x22:
        put(toks, 0x24 * 4 + 0x20, 'I', r.choice([0, 1, 2, 3, 3, 4, 0xffffffff]))
        for i in (2, 3):   # (a negative word's divisions by 256: not a shift)
            if r.random() < 0.4:
                put(toks, 0x24 * i + 0x20, 'I', r.getrandbits(32) | 0x80000000)
    if code == 0x27 and r.random() < 0.3:
        put(toks, 0x24 * 5 + 0x20, 'I', 0xffffffff)
    if code in (0xf, 0x10, 0x38):   # (the words the code asks for)
        put(toks, 0x24 + 0x20, 'I', r.choice([0, 0, 1, 5]))
        put(toks, 0x48 + 0x20, 'I', r.choice([0, 0, 1, 5]))
    count = r.choice([1, 2, 2, 3, 3, 4, 0])
    last = TOKENS + 0x24 * count
    if r.random() < 0.1:   # (a size that is not a whole number of tokens, or negative: the count's signed division)
        last = TOKENS + r.choice([0x20, 0x23, 0x44, 0x47, -4, -0x24, -0x48])
    regions[TOKVEC] = put(bytearray(r.bytes(0x10)), 4, '3I', TOKENS, last & 0xffffffff, TOKENS + 0xd8)
    regions[TOKENS] = toks
    if code == 0x52:
        session(r, regions, log)
    # One of the format's string arguments hashed (each a stack structure); the rest, and those not passed, logged as 0
    n = ARGS.get(code, 0)
    chosen = 4 + r.randrange(n) if n else None
    if n:
        blob(log, 25, chosen, 0xf)
    strings(log, 25, 0, out=sum(1 << a for a in (4, 5, 6) if a != chosen))
    return Call(0x4e6e80, [AVATAR, TOKVEC], regions, conv='t', ret='v', ignore=NEW_BUDDY if code == 0x38 else [])


def named(r, regions, candidates, pool):
    """A std::wstring at an address of any low byte, of the avatar's names or not."""
    at = NAMEW + r.randrange(0x40)
    word = bytearray(r.bytes(0x10))
    pool.string(word, 0, flip(r.choice(candidates), 0.4, r) if r.random() < 0.8 else name(r), null=0.5)
    regions[at] = word
    return at


def case(r):
    op = r.deal(OPS)
    if op == 'Event':
        return event(r)
    regions, avatar, pool, log, candidates, _ = world(r)
    at = named(r, regions, candidates, pool)
    if op == 'SetBuddy':
        return Call(0x4ee710, [AVATAR, at, r.getrandbits(32) if r.random() < 0.2 else r.randrange(8)], regions, conv='t', ret='b', ignore=NEW_BUDDY)
    if op == 'EraseBuddy':
        return Call(0x4f2700, [AVATAR + 0xe90, at], regions, conv='t', ret='a')
    regions[OUT] = r.bytes(0x10)
    return Call(0x4ea0e0, [OUT, at], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Event,Event,Event,Event,Event,Event,SetBuddy,EraseBuddy,CopyC').split(',') if k]

if __name__ == '__main__':
    B.main(case)
