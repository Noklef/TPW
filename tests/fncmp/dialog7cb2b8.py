# The dialog [0x7cb2b8]'s code (src/game/ui/dialog7cb2b8: 0x4a1440, 0x4a14f0) against the originals. ShowRightOf: windowtree.py's rect_env() tree
# (1-7 windows with GUI rectangles, margins, parents, objects and handlers; shown or hidden), the window its root mostly, one of the others or
# now and then 0 (a fault in both: its children are read all the same); the limit about the windows' right edges (their inner ones) or any,
# the upper half random.
# BoxHandler (0x4a14f0): rect_env()'s tree, its root the dialog [0x7cb2b8] (now and then 0x7cb2d0's too); the box a window of its own
# (msgtags.py's full_window(), 0x140 bytes), the root's first child (its parent, 0 now and then), with the close button 0x24 (a slot 5
# recorder) among its children or not; its timer node +0x120 one of its own or 0 (then a pool of 0-3 nodes [0xfaa630], free or not, the
# list [0xfaa634] and the clock [0x77c490] a recorder answering a random time); its count +0x130 0, 1, 10, small or any, its state +0x134
# 0, 2, 3, 4 or any (by the message), its dwords +0xa0 the step small or any, the tag shown (the tag window, linked as the box's first
# child or not, a tree window or 0) and a tag waiting (0, the tag or any); the tag a window of its own (full_window(); now and then with a
# leaf child, or a link set: AddChild refuses it). The message mostly 0xa (a the tag, 0 or a tree window), 0xb (a -1 or not), 0x10 (a
# mostly the timer 0x80080), 0x12 (a the new left: small, the high half any) or 0x100 (a mostly 0x24: dialogs.py's ResetChild21
# surroundings, the root then its dialog), others now and then; b any. window.py's queues and timed table; the sound globals as
# msgtags.py's (the manager's slot 2 command hashed). The objects' slot 3 events hashed by what each sets: SetDepth's (2) +6 / +7,
# SetRect's (0x10) +0x14, SetWordsE4's (0x40) +0xc-+0xf; Offset's (0x50) +0xc-+0xf of what it sets.
# usage: dialog7cb2b8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ShowRightOf,BoxHandler,BarHandler)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialogs  # noqa: E402
import msgtags  # noqa: E402
import window as gui  # noqa: E402
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, blob, blob_bytes, blob_when, log_region, put, set_return, vtable  # noqa: E402
from window import RANGES  # noqa: E402
from thingenv import SLOTS, THING, park  # noqa: E402
from windowtree import rect_env  # noqa: E402

CALLS = {'ShowRightOf': 0x4a1440, 'BoxHandler': 0x4a14f0, 'BarHandler': 0x4a1cd0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
BOX_RANGES = [(0x4a14f0, 0x4a1bbf), (0x485aa0, 0x485abd), (0x51bfc0, 0x51c060), (0x658ccc, 0x658e04), (0x659e3e, 0x659e93),
              (0x65c7fe, 0x65c989), (0x65cc04, 0x65cda5), (0x65cf5e, 0x65cfe4), (0x65d42e, 0x65d506), (0x65d547, 0x65d58c), (0x65d7d0, 0x65d832),
              (0x65e5fa, 0x65e650), (0x65e6d5, 0x65e73d), (0x65ef90, 0x65f0da), (0x65f16b, 0x65f256), (0x65f345, 0x65f420), (0x65f447, 0x65f5c7),
              (0x65f66e, 0x65f6a7), (0x6620cc, 0x6620f2), (0x66220d, 0x6622e2), (0x668820, 0x6688c3), (0x668ad3, 0x668c99), (0x6697e3, 0x6698b4),
              (0x65f697, 0x65f6e1), (0x65f286, 0x65f330), (0x65c9bd, 0x65ce20), (0x65d865, 0x65d8f7), (0x65d90b, 0x65da9d), (0x65db25, 0x65dc00),
              (0x65dcaf, 0x65dd5f), (0x6590f7, 0x6591d4), (0x65f150, 0x65f16b), (0x659d43, 0x659d90), (0x65d773, 0x65d832), (0x67afa0, 0x67b0d0),
              (0x661f2b, 0x661f9b), (0x661ece, 0x661f2b), (0x6620ac, 0x6620cc), (0x662097, 0x6620ac), (0x65968e, 0x6596c0), (0x5222e0, 0x52231b),
              (0x51feb0, 0x51ff06), (0x680050, 0x68005b)]
BAR_RANGES = [(0x4a1cd0, 0x4a1d70), (0x519510, 0x51954d), (0x4c7bb0, 0x4c7cb4), (0x51a280, 0x51a287), (0x516130, 0x5161b0), (0x50b350, 0x50b360),
              (0x4fa990, 0x4fa9fa), (0x536300, 0x53644c), (0x5da3c0, 0x5da3c1), (0x67a830, 0x67a860)]   # (BarHandler's: the park's average)
B = Bench(sorted(set(RANGES + msgtags.TIMER_RANGES + BOX_RANGES + [
    (0x4a1440, 0x4a14e1), (0x6589f9, 0x658abb), (0x658ccc, 0x658daf), (0x659710, 0x659740), (0x659d00, 0x659d53), (0x659d90, 0x659dbc),
    (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x65d42e, 0x65d4b0), (0x65d547, 0x65d56a), (0x65d56b, 0x65d58c), (0x65d773, 0x65d7cc),
    (0x65d90b, 0x65da9d), (0x65db25, 0x65dc00), (0x6697e3, 0x6698b4), (0x66bff7, 0x66c01b), (0x661700, 0x661800)] + BAR_RANGES)))
BW = RAM + 0x30000   # (BoxHandler's own: the box, the tag, its leaf, the close button, the timer node and pool, the clock, the sound)
BOX, TAG, LEAF, CLOSE, NODE, POOL, CLOCK, CVT, SND = BW, BW + 0x200, BW + 0x400, BW + 0x600, BW + 0x800, BW + 0x900, BW + 0xa00, BW + 0xa10, BW + 0xb00
SNDMGR, SNDVT = SND + 0x40, SND + 0x60
BARW = RAM + 0x50000   # (BarHandler's own: the bar window, its vtable, timer node, pool and clock, the game pointer, the park's things)
BAR, BARVT, BAR_NODE, BAR_POOL, BAR_CLOCK, BAR_GAME, BAR_THINGS = BARW, BARW + 0x200, BARW + 0x300, BARW + 0x400, BARW + 0x500, BARW + 0x600, BARW + 0x1000
BOX_MESSAGES = [0xa] * 8 + [0xb] * 3 + [0x10] * 9 + [0x12] * 4 + [0x100] * 2 + [0, 5, 0x11, 0x14, 0x101, 0x10a]


def small(r):
    return r.choice([0, 1, 2, -1, -5, 10, r.randrange(-40, 40), r.randrange(-40, 40)]) & 0xffffffff


def timer_env(r, regions, log, window, node, pool, clock):
    """the window's timer: its node +0x120 (node) or a pool's (0-3 nodes at pool, free or not), the list [0xfaa634] and the clock
    [0x77c490] (at clock, its vtable after it) a recorder answering a random time"""
    regions[node] = put(bytearray(r.bytes(0x20)), 0x18, 'I', 0)   # (the last of the list [0xfaa634]: WindowTimers_Add walks it)
    put(window, 0x120, 'IIII', node if r.random() < 0.6 else 0, *(r.choice([0, 0, node]) for _ in range(3)))
    n = r.choice([0, 1, 1, 2, 3])
    if n:
        regions[pool] = b''.join(struct.pack('<20sI8s', r.bytes(20), r.choice([1, 1, 0, r.getrandbits(32)]), r.bytes(8)) for _ in range(n))
    regions[0xfaa62c] = struct.pack('<i', r.choice([n, n, n, n - 1, 0]))
    regions[0xfaa630] = struct.pack('<I', pool if n else 0)
    regions[0xfaa634] = struct.pack('<I', r.choice([0, 0, node]))
    regions[clock] = struct.pack('<I', clock + 0x10)
    regions[clock + 0x10] = vtable({}, size=4)
    regions[0x77c490] = struct.pack('<I', clock)
    set_return(log, 0, r.getrandbits(32))


def bar_case(r):
    """BarHandler (0x4a1cd0): the bar window a window of its own (msgtags.py's full_window()), its vtable's slot 7 a recorder, its timer
    as BoxHandler's (timer_env()) and window.py's queues and timed table; the park (thingenv.py's, as thingcount.py's Person: kind 1 mostly,
    their floats +0x19c 0..100 mostly, else negative, past 255, huge or NaN; the world's +0x1da710 mostly 0), the game [[0x7cf6ec]]'s
    handle +0x1da720 one of the park's or 0 (its slot's thing then the random slot 0's, or 0 when the slots are). The message mostly 0x10
    (a mostly the timer 0x80083) or 0x15, others now and then; b any; the window 0 now and then (a fault in both: the default handler)"""
    log = log_region()
    regions = {LOG: log}
    message = r.choice([0x10] * 10 + [0x15] * 6 + [0, 0x11, 0x14, 0x16, 0x12])
    bar = msgtags.full_window(r, regions, BAR, 0x140)
    put(bar, 0, 'I', BARVT)
    regions[BARVT] = vtable({7: 1, 5: 1, 3: 2})
    timer_env(r, regions, log, bar, BAR_NODE, BAR_POOL, BAR_CLOCK)
    gui.timers(r, regions, BAR)
    count = r.choice([0, 1, 2, 3, 4, 6])
    park_regions, handles, _things = park(r, count=count, kinds=(1, 1, 1, 2, 3), defs=False, cells=True, pool=BAR_THINGS)
    regions.update(park_regions)
    pool = bytearray(regions[BAR_THINGS])
    for i in range(count):
        pool[THING * i + 2] = r.choice([1] * 6 + [0, 2, 3, 4, 8])
        f = r.choice([0.0, 1.0, 50.0, 99.9, 100.0, 0.5, r.uniform(0, 100)]) if r.random() < 0.85 else \
            r.choice([-1.0, -0.5, 255.9, 256.0, 300.0, float('nan'), 1e10, -1e10])
        put(pool, THING * i + 0x19c, 'f', f)
    regions[BAR_THINGS] = pool
    world = bytearray(r.bytes(0x20))
    put(world, 0, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    put(world, 0x10, 'H', r.choice(handles * 3 + [0, r.randrange(1, 12)]) if handles else r.choice([0, r.randrange(1, 12)]))
    regions[RAM + 0x1da710] = world
    if r.random() < 0.1:
        put(regions[SLOTS], 0, 'I', 0)
    regions[BAR_GAME] = struct.pack('<I', RAM)
    regions[0x7cf6ec] = struct.pack('<I', BAR_GAME)
    a = r.choice([0x80083] * 12 + [0x80080, 0x80082, 0, r.getrandbits(32)]) if message == 0x10 else r.choice([0, 0x80083, r.getrandbits(32)])
    b = r.choice([0, 0, 1, r.getrandbits(32)])
    window = BAR if r.random() < 0.97 else 0
    return Call(CALLS['BarHandler'], [window, message, a, b], regions, ret='a')


def box_case(r):
    log = log_region()
    regions = {LOG: log}
    wins = rect_env(r, regions, log)
    for w in wins:
        put(regions[w], 0, 'I', gui.VTABLE)
        put(regions[w], 0x118, 'I', CDECL_LOG4)
    root = wins[0]
    message = r.choice(BOX_MESSAGES)
    box = msgtags.full_window(r, regions, BOX, 0x140)
    put(box, 0x8c, 'I', 0x21)
    first = struct.unpack_from('<I', regions[root], 0x5c)[0]
    if r.random() < 0.95:   # (the root's first child)
        put(box, 0x60, 'I', root)
        put(box, 0x54, 'I', first)
        put(regions[root], 0x5c, 'I', BOX)
        if first:
            put(regions[first], 0x58, 'I', BOX)
    kids = []
    if r.random() < 0.7:
        close = msgtags.full_window(r, regions, CLOSE)
        put(close, 0x8c, 'I', r.choice([0x24, 0x24, 0x24, 0x25]))
        put(close, 0x60, 'I', BOX)
        kids.append(CLOSE)
    tag = msgtags.full_window(r, regions, TAG)
    if r.random() < 0.15:
        leaf = msgtags.full_window(r, regions, LEAF)
        put(leaf, 0x60, 'I', TAG)
        put(tag, 0x5c, 'I', LEAF)
    if message == 0xa:
        if r.random() < 0.08:
            put(tag, r.choice([0x54, 0x58, 0x60]), 'I', r.choice(wins))
    elif r.random() < 0.8:   # (the tag shown, the box's child)
        put(tag, 0x60, 'I', BOX)
        kids.insert(r.randrange(len(kids) + 1), TAG)
    for i, k in enumerate(kids):
        put(regions[k], 0x58, 'I', kids[i - 1] if i else 0)
        put(regions[k], 0x54, 'I', kids[i + 1] if i + 1 < len(kids) else 0)
    put(box, 0x5c, 'I', kids[0] if kids else 0)
    timer_env(r, regions, log, box, NODE, POOL, CLOCK)
    # the box's count, state and dwords
    put(box, 0x130, 'I', (r.choice([0] * 6 + [1, 2, 10, r.randrange(-3, 12), r.getrandbits(32)]) if message == 0x10 else r.getrandbits(32)) & 0xffffffff)
    box[0x134] = r.choice([0] * 12 + [2, 3, 4, 1]) if message == 0xa else r.choice([3, 4] * 3 + [2, 0, 1, r.getrandbits(8)])
    shown = r.choice([TAG] * 12 + [0, 0] + wins[1:]) if message != 0xa else r.choice([TAG, 0, r.getrandbits(32)])
    put(box, 0xa0, 'III', small(r) if r.random() < 0.8 else r.getrandbits(32), shown, r.choice([0, TAG, TAG, r.getrandbits(32)]))
    if message == 0x12:   # (the parent wide: the tag's right edge about its inner right)
        left = r.randrange(-40, 0)
        put(regions[root], 8, 'Ihhhh', 0x705920, left, r.randrange(-30, 30), left + r.randrange(20, 140), r.randrange(30, 90))
        if r.random() < 0.8:   # (the box's old rectangle +0x20 small: the tag moved by a few)
            x, width = r.randrange(-30, 30), r.randrange(0, 80)
            put(box, 0x20, 'Ihhhh', 0x705920, x, r.randrange(-20, 20), x + width, r.randrange(20, 60))
    regions[0x7cb2b8] = struct.pack('<I', r.choice([root] * 12 + [0]) if message == 0xa else root)
    regions[0x7cb2d0] = struct.pack('<I', r.choice(wins))
    gui.timers(r, regions, BOX)
    queue = regions[gui.QUEUES + 0x140]
    if r.random() < 0.85:   # (room in the queue [0xfaa5f0] mostly: the posts land, compared)
        put(queue, 16 * r.randrange(len(queue) // 16), 'I', 0)
        regions[0xfaa5f0] = struct.pack('<I', gui.QUEUES + 0x100)
    # the sound (msgtags.py's)
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1]))
    put(g, 0xc, 'I', SNDMGR)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1]))
    regions[0x802bc0] = g
    regions[SNDMGR] = struct.pack('<I', SNDVT) + r.bytes(0xc)
    regions[SNDVT] = vtable({2: 2}, 4)
    blob(log, 2, 1, 0x7f)
    regions[0x803a2c] = r.bytes(4)
    regions[0xfaa5ec] = struct.pack('<I', r.choice([0, root, BOX, TAG]))
    regions[gui.VTABLE] = vtable({5: 1, 3: 2})
    for k in (3, 22):   # (SetWordsE4's 0x40 sets +0xc-+0xf only, Offset's 0x50 +0x14 too: the default hashes the first four)
        blob_bytes(log, k, 1, 0xf000)
        blob_when(log, k, 0, 2, 0xc0, 0 if k == 3 else 1)
        blob_when(log, k, 0, 0x10, 0xf00000, 2 if k == 3 else 3)
    if message == 0xa:
        a = r.choice([TAG] * 20 + [0] + wins[1:]) if not box[0x134] else r.choice([TAG, 0, 0])   # (busy: a 0 leaves the waiting one)
    elif message == 0xb:
        a = r.choice([0xffffffff, 0xffffffff, 0, 1, r.getrandbits(32)])
    elif message == 0x10:
        a = r.choice([0x80080] * 16 + [0x80081, 0x80, 0])
    elif message == 0x12:
        a = r.randrange(-60, 160) & 0xffff | r.choice([0, 0, 0xffff0000, r.getrandbits(16) << 16])
    elif message == 0x100:
        a = r.choice([0x24] * 6 + [0x23, 0x25, 0x10024])
        dialogs.reset_case(r, 'Dialog7cb2b8_ResetChild21', regions)
    else:
        a = r.choice([TAG, 0, 0x24, 0x80080, r.getrandbits(32)])
    b = r.choice([0, 0, 1, r.getrandbits(32)])
    return Call(CALLS['BoxHandler'], [BOX, message, a, b], regions, ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'BoxHandler':
        return box_case(r)
    if op == 'BarHandler':
        return bar_case(r)
    log = log_region()
    regions = {LOG: log}
    wins = rect_env(r, regions, log)
    window = r.choice(wins[:1] * 4 + wins + [0] if r.random() < 0.03 else wins[:1] * 4 + wins)
    rights = []
    for w in wins:
        right, = struct.unpack_from('<h', regions[w], 0x10)
        margin, = struct.unpack_from('<h', regions[w], 0xfc)
        rights.append(right - margin)
    limit = r.choice([r.choice(rights) + r.choice([-1, 0, 0, 1]), r.randrange(-40, 80), r.randrange(-40, 80), 0x7fff, 0x8000, r.getrandbits(16)])
    return Call(CALLS['ShowRightOf'], [window, (limit & 0xffff) | r.choice([0, 0, r.getrandbits(16)]) << 16], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
