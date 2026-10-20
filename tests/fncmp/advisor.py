# The advisor (src/game/world/advisor: 0x5997b0, 0x5989c0) against the originals. Lookup: tables of up to eight entries ended by the id
# 0x270f anywhere (first included), ids repeated, the wanted id among them, past the end, the end id itself or missing. Init: model slots 0
# and 1 with a holder or none; a holder's parts (0-6, named for the mouth parts and the bug head, case varied, or otherwise) and its motion
# bank 5 (loaded or not, 0-24 entries); the advisor's block, the particle pools' block and the sine table random. MayPost and Post (0x59abc0,
# 0x59a940): the advisor random but for its 8 queue slots (set or not, some of the id, scores small or any) and its record for the id (a time
# or none, near the game's time, +4 near the definition's +0x28, +8 byte, +0xc small); the table at 0x76e300 random but for small indices
# (+8), counts (+0x20) and fields (+0x28, +0x2c), the id in its own slot, only another, none, 0x15f / 0x160 or past the table; gAdvisorScoring
# (built: its guard set) small values; the byte 0x78d90d set or not; urgent or not; the message's score set or not. PostKind (0x59afc0): the
# same with an event of kind 0-4 (another now and then, its id then the event's address: a crash in both) and n giving an id in the table or
# past it; its message scored (AdvisorMsg_Init) through the records' methods (recorders, +0xc 0-3) and the mode setting (guard set or not).
# The message it builds on the stack never has its bytes +2, +3 and +0x17 written, so the queue slot (+0x14 + 0x18 k) and the copy at +0x16d0
# that take it are masked there: nothing in 0x59afc0, 0x59a940 or 0x59abc0 reads them (the slots are read at +0xc, +0x10 and +0x14 only;
# +0x16d0 is also the advisor's record of id 0x15f, read by 0x59abc0, so it is not masked for that id, which has no definition to copy).
# PostId (0x486b00): PostKind's message for one of Post's ids, the advisor found through the world's table entry; masked the same.
# usage: advisor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Lookup,Init,MayPost,Post,PostKind,PostId)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import struct

from fncmp import LOG, RAM, Bench, Call, Mask, _cdecl_log, log_region, put, recorder, set_return  # noqa: E402

B = Bench([(0x5997b0, 0x5997f8), (0x429fe0, 0x42a010), (0x44b2e0, 0x44b3d0), (0x474070, 0x4740e0), (0x5442e0, 0x544340), (0x5989c0, 0x598ae0),
           (0x5ed580, 0x5ed5b0), (0x67a400, 0x6902c0), (0x59a940, 0x59aa63), (0x59abc0, 0x59ae1c), (0x59c2b0, 0x59c680), (0x41a980, 0x41a986),
           (0x41a990, 0x41a9a7), (0x5da3c0, 0x5da3c1), (0x59afc0, 0x59b049), (0x409cd0, 0x409cdd), (0x59b590, 0x59b611), (0x59bf20, 0x59c093),
           (0x550ca0, 0x550d90), (0x5d1750, 0x5d1751), (0x486b00, 0x486b3a), (0x5194d0, 0x51950d)], hooks={0x67a7d0: _cdecl_log})
OPS = [k for k in os.environ.get('OPS', 'Lookup,Init,MayPost,Post,PostKind,PostId').split(',') if k]
NAMES = [b'mouth - normal', b'MOUTH - aah', b'mouth - eee', b'Mouth - Ooh', b'mouth - sss', b'bug head', b'mouth', b'head', b'']


def lookup(r):
    table = bytearray(r.bytes(0x20 * 8))
    end = r.randrange(0, 8)
    ids = [r.choice([1, 2, 3, 100, 0x270e]) for _ in range(8)]
    for i in range(8):
        put(table, 0x20 * i, 'I', 0x270f if i == end else ids[i])
    wanted = r.choice(ids + [0x270f, 0, 4])
    return Call(0x5997b0, [wanted], {0x768fb8: table}, ret='a')


def holder(r, at, regions):
    """A holder at at: +8 -> P, P + 4 -> data (the parts: count word +0x42, elements of 0xa0 from [+0x70], names [+0x54]); motion bank 5:
    flags byte +0x54, count +0x58, entries from [+0x5c] + 4, 8 bytes apart, each a block whose end frame is at +8."""
    h = bytearray(r.bytes(0x100))
    n = r.choice([0, 1, 3, 6])
    data = bytearray(r.bytes(0x80))
    put(h, 8, 'I', at + 0x100)
    regions[at + 0x100] = put(bytearray(r.bytes(8)), 4, 'I', at + 0x200)
    put(data, 0x42, 'H', n)
    put(data, 0x70, 'I', at + 0x400)
    parts = bytearray(r.bytes(0xa0 * n))
    for i in range(n):
        put(parts, 0xa0 * i + 0x54, 'I', at + 0x800 + 0x20 * i)
        regions[at + 0x800 + 0x20 * i] = r.choice(NAMES) + bytes(1)
    regions[at + 0x200] = data
    if n:
        regions[at + 0x400] = parts
    h[0x54] = r.choice([0, 1, 2, 3, 4, r.getrandbits(8)])
    put(h, 0x58, 'I', r.choice([0, 5, 13, 14, 20, 24]))
    put(h, 0x5c, 'I', at + 0xc00)
    table = bytearray(r.bytes(4 + 8 * 24))
    for k in range(24):
        put(table, 4 + 8 * k, 'I', at + 0x1000 + 0x10 * k)
    regions[at + 0xc00] = table
    regions[at + 0x1000] = r.bytes(0x10 * 24)
    regions[at] = h
    return at


def init(r):
    regions = {0xf79660: r.bytes(0x170), 0xf868f0: r.bytes(0xf0), 0x877350: r.bytes(0x810)}
    slots = bytearray(r.bytes(0x160))
    for slot in range(2):
        put(slots, 0xb0 * slot + 4, 'I', holder(r, RAM + 0x2000 * slot, regions) if r.random() < 0.7 else 0)
    regions[0x790398] = slots
    return Call(0x5989c0, [], regions, ret='v')


ADVISOR, MESSAGE, NOW, EVENT, WORLD = RAM + 0x10000, RAM + 0x13000, RAM + 0x14000, RAM + 0x15000, RAM + 0x16000
BASES = [0x22, 0x21, 0x24, 0x23, 0x25]


def queue(r, op):   # Advisor_MayPost (thiscall (id, urgent)), Advisor_Post (thiscall (message)) or Advisor_PostKindEvent (thiscall (event))
    records = 0x15f
    table = bytearray(r.bytes(0x38 * records))
    for k in range(records):
        put(table, 0x38 * k + 8, 'I', r.choice([0, 1, 1, 2, 10, r.randrange(0x80)]))
        put(table, 0x38 * k + 0x20, 'B', r.choice([0, 1, 2, 3, 8, 8, 8, r.getrandbits(8)]))
        put(table, 0x38 * k + 0x28, 'II', r.choice([0, 1, 2, 5]), r.choice([0, 0, 1, 5]))
        put(table, 0x38 * k + 0xc, 'IIi', r.choice([0, 1, 2, 3]), recorder(20, 0), r.choice([0, 4, -4]))
    id = r.choice([r.randrange(records)] * 8 + [0x15f, 0x160, r.randrange(0x161, 0x200)])
    if op == 'PostKind':
        kind, n = r.choice([0, 1, 2, 3, 4] * 4 + [5]), r.choice([r.randrange(0x40)] * 6 + [r.randrange(0x40, 0x60)])
        event = put(bytearray(r.bytes(0x10)), 8, 'II', kind, n)
        id = (5 * n + BASES[kind]) & 0xffffffff if kind < 5 else EVENT
    if id < records:
        where = r.choice(['own'] * 6 + ['other', 'none'])
        put(table, 0x38 * id + 4, 'I', id if where == 'own' else id ^ (1 + r.getrandbits(8)))
        if where == 'other':
            put(table, 0x38 * r.randrange(records) + 4, 'I', id)
    scoring = bytearray(0x63c)
    for k in range(0, 0x63c, 4):
        put(scoring, k, 'I', r.choice([0, 0, 0, 1, 2, 5, 0x40, 100, r.getrandbits(32)]))
    now = r.getrandbits(32)
    advisor = bytearray(r.bytes(0x2100))
    full = r.random() < 0.5   # (every slot set: the lowest score's replaced or not)
    for k in range(8):
        slot = 0x14 + 0x18 * k
        put(advisor, slot + 0xc, 'Ii', r.choice([id, r.randrange(records), r.randrange(records)]), r.choice([0, 1, 5, 5, -1, -1, r.getrandbits(31)]))
        put(advisor, slot + 0x14, 'B', r.choice([1, 1, 2] if full else [1, 1, 1, 0]))
    rec = 0xe0 + 0x10 * id
    if rec + 0x10 <= len(advisor):   # (not for an event's address)
        wait = 0   # (the wait the record's index gives, when it lies in gAdvisorScoring: a time that many quarters ago, give or take one)
        index = struct.unpack_from('<I', table, 0x38 * id + 8)[0] if id < records else 0x1000
        if 0x24 + 12 * index + 4 <= len(scoring):
            wait = struct.unpack_from('<I', scoring, 0x24 + 12 * index)[0]
        exact = (((now >> 2) - wait + r.choice([-1, 0, 0, 1])) & 0x3fffffff) << 2 | r.randrange(4)
        put(advisor, rec, 'I', r.choice([0, 0, 3, now - r.randrange(0x400) & 0xffffffff, exact, exact, r.getrandbits(32)]))
        put(advisor, rec + 4, 'i', r.choice([0, 1, 4, -1, r.getrandbits(31)]))
        put(advisor, rec + 8, 'B', r.choice([0, 0, 1, 0x80]))
        put(advisor, rec + 0xc, 'i', r.choice([0, 1, 2, 5, 200, -1, r.getrandbits(31)]))
    regions = {0x76e300: table, 0xfb3540: scoring, ADVISOR: advisor, 0xfb2766: bytes([r.getrandbits(8) | 1]),
               0x78d90d: bytes([r.choice([0, 1])]), 0x80239c: struct.pack('<I', NOW - 0x1da70c & 0xffffffff), NOW: struct.pack('<I', now)}
    if op == 'MayPost':
        return Call(0x59abc0, [ADVISOR, id, r.choice([0, 0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='b')
    if op in ('PostKind', 'PostId'):
        log = log_region()
        set_return(log, 20, r.choice([0, 1, 5, 100, -1, r.getrandbits(32)]))
        if op == 'PostKind':
            regions[EVENT] = event
        regions.update({LOG: log, 0xfb34bc: bytes([r.choice([1, 1, 0, 3])]),
                        0xfb3b7c: struct.pack('<I', r.choice([0, 1, 2, 3])), 0x7a1a8c: struct.pack('<I', r.getrandbits(32))})
        copy = [0x16d0 + k for k in (2, 3, 0x17)] if id != 0x15f else []   # (+0x16d0 is 0x15f's record, read; 0x15f has no definition to copy)
        offsets = sorted(copy + [0x14 + 0x18 * s + k for s in range(8) for k in (2, 3, 0x17)])
        mask = Mask(ADVISOR, offsets, "the stack message's bytes +2, +3, +0x17, never written (0x59afc0, 0x486b00), copied into a slot or +0x16d0")
        if op == 'PostId':   # (the advisor the world's table entry: [0x7cf83c]'s word +0x1da71c indexes 0x7cfb90's entries of 20 bytes)
            h = r.randrange(16)
            regions[0x7cf83c] = struct.pack('<I', WORLD - 0x1da71c)
            regions[WORLD] = struct.pack('<H', h)
            regions[0x7cfb90 + 20 * h] = struct.pack('<I', ADVISOR)
            return Call(0x486b00, [id], regions, ret='b', masks=[mask])
        return Call(0x59afc0, [ADVISOR, EVENT], regions, conv='t', ret='b', masks=[mask])
    regions[MESSAGE] = put(bytearray(r.bytes(0x18)), 0xc, 'Ii', id, r.choice([0, 3, 9, -1, r.getrandbits(31)]))
    put(regions[MESSAGE], 0x14, 'BB', r.choice([1, 1, 0]), r.choice([0, 0, 1]))
    return Call(0x59a940, [ADVISOR, MESSAGE], regions, conv='t', ret='b')


def case(r):
    op = r.deal(OPS)
    if op in ('MayPost', 'Post', 'PostKind', 'PostId'):
        return queue(r, op)
    return init(r) if op == 'Init' else lookup(r)


if __name__ == '__main__':
    B.main(case)
