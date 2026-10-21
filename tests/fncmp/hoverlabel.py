# The hover-lit labels (src/game/ui/hoverlabel) against the originals.
# SetYellow (0x492d20): window.py's notify_env tree, any of its windows.
# Attach (0x485d20): windowtree.py's rect_env windows (small rectangles, margins, anchors; the objects' events logged as a hash of none of
# their bytes: windownotify.py checks those) with ids +0x8c from a few (repeats too), the parent any of them, the id one of its children's
# or another (a parent with children mostly; rectangles inside out now and then; the default handlers +0x118 recorders); the shared text part [0x7c2118] random bytes (its vtable the windows' recorder one, its words +0x10 / +0x12 small); the
# language's font group 0-2 (font 5 read inline); the text any.
# Handler (0x48d2e0): notify_env's window, its default handler +0x118 a cdecl recorder, its dword +0xa0 an owner whose slot 14 is a
# recorder (or none: a fault in both on a click); the message 0x10001, 0x10002, 0x10006 or another, a 0 or not.
# Bind (0x48d290): Attach's surroundings, the owner random bytes with its window +0x10 the parent; the windows' objects +0x98 all set (with none
# the rectangle set sends the new handler 0x48d2e0 a message, and the driver cannot call a game address in its own code; Attach covers that path).
# usage: hoverlabel.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetYellow,Attach,Handler,Bind)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, blob_bytes, log_region, put, recorder, vtable  # noqa: E402
import window  # noqa: E402
from windowtree import rect_env  # noqa: E402

CALLS = {'SetYellow': 0x492d20, 'Attach': 0x485d20, 'Handler': 0x48d2e0, 'Bind': 0x48d290}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def merged(ranges):
    out = []
    for lo, hi in sorted(set(ranges)):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(window.RANGES + [
    (0x485d20, 0x485e30), (0x48d290, 0x48d36d), (0x48f690, 0x48f6b0), (0x492d20, 0x492d3b), (0x6589f9, 0x658abb), (0x658ccc, 0x658e04),
    (0x659710, 0x659740), (0x659d00, 0x659d53), (0x659d90, 0x659df5), (0x659e3e, 0x659e93), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0),
    (0x65c388, 0x65c4c9), (0x65c5d5, 0x65c682), (0x65c7fe, 0x65c989), (0x65c9bd, 0x65cda5), (0x65d070, 0x65d1d8), (0x65d42e, 0x65d4b0),
    (0x65d773, 0x65d832), (0x65db25, 0x65dc00), (0x65e526, 0x65e559), (0x65e6d5, 0x65e73d), (0x65f5fe, 0x65f655), (0x65f66e, 0x65f6e1),
    (0x6616b0, 0x661780), (0x6617f0, 0x661820), (0x6697e3, 0x6698b4), (0x66bff7, 0x66c01b)]))
SHARED = 0x7c2118
OWNER, OWNVT = RAM + 0xc000, RAM + 0xc100


def attach_env(r, regions, log, bound=False):
    """Attach's surroundings (above); returns (parent, id, whether the parent has children)"""
    wins = rect_env(r, regions, log)
    for k in (3, 22):
        blob_bytes(log, k, 1, 0)
    pool = [r.randrange(1, 6) for _ in range(3)] + [r.getrandbits(32)]
    for w in wins:
        put(regions[w], 0x8c, 'I', r.choice(pool))
        put(regions[w], 0x4c, 'I', r.choice([0, window.LOOK]))   # (SetObject98 reads the look's byte +0x10)
        put(regions[w], 0x98, 'I', window.OBJECT98 if bound else r.choice([0, 0, window.OBJECT98]))
        put(regions[w], 0x118, 'I', recorder(5, 4, cdecl=True))   # (the default handler, behind the one Bind sets)
        if r.random() < 0.3:   # (the rectangle +8 turned inside out: a negative width, odd or even)
            left, top, right, bottom = struct.unpack_from('<4h', regions[w], 0xc)
            put(regions[w], 0xc, '4h', left, top, left - r.choice([1, 2, 5, 40, 41]), bottom)
    regions[window.LOOK] = r.bytes(0x20)
    regions[SHARED] = put(put(bytearray(r.bytes(0x60)), 0, 'I', window.VTABLE), 0x10, 'hh', r.choice([0, 1, 2]), r.choice([0, 1, 2]))
    lang = bytearray(r.bytes(0x400))
    put(lang, 4, 'I', r.choice([0, 1, 2]))
    regions[0x78b530] = lang
    parents = [w for w in wins if struct.unpack_from('<I', regions[w], 0x5c)[0]]
    parent = r.choice(parents * 6 + wins if parents else wins)
    kids, c = [], struct.unpack_from('<I', regions[parent], 0x5c)[0]
    while c:
        kids.append(struct.unpack_from('<I', regions[c], 0x8c)[0])
        c = struct.unpack_from('<I', regions[c], 0x54)[0]
    ident = r.choice(kids * 12 + pool) if kids else r.choice(pool)
    return parent, ident, bool(kids)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    if op == 'SetYellow':
        wins = window.notify_env(r, regions, log)
        window.notify_blobs(log, 0, 0)
        return Call(CALLS[op], [r.choice(wins)], regions, ret='v')
    if op == 'Handler':
        wins = window.notify_env(r, regions, log)
        window.notify_blobs(log, 0, 0)
        w = r.choice(wins)
        put(regions[w], 0x118, 'I', recorder(5, 4, cdecl=True))
        put(regions[w], 0xa0, 'I', r.choice([OWNER] * 9 + [0]))
        regions[OWNER] = put(bytearray(r.bytes(0x20)), 0, 'I', OWNVT)
        regions[OWNVT] = vtable({})
        message = r.choice([0x10001, 0x10002, 0x10006, 0x10006, 0x10000, 0x10003, 0x10005, 0x10007, 1, r.getrandbits(32)])
        a = r.choice([0, 0, 1, r.getrandbits(32)])
        return Call(CALLS[op], [w, message, a, r.getrandbits(32)], regions, ret='a')
    for _ in range(3):   # (a parent with children, mostly)
        log = log_region()
        regions = {LOG: log}
        parent, ident, kids = attach_env(r, regions, log, op == 'Bind')
        if kids:
            break
    if op == 'Attach':
        return Call(CALLS[op], [parent, ident, r.choice([0, RAM + 0xd000, r.getrandbits(32)])], regions, ret='v')
    regions[OWNER] = put(bytearray(r.bytes(0x40)), 0x10, 'I', parent)
    return Call(CALLS[op], [OWNER, ident], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
