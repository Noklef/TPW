# MovieSound_CreateStream (src/game/gfx/moviesound, 0x672a00) against the original. The device and the buffer are objects over vtables of COM recorders; the description logged with its format (the stack addresses left out), Lock's out pointers filled with two spans in memory (the second empty or not).
# Each step fails now and then; no device now and then. The player's and settings' bytes after the call compared.
# usage: moviesoundcreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, blob_bytes, fill, log_region, put, recorder, set_return, strings  # noqa: E402

B = Bench([(0x672a00, 0x672bf0)])
PLAYER, SETTINGS, DEVICE, BUFFER, DEVICE_VT, BUFFER_VT, SPANS = RAM, RAM + 0x400, RAM + 0x500, RAM + 0x510, RAM + 0x600, RAM + 0x700, RAM + 0x800
CREATE, FREQUENCY, VOLUME, CAPS, LOCK, UNLOCK, PLAY, RELEASE = range(1, 9)


def table(*slots):
    out = bytearray(0x50)
    for offset, stub in slots:
        struct.pack_into('<I', out, offset, stub)
    return out


def step(r, log, k):
    set_return(log, k, r.choice([0] * 9 + [1, 0x88780096, r.getrandbits(32) | 1]))


def case(r):
    log = log_region()
    regions = {LOG: log}
    regions[DEVICE] = struct.pack('<I', DEVICE_VT) + r.bytes(0xc)
    regions[BUFFER] = struct.pack('<I', BUFFER_VT) + r.bytes(0xc)
    regions[DEVICE_VT] = table((0xc, recorder(CREATE, 4, True)))
    regions[BUFFER_VT] = table((8, recorder(RELEASE, 1, True)), (0xc, recorder(CAPS, 2, True)), (0x2c, recorder(LOCK, 8, True)), (0x30, recorder(PLAY, 4, True)), (0x3c, recorder(VOLUME, 2, True)), (0x44, recorder(FREQUENCY, 2, True)), (0x4c, recorder(UNLOCK, 5, True)))
    settings = bytearray(r.bytes(0x100))
    put(settings, 0x54, 'I', 0 if r.random() < 0.08 else DEVICE)
    put(settings, 4, 'I', r.getrandbits(32) & ~0x2000 | r.choice([0, 0x2000]))
    regions[SETTINGS] = settings
    player = bytearray(r.bytes(0x400))
    put(player, 0, 'I', SETTINGS)
    regions[PLAYER] = player
    spans = r.bytes(0x100)
    regions[SPANS] = spans
    first, second = r.choice([0, 3, 4, 7, 0x40, 0x41]), r.choice([0, 0, 1, 4, 0x13, 0x20])
    blob_bytes(log, CREATE, 1, 0xffff, (0x10, (1 << 0x12) - 1))
    fill(log, CREATE, 2, struct.pack('<I', BUFFER))
    step(r, log, CREATE)
    for k in (FREQUENCY, VOLUME, UNLOCK, RELEASE):
        set_return(log, k, r.choice([0, 1, r.getrandbits(32)]))
    blob(log, CAPS, 1, 0x1f)
    fill(log, CAPS, 1, struct.pack('<3I', 0x14, r.getrandbits(32), r.getrandbits(32)), which=1)
    step(r, log, CAPS)
    strings(log, LOCK, 0, out=sum(1 << a for a in (3, 4, 5, 6)))
    fill(log, LOCK, 3, struct.pack('<I', SPANS), which=2)
    fill(log, LOCK, 4, struct.pack('<I', first), which=3)
    fill(log, LOCK, 5, struct.pack('<I', SPANS + 0x80), which=4)
    fill(log, LOCK, 6, struct.pack('<I', second), which=5)
    step(r, log, LOCK)
    step(r, log, PLAY)
    return Call(0x672a00, [PLAYER], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
