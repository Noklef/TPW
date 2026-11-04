# A sign style's two font files (src/game/files/signfonts: 0x5ec680) against the original: each extracted and added to Windows, its path put in the font vector when new. VFS_ExtractFile and
# AddFontResource recorders answering yes or no, the texts compared; the vector empty, full, with room, holding one of the paths or not.
# usage: signfonts.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, heap, log_region, put, recorder, strings  # noqa: E402

ADD, EXTRACT = 1, 8
B = Bench([(0x5ec680, 0x5ecb40), (0x5ed130, 0x5ed580), (0x619240, 0x619250), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)], hooks={0x5c5010: _recorder(EXTRACT, 2, cdecl=True)})
VECTOR, SOURCE, SEPARATOR, FOLDER, ROOT = 0x789c18, 0xf86558, 0xf85a08, 0xf86750, 0xf816e8
OBJ, TEXTS, ELEMS, DATA, ANSWER_ADD, ANSWER_EXTRACT = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x4100


def case(r):
    log = log_region()
    table = bytearray(0x600)
    put(table, 0x6fd030 - IMPORT_TABLE, 'I', recorder(ADD, 1, True))
    strings(log, ADD, 1)
    strings(log, EXTRACT, 3)
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000)}
    source, separator, folder, root = b'src', r.choice([b'\\', b'/']), b'fonts', b'C:\\tp'
    for i, (address, text) in enumerate(((SOURCE, source), (SEPARATOR, separator), (FOLDER, folder), (ROOT, root))):
        gamestring.string(r, 0, address, TEXTS + 0x40 * i, regions, text)
    obj = bytearray(r.bytes(0x400))
    first, second = r.choice([b'a.ttf', b'bb.ttf', b'']), r.choice([b'a.ttf', b'bb.ttf', b'c.ttf'])
    obj[0xc4:0xc4 + len(first) + 1] = first + bytes(1)
    obj[0x254:0x254 + len(second) + 1] = second + bytes(1)
    regions[OBJ] = obj
    size = r.choice([0, 0, 1, 2, 3])
    spare = r.choice([0, 1, 2])
    elems = bytearray(r.bytes(0x14 * (size + spare + 1)))
    for i in range(size):
        name = r.choice([first, second, b'zz.ttf'])
        path = root + separator + folder + separator + name
        base = RAM + 0x5000
        gamestring.string(r, 0, base, DATA + 0x40 * i, regions, path)
        elems[0x14 * i:0x14 * (i + 1)] = regions.pop(base)[:0x14]
    regions[ELEMS] = elems
    start = 0 if size == 0 and r.random() < 0.5 else ELEMS
    vector = bytearray(r.bytes(0x10))
    struct.pack_into('<3I', vector, 4, start, start + 0x14 * size, start + 0x14 * (size + spare) if start else 0)
    regions[VECTOR] = vector
    answers(log, ADD, ANSWER_ADD)
    answers(log, EXTRACT, ANSWER_EXTRACT)
    regions[ANSWER_ADD] = struct.pack('<2I', r.choice([1, 1, 1, 0]), r.choice([1, 1, 1, 0])) + bytes(0xf8)
    regions[ANSWER_EXTRACT] = struct.pack('<2I', r.choice([1, 1, 1, 0]), r.choice([1, 1, 1, 0])) + bytes(0xf8)
    return Call(0x5ec680, [OBJ, r.getrandbits(32)], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
