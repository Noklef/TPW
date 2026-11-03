# A scape's objects (src/game/world/scapeobjects: Scape_LoadObjects 0x550e00) against the original: File_Open a recorder that gives a memory stream or none, its bytes the "OBJ_" tag (now and then not), the count, the size (0x3c, less or more) and
# records of the kinds 1-3 (or others), coordinates any bits, their stream closed through recorders. The sound calls and Particle_Spawn recorders (env in driver.cpp's kScapeObjectsEnv).
# usage: scapeobjects.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, memory_stream, put, recorder, set_return, strings  # noqa: E402

OPEN, PLAY, RANGE, AREA, SPAWN = 21, 22, 23, 24, 25   # (driver.cpp's kScapeObjectsEnv)
B = Bench([(0x550e00, 0x5510e9), (0x5f5e20, 0x5f5e50), (0x5f5bf0, 0x5f5c10), (0x5f5b80, 0x5f5ba8)],
          hooks={0x5f5a20: _recorder(OPEN, 4, cdecl=True), 0x51bfc0: _recorder(PLAY, 6, cdecl=True), 0x51c130: _recorder(RANGE, 7, cdecl=True), 0x51c5d0: _recorder(AREA, 5, cdecl=True), 0x521e60: _recorder(SPAWN, 4, cdecl=True)})
STREAM, DATA, TEXT = RAM, RAM + 0x1000, RAM + 0x800


def record(r, size):
    words = [r.choice([1, 2, 3, 1, 2, 3, 0, 4, r.getrandbits(32)]), r.choice([0, 0, 1, r.getrandbits(32)]), r.getrandbits(32)]
    words += [r.choice([r.getrandbits(32), r.randrange(0, 0x100000), -r.randrange(0, 0x100000) & 0xffffffff, 1023, 0xfffffc01, 1024, 0xfffffc00]) for _ in range(12)]
    body = struct.pack('<15I', *words)
    return body[:size] if size <= 0x3c else body + r.bytes(size - 0x3c)


def case(r):
    count = r.choice([0, 1, 1, 2, 3, 4, 0xffffffff])
    size = r.choice([0x3c, 0x3c, 0x3c, 0x40, 0x48, 0x3d, 0x3a, 0x38, 0x20, 4, 0])
    records = max(count if count < 100 else 0, 0)
    data = struct.pack('<3I', 0x5f4a424f if r.random() < 0.92 else r.choice([0x5f4a424e, 0x5f4a4240, r.getrandbits(32)]), count, size)
    data += b''.join(record(r, size) for _ in range(records))
    log = log_region()
    opened = r.random() < 0.93
    fill(log, OPEN, 1, struct.pack('<I', STREAM if opened else 0))
    strings(log, OPEN, 1 << 2, out=0b11)
    set_return(log, PLAY, r.getrandbits(32))
    stream = memory_stream(STREAM, DATA, len(data))
    put(stream, 0x20, '2I', recorder(0, 1), recorder(1, 0))   # (its slots 0 and 1: the closing and the deleting)
    regions = {STREAM: stream, DATA: data, TEXT: b'Scape.omp\0', LOG: log, 0x803a20: r.bytes(4), 0x803a38: r.bytes(4)}
    return Call(0x550e00, [TEXT], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
