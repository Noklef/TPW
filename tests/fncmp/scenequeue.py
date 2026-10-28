# The front-end scene's id queue (src/game/frontend/scenequeue: 0x5d6060 Class702da0_Clear, Clear; 0x5d6110 Class702da0_Push, Push) against
# the originals, over advisorspeech.py's Stop world (the advisor speaking or not, 0xf7966c, Advisor_StopSpeech's sound and particle
# worlds; the heap from it, larger): the object's list at +8 of 0-4 nodes {item, next, prev} (the items any pointers: freed, hashed), its
# head +0xc and tail +0x10 theirs, the count +0x14 theirs or fewer (more would loop on a missing head, sic: Mem_Free(0) forever), the
# nodes' back links any. Push: the id any, clear 0, 1 or any.
# usage: scenequeue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear,Push)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, RAM, Bench, Call, _cdecl_log, _free, heap, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import advisorspeech as asp  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(asp.merged(asp.soundset.RANGES + asp.advisor.RANGES + [(0x5994e0, 0x5996c9), (0x402d80, 0x403220), (0x429d60, 0x429e00),
                                                               (0x5ed5a0, 0x5ed670), (0x51bfc0, 0x51c048), (0x5d6060, 0x5d60bf), (0x5d6110, 0x5d6191)]),
          hooks={0x67a7d0: _cdecl_log, 0x67ad90: _free})   # (advisorspeech.py's Stop ranges and hooks)
OBJ, NODES = RAM + 0x60000, RAM + 0x60100


def case(r):
    op = r.deal(OPS)
    regions = asp.stop_world(r)
    regions[HEAP] = heap(0x200)
    n = r.choice([0, 1, 1, 2, 3, 4])
    nodes = bytearray(r.bytes(0x10 * max(n, 1)))
    for k in range(n):
        put(nodes, 0x10 * k, 'II', r.choice([RAM + 0x61000 + 0x10 * k, r.getrandbits(32), 0]), NODES + 0x10 * (k + 1) if k + 1 < n else 0)
    obj = bytearray(r.bytes(0x18))
    put(obj, 0xc, 'III', NODES if n else 0, NODES + 0x10 * (n - 1) if n else r.choice([0, r.getrandbits(32)]),
        r.choice([n, n, n, max(n - 1, 0), 0]))
    regions.update({OBJ: obj, NODES: nodes})
    if op == 'Clear':
        return Call(0x5d6060, [OBJ], regions, conv='t', ret='v')
    return Call(0x5d6110, [OBJ, r.getrandbits(32), r.choice([0, 1, 1, r.getrandbits(32)])], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Clear,Push').split(',') if k]


if __name__ == '__main__':
    B.main(case)
