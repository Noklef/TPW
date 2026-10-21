# The dialog at 0x7cb25c (src/game/ui/dialog7cb25c: 0x49f860) against the original. SetTexts: the dialog [0x7cb25c] (or none now and then)
# with its children 0x12c4bd (the panel) and 0x12c4ca (the title: its slot 9 a recorder) among 0-2 decoys, the panel's children 0x11, 0x12
# and 0x15 among 0-2 decoys (now and then one of the five missing: a fault in both), each a window of random bytes whose impl +0x9c is a
# recorder object (slot 3: its event's second event's dword +0 hashed, the text) or none; the flag [0x7cb260] 0, 1 or any; the language
# texts as panelparts.py's texts() makes them.
# usage: dialog7cb25c.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetTexts)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import panelparts  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, blob_bytes, log_region, put, vtable  # noqa: E402

CALLS = {'SetTexts': 0x49f860}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x485b00, 0x485b40), (0x49f860, 0x49f9ff), (0x65d7bc, 0x65d832), (0x65f5b7, 0x65f5fe), (0x6ace60, 0x6aceb0), (0x6adc80, 0x6add40)])
ROOT, PANEL, NODES, VT, IMPL, IMPLVT = RAM + 0x1000, RAM + 0x1200, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200
NODE = 0x120


def window(r, ident):
    buf = bytearray(r.bytes(NODE))
    put(buf, 0, 'I', VT)
    put(buf, 0x54, 'I', 0)
    put(buf, 0x5c, 'I', 0)
    put(buf, 0x8c, 'I', ident)
    put(buf, 0x9c, 'I', r.choice([IMPL, IMPL, IMPL, 0]))
    return buf


def kids(r, regions, parent, ids, at):
    """the children `ids` of `parent` (one now and then missing) among 0-2 decoys, laid out from `at`"""
    ids = list(ids)
    if r.random() < 0.04:
        ids.remove(r.choice(ids))
    ids += [r.choice([0, 0x13, 0x12c4be, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(ids)
    head = 0
    for i, ident in reversed(list(enumerate(ids))):
        address = PANEL if ident == 0x12c4bd else at + NODE * i
        regions[address] = put(window(r, ident), 0x54, 'I', head)
        head = address
    put(regions[parent], 0x5c, 'I', head)


def case(r):
    r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    regions[ROOT] = window(r, 0)
    kids(r, regions, ROOT, (0x12c4bd, 0x12c4ca), NODES)
    if PANEL in regions:
        kids(r, regions, PANEL, (0x11, 0x12, 0x15), NODES + 0x800)
    regions[VT] = vtable({9: 1})
    regions[IMPL] = struct.pack('<I', IMPLVT)
    regions[IMPLVT] = vtable({3: 2})
    blob_bytes(log, 3, 1, 0, (0x10, 0xf))
    regions[0x7cb25c] = struct.pack('<I', ROOT if r.random() < 0.95 else 0)
    regions[0x7cb260] = struct.pack('<I', r.choice([0, 0, 1, 1, r.getrandbits(32)]))
    regions[0x78b530] = r.bytes(0x400)
    panelparts.texts(r, regions, list(range(0xaa, 0xb5)))
    return Call(CALLS['SetTexts'], [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
