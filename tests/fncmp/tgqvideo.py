# The video library's flat macroblock fills (src/game/gfx/tgqvideo: 0x671340 at 16 bits, 0x671650 at 32) against the originals. The
# scale [0xfb7820] 1.0, 0.5, any or negative; the luma and chroma tables (0xfb7930, 0xfb7b30, 0xfb7d30) random; the six values random
# bytes in dwords whose upper bytes are any; the pitch the squares' width, wider, or odd (the rows then (pitch >> 1) * 2 or
# (pitch >> 2) * 4 apart while the lower squares start at pitch * 8, sic); the output buffer random around the 16 rows.
# Pack (0x66fd70): the channels 0-255, around 0, the table builder's ranges or any; the bits the usual formats (5 6 5, 5 5 5, 8 8 8,
# 4 4 4) or each 0-9 now and then 31 / 32 (shift counts past 31: the CPU's mod 32); the shifts theirs or 0-35. Tables (0x66fc10): the
# same formats and shifts, the three tables random before.
# Quality: 0-100 mostly (else any: the products wrapping), the dequantiser 0xfb7820 random before.
# Row16 (0x673574, luma in ESI, out in EDI, chroma in EBX: conv 'esiediebx'): the tables, the 16 luma and 2 x 8 chroma dwords random,
# out random (its dwords 0 and 8 read).
# Coefficients (0x672e60, ret 'edi': the counter it leaves in EDI): a bit stream written for the jump table (the exe's, or two thirds of
# the time its 8 entries drawn from the 7 code addresses, so that 0x672ecc, 0x672f36 and the escape's 0xfc form come up too: those two
# reached only through the table are not in the listing, their instructions are added from the exe's bytes below): symbols
# drawn while the counter stays in the zigzag table (runs of 1 to counter / 2 pairs), then random bytes; a few cases with the exe's table
# use a code 5-7 (a jump to its non-code dwords: crash in both). The coefficients and the dequantiser random (small or any).
# Frame16 / Frame32 (0x671230 / 0x671540, fastcall (player, data; out, unused, pitch)): a frame of up to 3 x 3 macroblocks (sizes not
# always multiples of 16, sometimes 0), types mostly 3, 6 and 0xc, else 0-0xc (skipped); in a few cases one coded one (above 0xc), whose
# 0x673fee / 0x67380f faults in both (they cannot run to the end: see tgqvideo.h). Pitches odd now and then.
# Lanes (0x670090): the four values -1, 0 or 1 mostly (the borrows' edges), else -0x100-0x100 or any.
# LaneTables (0x66ffa0, fastcall (alpha)): the alpha -1, 0, 1, 0x7f, 0x80 or any; the three tables random before.
# BlockBase (0x66fb60): any value and state. SetupTables (0x66fbd0): a player whose format's kind +0x48 is 16, 15, 32, 24 or any, its
# bits and shifts Tables'.
# usage: tgqvideo.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Fill16,Fill32,Pack,Tables,Quality,Row16,Row32,Coefficients,Frame16,Frame32,
# Lanes,LaneTables)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Fill16': 0x671340, 'Fill32': 0x671650, 'Pack': 0x66fd70, 'Tables': 0x66fc10, 'Quality': 0x671000, 'Row16': 0x673574, 'Row32': 0x6732e9, 'Coefficients': 0x672e60, 'Frame16': 0x671230,
         'Frame32': 0x671540, 'Lanes': 0x670090, 'LaneTables': 0x66ffa0,
         'BlockBase': 0x66fb60, 'SetupTables': 0x66fbd0}
B = Bench([(0x671340, 0x671532), (0x671650, 0x6717fc), (0x66fd70, 0x66ff96), (0x66fc10, 0x66fd69), (0x671000, 0x6710b9), (0x673574, 0x67380f), (0x672e60, 0x673000), (0x6732e9, 0x673574),
           (0x671230, 0x671340), (0x671540, 0x671650), (0x67380f, 0x6747cd), (0x67673c, 0x676965), (0x670090, 0x6700d0), (0x66ffa0, 0x670090),
           (0x66fb60, 0x66fb81), (0x66fbd0, 0x66fc0b)])
# 0x672e60's jump-table targets 0x672ecc and 0x672f36, missing from the listing (no fall-through or jump reaches them): from the exe's bytes
B.emu.code.update({0x672ecc: ('MOV', 'EAX,dword ptr [EBX*0x4 + 0xfb7820]'), 0x672ed3: ('NEG', 'EAX'),
                   0x672ed5: ('MOV', 'dword ptr [EBX*0x4 + 0xfb7600],EAX'), 0x672edc: ('JMP', '0x00672f09'),
                   0x672f36: ('MOV', 'dword ptr [EBX*0x4 + 0xfb7600],0x0'), 0x672f41: ('MOV', 'EBX,dword ptr [EDI*0x4 + 0xfbb5a8]'),
                   0x672f48: ('DEC', 'EDI')})
B.emu.order = sorted(B.emu.code)
B.emu.next = {a: b for a, b in zip(B.emu.order, B.emu.order[1:])}
FORMATS = [(5, 6, 5, 11, 5, 0), (5, 5, 5, 10, 5, 0), (8, 8, 8, 16, 8, 0), (4, 4, 4, 8, 4, 0), (5, 6, 5, 0, 5, 11)]


def pixel_format(r):
    if r.random() < 0.7:
        return list(r.choice(FORMATS))
    return [r.choice(list(range(10)) + [31, 32]) for _ in range(3)] + [r.randrange(36) for _ in range(3)]


def channel(r):
    return r.choice([r.randrange(256), r.randrange(-8, 9), r.randrange(-300, 300), 0, 255, -0x80000000, r.getrandbits(32)]) & 0xffffffff
OUT = RAM
ZERO, DQ, NEG, TWO, ZERO_RUN, RUN, ESCAPE = 0x672ede, 0x672ebc, 0x672ecc, 0x672eeb, 0x672f36, 0x672f49, 0x672f7a
EXE_TABLE = [ZERO, RUN, DQ, ESCAPE, TWO]  # (then three dwords that are not code)


def coefficient_stream(r, table):
    """The bits (least significant first, as bytes) of a block's symbols for the jump table (8 targets, None for the non-code ones)."""
    bits = []

    def put(value, n):
        bits.extend((value >> i) & 1 for i in range(n))
    put(r.getrandbits(8), 8)  # (the DC)
    at = 0x3f
    while at > 0:
        codes = [k for k in range(8) if table[k] is not None and (table[k] != ZERO_RUN or at >= 3) and (table[k] != RUN or at >= 2)]
        k = r.choice(codes)
        target = table[k]
        put(k, 3)
        if target == RUN or target == ZERO_RUN:
            room = at if target == RUN else at - 1
            pairs = r.randrange(1, min(31, room // 2) + 1) if room >= 2 else 1
            put(pairs, 5)
            at = room - 2 * pairs
        elif target == ESCAPE:
            value = 31 if k >= 4 and r.random() < 0.3 else r.getrandbits(5)
            put(value, 5)
            if k >= 4 and value == 31:
                put(r.getrandbits(8), 8)
            at -= 1
        else:
            at -= 2 if target == TWO else 1
    put(r.getrandbits(24), 24)
    return bytes(sum(bits[i + j] << j for j in range(8)) for i in range(0, len(bits) - 7, 8))


def coefficients(r):
    if r.random() < 1 / 3:
        table, regions = EXE_TABLE + [None] * 3, {}
    else:
        table = [r.choice([ZERO, DQ, NEG, TWO, ZERO_RUN, RUN, ESCAPE]) for _ in range(8)]
        table[r.randrange(8)] = r.choice([ZERO, DQ, NEG, ESCAPE])  # (one that steps by 1, for the last place)
        regions = {0xfbbaac: struct.pack('<8I', *table)}
    stream = coefficient_stream(r, table)
    if not regions and r.random() < 0.1:  # a code 5-7 somewhere in the stream's first bytes
        stream = bytes([stream[0], stream[1] | 5]) + stream[2:]
    src = RAM + 0x1000
    dq = struct.pack('<64i', *[r.choice([r.randrange(-0x40000, 0x40000), r.getrandbits(32) - (1 << 31)]) for _ in range(64)])
    regions.update({0xfb7600: r.bytes(0x100), 0xfb7820: dq + struct.pack('<III', src + 1, 8, stream[0]), src: stream})  # (source, count, bits)
    return Call(CALLS['Coefficients'], [], regions, ret='edi')


def frame(r, wide):
    player, settings, data = RAM + 0x10000, RAM + 0x10100, RAM + 0x11000
    height, width = [r.choice([0, 16, 32, 48, r.randrange(1, 49)]) for _ in range(2)]
    across, down = (width + 15) // 16, (height + 15) // 16
    pitch = across * (0x40 if wide else 0x20) + r.choice([0, 0, 2, 0x20, 1, 3])
    coded = r.randrange(across * down) if across * down and r.random() < 0.04 else -1
    stream = bytearray()
    for k in range(across * down):
        kind = r.randrange(0xd, 0x100) if k == coded else r.choice([3, 6, 0xc] * 3 + list(range(13)))
        # (a coded one's first block decodes with the exe's table, to the column write that faults)
        stream += bytes([kind]) + (coefficient_stream(r, EXE_TABLE + [None] * 3) if k == coded else r.bytes(kind))
    regions = {player: struct.pack('<I', settings), settings: bytes(0x60) + struct.pack('<II', height, width), data: stream + r.bytes(16),
               0xfb7820: struct.pack('<I', r.choice([0x10000, 0x8000, 0x18000, r.getrandbits(20), r.getrandbits(32)])) + r.bytes(0xfc),
               0xfb7600: r.bytes(0x100), 0xfb7920: r.bytes(12), 0xfb7930: r.bytes(0x600), OUT: r.bytes(down * 16 * pitch + 0x40)}
    return Call(CALLS['Frame32' if wide else 'Frame16'], [player, data, OUT, r.getrandbits(32), pitch], regions, conv='f', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Lanes':
        return Call(CALLS[op], [(r.choice([-1, 0, 0, 1, r.randrange(-0x100, 0x101)]) if r.random() < 0.9 else r.getrandbits(32)) & 0xffffffff
                                    for _ in range(4)], {},
                    conv='f', ret='a')
    if op == 'Coefficients':
        return coefficients(r)
    if op in ('Frame16', 'Frame32'):
        return frame(r, op == 'Frame32')
    if op == 'Quality':
        quality = r.randrange(101) if r.random() < 0.85 else r.choice([-1, 101, 200, r.getrandbits(32)])
        return Call(CALLS[op], [quality], {0xfb7820: r.bytes(0x100)}, ret='v')
    if op in ('Row16', 'Row32'):
        luma, chroma = RAM + 0x1000, RAM + 0x2000
        regions = {0xfb7930: r.bytes(0x600), luma: r.bytes(0x40), chroma: r.bytes(0x120), OUT: r.bytes(0x24 if op == 'Row16' else 0x44)}
        return Call(CALLS[op], [luma, OUT, chroma], regions, conv='esiediebx', ret='v')
    if op == 'Pack':
        return Call(CALLS[op], [channel(r), channel(r), channel(r), r.getrandbits(32)] + pixel_format(r), {}, conv='f', ret='a')
    if op == 'LaneTables':
        return Call(CALLS[op], [r.choice([-1, 0, 1, 0x7f, 0x80, r.getrandbits(32)]) & 0xffffffff], {0xfb7930: r.bytes(0x600)}, conv='f',
                    ret='v')
    if op == 'BlockBase':
        return Call(CALLS[op], [r.getrandbits(32), RAM + 0x1000], {RAM + 0x1000: r.bytes(0x620)}, conv='f', ret='a')
    if op == 'SetupTables':   # (the format's kind 16 / 15 bits mostly, else 32, 24 or any; its bits and shifts pixel_format()'s)
        fmt = bytearray(r.bytes(0x50))
        struct.pack_into('<6I', fmt, 0x2c, *pixel_format(r))
        struct.pack_into('<I', fmt, 0x48, r.choice([0x10, 0x10, 0xf, 0xf, 0x20, 0x18, r.getrandbits(32)]))
        regions = {RAM + 0x1000: struct.pack('<I', RAM + 0x1100), RAM + 0x1100: bytes(fmt), 0xfb7930: r.bytes(0x600)}
        return Call(CALLS[op], [RAM + 0x1000], regions, conv='t', ret='v')
    if op == 'Tables':
        return Call(CALLS[op], [r.getrandbits(32)] + pixel_format(r), {0xfb7930: r.bytes(0x600)}, conv='f', ret='v')
    wide = op == 'Fill32'
    pitch = r.choice([0x40, 0x80, 0x42, 0x43, 0x41] if wide else [0x20, 0x40, 0x22, 0x21, 0x23])
    regions = {0xfb7820: struct.pack('<I', r.choice([0x10000, 0x8000, 0x18000, r.getrandbits(20), r.getrandbits(32)])),
               0xfb7930: r.bytes(0x600), OUT: r.bytes(pitch * 16 + 0x40)}
    values = [r.getrandbits(8) | r.choice([0, 0, r.getrandbits(24)]) << 8 for _ in range(6)]
    return Call(CALLS[op], values + [OUT, pitch], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
