# A movie's start, end and stop (src/game/gfx/moviestart: 0x51b010, 0x51b300, 0x51b350, 0x51b3e0) against the originals. The surfaces COM objects whose QueryInterface answers from a table (0 mostly) and fills in the next interface;
# Blt answering from a table, 0 mostly; the library's calls recorders; Sleep through the import table. The settings region random: Movie_Start's zeroing and fields are compared.
# usage: moviestart.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=start,finished,stop,forbid)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, import_table, log_region, set_return, strings, vtable  # noqa: E402

QI, BLT, SOUND, SOUND_ON, FLIP, OPEN, VOLUME, PLAYING, STOP = 0, 5, 6, 7, 8, 9, 10, 11, 12
IMPORTS = {'Sleep': (52, 0x6fd1d4, 1, 0, 0)}
B = Bench([(0x51b010, 0x51b2fe), (0x51b300, 0x51b34e), (0x51b350, 0x51b3c0), (0x51b3e0, 0x51b456)],
          hooks={0x51b770: _recorder(SOUND, 0, cdecl=True), 0x51c6f0: _recorder(SOUND_ON, 0, cdecl=True), 0x564210: _recorder(FLIP, 0), 0x66f4e0: _recorder(OPEN, 3, cdecl=True), 0x66f520: _recorder(VOLUME, 2, cdecl=True),
                 0x66f540: _recorder(PLAYING, 1, cdecl=True), 0x66f580: _recorder(STOP, 1, cdecl=True)})
STATE, RENDERER, VT, BACK, FRONT, IF_FRONT, IF_BACK, QI_TABLE, BLT_TABLE, PLAYER = 0x8023e8, RAM, RAM + 0x100, RAM + 0x200, RAM + 0x210, RAM + 0x220, RAM + 0x230, RAM + 0x300, RAM + 0x400, RAM + 0x500
OPS = [n for n in ('start', 'finished', 'stop', 'forbid') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def state(r, regions, allowed, on):
    s = bytearray(r.bytes(0x802bc4 - STATE))
    struct.pack_into('<I', s, 0x802bc0 - STATE, allowed)
    struct.pack_into('<I', s, 0x802bbc - STATE, on)
    struct.pack_into('<I', s, 0x802bb4 - STATE, r.choice([0, PLAYER, PLAYER]))
    struct.pack_into('<I', s, 0x802404 - STATE, r.choice([0, IF_FRONT, IF_FRONT]))
    struct.pack_into('<I', s, 0x802408 - STATE, r.choice([0, IF_BACK, IF_BACK]))
    struct.pack_into('<I', s, 0x802ba8 - STATE, r.choice([0, 1, 512, 1023, r.getrandbits(32)]))
    regions[STATE] = s


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, IMPORTS), VT: vtable({}, com={QI: 3, 2: 1, BLT: 6})}
    for at in (BACK, FRONT, IF_FRONT, IF_BACK):
        regions[at] = struct.pack('<I', VT) + r.bytes(4)
    regions[PLAYER] = r.bytes(0x10)
    set_return(log, PLAYING, r.choice([0, 1, 1, r.getrandbits(32)]))
    if op != 'start':
        state(r, regions, r.choice([1, 1, 1, 0, r.getrandbits(32)]), r.choice([1, 1, 1, 0, r.getrandbits(32)]))
        at = {'finished': 0x51b300, 'stop': 0x51b350, 'forbid': 0x51b3e0}[op]
        return Call(at, [], regions, conv='c', ret='v' if op != 'finished' else 'a')
    state(r, regions, r.choice([1] * 12 + [0]), r.choice([0] * 12 + [1]))
    regions[RENDERER] = struct.pack('<I', r.getrandbits(32)) + r.bytes(12) + struct.pack('<2I', FRONT, BACK)
    regions[0x879a24] = struct.pack('<I', RENDERER)
    regions[0x7a1930] = struct.pack('<I', r.choice([640, 800, 480, r.getrandbits(32)]))
    regions[0x7a1968] = r.bytes(4)
    regions[0x7a1a8c] = bytes([r.getrandbits(8)])
    strings(log, QI, 0, out=4)
    fill(log, QI, 2, struct.pack('<4I', BACK if r.random() < 0.97 else 0, FRONT, IF_FRONT, IF_BACK), records=4)
    regions[QI_TABLE] = struct.pack('<40I', *[r.choice([0] * 24 + [1, r.getrandbits(32)]) for _ in range(40)])
    answers(log, QI, QI_TABLE)
    regions[BLT_TABLE] = struct.pack('<40I', *[r.choice([0, 0, 0, 1, r.getrandbits(32)]) for _ in range(40)])
    answers(log, BLT, BLT_TABLE)
    blob(log, BLT, 5, (1 << 25) - 1)
    set_return(log, SOUND, r.choice([0, 0, 1, r.getrandbits(32)]))
    set_return(log, SOUND_ON, r.choice([0, 1, r.getrandbits(32)]))
    set_return(log, OPEN, r.choice([PLAYER, PLAYER, 0]))
    args = [r.choice([1, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), 0] if r.random() < 0.1 else [r.getrandbits(32) | 1]), r.getrandbits(32), r.choice([-1, r.getrandbits(31)]) & 0xffffffff, r.choice([-1, r.getrandbits(31)]) & 0xffffffff, r.choice([0, 1, r.getrandbits(32)])]
    return Call(0x51b010, args, regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
