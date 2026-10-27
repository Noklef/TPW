# The advisor's event handlers (src/game/world/advisor: Advisor_HandleCode 0x59ae20, Advisor_HandleMessage 0x59b060) against the
# originals, with all they call. The world is advisor.py's PostKind world (its table, gAdvisorScoring, the advisor, the scoring methods'
# recorder and the mode setting), with each id the handlers post given its own record in the table now and then (its queue limit +0x20
# mostly above 0) and an advisor record (none mostly, a time near now or any; a count, a flag, a limit); a
# queue slot freed now and then; the byte 0x78d90d mostly set; the mode setting's guard 0xfb2765 set or not; OutputDebugStringA a recorder. The
# event: its slot 0 a recorder answering the code (5, 8, 14, 17, 19, 23, 25 mostly; another, any); +8 a message (case 5: its +0x14 set
# or not), a word (8, 14: an index of the world table 0x7cfb90, whose 16 entries point at random blocks; 17) or a code (19: as
# HandleCode's); +0xc a kind (14: 0-2 or another; 17: 0-4 or another) or an id (25: in the table, 0x15f, 0x160 or past it); 23 the
# PostKind event. HandleCode's code: 0, 2, 3, 4, 10 mostly, another or any. The stack messages' unwritten bytes are masked as advisor.py
# masks them (the queue slots, and +0x16d0 unless the id 0x15f may be posted).
# usage: advisorevents.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Code,Message)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, Mask, _cdecl_log, import_table, put, set_return, vtable  # noqa: E402
import advisor  # noqa: E402

B = Bench(advisor.RANGES + [(0x59ae20, 0x59af95), (0x59b060, 0x59b4e8), (0x59b540, 0x59b544)], hooks={0x67a7d0: _cdecl_log})
ADVISOR, EVENT = advisor.ADVISOR, advisor.EVENT
VT, ENTRIES = RAM + 0x17000, RAM + 0x18000
OPS = {'Code': 0x59ae20, 'Message': 0x59b060}
CODE_IDS = {0: [0, 0x143], 2: [0x6a], 3: [0x80], 4: [0x81]}


def handle_code(r):
    return r.choice([0, 0, 2, 3, 4, 10, 10, 1, r.randrange(11), r.randrange(11, 0x20), r.getrandbits(32)])


def ready(r, regions, now, id):   # (id's own record in the table now and then, and an advisor record)
    if id >= 0x15f:
        return
    if r.random() < 0.85:
        put(regions[0x76e300], 0x38 * id + 4, 'I', id)
        put(regions[0x76e300], 0x38 * id + 0x20, 'B', r.choice([1, 2, 8, 8, 0]))
    rec = 0xe0 + 0x10 * id
    adv = regions[ADVISOR]
    put(adv, rec, 'I', r.choice([0, 0, 0, 0, 3, now - r.randrange(0x400) & 0xffffffff, r.getrandbits(32)]))
    put(adv, rec + 4, 'i', r.choice([3, 3, 6, 0, 1, 4, -1, r.getrandbits(31)]))   # (mostly not the definition's +0x28 less 1)
    put(adv, rec + 8, 'B', r.choice([0, 0, 0, 1, 0x80]))
    put(adv, rec + 0xc, 'i', r.choice([0, 1, 2, 5, 200, -1]))


def case(r):
    op = r.deal(PICK)
    regions = dict(advisor.queue(r, 'PostKind').regions)
    regions[0x76e300] = bytearray(regions[0x76e300])
    regions[ADVISOR] = bytearray(regions[ADVISOR])
    now = struct.unpack('<I', regions[advisor.NOW])[0]
    log = regions[LOG]
    regions[IMPORT_TABLE] = import_table(log, {'OutputDebugStringA': (45, 0x6fd1e0, 1, 1, 0)})
    regions[0xfb2765] = bytes([r.getrandbits(8) & ~1 | r.choice([0, 1])])
    kind_event = bytes(regions[EVENT])
    ids, mask = [], True
    if op == 'Code':
        code = handle_code(r)
        ids = CODE_IDS.get(code, [])
        args = [ADVISOR, code]
    else:
        code = r.choice([5, 5, 8, 14, 17, 19, 23, 25] * 3 + [r.randrange(30), r.getrandbits(32)])
        set_return(log, 0, code)
        event = bytearray(r.bytes(0x40))
        put(event, 0, 'I', VT)
        regions[VT] = vtable({}, size=1)
        if code == 5:
            id = r.choice([r.randrange(0x15f)] * 4 + [0x15f, 0x160])
            put(event, 0x14, 'Ii', id, r.choice([0, 3, 9, -1, r.getrandbits(31)]))
            put(event, 0x1c, 'BB', r.choice([1, 1, 0]), r.choice([0, 0, 1]))
            ids, mask = [id], False
        elif code in (8, 14, 17):
            w = r.randrange(16) if code == 14 or r.random() < 0.7 else r.getrandbits(16)
            put(event, 8, 'H', w)
            if code == 8:
                ids = [0x102]
            elif code == 14:
                kind = r.choice([0, 1, 2] * 2 + [3, r.getrandbits(32)])
                put(event, 0xc, 'I', kind)
                ids = [0x7d + kind] if kind < 3 else []
                table = bytearray(20 * 16)
                for k in range(16):
                    put(table, 20 * k, 'I', ENTRIES + 0x10 * k)
                regions.update({0x7cfb90: table, ENTRIES: r.bytes(0x100)})
            else:
                kind = r.choice([0, 1, 2, 3, 4] * 2 + [5, r.getrandbits(32)])
                put(event, 0xc, 'I', kind)
                ids = [[0x5d, 0x5f, 0x60, 0x61, 0x5e][kind]] if kind < 5 else []
        elif code == 19:
            sub = handle_code(r)
            put(event, 8, 'I', sub)
            ids = CODE_IDS.get(sub, [])
        elif code == 23:
            event[8:0x10] = kind_event[8:0x10]
            kind, n = struct.unpack_from('<II', event, 8)
            ids = [(5 * n + advisor.BASES[kind]) & 0xffffffff] if kind < 5 else []
        elif code == 25:
            id = r.choice([r.randrange(0x15f)] * 6 + [0x160, 0x160, 0x15f, r.randrange(0x161, 0x200)])
            put(event, 0xc, 'I', id)
            ids = [id] if id != 0x160 else []
        regions[EVENT] = event
        args = [ADVISOR, EVENT]
    for id in ids:
        ready(r, regions, now, id)
    regions[0x78d90d] = bytes([r.choice([1, 1, 1, 0])])   # (mostly: no Record_IsActive test, the posts land more often)
    if r.random() < 0.6:   # (a free queue slot)
        regions[ADVISOR][0x28 + 0x18 * r.randrange(8)] = 0
    masks = []
    if mask:
        copy = [0x16d0 + k for k in (2, 3, 0x17)] if 0x15f not in ids else []
        offsets = sorted(copy + [0x14 + 0x18 * s + k for s in range(8) for k in (2, 3, 0x17)])
        masks = [Mask(ADVISOR, offsets, "the stack message's bytes +2, +3, +0x17, never written (0x59ae20, 0x59b060, 0x59afc0), copied into a "
                      "slot or +0x16d0 (advisor.py's)")]
    return Call(OPS[op], args, regions, conv='t', ret='v', masks=masks)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
