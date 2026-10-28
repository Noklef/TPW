# A coaster car's sound mode (src/game/coaster/carsound: 0x439780) against the original, over the sound globals as rssescream.py
# sets them (on 0x802bc8 and started 0x802bd4 each 0 or not, the manager 0x802bcc, its engine's slot 3 recorder, its result now
# and then set); the car's mode +0x38 the new one now and then, the mode 0-17, -1 or any, 0x785a2c 0 or not (Mode). The car's sounds stopped
# (0x439670 CoasterCar_StopSounds, Stop) and a scream stopped (0x551300 RSSE_StopScream in src/game/rsse/scream, Scream) over soundset.py's
# senders' world for 0x51c300 (Sound_Stop: the sound globals, on and started either 0 now and then, the soundobject bench's object, ids of its
# voices or not), with its ranges: the car's +0x64 / +0x68 (Sound_Forward6's values) and +0x70 (the scream: one of the ids) each 0 now and then,
# 0x785a2c 0 or not; Scream's scream Stop's id or 0. The car's sounds started (0x4392a0 CoasterCar_StartSounds, Start) over ridesound.py's
# Play world (its ride records, their counts mostly 9 or more: elements 5-8; sound globals and manager; the manager's engine Mode's slot 3 recorder): the coaster's ride (+0x24) Play's
# id, the car's position floats small, large, past the int range or NaN, its old sounds 0 unless nothing reaches them (sound off or not
# started, or 0x785a2c set), quiet 0 mostly.
# usage: carsound.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Mode,Stop,Scream,Start)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, log_region, put, set_return, vtable  # noqa: E402

_ops = os.environ.pop('OPS', None)   # (soundset.py reads OPS as its senders' addresses)
import soundset  # noqa: E402
import ridesound  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops


B = Bench(sorted(set([(0x439780, 0x439812), (0x51bfc0, 0x51c048), (0x51bc40, 0x51bc70), (0x6b5b60, 0x6b5b75),
                      (0x6b5b80, 0x6b5b9a), (0x5da3c0, 0x5da3c1), (0x439670, 0x4396db), (0x551300, 0x551312),
                      (0x4392a0, 0x439386), (0x51eeb0, 0x51eeec), (0x55a3e0, 0x55a470), (0x55a070, 0x55a0b0)]
                     + soundset.RANGES)))
MANAGER, VT, ENGINE, EVT, CAR = RAM + 0x100, RAM + 0x200, RAM + 0x800, RAM + 0x900, RAM + 0x1000
SCAR = RAM + 0x20000   # (Stop's car: past the sound benches' regions)
COASTER = RAM + 0x21000


def start(r):
    """Start (0x4392a0) on ridesound.py's Play case."""
    ridesound.OPS = ['Play']
    c = ridesound.case(r)
    regions = dict(c.regions)
    regions[MANAGER] = struct.pack('<II', VT, ENGINE) + r.bytes(8)
    nodes, arrays = bytearray(regions[ridesound.NODES]), bytearray(regions[ridesound.ARRAYS])
    for i in range(4):   # (the elements 5-8 too: the records' counts mostly 9 or more)
        put(nodes, 0x100 * i + 0x8c, 'i', r.choice([9, 9, 9, 10, 16, 6, 8, -1]))
        for k in range(6, 9):
            put(arrays, 0x40 * i + 4 * k, 'I', r.choice([0, 1, 0x1d, r.getrandbits(32)]))
    regions.update({ridesound.NODES: nodes, ridesound.ARRAYS: arrays})
    regions.update({ENGINE: struct.pack('<I', EVT) + r.bytes(0x30), EVT: vtable({3: 3}, 8)})
    if r.random() < 0.6:
        set_return(regions[LOG], 3, r.choice([0, r.getrandbits(32)]))
    g = regions[0x802bc0]
    silenced = r.choice([0, 0, 0, 1, r.getrandbits(32)])
    unreached = not (struct.unpack_from('<I', g, 8)[0] and struct.unpack_from('<I', g, 0x14)[0]) or silenced
    car = bytearray(r.bytes(0x80))
    for at in (0x48, 0x4c, 0x50):
        put(car, at, 'f', r.choice([0.0, 1.5, -2.75, 123.25, -4000.5, 3e9, -3e9, 1e20, float('nan'), r.uniform(-1e5, 1e5)]))
    for at in range(0x54, 0x74, 4):
        put(car, at, 'I', r.getrandbits(32) if unreached and r.random() < 0.7 else 0)
    if r.random() < 0.3:
        put(car, 0x38, 'I', 2)
    regions.update({SCAR: car, COASTER: put(bytearray(r.bytes(0x100)), 0x24, 'I', c.args[0]), 0x803a3c: r.bytes(4),
                    0x785a2c: struct.pack('<I', silenced)})
    return Call(0x4392a0, [COASTER, SCAR, r.choice([0, 0, 0, 1, r.getrandbits(32)])], regions, ret='v')


def stop(r, op):
    """Stop (0x439670) / Scream (0x551300) on soundset's case for 0x51c300."""
    c = soundset.sender_case(r, 0x51c300)
    pick = c.args[0]
    if op == 'Scream':
        c.address, c.args = 0x551300, [r.choice([pick, pick, 0])]
        return c
    car = bytearray(r.bytes(0x80))
    for at in (0x64, 0x68):
        put(car, at, 'I', r.choice([0, r.getrandbits(32), r.getrandbits(32)]))
    put(car, 0x70, 'I', r.choice([0, pick, pick]))
    c.regions.update({SCAR: car, 0x785a2c: struct.pack('<I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))})
    c.address, c.args = 0x439670, [r.getrandbits(32), SCAR]
    return c


def case(r):
    op = r.deal(OPS)
    if op == 'Start':
        return start(r)
    if op != 'Mode':
        return stop(r, op)
    log = log_region()
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    if r.random() < 0.6:
        set_return(log, 3, r.choice([0, r.getrandbits(32)]))
    mode = r.choice(list(range(18)) + [1, 2, 4, 8, 16, -1, r.getrandbits(32)]) & 0xffffffff
    car = bytearray(r.bytes(0x80))
    if r.random() < 0.3:
        put(car, 0x38, 'I', mode)
    regions = {LOG: log, 0x802bc0: g, MANAGER: struct.pack('<II', VT, ENGINE) + r.bytes(8), VT: vtable({}, 4),
               ENGINE: struct.pack('<I', EVT) + r.bytes(0x30), EVT: vtable({3: 3}, 8), CAR: car,
               0x785a2c: struct.pack('<I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))}
    return Call(0x439780, [CAR, mode, r.getrandbits(32) if r.random() < 0.5 else r.randrange(200)], regions)


OPS = [k for k in os.environ.get('OPS', 'Mode,Stop,Scream,Start').split(',') if k]

if __name__ == '__main__':
    B.main(case)
