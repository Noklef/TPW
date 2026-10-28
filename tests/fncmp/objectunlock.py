# An object definition's availability (src/game/world/objectunlock: 0x4d4b90) against the original: globalsave.py's count() set-up for
# the count left (the instance [0xf7c58c] made: its info), the object definitions 0x7890a0 (1-3 of 0x100 bytes, the cache hit or not,
# the id now and then none of them: a crash in both), the definition's +0xc4 about the count or any (signed both ways).
# Unlock (0x4d4ad0): the same, the mode setting (0xfb3b7c) 0, 1, 2 or any and its guard (0xfb34ba) set or not (then read from 0x7a1a8c's
# bits 23-25, the atexit logged), the info's set (+0x28: a red-black tree of 0-8 16-bit keys, stdtree.py's build, its nil 0xf7c970)
# holding the id or not (an insert: the heap's block). FirstAdvice (0x4d5b30): globalsave.py's count() set-up for the count, mostly
# cut to one set byte or none (its bytes and the records' flags cleared, then one set); advisor.py's queue() for the scoring (its
# table, gAdvisorScoring built); thing_ctors.py's message centre, the post (0x40fb10) recorder 21 hashing message 05's bytes 0-0x1f
# but +0xa, +0xb and +0x1f (the stack's). AdviseLevel (0x4d4fa0): FirstAdvice's world, the group 0-3 or any, the level 1-3 mostly
# (0, 4, -1 or any now and then).
# usage: objectunlock.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=IsUnlocked,Unlock,FirstAdvice,AdviseLevel)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import globalsave  # noqa: E402
import stdtree  # noqa: E402
import thing_ctors  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, blob_bytes, log_region, put  # noqa: E402

CALLS = {'IsUnlocked': 0x4d4b90, 'Unlock': 0x4d4ad0, 'FirstAdvice': 0x4d5b30, 'AdviseLevel': 0x4d4fa0}


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(set(ranges)):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged([(0x5af7f0, 0x5af80b), (0x5afc30, 0x5afc5a), (0x5b09a0, 0x5b09b4), (0x5b0ca0, 0x5b0cc5), (0x5b0940, 0x5b0996), (0x67b0d0, 0x67b10f),
           (0x5aef50, 0x5af0dc), (0x409410, 0x40947a), (0x5b3110, 0x5b3130), (0x5b2fd0, 0x5b2ff0), (0x5b1680, 0x5b16a6),
           (0x5af4a0, 0x5af52a), (0x5af530, 0x5af5cf), (0x5af5d0, 0x5af674), (0x5af680, 0x5af734), (0x5b11c0, 0x5b12d3), (0x5b2480, 0x5b2530),
           (0x5b11a0, 0x5b11b9), (0x4d4b90, 0x4d4bbb), (0x412e90, 0x412eea),
           (0x4d4ad0, 0x4d4b6b), (0x550ca0, 0x550d90), (0x5d1750, 0x5d1751), (0x5da3c0, 0x5da3c1), (0x5af740, 0x5af77e), (0x5b1930, 0x5b1a30),
           (0x5b2ba0, 0x5b2ea0), (0x5b3130, 0x5b3200), (0x5b3210, 0x5b3230), (0x4d5b30, 0x4d5cc2), (0x4d4fa0, 0x4d5252), (0x40f300, 0x40f329), (0x59b550, 0x59b589)]
          + advisor.RANGES + thing_ctors.CENTRE_RANGES),
          hooks={0x691de0: globalsave.lock, 0x691ea0: lambda e: None, 0x67a7d0: _cdecl_log, 0x40fb10: _recorder(21, 1)})   # (globalsave.py's ranges and STL lock stand-ins)
DEFS = RAM + 0x8000
SET_HEAD, SET_NIL, SET_NODES = RAM + 0x3800, RAM + 0x3900, RAM + 0x4000   # (Unlock: the info's set)


def key_set(r, regions, key):
    """Unlock: the info's set at +0x28 (head +0x2c, unique +0x30, size +0x34): 0-8 keys, key among them or not."""
    n = r.choice([0, 1, 2, 3, 5, 8])
    nodes, root = stdtree.build(r, n, pool=[key] + [x for x in r.sample(range(0x10000), n + 1) if x != key][:n]) if n else ({}, None)
    keys = sorted(nodes)
    addr = {k: SET_NODES + 0x20 * i for i, k in enumerate(keys)}
    blob = bytearray(r.bytes(0x20 * max(n, 1)))
    for i, k in enumerate(keys):
        left, parent, right, colour = nodes[k]
        put(blob, 0x20 * i, 'IIII', SET_NIL if left is None else addr[left], SET_HEAD if parent is None else addr[parent],
            SET_NIL if right is None else addr[right], k | r.getrandbits(16) << 16)
        put(blob, 0x20 * i + 0x10, 'I', colour)
    head = struct.pack('<III', addr[keys[0]], addr[root], addr[keys[-1]]) if n else struct.pack('<III', SET_HEAD, SET_NIL, SET_HEAD)
    info = bytearray(regions[globalsave.INFO])
    put(info, 0x2c, 'I', SET_HEAD)
    info[0x30] = 0
    put(info, 0x34, 'I', n)
    regions[globalsave.INFO] = info
    regions[SET_HEAD] = head + bytes(0x20)
    regions[SET_NIL] = put(bytearray(0x20), 0x10, 'I', stdtree.BLACK)
    regions[0xf7c970] = struct.pack('<I', SET_NIL)
    if n:
        regions[SET_NODES] = blob
    regions[0xfb34ba] = bytes([r.choice([1, 1, 1, 3, 0, 2, r.getrandbits(8)])])
    regions[0xfb3b7c] = put(bytearray(4), 0, 'I', r.choice([0, 1, 2, 0, 2, 1, 3, r.getrandbits(32)]))
    regions[0x7a1a8c] = put(bytearray(4), 0, 'I', r.getrandbits(32) & ~(7 << 23) | r.choice([0, 1, 2, 3, 4, 6, 7]) << 23)
    regions.setdefault(LOG, log_region())


def first_advice(r, op='FirstAdvice'):
    c = globalsave.count(r, 'Count')
    regions = dict(c.regions)
    info = bytearray(regions[globalsave.INFO])
    if r.random() < 0.7:   # (one set byte mostly, or none)
        for k in (0x18, 0x19, 0x1a, 0x1b, 0x26, 0x27):
            info[k] = 0
        if globalsave.RECORDS in regions:
            recs = bytearray(regions[globalsave.RECORDS])
            for at in range(0, len(recs), 0x40):
                recs[at:at + 6] = bytes(6)
            regions[globalsave.RECORDS] = recs
        if r.random() < 0.85:
            info[r.choice([0x18, 0x19, 0x1a, 0x1b, 0x26, 0x27])] = r.choice([1, 1, 0x80, 0xff])
    regions[globalsave.INFO] = info
    regions[0xf7c58c] = struct.pack('<I', globalsave.INFO)
    adv = advisor.queue(r, 'MayPost')
    regions.update({k: v for k, v in adv.regions.items() if k != advisor.ADVISOR})
    regions[LOG] = log_region()
    thing_ctors.centre(r, regions)
    blob_bytes(regions[LOG], 21, 0, 0x7ffff3ff)
    if op == 'AdviseLevel':
        level = r.choice([1, 2, 3, 1, 2, 3, 0, 4, -1, r.getrandbits(32) - (1 << 31)])
        while level in (1, 2, 3) and HEAP in regions and regions[HEAP][0] & 15 == 1 and not any(regions.get(0x788d3c, bytes(4))[:4]):
            # (the centre made already: else the level's post would make it, the heap failing its block instead of the global save's
            # later one, and that global.sam block given takes the balance file's load, BalanceFile_Init 0x401030, out of the bench)
            thing_ctors.centre(r, regions)
        return Call(0x4d4fa0, [RAM + 0x7000, r.choice([0, 1, 2, 3, r.getrandbits(32)]), level], regions, conv='t')
    return Call(0x4d5b30, [RAM + 0x7000], regions, conv='t')


def case(r):
    op = r.deal(OPS)
    if op in ('FirstAdvice', 'AdviseLevel'):
        return first_advice(r, op)
    c = globalsave.count(r, 'CountLeft')
    regions = dict(c.regions)
    regions[0xf7c58c] = struct.pack('<I', globalsave.INFO)
    table = bytearray(r.bytes(0x740))
    n = r.randrange(1, 4)
    ids = r.sample(range(0x10000), n)
    for i in range(n):
        put(table, 4 * i, 'I', DEFS + 0x100 * i)
        d = put(bytearray(r.bytes(0x100)), 0, 'H', ids[i])
        put(d, 0xc4, 'i', r.choice([0, 1, 2, 3, 4, 6, 8, -1, r.getrandbits(31), -r.getrandbits(31)]))
        regions[DEFS + 0x100 * i] = d
    put(table, 0x528, 'i', n)
    k = r.randrange(n)
    put(table, 0x730, 'HHI', r.choice([ids[k], r.getrandbits(16)]), 0, DEFS + 0x100 * k)
    regions[0x7890a0] = table
    want = ids[k] if r.random() < 0.97 else r.getrandbits(16)
    if op == 'Unlock':
        key_set(r, regions, want)
    return Call(CALLS[op], [want | r.choice([0, r.getrandbits(16)]) << 16], regions, conv='s', ret='b')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
