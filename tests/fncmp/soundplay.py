# The sound play commands (src/game/core/soundplay: 0x51bfc0, 0x51c130, 0x485aa0, 0x486ae0, and the kids' sounds 0x551560, 0x551320 over
# the first) against the originals. The sound globals (on
# 0x802bc8, started 0x802bd4 each 0 or not, the manager 0x802bcc) and the category gSndCatUi 0x803a2c; the manager's vtable +8 a recorder
# logging the command as a hash of its 7 dwords (its result now and then set); the sound 0 or not, the position and range ints small,
# large (rounded as floats) or negative. StartMusic (0x51e730): the music [0x803aa0] and cat_music [0x803a44] random, the manager's engine
# +4 (Sound_SetParam's SoundManager_Call3) its slot 3 a recorder of three (its result now and then set). StopMusic, MusicLevel,
# KidsLevel, AmbientLevel (0x51e770-0x51e830): StartMusic's set-up, the engine also the stop's channel (Sound_Stop's 0x700bac command:
# its +0x24 none, its slot 6 a recorder (out, id), the out logged as 0), the music, kids' and ambient sounds 0 or not, the kids' flag
# [0x803aa8] 0 or not, the level about 0 and 100.
# usage: soundplay.py [CASES [SEED]] [--wild|--both] [--show N]
#   (env OPS=PlayAt,PlayAtRange,UiPlay,UiPlay1d,KidPlay,KidPlayGraded,SpritePlay,StartMusic,StopMusic,MusicLevel,KidsLevel,AmbientLevel)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, fb, log_region, put, set_return, strings, vtable  # noqa: E402

CALLS = {'PlayAt': 0x51bfc0, 'PlayAtRange': 0x51c130, 'UiPlay': 0x485aa0, 'UiPlay1d': 0x486ae0, 'KidPlay': 0x551560,
         'KidPlayGraded': 0x551320, 'SpritePlay': 0x4faa00, 'StartMusic': 0x51e730, 'StopMusic': 0x51e770, 'MusicLevel': 0x51e790,
         'KidsLevel': 0x51e7b0, 'AmbientLevel': 0x51e830}
B = Bench([(0x51bfc0, 0x51c048), (0x51c130, 0x51c1b8), (0x485aa0, 0x485abd), (0x486ae0, 0x486af9), (0x551320, 0x551560),
           (0x551560, 0x551600), (0x4faa00, 0x4faa5b), (0x4754e0, 0x475560), (0x51e730, 0x51e765), (0x51bc40, 0x51bc70),
           (0x6b5b80, 0x6b5b9a), (0x51e770, 0x51e88e), (0x51c300, 0x51c360), (0x6b5bd0, 0x6b5c00), (0x6b8b40, 0x6b8b70), (0x6b5930, 0x6b5960),
           (0x6b65a0, 0x6b65f0), (0x6b6830, 0x6b68e0), (0x6b6b40, 0x6b6bb0), (0x6b8930, 0x6b89a0)])
MANAGER, VT, SCRIPT, SCRIPTS, SPRITE = RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x600, RAM + 0x700
ENGINE, EVT = RAM + 0x800, RAM + 0x900


def coord(r):
    return r.choice([0, 1, -1, r.randrange(-2000, 2000), r.getrandbits(32) - (1 << 31), (1 << 24) + 1 + 2 * r.getrandbits(8)]) & 0xffffffff


def case(r):
    op = r.deal(OPS)
    log = log_region()
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    blob(log, 2, 1, 0x7f)
    if r.random() < 0.6:
        set_return(log, 2, r.getrandbits(32))
    regions = {LOG: log, 0x802bc0: g, MANAGER: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({2: 2}, 4),
               0x803a2c: r.bytes(4)}
    sound = r.choice([0, 1, 0x1d, r.getrandbits(16), r.getrandbits(32)])
    if op == 'StartMusic':
        regions[MANAGER] = struct.pack('<II', VT, ENGINE) + r.bytes(8)
        regions[ENGINE] = struct.pack('<I', EVT) + r.bytes(0x30)
        regions[EVT] = vtable({3: 3})
        if r.random() < 0.6:
            set_return(log, 3, r.getrandbits(32))
        regions[0x803aa0] = r.choice([bytes(4), r.bytes(4)])
        regions[0x803a44] = r.bytes(4)
        return Call(CALLS[op], [], regions, ret='v')
    if op in ('StopMusic', 'MusicLevel', 'KidsLevel', 'AmbientLevel'):
        regions[MANAGER] = struct.pack('<II', VT, ENGINE) + r.bytes(8)
        regions[ENGINE] = put(bytearray(struct.pack('<I', EVT) + r.bytes(0x30)), 0x24, 'I', 0)
        regions[EVT] = vtable({3: 3, 6: 2})
        strings(log, 6, 0, out=1)
        if r.random() < 0.6:
            set_return(log, 3, r.choice([0, r.getrandbits(32)]))
        if r.random() < 0.3:
            set_return(log, 2, 0)
        regions[0x803aa0] = struct.pack('<5I', *[r.choice([0, 0, 1, r.getrandbits(32)]) for _ in range(5)])
        regions[0x803a20] = r.bytes(8)
        level = r.choice([0, 0, 1, 50, 99, 100, 101, -1, -100, 0x7fffffff, -0x80000000, r.getrandbits(32) - (1 << 31)]) & 0xffffffff
        return Call(CALLS[op], [] if op == 'StopMusic' else [level], regions, ret='v')
    if op == 'UiPlay':
        return Call(CALLS[op], [sound], regions, ret='a')
    if op == 'UiPlay1d':
        return Call(CALLS[op], [], regions, ret='a')
    if op == 'SpritePlay':   # 0x4faa00: a sprite's script (its slot +0xc, the list ready) with float registers 1-3 about the ints' range and past it
        script = bytearray(r.bytes(0x100))
        for off in (0x88, 0x8c, 0x90):
            put(script, off, 'I', fb(r.choice([0.0, -0.0, 1.5, -1.5, 100.25, -2000.75, 16777217.0, 1e9, 3e9, -3e9, 1e20, float('nan'),
                                               r.uniform(-5000, 5000)])))
        g = bytearray(r.bytes(0x2c))
        put(g, 0, 'Ii', SCRIPTS, 3)
        put(g, 0x24, 'I', 1)
        sprite = bytearray(r.bytes(0x20))
        put(sprite, 0xc, 'I', r.choice([1, 2]))
        regions.update({SCRIPT: script, SCRIPTS: struct.pack('<4I', 0, SCRIPT, SCRIPT, 0), 0x7b49f0: g, SPRITE: sprite})
        return Call(0x4faa00, [SPRITE, r.getrandbits(32), sound], regions, conv='t', ret='a')
    if op in ('KidPlay', 'KidPlayGraded'):  # (the kids' category at 0x803a24; the level and the grade's parts around the thresholds)
        regions[0x803a24] = r.bytes(4)
        level = r.choice([0, 1, 2, 3, 4, 7, 8, 9, 100, -1, -5, r.getrandbits(32)]) & 0xffffffff
        args = [level] + ([r.choice([0, 1, 49, 50, 75, 149, 150, 199, 1000, -1, -50, -151, r.getrandbits(32)]) & 0xffffffff
                           for _ in range(2)] if op == 'KidPlayGraded' else []) + [coord(r), coord(r), coord(r)]
        return Call(CALLS[op], args, regions, ret='a')
    args = [r.getrandbits(32), r.getrandbits(32), sound, coord(r), coord(r), coord(r)] + ([coord(r)] if op == 'PlayAtRange' else [])
    return Call(CALLS[op], args, regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
