# The GUI library's window script readers (src/game/ui/windowscript: 0x65fbf0, 0x65fc18, 0x65fc38, 0x65fcc5) against the originals: a
# script of 0x200 random words from RAM + 0x100, the read position [0xfaa620] in it or up to 0x80 words before it; the words read to an
# output block or (now and then) into the script itself at the read position.
# usage: windowscript.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ReadDword,ReadWord,ReadRect,ReadPair)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'ReadDword': 0x65fbf0, 'ReadWord': 0x65fc18, 'ReadRect': 0x65fc38, 'ReadPair': 0x65fcc5}
B = Bench([(0x65fbf0, 0x65fd0e)])
SCRIPT, OUT = RAM + 0x100, RAM + 0x1000


def case(r):
    op = r.deal(OPS)
    pos = r.choice([0, 1, 2, 3, r.randrange(0x1f8), r.randrange(0x1f8), 0x1f8])
    if r.random() < 0.2:   # a little before the script (EAX's upper half 0xffff for ReadWord)
        pos = -r.randrange(1, 0x81) & 0xffffffff
    regions = {RAM: r.bytes(0x100 + 0x400 + 0x10), OUT: r.bytes(0x20), 0xfaa620: struct.pack('<I', pos)}
    if op in ('ReadDword', 'ReadWord'):
        return Call(CALLS[op], [SCRIPT], regions, ret='a')
    n = 4 if op == 'ReadRect' else 2
    outs = [OUT + 2 * r.randrange(16) for _ in range(n)]
    if r.random() < 0.2:   # into the script, over words still to be read
        outs = [(SCRIPT + 2 * pos + 2 * r.randrange(4)) & 0xffffffff for _ in range(n)]
    return Call(CALLS[op], [SCRIPT] + outs, regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
