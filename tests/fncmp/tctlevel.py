# A level's terrain sets (src/game/terrain/tct: 0x427ad0 TerrainSets_LoadLevel) against the original: the file's text through the real sprintf,
# Quickload_Locate (k 4) with the text hashed, found or not on either call, the folder with or without its last backslash, TerrainSets_Load
# (k 5) and Model_AddTerrainSets (k 6), GamePaths_Base (k 3) all recorders.
# usage: tctlevel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, set_return, strings  # noqa: E402
from texdirs import merged  # noqa: E402

BASE, LOCATE, LOAD, ADD = 3, 4, 5, 6
B = Bench(merged([(0x427ad0, 0x427ba4), (0x5c5520, 0x5c5530), (0x67a400, 0x690000)]),
          hooks={0x40ef40: _recorder(BASE, 0, cdecl=True), 0x44a280: _recorder(LOCATE, 3, cdecl=True), 0x427620: _recorder(LOAD, 2, cdecl=True), 0x46e130: _recorder(ADD, 3, cdecl=True)})
BASE_TEXT, NAME, FOUND, FOLDER, ANSWERS = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xc0, RAM + 0x100


def text(r):
    return bytes(r.choice(b'abcXYZ') for _ in range(r.randrange(1, 20)))


def case(r):
    log = log_region()
    set_return(log, BASE, BASE_TEXT)
    strings(log, LOCATE, 1 << 0)
    strings(log, LOAD, 1 << 0)
    strings(log, ADD, 1 << 2)
    answers(log, LOCATE, ANSWERS)
    regions = {LOG: log, BASE_TEXT: text(r) + bytes(1), NAME: text(r) + bytes(1), FOUND: text(r) + bytes(1), FOLDER: text(r) + r.choice([b'\\', b'', b'/']) + bytes(1),
               ANSWERS: struct.pack('<2I', r.choice([0, FOUND, FOUND]), r.choice([0, FOLDER, FOLDER, FOLDER])), 0x786b68: r.bytes(8) + struct.pack('<I', NAME) + r.bytes(4)}
    return Call(0x427ad0, [r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
