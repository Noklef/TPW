# The map saved and loaded cell by cell (src/game/world/mapcell: 0x4d7ea0 Map_Serialize) against the original. The 0x4000 cells' calls cost too
# much for real worlds, so the cell resets (0x536490, 0x53af00) and serializers (0x4d8dd0, 0x50c200, 0x502490) are stand-ins (driver.cpp's
# kMapSerializeEnv) mixing what they are given into a checksum at PROBE: the resets' id words, a map temp's +0x26 and models (not their stack
# pointers), the serializers' this and arguments; a serializer answers 1 for the cell at PROBE + 4. Those functions have their own benches.
# The stream is real: a memory stream of the status bytes, short by a random number or full. Save: the +0x26 and RE cells of the first cells
# (equal to an empty one or not), which the status bytes show; ended by the stream or a serializer within them. Load: the status bytes any;
# Full: all 0x4000 cells, the block values at +0x1b0000 cleared after a load (rarely: some seconds each). Versions other than 0, 1 do nothing.
# usage: mapserialize.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Save,Load,SaveFull,LoadFull)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import RAM, SEEK, STREAM_SIZE, STUBS, Bench, Call  # noqa: E402

PROBE, STREAM, SINK, MAP = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x100000
TRACK, RE, BLOCKS = MAP + 0x110000, MAP + 0x1b1104, MAP + 0x1b0000
CELLS = 0x4000


def mix(e, *values):
    s = e.u32(PROBE)
    for v in values:
        s = (s * 31 + v) & 0xffffffff
    e.write(PROBE, struct.pack('<I', s))


def pop(e, n):
    if e.tail:   # the return address goes up over the arguments the callee would have popped
        ret = e.u32(e.r['ESP'])
        e.r['ESP'] += n
        e.write(e.r['ESP'], struct.pack('<I', ret))
    else:
        e.r['ESP'] += n


def sp(e):
    return e.r['ESP'] + (4 if e.tail else 0)


def map_reset(e):
    cell = e.r['ECX']
    mix(e, e.u32(cell) & 0xffff | e.read(cell + 0x26, 1)[0] << 16 | (e.u32(sp(e)) & 0xff) << 24)
    pop(e, 4)


def track_reset(e):
    mix(e, e.u32(e.r['ECX']) & 0xffff)


def serialize(e):
    at = sp(e)
    mix(e, e.r['ECX'], *[e.u32(at + 4 * i) for i in range(4)])
    e.r['EAX'] = 1 if e.r['ECX'] == e.u32(PROBE + 4) else 0
    pop(e, 16)


B = Bench([(0x4d7ea0, 0x4d82f7), (0x4d8da0, 0x4d8dd0), (0x50c1d0, 0x50c1f8), (0x502460, 0x502490), (0x5f5bb0, 0x5f5bc0), (0x5f5e20, 0x5f5e80),
           (0x416e10, 0x416e20), (0x5da3c0, 0x5da3c1)],
          hooks={0x536490: map_reset, 0x53af00: track_reset, 0x4d8dd0: serialize, 0x50c200: serialize, 0x502490: serialize})
OPS = [k for k in os.environ.get('OPS', 'Save,Load,SaveFull,LoadFull').split(',') if k]
DECK = [k for k in OPS if not k.endswith('Full')] * 75 + [k for k in OPS if k.endswith('Full')] or OPS


def stream(size):
    """A memory stream over size bytes at SINK whose slot 6 (Stream_Tell's) answers its size."""
    return struct.pack('<8I8I', STREAM + 0x20, SINK, size, SINK, size, 0, 0, 0, 0, 0, STUBS, STUBS + 0x10, SEEK, 0, STREAM_SIZE, 0)


def case(r):
    op = r.deal(DECK)
    full, save = op.endswith('Full'), op.startswith('Save')
    count = CELLS if full else r.randrange(1, 40)
    size = CELLS + 8 if full else r.choice([count, count + 1, r.randrange(0, count + 1)])
    version = r.choice([0, 1] if full else [0, 1, 0, 1, 0, 1, 0, 1, 2, -1])
    target = r.choice([0, TRACK, RE, MAP])
    fail = 0
    if not full and target:  # (the cell a serializer stops at)
        fail = target + (r.randrange(count) * {TRACK: 0x28, RE: 10, MAP: 0x44}[target])
    regions = {PROBE: struct.pack('<2I', r.getrandbits(32), fail), STREAM: stream(size)}
    if save:
        arg = r.choice([1, 1, 0x100, r.getrandbits(32) | 1])
        regions[SINK] = bytes(max(size, 1))
        regions[MAP] = r.bytes(0x44 * (count + 2))
        cells = bytearray(10 * (count + 2))
        for i in range(0, len(cells), 10):
            if r.random() < 0.5:
                cells[i + r.randrange(10)] = r.randrange(1, 0x100)
        regions[RE] = cells
    else:
        arg = 0
        regions[SINK] = r.bytes(max(size, 1)) if r.random() < 0.8 else bytes(r.choice([0, 1, 2, 4, 3, 5, 7, 0xf8, 0xff]) for _ in range(max(size, 1)))
        regions[BLOCKS] = r.bytes(0x441 * 4 + 8)
    return Call(0x4d7ea0, [MAP, STREAM, arg, version, r.getrandbits(32)], regions, conv='t', ret='a', steps=30_000_000)


if __name__ == '__main__':
    B.main(case)
