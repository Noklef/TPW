# The staff's patrol region, rest area check and wage, and the staff pool's entry lookups
# (src/game/world/staff: 0x506650, 0x5068f0, 0x506e50-0x506ed0, 0x507bd0, 0x5087b0) and the staff
# training's constructor (0x50c620) and the world's list pushes (0x519650-0x519d80) against the
# originals: regions none, whole, inverted or wild, map ids 0 and about the edges, indices about
# 0x20, pools full or with gaps, heads empty or not, things over them.
# usage: staffpatrol.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=State3,Wage,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x506650, 0x506674), (0x5068f0, 0x50690f), (0x506870, 0x5068d4), (0x5da3c0, 0x5da3c1), (0x506e50, 0x506f2e), (0x507bd0, 0x507beb),
           (0x5087b0, 0x5087c9), (0x50c620, 0x50c638), (0x50b350, 0x50b35d),
           (0x519650, 0x519686), (0x5197c0, 0x5197f6), (0x519930, 0x519966), (0x519aa0, 0x519ad6), (0x519c10, 0x519c46),
           (0x519d80, 0x519db3)])
STAFF = RAM
CALLS = {'State3': 0x506650, 'Wage': 0x5068f0, 'GetWage': 0x506870, 'Set': 0x506e50, 'Get': 0x506e70, 'Clear': 0x506e90,
         'Has': 0x506eb0, 'In': 0x506ed0, 'Entry': 0x507bd0, 'FirstInvalid': 0x5087b0, 'Training': 0x50c620, 'Push': 0}


def mapid(r):
    return r.choice([0, 1, 2, 0x7f, 0x80, 0x81, 0x4000, 0x4001, 0xffff, r.randrange(1, 0x4001), r.getrandbits(16)])


def case(r):
    op = r.choice(OPS)
    staff = bytearray(r.bytes(0x280))
    put(staff, 0x19c, 'I', r.choice([3, 3, 0, 2, 0x103, r.getrandbits(32)]))
    area = r.getrandbits(16)
    put(staff, 0x208, 'H', area)
    if r.random() < 0.3:
        put(staff, 0x20a, 'HH', *r.choice([(1, 0x4000), (1, 0x4001), (2, 0x4000), (0x4000, 1)]))
    else:
        put(staff, 0x20a, 'HH', mapid(r), mapid(r))
    regions = {STAFF: staff}
    if op == 'State3':
        return Call(0x506650, [STAFF, r.choice([area, area | r.getrandbits(16) << 16, r.getrandbits(32)])], regions,
                    conv='t', ret='a')
    if op == 'Push':  # the heads at RAM + 0x33c on; the thing apart from them, or its next over one
        mem = bytearray(r.bytes(0x800))
        for k in range(0x33c, 0x348, 2):
            if r.random() < 0.3:
                put(mem, k, 'H', 0)
        thing = RAM + r.choice([0x400, 0x400, 0x33c - 0x210, 0x33e - 0x210, 0x346 - 0xc, 0x33c - 0xc])
        return Call(r.choice([0x519650, 0x5197c0, 0x519930, 0x519aa0, 0x519c10, 0x519d80]), [RAM + 0x300 - 0x1da700, thing],
                    {RAM: mem}, conv='t')
    if op == 'Training':
        return Call(0x50c620, [STAFF + r.randrange(16)], regions, conv='t', ret='a')
    if op == 'GetWage':
        put(staff, 2, 'B', r.choice([4, 5, 6, 7, 8, 3, 9, 0, 0xff, r.getrandbits(8)]))
        put(staff, 0x1e4, 'I', r.choice([0, 1, 2, 3, 4, 0x10, r.randrange(0x10)]))
        return Call(0x506870, [STAFF], {STAFF: staff, 0x785300: r.bytes(0x1100)}, conv='t', ret='a')
    if op == 'Wage':
        regions = {0x785300: r.bytes(0x1100)}
        return Call(0x5068f0, [r.randrange(0x30), r.choice([r.randrange(8), r.getrandbits(32)])], regions, ret='a')
    if op == 'Set':
        return Call(0x506e50, [STAFF, r.getrandbits(32), r.getrandbits(32)], regions, conv='t')
    if op == 'Get':  # the outs anywhere, the same, or over the region
        bl = STAFF + r.choice([0x260, 0x20a, 0x20b, 0x20c])
        return Call(0x506e70, [STAFF, bl, r.choice([bl, STAFF + 0x270, STAFF + 0x20a])], regions, conv='t')
    if op == 'In':
        bl, tr = r.choice([(1, 0x4000), (mapid(r), mapid(r))])
        put(staff, 0x20a, 'HH', bl, tr)
        pos = r.choice([mapid(r), bl, tr, (bl + r.choice([-1, 1, -128, 128])) & 0xffff,
                        (tr + r.choice([-1, 1, -128, 128])) & 0xffff])
        return Call(0x506ed0, [STAFF, pos | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)], regions,
                    conv='t', ret='a')
    if op in ('Entry', 'FirstInvalid'):
        pool = bytearray(r.bytes(0x20 * 20 + 0x20))
        full = r.random() < 0.3
        for i in range(0x21):
            pool[i * 20 + 0xa] = r.choice([1, 0xff, r.getrandbits(8)]) if full or r.random() < 0.85 else 0
        if op == 'FirstInvalid':
            return Call(0x5087b0, [RAM + 0x400], {RAM + 0x400: pool}, conv='t', ret='a')
        return Call(0x507bd0, [RAM + 0x400, r.choice([r.randrange(0x22), 0xffffffff, 0x80000000, r.getrandbits(32)])],
                    {RAM + 0x400: pool}, conv='t', ret='a')
    return Call(CALLS[op], [STAFF], regions, conv='t', ret='v' if op == 'Clear' else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
