# An advisor message sent (src/game/world/advisorsend: 0x59b620, 0x59b870) against the original, message 18's constructor and the message centre's creation real, the rest recorders: the byte +0x14 clear, the score under, at and over the bar, the offset in and past the count or negative, speech id 0x266, the centre made, not yet made, or its block failing, the scoring static made or not.
# usage: advisorsend.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob_bytes, fill, heap, log_region, set_return, strings  # noqa: E402
import messagecentre  # noqa: E402
import thing_ctors  # noqa: E402

CONSTRUCT, ATEXIT, SCORE, COUNT, BASE, SAY, LOOKUP, POST = range(1, 9)
B = Bench([(0x59b620, 0x59b860), (0x59b870, 0x59ba30), (0x40f740, 0x40f7a0)] + thing_ctors.CENTRE_RANGES,
          hooks={**thing_ctors.CENTRE_HOOKS, 0x59bea0: _recorder(CONSTRUCT, 0), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x59c0a0: _recorder(SCORE, 3), 0x59c2b0: _recorder(COUNT, 1), 0x59c240: _recorder(BASE, 1),
                 0x599050: _recorder(SAY, 1, cdecl=True), 0x5997b0: _recorder(LOOKUP, 1, cdecl=True), 0x40fb10: _recorder(POST, 1)})
MSG, OUT = RAM, RAM + 0x40
OPS = ['ready', 'ready', 'ready', 'taken', 'under', 'at bar', 'past', 'negative', 'silent', 'silent past', 'no memory', 'first use']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    me = bytearray(r.bytes(0x18))
    me[0x14] = 0 if op == 'taken' else r.deal([1, 1, 0xff, 0x80])
    count = r.randrange(1, 10)
    offset = count + r.randrange(0, 3) if op in ('past', 'silent past') else -r.randrange(1, 5) if op == 'negative' else r.randrange(0, count)
    base = 0x266 if op == 'silent past' else 0x266 - offset if op == 'silent' else r.randrange(0x400)
    bar = r.randrange(-50, 50)
    score = bar - r.randrange(1, 20) if op == 'under' else bar if op == 'at bar' else bar + r.randrange(0, 80)
    set_return(log, SCORE, score & 0xffffffff)
    set_return(log, COUNT, count)
    set_return(log, BASE, base)
    set_return(log, SAY, r.getrandbits(32))
    set_return(log, LOOKUP, r.getrandbits(32))
    strings(log, SCORE, 0, 2 | 4)
    fill(log, SCORE, 1, struct.pack('<H', r.deal([0, r.randrange(1, 0x10000)])), which=0)
    fill(log, SCORE, 2, struct.pack('<I', r.deal([0, r.randrange(1, 1 << 32)])), which=1)
    blob_bytes(log, POST, 0, 0xff3fff)   # (not message 18's +0xe, +0xf: the rest of its word, stack garbage)
    guard = r.getrandbits(8)
    regions = {LOG: log, MSG: me, OUT: r.bytes(4), HEAP: heap(), 0xfb2766: bytes([guard & 0xfe if op == 'first use' else guard | 1]), 0xfb3560: struct.pack('<i', bar)}
    thing_ctors.centre(r, regions)
    if op == 'no memory':
        regions[messagecentre.CENTRE] = bytes(4)
        regions[HEAP] = heap(0x4000, fail=1)
    entry = r.deal([0x59b620, 0x59b870])
    return Call(entry, [MSG, OUT, offset & 0xffffffff], regions, conv='t', ret='a', masks=thing_ctors.centre_masks(regions, 0x10))


if __name__ == '__main__':
    B.main(case)
