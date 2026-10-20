# The advisor's scoring (src/game/world/advisorscore) against the originals: AdvisorScoring_Score (0x59bf20) and ScoreDetail (0x59c0a0)
# on a random object, and AdvisorMsg_Init (0x59b590) on a random message. The table at 0x76e300 random but for the id's record: the id in
# its own slot, or only in another (the slot then still used, sic), or nowhere, or 0x15f / 0x160 / past the table; the record's +0xc 0, 1,
# 2 or other, its method a recorder answering anything, its this offset small. The mode setting (0xfb3b7c) any, its guard (0xfb34bc) set
# or not (then read from 0x7a1a8c's bits 23-25, the atexit logged). Init: the score given (at or above 0) or not, then gAdvisorScoring's
# guard (0xfb2766) mostly set; unset, it is built (AdvisorScoring_Construct) from the advisor file as balanceload.py serves it.
# usage: advisorscore.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Score,Detail,Init)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, _cdecl_log, log_region, put, recorder, set_return  # noqa: E402

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import balanceload  # noqa: E402

FILE_RANGES = [(0x401030, 0x401212), (0x401250, 0x40127a), (0x401280, 0x4013dc), (0x4013e0, 0x40161d), (0x401650, 0x4017a0),
               (0x4017a0, 0x402720), (0x402720, 0x402a1e), (0x405710, 0x405742), (0x5f5660, 0x5f5690), (0x5b0d60, 0x5b0d64),
               (0x5f56f0, 0x5f5a30), (0x5f5ad0, 0x5f5bc0), (0x5f5c70, 0x5f5d0e), (0x5f5e20, 0x5f5e90), (0x5f5fc0, 0x5f7f00),
               (0x67a400, 0x6902c0), (0x583280, 0x5832f0), (0x585710, 0x58571d), (0x59bea0, 0x59bf0f), (0x415e90, 0x415ea4)]
B = Bench([(0x59bf20, 0x59c093), (0x59c0a0, 0x59c235), (0x59b590, 0x59b611), (0x550ca0, 0x550d90), (0x5d1750, 0x5d1751),
           (0x5da3c0, 0x5da3c1)] + FILE_RANGES, hooks={0x67a7d0: _cdecl_log})
TABLE, RECORDS = 0x76e300, 0x15f
SLOT = 20   # the method's recorder
SELF, WORD, DWORD, MSG = RAM + 0x10100, RAM + 0x10200, RAM + 0x10210, RAM + 0x10300


def table(r, regions, log):   # the table and the id looked up in it
    t = bytearray(r.bytes(0x38 * RECORDS))
    id = r.choice([r.randrange(RECORDS)] * 8 + [0x15f, 0x160, r.randrange(0x161, 0x200)])
    if id < RECORDS:
        where = r.choice(['own'] * 6 + ['other', 'other', 'none'])
        put(t, 0x38 * id + 4, 'I', id if where == 'own' else id ^ (1 + r.getrandbits(8)))
        if where == 'other':
            put(t, 0x38 * r.randrange(RECORDS) + 4, 'I', id)
        put(t, 0x38 * id + 0xc, 'IIi', r.choice([0, 1, 2, 0, 1, 2, 3, r.getrandbits(32)]), recorder(SLOT, 0), r.choice([0, 4, 0x10, -4, 0x1234]))
    regions[TABLE] = t
    set_return(log, SLOT, r.choice([0, 1, 5, 100, -1, -7, r.getrandbits(32)]))
    regions[0xfb34bc] = bytes([r.choice([1, 1, 0, 0, 2, 3, r.getrandbits(8)])])
    regions[0xfb3b7c] = put(bytearray(4), 0, 'I', r.choice([0, 1, 2, 0, 1, 2, 3, r.getrandbits(32)]))
    regions[0x7a1a8c] = put(bytearray(4), 0, 'I', r.getrandbits(32) & ~(7 << 23) | r.choice([0, 1, 2, 3, 4, 6, 7]) << 23)
    return id


def init(r):
    if r.random() < 0.15:   # gAdvisorScoring not built yet
        regions = balanceload.common(r, 'Graphics')
        regions[balanceload.FILE_STREAM_VTABLE] = regions[balanceload.FILE_STREAM_VTABLE][:0x28]   # (gAdvisorScoring's file's vtable follows)
        log = regions[LOG]
        regions[0xfb2766] = bytes([r.choice([0, 2, 0xfe])])
    else:
        log = log_region()
        regions = {LOG: log, 0xfb2766: bytes([r.choice([1, 1, 3, 0xff])])}
    regions[0xfb3540] = r.bytes(0x63c)
    id = table(r, regions, log)
    regions[MSG] = r.bytes(0x18)
    score = r.choice([-1, -1, -1, -2, -0x80000000, 0, 1, 7, 0x7fffffff, r.getrandbits(32)])
    args = [MSG, id, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), score]
    return Call(0x59b590, args, regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Init':
        return init(r)
    log = log_region()
    regions = {LOG: log, SELF: r.bytes(0x40)}
    id = table(r, regions, log)
    if op == 'Score':
        return Call(0x59bf20, [SELF, id], regions, conv='t', ret='a')
    regions[WORD] = r.bytes(4)
    regions[DWORD] = r.bytes(4)
    return Call(0x59c0a0, [SELF, id, WORD, DWORD], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Score,Detail,Init').split(',') if k]

if __name__ == '__main__':
    B.main(case)
