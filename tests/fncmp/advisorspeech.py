# The advisor's speech stopped or a message withdrawn (src/game/world/advisor: 0x5994e0 Advisor_StopSpeech, Stop; 0x59aa70 Advisor_Cancel,
# Cancel) against the originals. Stop: soundset.py's sender world for 0x51c0e0 (soundobject.py's object with voices as the manager's engine,
# the sound system; on and started mostly), which also serves Sound_VoiceFunctor3 and Sound_Forward6 (the object's own slot 6 given its own
# recorder, 9, so the voices' functor blobs stay on 6) and Sound_PlayAt (the manager's slot 2 a recorder, the command hashed as
# soundplay.py's); the advisor's block (0xf79660): speaking or not, its voice one of the object's or none, 0xf797c4 / the byte 0x78d90c
# either, its text none, the static 0xf797d0 or another (the CRT's free hooked: the heap's hash), its model slot 0-3 with no animator (the
# other slots none, or now and then a bad pointer, so a wrong slot crashes), particle pool 0-3's counts 0-3 with no systems; the rand seed
# any. Cancel: advisor.py's MayPost world (the scoring, the definitions table, the advisor's slots of id or not), the byte +0x16e4 either,
# +0x16dc id or not; the game clock paused at a time (as vlist.py's) about +0xd8 + +0xdc (wrapping now and then), +0x16ec id, another or
# 0x160; Stop's world besides.
# usage: advisorspeech.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Stop,Cancel)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, RAND_SEED, Bench, Call, _cdecl_log, _free, blob, heap, put, recorder, strings, vtable  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import advisor  # noqa: E402
import soundobject as so  # noqa: E402
import soundset  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(soundset.RANGES + advisor.RANGES + [(0x5994e0, 0x5996c9), (0x59aa70, 0x59ab4c), (0x402d80, 0x403220), (0x429d60, 0x429e00),
                                                     (0x5ed5a0, 0x5ed670), (0x51bfc0, 0x51c048)]),
          hooks={0x67a7d0: _cdecl_log, 0x67ad90: _free})
PVT, TEXT = RAM + 0x9000, RAM + 0x9100   # (the manager's vtable: slot 2 Sound_PlayAt's recorder; a freed text)


def stop_world(r):
    regions = dict(soundset.sender_case(r, 0x51c0e0).regions)
    log = regions[LOG]
    ovt = bytearray(regions[so.OVT])
    put(ovt, 4 * 6, 'I', recorder(9, 2))   # (the object's own slot 6: Sound_Forward6's command, (out, value))
    strings(log, 9, 0, out=1)
    blob(log, 2, 1, 0x7f)
    smgr = bytearray(regions[soundset.SMGR])
    put(smgr, 0, 'I', PVT)
    page = regions[so.PAGE]
    ids = [struct.unpack_from('<I', page, 0xc + so.ENTRY * i + 0x2c)[0] for i in range(6)
           if struct.unpack_from('<I', page, 0xc + so.ENTRY * i)[0] == so.VVT]
    block = bytearray(r.bytes(0x168))
    put(block, 0xc, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    put(block, 0x14, 'I', r.randrange(4))
    put(block, 0x20, 'I', r.choice(ids * 3 + [0, 0, 1, r.getrandbits(32)]) if ids else r.choice([0, 0, 1, r.getrandbits(32)]))
    put(block, 0x140, 'I', r.choice([0, 0, 0xf797d0, TEXT]))
    put(block, 0x164, 'I', r.choice([0, 1, 1, r.getrandbits(32)]))
    slots = bytearray(r.bytes(0xb0 * 4))
    bad = r.random() < 0.25
    for k in range(4):
        put(slots, 0xb0 * k + 4, 'I', 0x10 if bad and k != block[0x14] else 0)
    pools = bytearray(0x44 * 4)
    for k in range(4):
        put(pools, 0x44 * k, 'I', r.choice([0, 1, 2, 3]))
    regions.update({so.OVT: ovt, soundset.SMGR: smgr, PVT: vtable({2: 2}, 4), 0xf79660: block, 0x78d90c: bytes([r.choice([0, 1, 0x80])]),
                    0xf79bb8: r.bytes(4), 0x803a34: r.bytes(4), 0x803ac8: r.bytes(8), 0x790398: slots, 0xf8692c: pools, HEAP: heap(0x40),
                    RAND_SEED: r.bytes(4)})
    return regions


def case(r):
    op = r.deal(OPS)
    if op == 'Stop':
        return Call(0x5994e0, [], stop_world(r), ret='v')
    c = advisor.queue(r, 'MayPost')
    regions = dict(c.regions)
    regions.update(stop_world(r))
    id = c.args[1]
    adv = bytearray(regions[advisor.ADVISOR])
    adv[0x16e4] = r.choice([0, 1, 1, 0x80])
    put(adv, 0x16dc, 'I', r.choice([id, id, r.randrange(0x15f)]))
    now = r.getrandbits(32)
    start = (now - r.choice([0, 1, 100, 5000, r.getrandbits(32)])) & 0xffffffff
    length = r.choice([0, 1, 100, 5000, r.getrandbits(32), (now - start + r.choice([-1, 0, 1])) & 0xffffffff])
    put(adv, 0xd8, 'II', start, length & 0xffffffff)
    put(adv, 0x16ec, 'I', r.choice([id, id, id, 0x160, r.randrange(0x15f)]))
    regions[advisor.ADVISOR] = adv
    clock = bytearray(r.bytes(0xa0))
    put(clock, 0x60, 'III', now, 0, r.choice([1, r.getrandbits(32) | 1]))
    put(clock, 0x70, 'I', 0)
    regions[0x785970] = clock
    return Call(0x59aa70, [advisor.ADVISOR, id], regions, conv='t', ret='b')


OPS = [k for k in os.environ.get('OPS', 'Stop,Cancel').split(',') if k]

if __name__ == '__main__':
    B.main(case)
