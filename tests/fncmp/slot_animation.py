# The model slots' animations (src/game/model/slots: 0x429d60, 0x429e90) against the originals: a slot table (0x790398, 0xb0 each) whose slot
# holds animator.py's animator (rebased past holder.py's layout) or not. Idle (0x429e90) plays bank 5's entry - 1 (mode 0x10) or answers 1000;
# Stop (0x429d60), when the animator's holder is awake, plays bank 0xc and takes the holder out of the renderer: holder.py's Release setup (its
# holder at RAM, by the animator's rectangle +0xc0 / +0xc4 when the animator has flag 0x4000, else Holder_Sleep).
# usage: slot_animation.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Idle,Stop)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import animator  # noqa: E402
import holder  # noqa: E402
import vobj3  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x429d60, 0x429e00), (0x429e90, 0x429ef0), (0x46f150, 0x475010), (0x67a400, 0x690000), (0x45bd30, 0x45bdb0), (0x45bdc0, 0x45bee0),
           (0x576270, 0x576320), (0x576410, 0x5765d0), (0x5766e0, 0x57681a), (0x57a260, 0x57a430), (0x5d8820, 0x5d8870), (0x5da3c0, 0x5da3c1)],
          sines=256, hooks={0x691de0: vobj3.lock, 0x691ea0: lambda e: None})
SLOTS, NSLOTS = 0x790398, 4
BASE = RAM + 0x10000   # (animator.py's layout moved here: holder.py's Release uses RAM .. RAM + 0x3300)
(animator.ANIM, animator.LAYERS, animator.HOLDER, animator.MODEL, animator.NODES, animator.REST, animator.RNODES, animator.DATA, animator.TFLAGS,
 animator.OUT) = (BASE + 0x1000 * k for k in range(10))
animator.OPS = ['AnimPlay']


def slots(r, slot):
    table = bytearray(r.bytes(0xb0 * NSLOTS))
    for k in range(NSLOTS):
        put(table, 0xb0 * k + 4, 'I', animator.ANIM if k == slot and r.random() < 0.9 else 0)
    return table


def idle(r):
    c = animator.case(r)
    slot = r.randrange(NSLOTS)
    c.regions[SLOTS] = slots(r, slot)
    return Call(0x429e90, [r.choice([0, 1, 2, 3, 4, 5]), slot], c.regions, conv='c', ret='a')


def stop(r):
    c = animator.case(r)
    regions = dict(c.regions)
    rel = holder.release(r)   # (its target is the animator's stand-in: flags +4, holder +8, rectangle +0xc0 / +0xc4)
    regions.update(rel.regions)
    target = rel.regions[holder.TARGET]
    anim = regions[animator.ANIM]
    flags = struct.unpack_from('<I', anim, 4)[0] & ~0x4000 | struct.unpack_from('<I', target, 4)[0] & 0x4000
    put(anim, 4, 'I', flags)
    put(anim, 8, 'I', RAM)
    anim[0xc0:0xc8] = target[0xc0:0xc8]
    obj = regions[RAM]   # (the holder: animator.py's model at +4, nothing attached at +0x28)
    put(obj, 4, 'I', animator.MODEL)
    put(obj, 0x28, 'I', 0)
    slot = r.randrange(NSLOTS)
    regions[SLOTS] = slots(r, slot)
    return Call(0x429d60, [slot], regions, conv='c', ret='v')


CASES = {'Idle': idle, 'Stop': stop}


def case(r):
    return CASES[r.choice(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
