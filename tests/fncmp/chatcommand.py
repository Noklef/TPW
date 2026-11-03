# The chat avatar's commands of src/game/online/chatcommand (0x4e9fa0, 0x4ea890, 0x4ec030) against the originals.
# Text (0x4e9fa0): a token's std::wstring (a block of the pool after a count byte 0, 1, 2, 0xfe or 0xff, or null) copied into a random out.
# Notice (0x4ea890): such a string by value and a colour; the global save info [0xf7c58c] (its byte +0x25 set or not; now and then none, made by
# 0x409410 as globalsave.py's Instance) and the censor 0x5d3230 (agent B's) and Window7cc8bc_ChatAddStd recorders (the text hashed).
# Run (0x4ec030): an avatar (+8 its own name, +0xe60 / +0xe70 the reply targets, +0xe80 a tree D of 0-5 names, +0xe90 / +0xea0 the buddy /
# ignore sets of 0-5 names sorted without case, +0xeb0 / +0xeb1 / +0xef8 bytes, +0xec4 a vote or none, +0xefc a string) told a command of
# every kind (Goto, the vote, Ignore and the buddies more often) with 0-4 tokens (4 always mapped; none at all now and then); the names
# are a few strings of a small alphabet about A-Z / a-z, the tokens' often one of the avatar's own, a set's or a target's with case
# flips. The Chat's methods (imports 0x6fd484-0x6fd4d4, 0x6fd5f4-0x6fd600) and OutputDebugStringA recorders; the std::wstring arguments
# hashed as their pointer, length and reserve and their first characters (Tell's second argument, a stack string, logged as 0). The
# charset's format (0x6acd60) a recorder writing one of two texts (or one of up to 55 characters) into the avatar's +0x260, its string
# argument hashed for the entries 0x56, 0x57 and 0x7f; agent B's assign (0x4051d0: the original's, the driver's stand-in
# ChatCommand_AssignChars), session constructor (0x5b50b0: answering the session) and dialog (0x47f020: its message hashed but byte 7).
# The vote's session [0xf7d88c] (none: made) has its +0x3a4 object (+0x48 the key, +8 y) and the owner +0xe0 whose list always holds an
# item with the key (its +0x60 flag set or not); the Vote's constructor answering 0 now and then, Connect 1 or not.
# The callbacks (0x4ead00-0x4eb5d0: Blackmark, Page, Locate, Goto, Ignore with a name; Vote, Earmuffs alone): Run's world for their
# command, the name a std::wstring (at an address of any low byte: the allocator bytes' source) of the avatar's names or not; their tokens
# built by the originals' CountedPair inserts (countedpair.py's ranges) on the heap and run by 0x4ec030. NoticeText (0x4ee590): the name
# as the text; GotoDone (0x4eea50): Run's world (notice 0x9d).
# usage: chatcommand.py [CASES [SEED]] [--wild|--both] [--show N]
#   (env OPS=Text,Notice,Run,Blackmark,Page,Locate,Goto,Ignore,Vote,Earmuffs,NoticeText,GotoDone)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, Mask, _recorder, blob, blob_bytes, blob_when, fill, heap, log_region, put, recorder, set_return, strings  # noqa: E402
import stdtree  # noqa: E402
import wstringmap  # noqa: E402

GLOBALSAVE = [(0x5af7f0, 0x5af80b), (0x5afc30, 0x5afc5a), (0x5b09a0, 0x5b09b4), (0x5b0ca0, 0x5b0cc5), (0x5b0940, 0x5b0996), (0x67b0d0, 0x67b10f),
              (0x5aef50, 0x5af0dc), (0x409410, 0x40947a), (0x5b3110, 0x5b3130), (0x5b2fd0, 0x5b2ff0), (0x5b1680, 0x5b16a6), (0x5b11a0, 0x5b11b9),
              (0x5b2480, 0x5b2530)]
WSTDSTRING = [(0x4f21d0, 0x4f23e2), (0x4f23f0, 0x4f2437), (0x4f39e0, 0x4f3b06), (0x4f6080, 0x4f6086), (0x4f6090, 0x4f618e), (0x4f6190, 0x4f6266),
              (0x479150, 0x47919d), (0x47ba30, 0x47ba50), (0x67acd0, 0x67acee), (0x4f0250, 0x4f0367), (0x479230, 0x479256), (0x4f7720, 0x4f776f),
              (0x4f7900, 0x4f7966), (0x67c8d0, 0x67c9f0)]
COUNTEDPAIR = [(0x405280, 0x405390), (0x45b380, 0x45b5f0), (0x45b680, 0x45b9b0), (0x479230, 0x479270), (0x4e6410, 0x4e6560), (0x4f23f0, 0x4f2450),
               (0x4f5860, 0x4f5988), (0x4f6080, 0x4f60a0), (0x67acd0, 0x67ad00), (0x4f5560, 0x4f5852), (0x4e51a0, 0x4e520e),
               (0x4f67b0, 0x4f67e1), (0x4f67f0, 0x4f6819), (0x4f7410, 0x4f7423), (0x4f6e70, 0x4f6ee9), (0x4f36c0, 0x4f36e1)]
B = Bench([(0x4e9fa0, 0x4ea0d3), (0x4ea890, 0x4ea998), (0x4ec030, 0x4ee584), (0x4051d0, 0x405272), (0x4a77a0, 0x4a77d0), (0x5b96b0, 0x5b96b7),
           (0x5bc9c0, 0x5bc9e7), (0x5b6490, 0x5b6494), (0x41ce80, 0x41ce94), (0x4bd450, 0x4bd492), (0x4bae30, 0x4bae3f), (0x6918f0, 0x691a30),
           (0x691ba0, 0x691ce0), (0x4ead00, 0x4ebafa), (0x4ebee0, 0x4ebf00), (0x4ee590, 0x4ee6cb), (0x4eea50, 0x4eeb49), (0x4ea740, 0x4ea7d9), (0x4f0150, 0x4f0169)]
          + wstringmap.RANGES + WSTDSTRING + GLOBALSAVE + COUNTEDPAIR,
          hooks={**wstringmap.HOOKS, 0x6acd60: _recorder(25, 5, cdecl=True), 0x5d3230: _recorder(21, 2), 0x481800: _recorder(24, 2, com=True),
                 0x5b50b0: _recorder(22, 0), 0x47f020: _recorder(26, 8, cdecl=True)})
CALLS = {'Text': 0x4e9fa0, 'Notice': 0x4ea890, 'Run': 0x4ec030}
# The callbacks: the address, the command they run (Run's world), whether they take a name (a std::wstring *)
CALLBACKS = {'Blackmark': (0x4ead00, 0xe, True), 'Page': (0x4eae80, 0x11, True), 'Locate': (0x4eb000, 0x14, True), 'Goto': (0x4eb180, 0x13, True),
             'Ignore': (0x4eb450, 0xd, True), 'Vote': (0x4eb300, 0x15, False), 'Earmuffs': (0x4eb5d0, 0xc, False),
             'NoticeText': (0x4ee590, 0x40, True), 'GotoDone': (0x4eea50, 0x40, False), 'AddBuddy': (0x4eb720, 0xf, True),
             'RemoveBuddy': (0x4eb910, 0xf, True), 'EarmuffsOn': (0x4ebee0, 0xc, False), 'EarmuffsOff': (0x4ebef0, 0xc, False),
             'NameFlags': (0x4ee5c0, 0x40, True)}
AVATAR, CHAT, TOKVEC, TOKENS, OUT, NAMEW = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1300, RAM + 0x1400
POOL = RAM + 0x2000   # the strings' blocks: a count byte, then the characters
TREES = {'D': (RAM + 0x3000, 0xe80), 'B': (RAM + 0x4000, 0xe90), 'I': (RAM + 0x5000, 0xea0)}   # nodes (+0x800 the head, +0x900 the nil), the avatar's offset
GSAVE, SESSION, OBJ, OWNER, ITEMS, HOST, NILS = RAM + 0x6000, RAM + 0x6100, RAM + 0x6600, RAM + 0x6700, RAM + 0x6800, RAM + 0x6a00, RAM + 0x6b00
ALPHA = [0x41, 0x42, 0x61, 0x62, 0x5a, 0x7a, 0x100, 0xffff]
GARBAGE = [0, 0xc, 0x28, 0x29, 0x38, 0x39]
# (the commands: every kind, Goto, the vote, Ignore and the buddies more often, some past the table)
COMMANDS = list(range(0x19)) + [0x13, 0x13, 0x15, 0x15, 0xd, 0xd, 0xf, 0xf, 0x1, 0x2, 0x3, 0x19, 0x40, 0xffffffff]
# Kinds of recorder (k): the Chat's methods (thiscall: slot address, k, stack arguments with the context)
METHODS = {0x6fd4bc: (1, 1), 0x6fd4b8: (2, 3), 0x6fd4b4: (3, 4), 0x6fd4b0: (4, 3), 0x6fd4d4: (5, 2), 0x6fd4a8: (6, 1), 0x6fd4ac: (7, 1),
           0x6fd4a4: (8, 2), 0x6fd484: (9, 2), 0x6fd4a0: (10, 2), 0x6fd49c: (11, 2), 0x6fd488: (12, 2), 0x6fd4d0: (13, 2), 0x6fd4c8: (14, 1),
           0x6fd490: (15, 2), 0x6fd48c: (16, 2), 0x6fd5f4: (17, 0), 0x6fd5f8: (18, 4), 0x6fd5fc: (19, 4), 0x6fd600: (20, 4)}
TEXTS = (2, 3, 4, 8, 9, 10, 11, 12, 13, 15, 16)   # (the methods whose first argument is a std::wstring)


def flip(s, p=0.5, r=None):
    return tuple(c ^ 0x20 if (0x41 <= c <= 0x5a or 0x61 <= c <= 0x7a) and r.random() < p else c for c in s)


def fold(s):
    return tuple(c | 0x20 if 0x41 <= c <= 0x5a else c for c in s)


class Pool:
    """Strings' blocks in the pool region: each a count byte, the characters, a terminator and room for the reserve."""

    def __init__(self, r):
        self.r, self.buf, self.at = r, bytearray(r.bytes(0x1000)), 2

    def text(self, s, null=0.4):
        """(ptr, len, res) of a std::wstring of the characters s: a block (count 0, 1, 2, 0xfe or 0xff), an empty one null now and then."""
        r = self.r
        if not s and r.random() < null:
            return 0, 0, 0
        res = len(s) + r.choice([0, 0, 0, 2, 31])
        at = self.at
        self.buf[at - 1] = r.choice([0, 0, 1, 2, 0xfe, 0xff])
        struct.pack_into('<%dH' % (len(s) + 1), self.buf, at, *s, 0)
        self.at += 2 * res + 8
        assert self.at < len(self.buf)
        return POOL + at, len(s), res

    def string(self, buf, off, s, null=0.4):
        """The std::wstring at buf + off (its allocator byte and padding random) the characters s."""
        put(buf, off, 'I3I', self.r.getrandbits(32), *self.text(s, null))


def name(r, k=None):
    return tuple(r.choice(ALPHA) for _ in range(r.choice([0, 1, 1, 2, 3, 4] if k is None else [k])))


def tree(r, regions, avatar, which, names, pool):
    """Tree `which` of the names (in order as given), its nodes, head and nil; the avatar's tree header points at its head."""
    nodes_at, off = TREES[which]
    head_at, nil_at = nodes_at + 0x800, nodes_at + 0x900
    n = len(names)
    nodes, root = stdtree.build(r, n) if n else ({}, None)
    keys = sorted(nodes)
    addr = {k: nodes_at + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: nil_at if k is None else addr[k]  # noqa: E731
    buf = bytearray(r.bytes(0x40 * max(n, 1)))
    for i, k in enumerate(keys):
        base = addr[k] - nodes_at
        put(buf, base, '3I', ptr(nodes[k][0]), head_at if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        pool.string(buf, base + 0xc, names[i], null=0.5)
        put(buf, base + 0x1c, 'I', nodes[k][3])
    head = put(bytearray(r.bytes(0x40)), 0, '3I', addr[keys[0]] if keys else head_at, addr[root] if keys else nil_at, addr[keys[-1]] if keys else head_at)
    regions.update({nodes_at: buf, head_at: head, nil_at: put(bytearray(r.bytes(0x40)), 0, '3I', nil_at, nil_at, nil_at)})
    put(regions[nil_at], 0x1c, 'I', 1)
    put(avatar, off + 4, 'I', head_at)
    put(avatar, off + 0xc, 'I', n)
    return nil_at


def imports(r, regions, log):
    table = bytearray(0x604)   # (to slot 0x6fd600, short of _Nullstr 0x6fd6c4)
    for slot, (k, n) in METHODS.items():
        put(table, slot - IMPORT_TABLE, 'I', recorder(k, n))
    put(table, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(23, 1, com=True))   # OutputDebugStringA
    for k in TEXTS:   # (a std::wstring argument: its pointer, length, reserve and first characters; the allocator byte is stack garbage at times)
        blob_bytes(log, k, 0, 0xfff0, inner=(4, 0x7fff))
    strings(log, 3, 0, out=0b10)   # (Tell's message: a stack string, logged as 0)
    strings(log, 23, 1)
    blob_bytes(log, 21, 0, 0xfff0, inner=(4, 0x7fff))   # the censor's text (a stack std::wstring)
    blob(log, 24, 0, 0xffff)   # ChatAddStd's text: its first 32 characters
    blob(log, 25, 4, 0, only_when=True)   # the format's string argument for the entries that take one (else stack garbage, logged as 0)
    for entry, value in enumerate((0x56, 0x57, 0x7f)):
        blob_when(log, 25, 3, value, 0b1111, entry=entry)
    blob_bytes(log, 26, 0, 0xff7f)   # the dialog's message (byte 7 is stack garbage)
    blob(log, 20, 1, 0xf)   # SendVote's GUID
    put(log, LOG_BYTES + 4 * 18, 'I', 1 << 18)   # (Connect's port: a WORD)
    regions.update({IMPORT_TABLE: table, LOG: log})


def save(r, regions, instance=True):
    """The global save info [0xf7c58c] (its byte +0x25 0, 1 or other), or none: made by 0x409410 (globalsave.py's construct)."""
    info = bytearray(r.bytes(0x60))
    info[0x25] = r.choice([0, 0, 1, 1, 0xff])
    regions[GSAVE] = info
    if instance or r.random() < 0.7:
        regions[0xf7c58c] = struct.pack('<I', GSAVE)
        return []
    regions[0xf7c58c] = bytes(4)
    for nil, users, k in ((0xf7c970, 0xf7c96c, 0), (0xf7c968, 0xf7c964, 1)):
        regions[nil] = struct.pack('<I', r.choice([0, 0, NILS + 0x40 * k]))
        regions[users] = struct.pack('<I', r.choice([0, 1, 5]))
    regions[NILS] = r.bytes(0x80)
    return [Mask(HEAP, [0x10 + k for k in GARBAGE], 'uninitialised stack bytes (the allocators and comparators) written by 0x5aef50; nothing reads them')]


def text_case(r):
    pool = Pool(r)
    token = bytearray(r.bytes(0x24))
    pool.string(token, 0x10, name(r))
    regions = {TOKENS: token, OUT: r.bytes(0x10), POOL: pool.buf, HEAP: heap(0x400)}
    return Call(CALLS['Text'], [TOKENS, OUT], regions, conv='t', ret='a')


def notice_case(r):
    pool = Pool(r)
    regions = {AVATAR: r.bytes(0x10), HEAP: heap(0x1000)}
    masks = save(r, regions, instance=False)
    log = log_region()
    imports(r, regions, log)
    set_return(log, 21, r.getrandbits(32))
    s = pool.text(name(r, r.choice([0, 1, 3, 8, 20])))
    regions[POOL] = pool.buf
    return Call(CALLS['Notice'], [AVATAR, r.getrandbits(32), *s, r.choice([7, 0, r.getrandbits(32)])], regions, conv='t', ret='v', masks=masks)


def formats(r, log):
    """The format's texts: two of up to 27 characters (calls in turn), or one of up to 55; empty now and then."""
    if r.random() < 0.3:
        s = name(r, r.choice([0, 20, 31, 32, 40, 55]))
        fill(log, 25, 1, struct.pack('<%dH' % (len(s) + 1), *s, 0) + bytes(0x6e - 2 * len(s)))
        return
    data = b''
    for _ in range(2):
        s = name(r, r.choice([0, 1, 3, 8, 15, 27]))
        data += struct.pack('<%dH' % (len(s) + 1), *s, 0) + bytes(0x36 - 2 * len(s))
    fill(log, 25, 1, data, records=2)


def vote(r, regions, log):
    """The vote's world: the session (none now and then: made, Mem_Alloc and 0x5b50b0 answering it), its +0x3a4 object, the owner's list."""
    key = r.choice([1, 2, 0x55, r.getrandbits(32)])
    obj = put(bytearray(r.bytes(0x50)), 0x48, 'I', key)
    session = bytearray(r.bytes(0x400))
    put(session, 0x3a4, 'I', OBJ)
    put(session, 0xe0, 'I', OWNER)
    owner = put(bytearray(r.bytes(0x40)), 0x2c, 'I', OWNER + 0x30)
    put(owner, 0x34, 'I', ITEMS)   # (the list's first node)
    n = r.choice([1, 1, 2, 3])
    match = r.sample(range(n), r.choice([1, 1, 2]) if n > 1 else 1)
    items = bytearray(r.bytes(0x200))
    for i in range(n):
        node, item = 0x10 * i, 0x40 + 0x60 * i   # (nodes {item, next}; items 0x60 bytes: +8 the key, +0x60 the next's +0: the flag)
        put(items, node, '2I', ITEMS + item, ITEMS + 0x10 * (i + 1) if i + 1 < n else 0)
        put(items, item + 8, 'I', key if i in match else key ^ r.choice([1, 0x100]))
    for i in range(n + 1):
        put(items, 0x40 + 0x60 * i, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))   # (item i's +0x60 is item i + 1's +0)
    regions.update({OBJ: obj, SESSION: session, OWNER: owner, ITEMS: items})
    if r.random() < 0.3:
        regions[0xf7d88c] = bytes(4)
        set_return(log, 22, SESSION)
    else:
        regions[0xf7d88c] = struct.pack('<I', SESSION)
    host = bytearray(0x18)
    put(host, 0, 'I', r.choice([HOST, HOST, 0]))
    put(host, 0x16, 'H', r.getrandbits(16))
    regions.update({0x7a1544: host, HOST: b'chat.example\0', 0x7cc53c: bytes([r.choice([0, 1])]), 0x7cc8bc: bytes(4)})
    if r.random() < 0.2:
        set_return(log, 17, 0)
    set_return(log, 18, r.choice([1, 1, 1, 0, 2]))


def run_case(r, op='Run'):
    pool = Pool(r)
    avatar = bytearray(r.bytes(0xf10))
    put(avatar, 0, 'I', CHAT)
    people = [name(r, r.choice([1, 2, 3, 4])) for _ in range(5)]
    own = r.choice(people + [()])
    pool.string(avatar, 8, own)
    for off in (0xe60, 0xe70, 0xefc):
        pool.string(avatar, off, r.choice(people) if r.random() < 0.6 else name(r))
    regions = {}
    dnames = [flip(r.choice(people), 0.3, r) for _ in range(r.choice([0, 1, 2, 3, 5]))]
    nil_d = tree(r, regions, avatar, 'D', dnames, pool)
    sets = {}
    for which in 'BI':
        chosen = {}
        for _ in range(r.choice([0, 1, 2, 3, 5])):
            s = flip(r.choice(people), 0.3, r) if r.random() < 0.8 else name(r)
            chosen.setdefault(fold(s), s)
        sets[which] = [chosen[k] for k in sorted(chosen)]
    nil_b = tree(r, regions, avatar, 'B', sets['B'], pool)
    if op == 'NameFlags':   # (a buddy's two bytes: NameFlags's bits; nothing here walks by colour)
        for i in range(len(sets['B'])):
            regions[TREES['B'][0]][0x40 * i + 0x1c:0x40 * i + 0x1e] = bytes(r.choice([0, 0, 1, 0xff]) for _ in range(2))
    nil_i = tree(r, regions, avatar, 'I', sets['I'], pool)
    avatar[0xeb0] = r.choice([0, 0, 1, 0xff])
    avatar[0xeb1] = r.choice([0, 0, 1, 0xff])
    avatar[0xef8] = r.choice([0, 0, 0, 1, 2])
    command = r.deal(COMMANDS) if op == 'Run' else CALLBACKS[op][1]
    put(avatar, 0xec4, 'I', r.getrandbits(32) if r.random() < 0.15 else 0)
    candidates = people + dnames + sets['B'] + sets['I'] + [own]
    toks = bytearray(r.bytes(0x90))
    for i in range(4):
        s = flip(r.choice(candidates), 0.4, r) if r.random() < 0.75 else name(r)
        if i and r.random() < 0.08:
            s = ()
        pool.string(toks, 0x24 * i + 0x10, s, null=0.5)
        put(toks, 0x24 * i + 0x20, 'I', command if i == 0 else r.choice([0, 1, 5, 0x40, r.getrandbits(32)]))
    count = r.choice([1, 1, 2, 2, 2, 3, 4, 0])
    last = TOKENS + 0x24 * count
    if r.random() < 0.1:   # (a size that is not a whole number of tokens, or negative: the count's signed division)
        last = TOKENS + r.choice([0x20, 0x23, 0x44, 0x47, -4, -0x24, -0x48])
    vec = put(bytearray(r.bytes(0x10)), 4, '3I', TOKENS, last & 0xffffffff, TOKENS + 0x90)
    if count == 0 and r.random() < 0.5:
        put(vec, 4, '2I', 0, 0)
    if op != 'Run':   # (the callbacks' name: a std::wstring at an address with any low byte, one of the avatar's names or not)
        at = NAMEW + r.randrange(0x40)
        word = bytearray(r.bytes(0x10))
        pool.string(word, 0, flip(r.choice(candidates), 0.4, r) if r.random() < 0.8 else name(r), null=0.5)
        chars = struct.unpack_from('<I', word, 4)[0]
        if chars and op != 'NoticeText':   # (shared, not copied: no block of its own before the vector's, see ignore below)
            pool.buf[chars - POOL - 1] = r.choice([0, 0, 1, 2])
    log = log_region()
    regions.update({AVATAR: avatar, CHAT: r.bytes(0x20), TOKVEC: vec, TOKENS: toks, POOL: pool.buf, HEAP: heap(0x4000),
                    0x7ced24: struct.pack('<5I', nil_d, TREES['D'][0] + 0x800, nil_b, TREES['B'][0] + 0x800, nil_i),
                    0xfb1440: bytes(4), 0x78bac8: struct.pack('<I', r.getrandbits(32))})
    save(r, regions)
    imports(r, regions, log)
    formats(r, log)
    set_return(log, 21, r.getrandbits(32))
    if command == 0x15:
        vote(r, regions, log)
    if op == 'Run':
        return Call(CALLS['Run'], [AVATAR, TOKVEC], regions, conv='t', ret='v')
    del regions[TOKVEC], regions[TOKENS]
    # The tokens copied from the stack token (CountedPair_CopyCtor) take the low byte of its address as their strings' allocator bytes (sic),
    # which the two sides' stacks cannot share: the command's in the vector's first block (the heap's first), the name's in its second
    # (the next: the name's block is shared, not copied); the first token's copy in the second block takes the first block's address's.
    ignore = [(HEAP + 0x10, 1), (HEAP + 0x20, 1)] if op not in ('NoticeText', 'GotoDone', 'NameFlags') else []
    returns = 'a' if op == 'NameFlags' else 'v'
    if CALLBACKS[op][2]:
        regions[at] = word
        if ignore:
            ignore += [(HEAP + 0x40 + 0x24, 1), (HEAP + 0x40 + 0x34, 1)]
        return Call(CALLBACKS[op][0], [AVATAR, at], regions, conv='t', ret=returns, ignore=ignore)
    return Call(CALLBACKS[op][0], [AVATAR], regions, conv='t', ret=returns, ignore=ignore)


def case(r):
    op = r.deal(OPS)
    if op == 'Text':
        return text_case(r)
    if op == 'Notice':
        return notice_case(r)
    return run_case(r, op)


OPS = [k for k in os.environ.get('OPS', 'Text,Notice,Run,Run,Run,Run,Run,Run,' + ','.join(CALLBACKS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
