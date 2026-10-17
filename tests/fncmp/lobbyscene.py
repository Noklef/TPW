# The lobby's model owners (src/game/frontend/lobbymodel: 0x5da120, 0x5dfca0, 0x5e1350) against the originals: a list of items
# whose models (lobby models, RideUser_Acquire's users) are set up once, some already set up, one or several the holder bench's
# ride user (tests/fncmp/holder.py acquire()); an owner of two such models; an item whose pending play (state 2) is started on
# the lobby model bench's world (tests/fncmp/lobbymodel.py), other states left or cleared.
# usage: lobbyscene.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=List,Pair,Play)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import holder  # noqa: E402
import lobbymodel  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(a, b) for (a, b) in holder.RANGES if not 0x67a400 <= a < 0x690000] +
          [(0x5d8190, 0x5d81be), (0x5d8380, 0x5d8463), (0x45bf20, 0x45bf46), (0x46f150, 0x475010), (0x67a400, 0x690000),
           (0x5da120, 0x5da161), (0x5dfca0, 0x5dfcc3), (0x5e1350, 0x5e137d)], hooks=holder.HOOKS, sines=256)
OWNER, LIST, NODES, ITEMS, DONE = RAM + 0xc000, RAM + 0xc100, RAM + 0xc200, RAM + 0xc400, RAM + 0xc800
CALLS = ['List', 'Pair', 'Play']


def users(r):
    """The holder bench's ride user case; returns its regions and a picker of users: the real one or done ones."""
    regions = dict(holder.acquire(r, True).regions)
    real = r.random() < 0.85
    if real:
        put(regions[holder.ACQ_USER], 8, 'I', 0)
    done = bytearray(r.bytes(0x40))
    for k in range(4):
        put(done, 0x10 * k + 8, 'I', r.choice([1, 1, 5, 0xffffffff]))
    regions[DONE] = done
    return regions, lambda: holder.ACQ_USER if r.random() < 0.4 else DONE + 0x10 * r.randrange(4)


def case(r):
    op = r.deal(OPS)
    if op == 'Play':
        lobbymodel.OPS = ['Play']
        regions = dict(lobbymodel.case(r).regions)
        item = bytearray(r.bytes(0x20))
        put(item, 0xc, 'I', OWNER + 0x40)
        put(item, 0x14, 'I', r.choice([0, 0, 1, 2, 2, 2, 3, r.getrandbits(32)]))
        owner = put(bytearray(r.bytes(0x10)), 8, 'I', lobbymodel.OBJ)
        regions.update({OWNER: item, OWNER + 0x40: owner})
        return Call(0x5e1350, [OWNER], regions, conv='t', ret='a')
    regions, user = users(r)
    owner = bytearray(r.bytes(0x60))
    put(owner, 4, 'I', r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))
    if op == 'Pair':
        put(owner, 0x10, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
        put(owner, 4, 'I', user())
        put(owner, 8, 'I', user())
        regions[OWNER] = owner
        return Call(0x5dfca0, [OWNER], regions, conv='t', ret='v')
    n = r.choice([0, 1, 2, 3, 5, 8])
    nodes, items = bytearray(r.bytes(8 * max(n, 1))), bytearray(r.bytes(0x10 * max(n, 1)))
    for k in range(n):
        put(nodes, 8 * k, 'II', ITEMS + 0x10 * k, NODES + 8 * (k + 1) if k + 1 < n else 0)
        put(items, 0x10 * k + 8, 'I', user())
        put(items, 0x10 * k + 0xc, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    put(owner, 0x58, 'I', LIST)
    regions.update({OWNER: owner, LIST: put(bytearray(r.bytes(8)), 4, 'I', NODES if n else 0), NODES: nodes, ITEMS: items})
    return Call(0x5da120, [OWNER], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
