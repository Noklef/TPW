# A coaster car woken and put to sleep (src/game/coaster/cars: 0x43be10 CoasterCar_Wake, 0x43bf20 CoasterCar_Sleep) against the originals:
# the car's 0-5 segments (0x60 bytes; Sleep from a first one 0-3 on), each with no strip index or one of two (1, 2) whose strip object is
# holder.py's RideAcquire target (its holder in the grid environment, put back by the grid rectangle or Holder_Wake, taken out by
# Obj_RemoveFromRect or Holder_Sleep; flag 0x4000 either way) and, for Wake's Animator_Play, an animator with 0-4 layers (+0xe, +0x10), the first on
# no bank (0xc), and bank 5 not loaded (the cheap path: the layer's flags and fields set, nothing played; the holder's [+4] + 0x30 flag cleared
# unless the animator's flag 0x1000000); the segments' types' bit 0 set or not.
# The coaster's flag 4 of +0x3c set mostly. The sounds (CoasterCar_StopSounds / StartSounds) with sound off or not started and no ride
# system: their own cases are carsound.py's; 0x785a2c 0 or not.
# usage: carwake.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Wake,Sleep)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import holder  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

SOUND = [(0x4392a0, 0x439386), (0x439670, 0x4396db), (0x439780, 0x439812), (0x551300, 0x551312), (0x51c2c0, 0x51c360), (0x51eeb0, 0x51eeec),
         (0x55a3e0, 0x55a470), (0x55a070, 0x55a0b0), (0x51bfc0, 0x51c048), (0x51bc40, 0x51bc70)]
B = Bench(sorted(set(holder.RANGES + SOUND + [(0x43be10, 0x43bf1d), (0x43bf20, 0x43bfa5), (0x46f150, 0x475010)])), hooks=holder.HOOKS)
CALLS = {'Wake': 0x43be10, 'Sleep': 0x43bf20}
COASTER, CAR, SEGS, TYPES, LAYERS = RAM + 0x30000, RAM + 0x30100, RAM + 0x30200, RAM + 0x30600, RAM + 0x30700


def case(r):
    op = r.deal(OPS)
    c = holder.acquire(r, True)
    regions = dict(c.regions)
    target = bytearray(regions[holder.ACQ_TARGET])
    put(target, 0x10, 'I', LAYERS)
    target[0x54] &= ~3   # (bank 5 not loaded)
    put(target, 0xe, 'H', r.choice([0, 1, 1, 2, 4]))   # (the layers: Layer_Play counts those on a bank)
    regions[holder.ACQ_TARGET] = target
    hold = struct.unpack_from('<I', target, 8)[0]   # (Layer_Play's flag 0x80000 of [[holder + 4] + 0x30], unless the animator's flag 0x1000000)
    regions[hold] = put(bytearray(regions[hold]), 4, 'I', LAYERS + 0x100)
    regions[LAYERS + 0x100] = r.bytes(0x40)
    regions[0x7a4610] = struct.pack('<4I', r.getrandbits(32), holder.ACQ_TARGET, holder.ACQ_TARGET, r.getrandbits(32))
    layers = bytearray(r.bytes(0x38 * 4))
    for k in range(1, 4):
        put(layers, 0x38 * k + 4, 'I', r.choice([0xc, 0, 5, 0xb, 0xd, r.getrandbits(32)]))
    regions[LAYERS] = put(layers, 4, 'I', 0xc)
    regions[TYPES] = bytes([r.getrandbits(8) | 1]) + r.bytes(0xf) + bytes([r.getrandbits(8) & ~1]) + r.bytes(0xf)
    count = r.choice([0, 1, 2, 3, 3, 4, 5])
    first = r.randrange(4) if op == 'Sleep' else 0
    segs = bytearray(r.bytes(0x60 * 9))
    for k in range(9):
        put(segs, 0x60 * k + 4, 'I', TYPES + r.choice([0, 0x10, 0x10]))
        put(segs, 0x60 * k + 0xc, 'I', r.choice([0, 0, 1, 2]))
    car = bytearray(r.bytes(0x80))
    put(car, 8, 'I', first)
    put(car, 0xc, 'I', count)
    put(car, 0x28, 'I', SEGS)
    coaster = bytearray(r.bytes(0x100))
    if r.random() < 0.85:
        coaster[0x3c] |= 4
    else:
        coaster[0x3c] &= ~4
    sound = bytearray(r.bytes(0x20))   # 0x802bc0..: on (+8) or started (+0x14) 0
    put(sound, r.choice([8, 0x14]), 'I', 0)
    regions.update({COASTER: coaster, CAR: car, SEGS: segs, 0x802bc0: sound, 0x8791a0: bytes(4), 0x8791b0: bytes(4),
                    0x785a2c: struct.pack('<I', r.choice([0, 0, 1])), 0x803a3c: r.bytes(4), 0x7b4960: r.bytes(0x40)})
    return Call(CALLS[op], [COASTER, CAR], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
