# The message tags' windows (src/game/ui/msgtags: 0x48ea00, 0x48eaa0, 0x48d9c0, 0x48eb20, 0x48ede0, 0x48f090, 0x48ef70, 0x481680; the
# thunks 0x481b70, 0x481580) against the originals. NewWindow: window.py's tree and queues (Window_Ctor's surroundings, as listrows.py's NewRow), the heap failing now and then,
# the id any; the GUI's registered windows of listrows.py's register() with "msgtag" among them or not (the object +0x98's slot 3 event:
# its words and the window pointer hashed). Create: the same, the ten records [0x7c26b0] random bytes, an allocation of the ten (or none)
# failing.
# Open / Add / Post / AddVersion / its thunk 0x481b70 (tags_env): the ten records with their windows (each word +4 in use or not, now and
# then all), 0-4 of the windows the tags' list [0x7cb2c8] (by +0x54) or none; each window's dwords +0xa0 small (1, 3 and 8 now and then
# the message's, 0x17e / 0 / 0 for the version's; 2 its record's text or 0; 6 0x1b or not; 9 -1, a particle handle alive or not, past the
# pool, negative (a high half 0xffff, alive or not) or any), its byte +0x134 0 / 1, its timer nodes +0x120 / +0x124 set or 0 (windowtimers.py's pool, list and clock), its +0x98 /
# +0x9c recorder objects or 0 (slot 3: the event's word +4 hashed); the tag shown [0x7ca090] one of them, 0 or other; the message box
# [0x7ca098] (its children: 0x10d32 a slot 9 recorder among others); the dialog [0x7cb2b8] or 0 and its child 0x21 (dialogs.py's
# Child21Post); the tags' window [0x7cb2c4] (a handler logger or none, byte +4 set now and then) or 0, the hook [0xfaa5fc] a logger or 0;
# the dialog flag [0x7cb2ac], the count [0x7ca09c] about the limit [0x750610]; the particle pool [0x816d40] / 0x80cf00 (eight entries);
# the sound globals 0x802bc0 (on / started or not; the manager a recorder, slot 2's command hashed) and the category; advisor.py's
# PostId surroundings (its queue masks, and the copy at +0x16d0, which never gets read for 0x11c). The message: kind = id or not, a text
# of 0-0x120 characters (0xff and 0x100 included) or none, a life of 0, 1, 5000, more or negative, the bytes +0x24 1 or not. PostThunk
# (0x481580): Post's. Load / UILoad (0x48ef70 / 0x481680): the same tags (their handlers +0x114 fncmp's logger or none, for UILoad's
# message 5), a record stream holding a count of 0-3 and that many whole records (a short one would leave the stack's bytes in the
# buffer), each of the kind 0x17f's tags' dwords now and then, its id 0x17f or not, its text 0-0x120 characters long (no end within
# 0xff of them included).
# usage: msgtags.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=NewWindow,Create,Open,Add,Post,AddVersion,Thunk,PostThunk,Load,UILoad)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import listrows  # noqa: E402
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, Mask, blob, blob_bytes, heap, log_region, put, record_stream, set_return, vtable  # noqa: E402
from window import RANGES, queues, tree  # noqa: E402

CALLS = {'NewWindow': 0x48ea00, 'Create': 0x48eaa0, 'Open': 0x48d9c0, 'Add': 0x48eb20, 'Post': 0x48ede0, 'AddVersion': 0x48f090,
         'Thunk': 0x481b70, 'PostThunk': 0x481580, 'Load': 0x48ef70, 'UILoad': 0x481680}
TIMER_RANGES = [(0x661f9b, 0x661fe5), (0x6620e2, 0x66213e), (0x662306, 0x662384), (0x662500, 0x66252c), (0x661ece, 0x661f2b),
                (0x661f2b, 0x661f9b), (0x66220d, 0x6622d2), (0x65968e, 0x6596a2), (0x6620ac, 0x6620cc), (0x6620cc, 0x6620e2),
                (0x662097, 0x6620ac), (0x65ef90, 0x65f024), (0x65f024, 0x65f077), (0x65f077, 0x65f0ca)]
ADVISOR_RANGES = [(0x429fe0, 0x42a010), (0x44b2e0, 0x44b3d0), (0x474070, 0x4740e0), (0x5442e0, 0x544340), (0x5ed580, 0x5ed5b0),
                  (0x59a940, 0x59aa63), (0x59abc0, 0x59ae1c), (0x59c2b0, 0x59c680), (0x41a980, 0x41a986), (0x41a990, 0x41a9a7),
                  (0x5da3c0, 0x5da3c1), (0x409cd0, 0x409cdd), (0x59b590, 0x59b611), (0x59bf20, 0x59c093), (0x550ca0, 0x550d90),
                  (0x5d1750, 0x5d1751), (0x486b00, 0x486b3a), (0x5194d0, 0x51950d)]
def _swprintf(e):
    """the CRT's swprintf (0x67af00) for AddVersion's "Ver %d.%d": %d, %s (wide) and %% only. The ports' side has the driver's stand-in
    (the host's), whose integers the emulator cannot follow into MSVC6's code (its locale lock's imports); tests/fncmp/crt.py checks the rest."""
    sp = e.r['ESP'] + (4 if e.tail else 0)
    to, fmt = e.u32(sp), e.u32(sp + 4)
    arg, out, i = sp + 8, '', 0
    while True:
        c = struct.unpack('<H', e.read(fmt + 2 * i, 2))[0]
        i += 1
        if not c:
            break
        if c != 0x25:
            out += chr(c)
            continue
        f = chr(struct.unpack('<H', e.read(fmt + 2 * i, 2))[0])
        i += 1
        if f == 'd':
            out += str(struct.unpack('<i', e.read(arg, 4))[0])
            arg += 4
        elif f == 's':
            p, k = e.u32(arg), 0
            arg += 4
            while True:
                u = struct.unpack('<H', e.read(p + 2 * k, 2))[0]
                if not u:
                    break
                out += chr(u)
                k += 1
        else:
            assert f == '%', f
            out += '%'
    e.write(to, out.encode('utf-16-le') + b'\0\0')
    e.r['EAX'] = len(out)


B = Bench(sorted(set(RANGES + listrows.REG_RANGES + TIMER_RANGES + ADVISOR_RANGES + [
    (0x48ea00, 0x48ea9b), (0x48eaa0, 0x48eae5), (0x65e526, 0x65e549), (0x65e6d5, 0x65e705),
    (0x48d9c0, 0x48dad2), (0x48eb20, 0x48edd1), (0x48ede0, 0x48ee18), (0x48f090, 0x48f10a), (0x481b70, 0x481b75),
    (0x481580, 0x481590), (0x48ef70, 0x48f08f), (0x481680, 0x481695), (0x5f5e20, 0x5f5e4d),
    (0x65d865, 0x65d8e7), (0x659e3e, 0x659e83), (0x65d3a3, 0x65d40c), (0x65d56b, 0x65d57c), (0x5222e0, 0x52231b), (0x51feb0, 0x51ff06),
    (0x51bfc0, 0x51c048), (0x67a400, 0x6902c0)])), hooks={0x67a7d0: advisor._cdecl_log, 0x67af00: _swprintf})
X = RAM + 0x20000
TAGS, NODES, MSG, MTEXT, BOX, BOXKIDS, DIALOG, CHILD21, TAGSWIN = X, X + 0x1400, X + 0x1800, X + 0x1900, X + 0x1c00, X + 0x1e00, X + 0x2200, X + 0x2400, X + 0x2600
OBJ, OVT, KVT, CLOCK, CVT, POOL, SNDMGR, SNDVT = X + 0x2800, X + 0x2840, X + 0x2900, X + 0x2a00, X + 0x2a10, X + 0x2b00, X + 0x2c00, X + 0x2c20
STREAM, SINK = X + 0x2d00, X + 0x3000
RECORDS = 0x7c26b0
PARTICLES = 0x80cf00   # (eight of the pool's 0x140-byte entries: the word +6 a handle's high half, +0x1c set -2 when ended)


def tag_window(r, i):
    w = bytearray(r.bytes(0x138))
    w[4] = r.choice([0, 0, 0, 1])
    for k in range(16):
        put(w, 0xa0 + 4 * k, 'I', r.choice([0, 1, 2, 0x17e, 0x17f, r.getrandbits(32)]))
    put(w, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    put(w, 0xa8, 'I', r.choice([RECORDS + 0x208 * i + 6] * 6 + [0]))
    put(w, 0xb8, 'I', r.choice([0x1b, 0x1b, 0, 1, r.getrandbits(32)]))
    put(w, 0xc4, 'I', r.choice([0xffffffff, 0xffffffff, 0xfffffffe, 0, particle(r), particle(r), particle(r), 0x7f0078, r.getrandbits(32)]))
    put(w, 0x98, 'II', r.choice([OBJ, 0]), r.choice([OBJ, 0]))
    for k in range(5):
        put(w, 0x120 + 4 * k, 'I', r.choice([0, 0, NODES + 0x20 * r.randrange(8)]))
    w[0x134] = r.choice([0, 1, 1, r.getrandbits(8)])
    return w


def particle(r):
    return (r.choice([r.randrange(4), r.randrange(4), r.getrandbits(15), 0xffff]) << 16 | r.randrange(8)) & 0xffffffff


def tags_env(r, regions, log, version=False):
    """the tags, their records and list, the box, dialog, tags' window, timers, particles and sound (see the header); returns the tags"""
    tags = [TAGS + 0x200 * i for i in range(10)]
    records = bytearray(r.bytes(0x208 * 10))
    used = r.random() < 0.1
    for i, t in enumerate(tags):
        put(records, 0x208 * i, 'IH', t, 1 if used else r.choice([0, 0, 1, 1, 1, r.getrandbits(16)]))
    regions[RECORDS] = records
    wins = [tag_window(r, i) for i in range(10)]
    listed = r.sample(range(10), r.choice([0, 1, 2, 3, 4]))
    for w in wins:   # (unlisted: no next, as Load's later adds walk the list from a tag the first one put at its head)
        put(w, 0x54, 'I', 0)
    for j, i in enumerate(listed):
        put(wins[i], 0x54, 'I', tags[listed[j + 1]] if j + 1 < len(listed) else 0)
    if version:
        for w in wins:
            if r.random() < 0.5:
                put(w, 0xa4, 'I', 0x17e)
                put(w, 0xac, 'I', r.choice([0, 0x10000]))
                put(w, 0xc0, 'I', r.choice([0, 0, 1]))
    for t, w in zip(tags, wins):
        regions[t] = w
    regions[0x7cb2c8] = struct.pack('<I', tags[listed[0]] if listed else 0)
    regions[0x7ca090] = struct.pack('<I', r.choice(tags + [0, 0, BOX]))
    # the message box and its children
    ids = [0x10d32] + r.choice([[], [1], [0x10d31, 2]])
    r.shuffle(ids)
    for i, n in enumerate(ids):
        kid = bytearray(r.bytes(0x90))
        put(kid, 0, 'I', KVT)
        put(kid, 0x54, 'I', BOXKIDS + 0x90 * (i + 1) if i + 1 < len(ids) else 0)
        put(kid, 0x8c, 'I', n)
        regions[BOXKIDS + 0x90 * i] = kid
    regions[KVT] = vtable({9: 1})
    regions[BOX] = put(bytearray(r.bytes(0x138)), 0x5c, 'I', BOXKIDS)
    regions[0x7ca098] = struct.pack('<I', BOX)
    # the dialog [0x7cb2b8] and its child 0x21
    child = put(bytearray(r.bytes(0x120)), 0x54, 'I', 0)
    child[4] = r.choice([0, 0, 0, 1])
    put(child, 0x8c, 'I', r.choice([0x21, 0x21, 0x21, 0x22]))
    put(child, 0xa8, 'I', r.choice([0, 0, 1]))
    put(child, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    regions[CHILD21] = child
    regions[DIALOG] = put(bytearray(r.bytes(0x120)), 0x5c, 'I', CHILD21)
    regions[0x7cb2b8] = struct.pack('<I', r.choice([DIALOG, DIALOG, 0]))
    # the tags' window
    win = bytearray(r.bytes(0x120))
    win[4] = r.choice([0, 0, 0, 0, 1])
    put(win, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    regions[TAGSWIN] = win
    regions[0x7cb2c4] = struct.pack('<I', r.choice([TAGSWIN] * 8 + [0]))
    regions[0xfaa5fc] = struct.pack('<I', r.choice([CDECL_LOG4, 0]))
    regions[0x7cb2ac] = struct.pack('<I', r.choice([1, 1, 1, 1, 0, r.getrandbits(32)]))
    limit = r.choice([10, 10, 3, 0, -1])
    regions[0x750610] = struct.pack('<i', limit)
    regions[0x7ca09c] = struct.pack('<i', r.choice([0, 1, 2, limit - 1, limit, limit + 1, -5]))
    queues(r, regions)
    # the objects told of the new tag, the timers' nodes, pool, list and clock
    regions[OBJ] = struct.pack('<I', OVT) + r.bytes(0x1c)
    regions[OVT] = vtable({3: 2}, size=8)
    blob_bytes(log, 3, 1, 0x30)
    nodes = bytearray(r.bytes(0x20 * 8))
    for k in range(8):
        put(nodes, 0x20 * k, 'I', r.choice(tags[:3] + [0]))
        put(nodes, 0x20 * k + 0xc, 'I', r.choice([0x80080, 0x80081, 1]))
        put(nodes, 0x20 * k + 0x14, 'I', r.choice([0, 1]))
        put(nodes, 0x20 * k + 0x18, 'II', NODES + 0x20 * (k + 1) if k < 7 and r.random() < 0.5 else 0, 0)
    regions[NODES] = nodes
    regions[0xfaa634] = struct.pack('<I', r.choice([0, NODES]))
    n = r.choice([0, 2, 4])
    pool = bytearray(r.bytes(0x20 * max(n, 1)))
    for k in range(n):
        put(pool, 0x20 * k + 0x14, 'I', r.choice([0, 1, 1]))
    regions[POOL] = pool
    regions[0xfaa62c] = struct.pack('<i', n)
    regions[0xfaa630] = struct.pack('<I', POOL if n or r.random() < 0.5 else 0)
    set_return(log, 0, r.getrandbits(32))
    regions[CVT] = vtable({}, size=4)
    regions[CLOCK] = struct.pack('<I', CVT)
    regions[0x77c490] = struct.pack('<I', CLOCK)
    # particles
    pool = bytearray(r.bytes(0x140 * 8))
    for k in range(8):
        put(pool, 0x140 * k + 6, 'H', r.choice([0, 1, 2, 3, 0xffff]))
    regions[PARTICLES] = pool
    regions[0x816d40] = struct.pack('<I', r.choice([1, 1, 1, 0]))
    # sound
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1]))
    put(g, 0xc, 'I', SNDMGR)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1]))
    regions[0x802bc0] = g
    regions[SNDMGR] = struct.pack('<I', SNDVT) + r.bytes(0xc)
    regions[SNDVT] = vtable({2: 2}, 4)
    blob(log, 2, 1, 0x7f)
    regions[0x803a2c] = r.bytes(4)
    return tags, wins


def message(r, regions, wins):
    """a message (see the header) at MSG, now and then matching one of the tags' dwords 1, 3 and 8"""
    m = bytearray(r.bytes(0x28))
    put(m, 0, 'II', r.choice([0, 1, 2, 0x17e]), r.choice([0, 1, 2, 0x17e]))
    if r.random() < 0.4:
        put(m, 4, 'I', struct.unpack_from('<I', m, 0)[0])
    put(m, 0xc, 'i', r.choice([0, 0, 1, 5000, 5001, 30000, -1, r.getrandbits(31)]))
    put(m, 0x10, 'H', r.choice([0, 1, 0xffff, r.getrandbits(16)]))
    put(m, 0x1c, 'H', r.choice([0, 1, r.getrandbits(16)]))
    put(m, 0x20, 'I', r.choice([0, 1, r.getrandbits(32)]))
    m[0x24] = r.choice([1, 1, 0, 2])
    if r.random() < 0.6:
        w = r.choice(wins)
        put(m, 4, 'I', struct.unpack_from('<I', w, 0xa4)[0])
        put(m, 0x1c, 'H', struct.unpack_from('<I', w, 0xac)[0] & 0xffff)
        put(m, 0x20, 'I', struct.unpack_from('<I', w, 0xc0)[0])
        if r.random() < 0.5:
            put(m, 0, 'I', struct.unpack_from('<I', m, 4)[0])
    n = r.choice([0, 1, 5, 40, 0xfe, 0xff, 0x100, 0x120])
    regions[MTEXT] = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(n)) + b'\0\0' + r.bytes(8)
    put(m, 8, 'I', r.choice([MTEXT] * 9 + [0]))
    regions[MSG] = m
    return MSG


def load_stream(r, regions, wins):
    """a count of 0-3 records (see the header) at SINK and the stream over them"""
    n = r.choice([0, 1, 1, 2, 3])
    data = bytearray(struct.pack('<I', n))
    for _ in range(n):
        rec = bytearray(r.bytes(0x21c))
        put(rec, 0, 'I', r.choice([0x17f, 0x17f, 1, r.getrandbits(32)]))
        k = r.choice([0, 1, 5, 0xfe, 0xff, 0x100, 0x120])
        text = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(k))
        rec[4:4 + len(text) + 2] = text + b'\0\0'
        put(rec, 0x204, 'i', r.choice([0, 0, 1, 6000, -1]))
        if r.random() < 0.6:
            w = r.choice(wins)
            put(w, 0xa4, 'I', 0x17f)
            put(rec, 0x214, 'H', struct.unpack_from('<I', w, 0xac)[0] & 0xffff)
            put(rec, 0x218, 'I', struct.unpack_from('<I', w, 0xc0)[0])
        data += rec
    regions[SINK] = data + r.bytes(r.choice([0, 0, 4]))
    return record_stream(STREAM, SINK, len(regions[SINK]))


def tags_case(r, op):
    adv = advisor.queue(r, 'PostId')   # (the advisor's surroundings: its regions and log)
    regions = dict(adv.regions)
    log = regions[LOG]
    table = regions[0x76e300]
    if r.random() < 0.5:
        put(table, 0x38 * 0x11c + 4, 'I', 0x11c)
    offsets = [0x16d0 + k for k in (2, 3, 0x17)] + [0x14 + 0x18 * s + k for s in range(8) for k in (2, 3, 0x17)]
    masks = [Mask(advisor.ADVISOR, offsets, "Advisor_PostId's stack message's bytes +2, +3, +0x17, never written (0x486b00), copied into a "
                                            "queue slot or +0x16d0 (read only for the id 0x15f)")]
    tags, wins = tags_env(r, regions, log, version=op in ('AddVersion', 'Thunk'))
    if op == 'Open':
        tag = r.choice(tags)
        if r.random() < 0.15:
            regions[0x7ca090] = struct.pack('<I', tag)
        return Call(CALLS[op], [tag], regions, ret='v', masks=masks)
    if op in ('Load', 'UILoad'):
        regions[STREAM] = load_stream(r, regions, wins)
        return Call(CALLS[op], [STREAM], regions, conv='s' if op == 'UILoad' else 'c', ret='a', masks=masks)
    if op == 'PostThunk':
        return Call(CALLS[op], [message(r, regions, wins)], regions, conv='s', ret='a', masks=masks)
    if op in ('AddVersion', 'Thunk'):
        return Call(CALLS[op], [], regions, ret='v', masks=masks)
    return Call(CALLS[op], [message(r, regions, wins)], regions, ret='a', masks=masks)


def case(r):
    op = r.deal(OPS)
    if op not in ('NewWindow', 'Create'):
        return tags_case(r, op)
    log = log_region()
    regions = {LOG: log}
    wins = tree(r, regions)
    queues(r, regions)
    regions[0xfaa540] = struct.pack('<II', r.randrange(-10, 120) & 0xffffffff, r.randrange(-10, 40) & 0xffffffff)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], 0]), r.getrandbits(32), r.choice(wins + [0, 0]), r.getrandbits(32),
                                    r.choice([0] * 6 + [1]))
    listrows.register(r, regions, 'msgtag')
    regions[listrows.OBJ98] = put(bytearray(r.bytes(0x20)), 0, 'I', listrows.VT98)   # (register()'s other object)
    blob_bytes(log, 3, 1, 0x0ff0f3f0)   # (the event's words +4-+9 and +0xc-+0xf and the window pointer +0x14, its flags +0x18)
    if op == 'NewWindow':
        regions[HEAP] = heap(0x400, r.choice([0] * 5 + [1]))
        return Call(CALLS[op], [r.choice([0, 900, 909, r.getrandbits(16), r.getrandbits(32)])], regions, ret='a')
    regions[HEAP] = heap(0x2000, r.choice([0] * 4 + list(range(1, 11))))
    regions[0x7c26b0] = r.bytes(0x208 * 10)
    return Call(CALLS[op], [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
