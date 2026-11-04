# The DCT video frames of kinds 5, 6 and 7 (src/game/gfx/madvideo: 0x6771a0) against the original. Recorders: calloc (each answer its own) and MoviePixels_FromYuv. The two fastcalls with a register
# argument, DctVlc_BeginFrame and DctVideo_DecodeMacroblock (their own benches have them), are stand-ins as driver.cpp's mad_video_stand_in: each hashes its number and arguments into HASH.
# usage: madvideo.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put  # noqa: E402

CALLOC, YUV = 10, 11
PLAYER, SETTINGS, HASH, TABLE, DATA = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x5000


def stand_in(k):
    def hook(e):  # fastcall (ecx, edx, one stack argument): as driver.cpp's mad_video_stand_in
        sp = e.r['ESP'] + (4 if e.tail else 0)
        h = e.u32(HASH)
        for value in (k, e.r['ECX'], e.r['EDX'], e.u32(sp)):
            h = (h * 33 ^ value) & 0xffffffff
        e.write(HASH, struct.pack('<I', h))
        if e.tail:
            ret = e.u32(e.r['ESP'])
            e.r['ESP'] += 4
            e.write(e.r['ESP'], struct.pack('<I', ret))
        else:
            e.r['ESP'] += 4
    return hook


B = Bench([(0x6771a0, 0x677307)], hooks={0x67fcb0: _recorder(CALLOC, 2, cdecl=True), 0x676bc0: stand_in(1), 0x676e40: stand_in(2), 0x6764bc: _recorder(YUV, 3, cdecl=True)})


def case(r):
    log = log_region()
    answers(log, CALLOC, TABLE)
    player = bytearray(r.bytes(0x670))
    put(player, 0, 'I', SETTINGS)
    settings = bytearray(r.bytes(0x100))
    put(settings, 0x60, 'I', r.deal([1, 1, 2, 3, 16, 17, 32, 33]))
    put(settings, 0x64, 'I', r.deal([0, 1, 2, 16, 17, 32, 33, 48, 320]))
    data = bytearray(r.bytes(0x20))
    data[0xd] = r.deal([0, 1, 0x7f, 0x80, 0xff, r.getrandbits(8)])
    regions = {LOG: log, PLAYER: player, SETTINGS: settings, HASH: r.bytes(4), TABLE: b''.join(struct.pack('<I', r.getrandbits(32)) for _ in range(40)), DATA: data, 0xfaac60: struct.pack('<III', r.getrandbits(32), r.deal([0, 0, 1, 2]), r.getrandbits(32))}
    return Call(0x6771a0, [PLAYER, DATA, r.getrandbits(32), r.deal([0x100, 0x280, r.getrandbits(32)]), r.deal([5, 5, 6, 6, 7, 7, 0, 4, 8])], regions, conv='f', ret='a')


if __name__ == '__main__':
    B.main(case)
