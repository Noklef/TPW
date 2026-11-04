# The overlays made from a named folder's files (src/game/gfx/overlayloadall: 0x4891d0, 0x4893c0) against the originals. The three loaders, Record_CopyWithText and the Strings real, overlayload.py's
# surroundings under them; Quickload_LoadRide, NamedList_Find, 0x41d570 and the next file (0x41d710) recorders as driver.cpp's kOverlayLoadAllEnv: the out's folder and the entries' names filled (their other
# bytes unset, stack garbage nothing reads), the pattern logged by its text, at most 4 files.
# usage: overlayloadall.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import overlayload  # noqa: E402
from fncmp import RAM, Bench, Call, _recorder, answers, blob_bytes, fill, put, set_return, strings  # noqa: E402

RIDE, FIND, FIRST, NEXT = 10, 11, 12, 13
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4891d0, 0x489350), (0x489350, 0x4893c0),(0x4893c0, 0x4895a8), (0x488d20, 0x488ed4), (0x488ee0, 0x489058), (0x489060, 0x4891ca), (0x424cf0, 0x424dc0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000),
           (0x41ce80, 0x41ce94), (0x51c050, 0x51c054), (0x41a820, 0x41a826), (0x5c5480, 0x5c5540)],
          hooks={0x46dcf0: C(17, 5), 0x46ead0: C(18, 3), 0x477250: _recorder(19, 1), 0x477580: _recorder(20, 1), 0x476a90: _recorder(21, 1), 0x659a58: C(22, 2),
                 0x461a60: C(RIDE, 3), 0x421770: _recorder(FIND, 2), 0x41d570: _recorder(FIRST, 2), 0x41d710: _recorder(NEXT, 1)})
FOLDER, NEXTS = RAM + 0x62000, RAM + 0x62100
WIDTH = 0x18


def name(r, stems, tail, language):
    if language and r.random() < 0.95:   # (0x4893c0 faults on a file that makes no overlay)
        return r.choice(['plain', 'other', 'tab']) + tail
    picked = r.choice(stems[:2] * 2 + stems[2:] * 2 + ['!cbut', '!other', 'plain', 'other', overlayload.SHADOW])
    return picked if picked == overlayload.SHADOW else picked + tail


def case(r):
    log, regions, tail, stems = overlayload.setup(r)
    language = r.random() < 0.5
    regions[0xf7f230] = r.choice([b'english', b'', b'de\\']) + b'\0'
    overlayload.string(regions, 0xf80be0, 6, '\\')
    overlayload.string(regions, 0x7b9760, 7, r.choice(['*.md2', '*', '']))
    regions[0x786b9c] = r.bytes(4)
    folder = bytearray(r.bytes(0x40))
    words = overlayload.directory(r)
    overlayload.text(regions, 8, words)
    folder[0x18:0x2c] = struct.pack('<5I', 0x703090, len(words), overlayload.TEXTS + 0x40 * 8, (len(words) & ~7) + 8, 8)
    regions[FOLDER] = folder
    set_return(log, FIND, r.deal([1] * 12 + [0, 2]))
    fill(log, FIND, 1, struct.pack('<I', FOLDER), which=1)
    strings(log, FIND, 0, 0b10)
    set_return(log, FIRST, r.deal([1] * 12 + [0, 2]))
    if r.random() < 0.95:   # (else the entry's empty name, as it was made)
        fill(log, FIRST, 1, name(r, stems, tail, language).encode() + b'\0', which=0, offset=0x24)
    blob_bytes(log, FIRST, 0, 0xfffff, inner=(8, 0x7fff))
    strings(log, FIRST, 0, 0b10)
    records = b''.join((name(r, stems, tail, language).encode() + bytes(WIDTH))[:WIDTH] for _ in range(4))
    fill(log, NEXT, 0, records, which=2, offset=0x24, records=4)
    strings(log, NEXT, 0, 0b1)
    more = [r.deal([1, 1, 2, 0x80000000]) for _ in range(r.deal([0, 0, 1, 2, 3]))]
    regions[NEXTS] = struct.pack('<40I', *(more + [0] * (40 - len(more))))
    answers(log, NEXT, NEXTS)
    set_return(log, RIDE, r.getrandbits(32))
    strings(log, RIDE, 0b11)
    return Call(0x4893c0 if language else 0x4891d0, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
