# The tag system's text and message (src/game/world/tagsystem: TagSystem_FormatTag 0x509e10, TagSystem_HandleMessage 0x509bf0) against the originals.
# Format: a tag system's text buffer +0xc, a record of each kind 0-0x24 (or past them, or none) from the table at 0x75eca8, a word a (with or without
# its slot at 0x7cfb90), b, the record c; the formatter 0x6acd60 the log stub with as many text arguments as the kind has, the texts 1-9 of [0x78bac0]
# without any (only: the record's ids from 0x100; the kind of c 0-4, past that the original's buffer is uninitialised stack), the objects' definitions
# at 0x7890a0 with full names. Message: the types 0x13 (+8 7-9 or other), 0x18, 0x19 and others through a message's slot 0 (a recorder), the records of
# the ids sometimes missing, posted to msgtags.py's real tags (the message's fields go into a fresh tag or compare against a listed one).
# usage: tagmessage.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Format,Message)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import crt  # noqa: E402
import msgtags  # noqa: E402
import thingenv  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _cdecl_log4, put, recorder, set_return, strings, text_args, vtable  # noqa: E402

B = Bench(sorted(set(msgtags.ALL_RANGES + [(0x509bf0, 0x509e10), (0x509e10, 0x50ae3c), (0x50af40, 0x50afa0), (0x50aef0, 0x50af30), (0x4a77a0, 0x4a77c0), (0x4ea220, 0x4ea250),
                                           (0x412e90, 0x412f50), (0x412f50, 0x412fb0),(0x412fb0, 0x413040),(0x409cd0, 0x409cdd), (0x414670, 0x41467a)])),
          hooks={0x67a7d0: msgtags.advisor._cdecl_log, 0x6acd60: _cdecl_log4, 0x67af00: crt.swprintf_hook})
CALLS = {'Format': 0x509e10, 'Message': 0x509bf0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
TABLE, COUNT = 0x75eca8, 0x17e
X = RAM + 0x40000
SELF, CREC, MSG, VT, NAMES = X, X + 0x1000, X + 0x1100, X + 0x1200, X + 0x1400
# (how many text arguments each kind's text takes)
ARGS = {3: 1, 2: 1, 5: 1, 6: 1, 7: 1, 8: 2, 9: 3, 0xb: 3, 0xa: 4, 0xe: 4, 0xc: 4, 0xd: 4, 0xf: 4, 0x10: 4, 0x12: 4, 0x11: 5, 0x15: 5, 0x13: 5, 0x14: 5, 0x16: 4}
ARGS.update({k: 1 for k in range(0x17, 0x1e)})
ARGS.update({k: 2 for k in range(0x1e, 0x25)})


def table(r, ids):
    """The table's records: each its own id (a few moved away), any bytes else; returns it"""
    mem = bytearray(r.bytes(COUNT * 0x20))
    for i in range(COUNT):
        put(mem, 0x20 * i, 'I', i)
    for i in ids:
        if r.random() < 0.1:
            put(mem, 0x20 * i, 'I', 0xffff0000 | i)
    return mem


def names(r, regions):
    """The definitions' full names (texts): the found one's +0x4f4 and its part +0x410, whose start is a wide string"""
    defs, _, _ = thingenv.park(r, count=0)
    regions[thingenv.DEFS], items = defs[thingenv.DEFS], bytearray(defs[thingenv.ITEMS])
    regions[thingenv.ITEMS] = items
    found = struct.unpack_from('<i', regions[thingenv.DEFS], 0x528)[0]
    ids = [struct.unpack_from('<H', items, i * thingenv.ITEM)[0] for i in range(found)]
    for i in range(thingenv.NITEMS):
        put(items, i * thingenv.ITEM + 0x4f4, 'I', NAMES + 0x40 * i)
        put(items, i * thingenv.ITEM + 0x410, 'I', 0x41 + i)
    regions[NAMES] = b''.join(struct.pack('<H', 0x41 + i) * r.randrange(1, 30) + bytes(0x40) for i in range(thingenv.NITEMS))[:0x40 * thingenv.NITEMS * 2]
    return ids


def format_regions(r, kind, log, regions):
    ids = names(r, regions)
    regions[SELF] = bytearray(r.bytes(0x900))
    regions[TABLE] = table(r, [0x17b, 0x17c, 0x17d])
    regions[0x6fd1e0] = struct.pack('<I', recorder(23, 1, com=True))   # OutputDebugStringA
    regions[0x78ba90] = struct.pack('<I', r.getrandbits(32))
    regions[0x78bac0] = struct.pack('<I', r.getrandbits(32))
    regions[0x785790] = bytearray(r.bytes(0x20))
    for at in (0, 4, 0x10, 0x14, 0x18):
        put(regions[0x785790], at, 'i', r.choice([0, 1, -1, 100, -100, r.getrandbits(31), -r.getrandbits(31), -0x80000000]))
    strings(log, 0x3d, 0, out=2)   # (the buffer: the kinds' texts go to the stack)
    if kind in ARGS:
        text_args(log, ARGS[kind], only=(0x100, 0xffff))
    regions[0x78a374] = b'E\0m\0\0\0'   # (in the file's unset data)
    regions[0x7cfb90] = bytearray(r.bytes(20 * 0x200))
    for i in range(0x200):
        put(regions[0x7cfb90], 20 * i, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    return ids


def record_c(r, ids):
    c = bytearray(r.bytes(0x40))
    for at in (0x14, 0x18, 0x1c, 0x20, 0x24):
        put(c, at, 'i', r.choice([0, 1, -1, 0x7fffffff, -0x80000000, r.randrange(-1000, 1000), r.getrandbits(31), -r.getrandbits(31)]))
    for at in (0x28, 0x2a):
        put(c, at, 'H', r.choice(ids))
    put(c, 0x2c, 'I', r.randrange(5))
    return c


def format_case(r, log, regions):
    kind = r.choice(list(range(0x25)) * 8 + [0x25, 0xffffffff, r.getrandbits(32)])
    ids = format_regions(r, kind, log, regions)
    regions[CREC] = record_c(r, ids)
    i = r.randrange(COUNT)
    rec = TABLE + 0x20 * i
    put(regions[TABLE], 0x20 * i + 0x1c, 'I', kind)
    put(regions[TABLE], 0x20 * i + 0xc, 'I', r.randrange(0x100, 0x10000))
    a = r.choice([0, 0x10000, 1, r.randrange(1, 0x200), r.randrange(1, 0x200) | r.getrandbits(16) << 16])
    b = r.choice([0, 0x10000, r.choice(ids), r.choice(ids) | r.getrandbits(16) << 16, r.getrandbits(32)]) if kind == 2 else r.choice([0, 1, 0xffffffff, r.getrandbits(32)])
    return Call(CALLS['Format'], [SELF, 0 if r.random() < 0.03 else rec, a, b, CREC], regions, conv='t', ret='b')


def message_case(r, log, regions):
    kind = r.choice([0, 3, 4, 8, 9, 0xa, 0xc, 0x11, 0x13, 0x16, 0x17, 0x1e, 0x25])
    ids = format_regions(r, kind, log, regions)
    type_ = r.choice([0x13, 0x13, 0x18, 0x18, 0x19, 0x19, 0x12, 0x14, 0x1a, 0, r.getrandbits(32)])
    if type_ != 0x19 and 9 <= kind < 0x25:   # (c is null for the others: a fault in both, a few kept)
        if r.random() < 0.9:
            kind = r.choice([0, 3, 4, 5, 8])
            text_args(log, ARGS.get(kind, 1), only=(0x100, 0xffff))
            if kind in (0, 4):
                text_args(log, 1, only=(0xfffe, 0xffff))
    id_ = r.choice([7, 8, 9, 7, 8, 9, 6, 10, r.getrandbits(32)]) if type_ == 0x13 else r.randrange(COUNT)
    for i in (id_ if id_ < COUNT and type_ != 0x13 else 0x17b, 0x17c, 0x17d):
        put(regions[TABLE], 0x20 * i + 0x1c, 'I', kind)
        put(regions[TABLE], 0x20 * i + 0xc, 'I', r.randrange(0x100, 0xfffe))
        put(regions[TABLE], 0x20 * i + 8, 'H', r.getrandbits(16))
    if type_ != 0x13 and r.random() < 0.1:
        id_ = r.choice([0x17e, 0x17f, COUNT + 5])
    msg = record_c(r, ids)
    put(msg, 0, 'I', VT)
    put(msg, 8, 'I', id_)
    put(msg, 0xc, 'H', r.getrandbits(16))
    put(msg, 0x10, 'i', r.choice([0, 1, -1, r.getrandbits(31), -r.getrandbits(31)]))
    if type_ == 0x18:
        put(msg, 0x14, 'I', r.getrandbits(32))
    regions[MSG] = msg
    regions[VT] = vtable({})
    set_return(log, 0, type_)
    return Call(CALLS['Message'], [SELF, MSG], regions, conv='t')


def case(r):
    op = r.deal(OPS)
    c = msgtags.tags_case(r, 'Post')
    regions = c.regions
    log = regions[LOG]
    made = format_case(r, log, regions) if op == 'Format' else message_case(r, log, regions)
    made.masks = c.masks
    return made


if __name__ == '__main__':
    B.main(case)
