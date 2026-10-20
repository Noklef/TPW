# An animator's mesh built (0x424ef0 Animator_Create, src/game/model/holder) against the original: a random animator of 0-5 nodes
# (their counts small, flag 4 or not), a record shared (flag 0) or owned (its 0-5 records' names copied), a name or none, flags 0-3
# and more. The block comes from global_heap(); the texture directories (0x46e4e0) and the holder (0x45bbc0) are logged
# (CDECL_LOG2 / CDECL_LOG, as driver.cpp's AnimatorMeshEnv): their own benches are texdirs.py and objmodel.py.
# Clone (0x455690 Animator_Clone): an animator of 0-4 nodes whose holder is clone.py's (Holder_Clone run, the holder made logged).
# usage: animatormesh.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Create,Clone)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _cdecl_log2, global_heap, heap, log_region, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import clone  # noqa: E402

RANGES = [(0x424ef0, 0x425440), (0x45b9d0, 0x45ba15), (0x45ba70, 0x45bab0), (0x46f150, 0x46f190), (0x67ae00, 0x67af10)]
HOOKS = {0x46e4e0: _cdecl_log2, 0x45bbc0: _cdecl_log}
B = Bench(RANGES + [(0x455690, 0x4557bf), (0x454a10, 0x455689), (0x44b140, 0x44b220)], hooks=HOOKS)
ANIM, NODES, REC, ENTRIES, TEXTS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400


def text(r):
    return bytes(r.randrange(0x20, 0x7f) for _ in range(r.choice([0, 3, 12, 19, 20, 21, 30]))) + b'\0'


def mesh(r, anim=ANIM, nodes_at=NODES, rec=REC, entries_at=ENTRIES, texts_at=TEXTS):
    """The regions of a case: the animator (its nodes), the record (its entries, their names), the heap and the log; returns (regions, name)."""
    count = r.choice([0, 1, 1, 2, 3, 5])
    small = lambda: r.choice([0, r.randrange(1, 4), r.randrange(4, 13)])
    nodes = bytearray(r.bytes(max(count, 1) * 0x30))
    for i in range(count):
        put(nodes, i * 0x30, 'I', r.getrandbits(32))
        put(nodes, i * 0x30 + 4, '5H', *(small() for _ in range(5)))
    animator = put(put(bytearray(r.bytes(0x40)), 0, 'I', r.getrandbits(32)), 4, 'H', count)
    put(animator, 8, 'I', nodes_at)
    records = r.choice([0, 1, 2, 5])
    texts, entries = bytearray(), bytearray(r.bytes(max(records, 1) * 0x10))
    for i in range(records + 1):
        if i < records:
            put(entries, i * 0x10 + 4, 'I', texts_at + len(texts))
        texts += text(r)
    record = put(put(bytearray(r.bytes(0x20)), 0, 'I', r.getrandbits(32)), 4, 'H', records)
    put(record, 0x10, 'I', entries_at)
    put(record, 0x1c, 'I', r.choice([0, 1, 7, r.getrandbits(32)]))
    name = r.choice([0, texts_at + len(texts) - 1, texts_at])   # (no name, an empty one, the first text)
    regions = {anim: animator, nodes_at: nodes, rec: record, entries_at: entries, texts_at: texts, HEAP: heap(0x1c00), LOG: log_region()}
    regions.update(global_heap())
    return regions, name


def create(r):
    regions, name = mesh(r)
    flags = r.choice([0, 1, 2, 3, r.getrandbits(32)])
    return Call(0x424ef0, [ANIM, REC, name, flags], regions, ret='a')


SOURCE, SNODES, SREC, SOUT = RAM + 0x5000, RAM + 0x5100, RAM + 0x5400, RAM + 0x5500


def clone_case(r):
    """0x455690 (cdecl; animator, unused, flags, out): clone.py's holder case (its flags and holder) under an animator of 0-4 nodes;
    unused 0 now and then ([0x79fb2c] then)."""
    c = clone.holder(r)
    regions = dict(c.regions)
    count = r.choice([0, 1, 2, 4])
    regions[SOURCE] = put(put(put(bytearray(r.bytes(0x40)), 4, 'H', count), 8, 'I', SNODES), 0x24, 'I', c.args[0])
    put(regions[SOURCE], 0x3c, 'I', SREC)
    regions[SNODES] = r.bytes(max(count, 1) * 0x30)
    regions[SREC] = r.bytes(0x20)
    regions[SOUT] = r.bytes(4)
    regions[0x79fb2c] = r.bytes(4)
    regions[LOG] = log_region()
    return Call(0x455690, [SOURCE, r.choice([0, r.getrandbits(32)]), c.args[2], SOUT], regions, ret='a')


CASES = {'Create': create, 'Clone': clone_case}
OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
