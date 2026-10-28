# The chat avatar's moves of src/game/online/chatavatar (0x4ea370, 0x4ea3f0, 0x4ea410, 0x4ebfb0, 0x4eab40, 0x4eac30) against the originals:
# things_methods.py's SetPos world (a park whose things sit in a cell grid), the avatar's thing now and then of kind 0x12 (so it is placed);
# Chat::MovePlayer and Chat::NotAFK (imports 0x6fd4cc, 0x6fd4bc) recorders; the cell moves with cells in the grid's four rows and random high
# words. GoTo: movingperson.py's person sent to a cell. Arrive (0x4ea240): the thing's script slots as personscripts.py's release. Find and
# NameIs (0x4ebf00, 0x4f7d70): a park of named chat players and others, looked up by a name of theirs, one near it or another. Move1 (0x4eace0):
# Chat::MovePlayer with mode 1 alone. Room (0x4eeb50): the avatar's wide string +0xf00 (or none) as a std::wstring told to Chat::Goto (import
# 0x6fd490) and released. Enter (0x4ea2d0): Move's world, the thing's script list as personscripts.py's Place's (entry point 3), a small ground.
# AddName (0x4ee6d0): see add_name() below. EnterChat (0x4bd090, src/game/ui/window7cc8bc): see enter_chat() below. LeaveChat
# (0x4bd260, the same): see leave_chat() below.
# usage: chatavatar.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Move,Say,Move3,Cell,CellMove,GoTo,Arrive,Find,NameIs,Move1,Room,Enter,AddName,EnterChat,LeaveChat)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import things_methods  # noqa: E402
from fncmp import CDECL_LOG4, HEAP, IMPORT_TABLE, LOG, Bench, Call, RAM, blob, heap, log_region, put, recorder  # noqa: E402
import personscripts  # noqa: E402
import stdtree  # noqa: E402
import terrainheight  # noqa: E402
import thingenv  # noqa: E402
import dialog7cc5f4  # noqa: E402
import window7059f0 as w7  # noqa: E402
import window7cc8bc  # noqa: E402
import wstringmap  # noqa: E402
from thingenv import POOL, SLOTS  # noqa: E402

_ops = os.environ.pop('OPS', None)   # (movingperson.py reads its own OPS, as hex addresses)
import movingperson  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(movingperson.RANGES + personscripts.PLACE_RANGES + [(0x4ea2d0, 0x4ea36e), 
           (0x4ea240, 0x4ea2c4), (0x4ea370, 0x4ea3e5), (0x4ea3f0, 0x4ea409), (0x4ea410, 0x4ea485), (0x4eab40, 0x4eac22), (0x4eac30, 0x4eacdc),
           (0x4ebf00, 0x4ebfa8), (0x4ebfb0, 0x4ec021), (0x4f7d70, 0x4f7d98), (0x4f7e20, 0x4f7e4c), (0x4217f0, 0x421800), (0x67c290, 0x67c2e2),
           (0x515f30, 0x516310), (0x4f9ed0, 0x4f9efc), (0x4d4170, 0x4d4186), (0x475550, 0x47558b), (0x4fe950, 0x4fe977), (0x475010, 0x476a30),
           (0x540b90, 0x540da0), (0x50c0d0, 0x50c0f2), (0x50bab0, 0x50bac4), (0x50be40, 0x50be7c), (0x4d91f0, 0x4d93c0), (0x4fa930, 0x4fa9a0), (0x50b350, 0x50b370), (0x50b6a0, 0x50b790), (0x5100e0, 0x510110), (0x510160, 0x510190),
           (0x412e90, 0x412f00), (0x516270, 0x516320), (0x516330, 0x516390), (0x541f60, 0x541fb0), (0x6ace60, 0x6aceb0), (0x5da3c0, 0x5da3c1),
           (0x4eace0, 0x4eacfa), (0x4eeb50, 0x4eec32), (0x405280, 0x405390), (0x47bad0, 0x47bc10), (0x67acd0, 0x67ad00), (0x45b380, 0x45b5f0),
           (0x45b680, 0x45b9b0), (0x67bb90, 0x67bd98), (0x67d250, 0x67d2c0), (0x67d3a0, 0x67d400), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0),
           (0x4ee6d0, 0x4ee70d)] + wstringmap.RANGES   # (AddName's: window7cc8bc.py's list and the tree D insert)
          + dialog7cc5f4.merged(w7.LIST_RANGES + [(0x4beb60, 0x4bedd1), (0x481820, 0x4818b3), (0x65f538, 0x65f5b7), (0x65e59b, 0x65e60a), (0x65f66e, 0x65f6e1),
                                (0x4bd090, 0x4bd25c), (0x658d9f, 0x658e04), (0x659710, 0x659740), (0x659d00, 0x659d53), (0x659d90, 0x659dbc),
                                (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x65d42e, 0x65d4b0), (0x65f697, 0x65f6e1), (0x66bff7, 0x66c01b),
                                (0x4bd260, 0x4bd3f5), (0x65f11d, 0x65f150), (0x46cf10, 0x46cf7d)]),
          hooks=wstringmap.HOOKS)
CALLS = {'Move': 0x4ea370, 'Say': 0x4ea3f0, 'Move3': 0x4ea410, 'Cell': 0x4ebfb0, 'CellMove': 0x4eab40, 'GoTo': 0x4eac30, 'Arrive': 0x4ea240,
         'Find': 0x4ebf00, 'NameIs': 0x4f7d70, 'Move1': 0x4eace0, 'Room': 0x4eeb50, 'Enter': 0x4ea2d0, 'AddName': 0x4ee6d0,
         'EnterChat': 0x4bd090, 'LeaveChat': 0x4bd260}
AVATAR, CHAT, NAME, CHARS, ROOM = RAM + 0x20000, RAM + 0x21000, RAM + 0x21100, RAM + 0x21200, RAM + 0x21300
WIN_A, WIN_B = RAM + 0x23000, RAM + 0x23200   # (EnterChat's windows)
GROUND = RAM + 0x22000   # (Enter's ground: the map, its grid, heights and cells)
TNODES, THEAD, TNIL, TCHARS, NAMEW = RAM + 0x40000, RAM + 0x41000, RAM + 0x41100, RAM + 0x41200, RAM + 0x41800   # (AddName's set and name)


def imports(regions):
    table = bytearray(0x600)
    struct.pack_into('<I', table, 0x6fd4cc - IMPORT_TABLE, recorder(1, 4))
    struct.pack_into('<I', table, 0x6fd4bc - IMPORT_TABLE, recorder(2, 1))
    struct.pack_into('<I', table, 0x6fd490 - IMPORT_TABLE, recorder(3, 2))
    for slot in (0x6fd1cc, 0x6fd26c):   # (global_heap's GlobalAlloc / GlobalFree, when the case has them)
        if slot in regions:
            table[slot - IMPORT_TABLE:slot - IMPORT_TABLE + 4] = regions.pop(slot)
    log = log_region()
    blob(log, 3, 0, 0b1110)   # (Chat::Goto's std::wstring is on the stack: its pointer, length and reserve hashed)
    regions.update({LOG: log, IMPORT_TABLE: table})


def ground(r, regions):
    """Enter's ground for Terrain_HeightAt at (0, 0) (terrainheight.py's own sits where the park does): a grid of 0-3 cells each way of
    size 10 (or 0 or NaN), terrainheight.py's heights, any cell flags."""
    grid = bytearray(r.bytes(0x30))
    put(grid, 0x10, '2f', *(r.choice([10.0, 10.0, 10.0, 0.0, float('nan')]) for _ in range(2)))
    put(grid, 0x18, '2I', r.randrange(4), r.randrange(4))
    put(grid, 0x28, '2I', GROUND + 0x100, GROUND + 0x200)
    regions.update({0x7a0854: struct.pack('<I', GROUND), GROUND: put(bytearray(r.bytes(0x70)), 0x6c, 'I', GROUND + 0x80), GROUND + 0x80: grid,
                    GROUND + 0x100: struct.pack('<16f', *(terrainheight.height(r) for _ in range(16))), GROUND + 0x200: r.bytes(0x40)})


def move1(r):
    """Move1 (0x4eace0): Chat::MovePlayer(x, y as 16-bit values, mode 1, the avatar); its result."""
    regions = {AVATAR: put(bytearray(r.bytes(0x10)), 0, 'I', CHAT)}
    imports(regions)
    x, y = [r.choice([0, 1, 0x7fff, 0x8000, 0xffff, 0x10000, r.getrandbits(16), r.getrandbits(32)]) for _ in range(2)]
    return Call(CALLS['Move1'], [AVATAR, x, y], regions, conv='t', ret='a')


def room(r):
    """Room (0x4eeb50): the avatar's wide string at +0xf00 (0-40 characters at ROOM; null: the empty string 0x6fd6c4) copied into a std::wstring on
    the stack (StdWString_Grow's block from the heap) and told to Chat::Goto with the avatar; the string released."""
    n = r.choice([0, 0, 1, 2, 5, 6, 31, 32, 40])
    regions = {ROOM: struct.pack('<%dH' % (n + 1), *[r.choice([0x41, 0x100, 0xffff, r.getrandbits(16) | 1]) for _ in range(n)], 0),
               HEAP: heap(0x200), AVATAR: put(bytearray(r.bytes(0xf04)), 0, 'I', CHAT)}
    put(regions[AVATAR], 0xf00, 'I', ROOM if r.random() < 0.8 else 0)
    imports(regions)
    return Call(CALLS['Room'], [AVATAR], regions, conv='t', ret='v')


def goto(r):
    """GoTo: movingperson.py's person (at RAM) sent to a cell (ids over the map, 0 and past it), its handle's slot it, of kind 0x12 or not."""
    c = movingperson.cell(r, 0x4f7da0)
    regions = dict(c.regions)
    person = bytearray(regions[RAM])
    handle = r.randrange(1, 12)
    struct.pack_into('<H', person, 0, handle)
    if r.random() < 0.7:
        person[2] = 0x12
    slots = bytearray(r.bytes(20 * 12))
    struct.pack_into('<I', slots, 20 * handle, RAM)
    avatar = bytearray(r.bytes(0xec0))
    struct.pack_into('<II', avatar, 0, CHAT, RAM)
    regions.update({RAM: person, SLOTS: slots, AVATAR: avatar, CHAT: r.bytes(0x20)})
    imports(regions)
    return Call(CALLS['GoTo'], [AVATAR, c.args[1]], regions, conv='t', ret='b')


def names(r):
    """Find / NameIs: thingenv.py's park, its things now and then chat players (kind 0x12) named from a small alphabet (0-3 characters at
    +0x19c), the name (a std::wstring at NAME: characters at CHARS, or null) one of theirs or not."""
    regions, handles, things = thingenv.park(r, defs=False)
    pool = bytearray(regions[POOL])
    alphabet = [0x41, 0x42, 0x100, 0xffff]
    named = []
    for t in things:
        if r.random() < 0.5:
            pool[t - POOL + 2] = 0x12
        s = [r.choice(alphabet) for _ in range(r.choice([0, 1, 2, 3]))]
        struct.pack_into('<%dH' % (len(s) + 1), pool, t - POOL + 0x19c, *s, 0)
        named.append(s)
    regions[POOL] = pool
    s = r.choice(named) if named and r.random() < 0.6 else [r.choice(alphabet) for _ in range(r.choice([0, 1, 2, 3]))]
    if r.random() < 0.3 and s:
        s = s[:-1] if r.random() < 0.5 else s + [0x41]
    ptr = 0 if not s and r.random() < 0.5 else CHARS
    regions[CHARS] = struct.pack('<%dH' % (len(s) + 1), *s, 0)
    regions[NAME] = struct.pack('<IIII', r.getrandbits(32), ptr, len(s), len(s))
    return regions, things


def add_name(r):
    """AddName (0x4ee6d0): window7cc8bc.py's Add (mode 0) with the name's characters (one of window7059f0.py's add_case strings, copied after
    a count byte; an empty one null half the time: the empty string 0x6fd6c4) and data 0; the avatar's set of names at +0xe80 as wstringmap.py's MapInsert
    tree (0-8 names of window7059f0.py's alphabet, the name among them or not, the multi flag now and then set)."""
    while True:
        c = w7.add_case(r, columns=(0, 1, 1, 1))
        text = struct.unpack_from('<I', c.regions[w7.ATEXTS])[0]
        if c.args[1] and w7.ACHARS <= text < w7.ACHARS + 0x100:   # (the masks are for a call with texts)
            break
    regions = dict(c.regions)
    window7cc8bc.top(r, regions, 0)
    chars = regions[w7.ACHARS]
    at = text - w7.ACHARS
    name = struct.unpack_from('<16H', chars, at)
    name = name[:name.index(0)]
    n = r.choice([0, 1, 2, 3, 5, 8])
    nodes, root = stdtree.build(r, n) if n else ({}, None)
    keys = sorted(nodes)
    pool = {tuple(r.choice(w7.ALPHA) for _ in range(r.choice([0, 1, 1, 2, 3]))) for _ in range(2 * n)}
    strings = sorted(pool)[:n]
    if strings and r.random() < 0.5:
        strings[r.randrange(len(strings))] = name
        strings = sorted(set(strings))
    while len(strings) < n:
        strings.append(strings[-1] + (0xffff,) if strings else (1,))
    addr = {k: TNODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: TNIL if k is None else addr[k]  # noqa: E731
    blob_ = bytearray(0x40 * max(n, 1))
    tchars = bytearray(0x400)
    at = 2   # (each string after a count byte, as wstringmap.py's)
    for i, k in enumerate(keys):
        base = addr[k] - TNODES
        struct.pack_into('<III', blob_, base, ptr(nodes[k][0]), THEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<I', blob_, base + 0x1c, nodes[k][3])
        s = strings[i]
        struct.pack_into('<%dH' % len(s), tchars, at, *s)
        tchars[at - 1] = r.choice([0, 0, 1, 2, 0xff])
        struct.pack_into('<III', blob_, base + 0x10, TCHARS + at if s or r.random() < 0.5 else 0, len(s), len(s) + r.choice([0, 0, 3]))
        at += 2 * len(s) + 4
    head = struct.pack('<III', addr[keys[0]] if keys else THEAD, addr[root] if keys else TNIL, addr[keys[-1]] if keys else THEAD) + bytes(0x20)
    nil = put(bytearray(0x40), 0x1c, 'I', 1)
    avatar = bytearray(r.bytes(0xf0c))
    put(avatar, 0xe84, 'III', THEAD, 0, n)
    avatar[0xe88] = r.choice([0, 0, 0, 1, 0xff])
    word = bytearray(r.bytes(0x10)) + bytearray(0x40)   # (the name: a std::wstring, its characters after a count byte at +0x12)
    word[0x11] = r.choice([0, 0, 1, 0xff])
    struct.pack_into('<%dH' % (len(name) + 1), word, 0x12, *name, 0)
    if name or r.random() < 0.5:
        put(word, 4, 'III', NAMEW + 0x12, len(name), len(name) + r.choice([0, 0, 2]))
    else:   # (a null std::wstring, for an empty name only: add_case's masks are for its text's length)
        put(word, 4, 'III', 0, 0, 0)
    regions.update({AVATAR: avatar, THEAD: head, TNIL: nil, TNODES: blob_, TCHARS: tchars, NAMEW: word, 0x7ced24: struct.pack('<I', TNIL)})
    ready = struct.unpack('<I', regions[0x7cc8bc])[0] and struct.unpack('<I', regions[0x7cc8d0])[0] == 0
    # (add_case's masks are its strings' blocks: with the window not ready (none, or another mode) the set's node takes the first block)
    return Call(CALLS['AddName'], [AVATAR, NAMEW, r.getrandbits(32)], regions, conv='t', ret='v', masks=c.masks if ready else [])


def enter_chat(r, regions):
    """EnterChat (0x4bd090, src/game/ui/window7cc8bc): Arrive's world with the avatar as the chat [0x7cddec] (none now and then, with a
    heap whose first allocation fails: Arrive on none, a fault in both); the windows [0x7cc8bc] (now and then none) / [0x7cc8c0] random
    (their byte +4 mostly 0, the handler +0x114 fncmp's logger or 0), the hook [0xfaa5fc] the logger or 0; the floats [0x7a1c70] /
    [0x7a1c78] park positions mostly (negative, huge or NaN now and then); [0x790ab0] bit 2 half the time; the globals written random."""
    def pos():
        return r.choice([r.uniform(0, 2560), r.uniform(0, 2560), r.uniform(0, 300), r.randrange(256) * 10.0, -r.uniform(0, 50),
                         r.uniform(-1e10, 1e10), float('nan')])
    for at in (WIN_A, WIN_B):
        win = bytearray(r.bytes(0x120))
        win[4] = r.choice([0, 0, 0, 1])
        put(win, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
        regions[at] = win
    regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
    glob = bytearray(r.bytes(0x30))   # (0x7cc8ac-0x7cc8dc: the byte 0x7cc8b4 +8, the windows +0x10 / +0x14)
    glob[8] = r.choice([0, 1, 1, 0x80])
    put(glob, 0x10, '2I', r.choice([WIN_A] * 5 + [0]), WIN_B)
    regions.update({0x7cc8ac: glob, 0x7cc690: r.bytes(8), 0x790ab0: struct.pack('<I', r.getrandbits(32)),
                    0x7a1c70: struct.pack('<fIf', pos(), r.getrandbits(32), pos())})
    if r.random() < 0.04:
        regions[0x7cddec] = bytes(4)
        regions[HEAP] = heap(0x100, fail=1)
    else:
        regions[0x7cddec] = struct.pack('<I', AVATAR)


def leave_chat(r, regions):
    """LeaveChat (0x4bd260, src/game/ui/window7cc8bc): Enter's world (the caller's) with enter_chat()'s windows, chat and hook; the chosen
    window [0x7cc8b8] the first, the second or other; [0x790ab0] bit 10 set now and then; the floats [0x790aa0] / [0x790aa8] as enter_chat's
    [0x7a1c70] / [0x7a1c78] but mostly within the ground's 4 rows (further a fault in both, the avatar moved off it); the hovered window
    [0xfaa5e0] none of the two (Window_SetCursor's Gui_SetCursor left out)."""
    enter_chat(r, regions)
    def pos():
        return r.choice([r.uniform(0, 40), r.uniform(0, 40), r.uniform(0, 40), r.randrange(4) * 10.0, r.randrange(40) / 10, r.uniform(0, 2560),
                         -r.uniform(0, 50), r.uniform(-1e10, 1e10), float('nan')])   # (mostly within the ground's 4 rows: the avatar moves there)
    glob = bytearray(regions[0x7cc8ac])
    put(glob, 0xc, 'I', r.choice([struct.unpack_from('<I', glob, 0x10)[0], WIN_B, WIN_B, 0, r.getrandbits(32)]))
    regions[0x7cc8ac] = glob
    regions[0x790ab0] = struct.pack('<I', r.getrandbits(32) & ~0x400 if r.random() < 0.85 else r.getrandbits(32) | 0x400)
    regions.update({0x790aa0: struct.pack('<fIf', pos(), r.getrandbits(32), pos()), 0x790ad0: r.bytes(12), 0xfaa5e0: struct.pack('<I', r.choice([0, RAM]))})


def case(r):
    op = r.deal(OPS)
    if op == 'AddName':
        return add_name(r)
    if op == 'GoTo':
        return goto(r)
    if op == 'Move1':
        return move1(r)
    if op == 'Room':
        return room(r)
    if op in ('Find', 'NameIs'):
        regions, things = names(r)
        if op == 'Find' or not things:
            return Call(CALLS['Find'], [NAME], regions, conv='s', ret='a')
        return Call(CALLS[op], [r.choice(things), NAME], regions, conv='t', ret='b')
    c = things_methods.setpos(r)
    regions = dict(c.regions)
    thing, x, y = c.args
    pool = bytearray(regions[POOL])
    if r.random() < 0.6:
        pool[thing - POOL + 2] = 0x12
    if op in ('Enter', 'LeaveChat'):   # (the thing's script as personscripts.py's Place's, at entry point 3; the ground at (0, 0))
        person = bytearray(pool[thing - POOL:thing - POOL + 0x40])
        personscripts.script_list(r, regions, person, 3)
        pool[thing - POOL:thing - POOL + 0x40] = person
        ground(r, regions)
    if op in ('Arrive', 'EnterChat'):   # (personscripts.py's release: script slots set or not, the script list not ready)
        for off in (0xc, 0x30, 0x210):
            struct.pack_into('<I', pool, thing - POOL + off, r.choice([0, 0, 1, 2, 7]))
        if r.random() < 0.2:
            pool[thing - POOL + 2] = 1
        regions[0x7b49f0] = bytes(16)
    regions[POOL] = pool
    avatar = bytearray(r.bytes(0xec0))
    struct.pack_into('<II', avatar, 0, CHAT, thing)
    regions.update({AVATAR: avatar, CHAT: r.bytes(0x20)})
    imports(regions)
    cell = r.choice([1, 2, 128, 129, 512, r.randrange(1, 513)]) | r.choice([0, 0, 0x10000, r.getrandbits(16) << 16])
    if op == 'CellMove':
        return Call(CALLS[op], [AVATAR, cell], regions, conv='t', ret='b')
    if op == 'Cell':
        handle = struct.unpack_from('<H', pool, thing - POOL)[0]
        return Call(CALLS[op], [r.getrandbits(32), handle | r.choice([0, 0, r.getrandbits(16) << 16]), cell], regions, conv='t', ret='b')
    if op == 'Say':
        return Call(CALLS[op], [AVATAR], regions, conv='t', ret='a')
    if op == 'EnterChat':
        enter_chat(r, regions)
        return Call(CALLS[op], [], regions, ret='v')
    if op == 'LeaveChat':
        leave_chat(r, regions)
        return Call(CALLS[op], [], regions, ret='v')
    return Call(CALLS[op], [AVATAR, x, y], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
