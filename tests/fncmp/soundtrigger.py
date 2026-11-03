# A sound's start through the engine and mixer (src/game/core/soundtrigger: 0x6b66d0, 0x6b6700, 0x6b87d0) against the original: the voice makers, the mixer's calls and the sound's and player's methods recorders.
# usage: soundtrigger.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, fill, import_table, log_region, mask_this, recorder, set_return, strings  # noqa: E402

ADD, TAKE, ONE, SEQUENCE, SWITCH, M878, M338, M2E0, M390, M3E8, CATEGORY, ADVANCE, FIND, FINDVOICE, SIGNAL, ENTER, LEAVE, KIND, STEPS, ALLOW = range(1, 21)
HOOKS = {0x6b6650: (ADD, 3), 0x6b68d0: (TAKE, 1), 0x6b9b30: (ADVANCE, 1), 0x6c5870: (FIND, 2), 0x6b6830: (FINDVOICE, 1)}
CDECL = {0x6be750: (ONE, 4), 0x6be680: (SEQUENCE, 4), 0x6be610: (SWITCH, 4), 0x6be330: (M878, 4), 0x6be090: (M338, 4), 0x6bdd30: (M2E0, 4), 0x6bdc60: (M390, 4), 0x6bd7b0: (M3E8, 4), 0x6bead0: (CATEGORY, 0)}
hooks = {at: _recorder(k, n) for at, (k, n) in HOOKS.items()}
hooks.update({at: _recorder(k, n, cdecl=True) for at, (k, n) in CDECL.items()})
B = Bench([(0x6b66d0, 0x6b66fd), (0x6b6700, 0x6b6823), (0x6b87d0, 0x6b8927)], hooks=hooks)
MIXER = RAM + 0x1000
ENGINE = RAM + 0x2000
OWNER = RAM + 0x3000
GROUP = RAM + 0x3100
DEFINITION = RAM + 0x3200
SOUND = RAM + 0x3300
SOUND_VT = RAM + 0x3400
PLAYER = RAM + 0x3500
PLAYER_VT = RAM + 0x3600
VOICE = RAM + 0x3700
PLAYING = RAM + 0x3800
CATEGORY_AT = RAM + 0x4000
SLOT = RAM + 0x4100


def definition(r):
    d = bytearray(r.randbytes(0x20))
    struct.pack_into('<H', d, 0x10, r.choice([0, 4, 6, 0x14, 0x16, 0x104, 0x106, 0x404, 0x406, 0x504, 0x506, 8, 0xc, 1, 5, r.getrandbits(16)]))
    struct.pack_into('<I', d, 0xc, r.choice([0, 1, 0x100, r.getrandbits(16), 0x80000000 | r.getrandbits(16)]))
    return d


def case(r):
    log = log_region()
    table = import_table(log, {'Signal': (SIGNAL, 0x6fd200, 2, 0, 0), 'Enter': (ENTER, 0x6fd140, 1, 0, 0), 'Leave': (LEAVE, 0x6fd130, 1, 0, 0)})
    set_return(log, SIGNAL, r.getrandbits(32))
    strings(log, TAKE, 0, out=1)
    fill(log, TAKE, 0, r.randbytes(4))
    set_return(log, TAKE, r.choice([VOICE, VOICE, VOICE, 0]))
    set_return(log, ADD, r.choice([0, 1, 1]))
    set_return(log, KIND, r.choice([1, 0, 2]))
    set_return(log, STEPS, r.getrandbits(32))
    set_return(log, ALLOW, r.choice([0, 1, 1, 1]))
    set_return(log, CATEGORY, CATEGORY_AT)
    set_return(log, FIND, r.choice([DEFINITION, DEFINITION, DEFINITION, 0]))
    set_return(log, FINDVOICE, r.choice([0, 0, PLAYING]))
    mask_this(log, FIND)
    mask_this(log, ADVANCE)
    category = bytearray(r.randbytes(0x80))
    struct.pack_into('<I', category, 0x3c + 8, SLOT)   # (the iterator's dword 2, kept by the recorder's Advance: where the owner's pointer is)
    owner = bytearray(r.randbytes(0x20))
    struct.pack_into('<I', owner, 8, r.choice([GROUP, GROUP, GROUP, 0]))
    group = bytearray(r.randbytes(0x20))
    group[8] = r.choice([0, 1, 2, 4, 8, 0xf, r.getrandbits(8)])
    player = bytearray(r.randbytes(0x50))
    struct.pack_into('<I', player, 0, PLAYER_VT)
    for i in range(4):
        player[0x44 + i] = r.choice([0, 1, 1, 1])
    engine = bytearray(r.randbytes(0x30))
    struct.pack_into('<I', engine, 0x20, r.choice([MIXER, MIXER, MIXER, MIXER, 0]))
    struct.pack_into('<I', engine, 0x24, r.choice([PLAYER, PLAYER, PLAYER, PLAYER, 0]))
    playing = bytearray(r.randbytes(0x40))
    struct.pack_into('<I', playing, 0x30, r.choice([0, 0x10, r.getrandbits(32)]))
    struct.pack_into('<H', playing, 0x22, r.choice([0, 1, 0x100, r.getrandbits(16)]))
    player_vt = bytearray(0x40)
    struct.pack_into('<I', player_vt, 0x30, recorder(ALLOW, 2))
    regions = {LOG: log, IMPORT_TABLE: table, MIXER: r.randbytes(0x40), ENGINE: engine, OWNER: owner, GROUP: group, DEFINITION: definition(r), SOUND: struct.pack('<I', SOUND_VT), SOUND_VT: struct.pack('<2I', recorder(KIND, 0), recorder(STEPS, 0)),
               PLAYER: player, PLAYER_VT: player_vt, VOICE: r.randbytes(0x20), PLAYING: playing, CATEGORY_AT: category, SLOT: struct.pack('<I', r.choice([OWNER, OWNER, OWNER, 0]))}
    op = r.choice(['signal', 'start', 'play', 'play', 'play', 'play'])
    if op == 'signal':
        return Call(0x6b66d0, [MIXER, OWNER, DEFINITION, SOUND], regions, conv='t')
    if op == 'start':
        return Call(0x6b6700, [MIXER, OWNER, DEFINITION, SOUND], regions, conv='t')
    return Call(0x6b87d0, [ENGINE, r.getrandbits(32), SOUND], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
