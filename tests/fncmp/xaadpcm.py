# The sound library's ADPCM decoders (src/game/core/xaadpcm: 0x672090 mono, 0x672210 stereo) against the originals. The count 0, negative,
# 1-4 blocks' worth or one sample past; the position a block multiple 0-200 (now and then any, or -28: data before the start), the data
# random bytes from 0x40 before it on, the header bytes' filters mostly 0-3 (now and then any: past the coefficient tables, sic) and
# their shifts any; the channels' last / before samples small, at the 16-bit edges or any (products wrapping); the output buffer's
# pointer through a dword. Return: mono the before pointer, stereo the right channel's before.
# usage: xaadpcm.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Mono,Stereo)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Mono': 0x672090, 'Stereo': 0x672210}
B = Bench([(0x672090, 0x672209), (0x672210, 0x672493)])
STATE, DATA, OUT, OUTP = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x100


def value(r):
    return r.choice([0, 1, -1, 100, -100, 0x7fff, -0x8000, r.randrange(-0x8000, 0x8000), r.getrandbits(32)]) & 0xffffffff


def case(r):
    op = r.deal(OPS)
    stereo = op == 'Stereo'
    size = 30 if stereo else 15
    blocks = r.choice([0, 1, 1, 2, 3, 4])
    count = r.choice([blocks * 28, blocks * 28 - 27, blocks * 28 - 1, blocks * 28 + 1 if blocks < 4 else 1, 0, -1, -0x80000000]) & 0xffffffff
    position = r.choice([28 * r.randrange(8)] * 6 + [r.randrange(200), -28]) & 0xffffffff
    data = bytearray(r.bytes(0x40 + 8 * 28 * size // 28 + 5 * size + 0x40))
    start = 0x40 + struct.unpack('<i', struct.pack('<I', position))[0] * size // 28 if position < 0x80000000 else 0x40 - size
    for b in range(5):   # (the headers: filters mostly 0-3)
        at = start + b * size
        if 0 <= at < len(data) and r.random() < 0.85:
            data[at] = r.randrange(4) << 4 | (r.randrange(4) if stereo else data[at] & 0xf)
    regions = {DATA - 0x40: data, OUT: r.bytes(0x70 * 5 + 0x10), OUTP: struct.pack('<I', OUT),
               STATE: struct.pack('<5I', position, *[value(r) for _ in range(4)])}
    if stereo:
        args = [count, STATE, STATE + 4, STATE + 8, STATE + 12, STATE + 16, DATA, OUTP]
    else:
        args = [count, STATE, STATE + 4, STATE + 8, DATA, OUTP]
    return Call(CALLS[op], args, regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
