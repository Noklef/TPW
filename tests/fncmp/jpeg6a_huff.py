# libjpeg 6a's jchuff.c, the sequential Huffman encoder (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a, the
# module through its wrapper unit third_party/wrap/jpeg-6a/jchuff.c) against the exe's 6a copies. cinfo at CINFO (6a's
# jpeg_compress_struct: err +0 at ERR, error_exit +0 and emit_message +4 cdecl recorders that return; mem +4 at MEM, alloc_small
# fncmp's ZALLOC over the case heap; dest +0x14 at DEST (next_output_byte into OUT, free_in_buffer, empty_output_buffer +0xc a cdecl
# recorder, made to suspend after a few calls in some cases); quant_tbl_ptrs[3] +0x4c, dc / ac_huff_tbl_ptrs +0x50 / +0x60 and the
# dword at +0x70 Huffman tables HT0-HT2 (or null, rarely); restart_interval +0xc0, comps_in_scan +0xec, cur_comp_info +0xf0 at
# COMPS (dc / ac_tbl_no +0x14 / +0x18), blocks_in_MCU +0x108, MCU_membership +0x10c; entropy +0x164 at ENT (huff_entropy_encoder,
# 0x6c bytes, and the dword after it: saved bit buffer and DC predictions, the restart count and number, derived tables DT0-DT3,
# counts CNT0-CNT3)). The working_state methods get STATE (0x24 bytes: the buffer, the bit buffer, last_dc_val[4], cinfo); the MCU
# methods MCUP, ten block pointers into BLOCKS. Blocks are mostly zeros, with runs past 16 and coefficients up to 15 bits.
# start_pass_huff meets table numbers out of range (-1, 4): what they index past the arrays (the dword before or after) is set to a
# table or null, so its ERREXITs, which return, go on safely; a missing table when encoding crashes in both (the derived table
# reads it). Huffman tables have at most 256 symbols (huffsize[257]); count tables at least one symbol besides the reserved 256
# (else jpeg_gen_optimal_table's search for the longest length runs below bits[0]), lengths past 16 from Fibonacci counts, never past
# 32 (JERR_HUFF_CLEN_OVERFLOW's ERREXIT returns into a write past bits[]). finish_pass_gather's tables are numbered 0..1 with a count
# table each (did_dc / did_ac are indexed by them; a count table used twice would be the merged tree's).
# usage: jpeg6a_huff.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=jinit,start,derived,optimal,dump,emit,flush,one,restart,mcu,finish,htest,gather,fgather)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, fail_after, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); then jcomapi.c's jpeg_alloc_huff_table
FUNCS = {'jpeg_make_c_derived_tbl': (0x6510e0, 0x6512ad), 'jpeg_gen_optimal_table': (0x6512ad, 0x651772),
         'jinit_huff_encoder': (0x651772, 0x6517f9), 'start_pass_huff': (0x6517f9, 0x651a21),
         'encode_mcu_huff': (0x651a21, 0x651bb5), 'encode_one_block': (0x651bb5, 0x651db5), 'emit_bits': (0x651db5, 0x651f05),
         'dump_buffer': (0x651f05, 0x651f4c), 'emit_restart': (0x651f4c, 0x652030), 'flush_bits': (0x652030, 0x652066),
         'finish_pass_huff': (0x652066, 0x6520fe), 'encode_mcu_gather': (0x6520fe, 0x652210),
         'htest_one_block': (0x652210, 0x652345), 'finish_pass_gather': (0x652345, 0x652480)}
CALLED = [(0x64f46f, 0x64f4a0)]
B = Bench(sorted(FUNCS.values()) + CALLED)

CINFO, ERR, MEM, DEST, ENT, STATE, PD, MCUP, COMPS = (
    RAM, RAM + 0x200, RAM + 0x280, RAM + 0x2c0, RAM + 0x300, RAM + 0x380, RAM + 0x3c0, RAM + 0x3d0, RAM + 0x400)
HT = [RAM + 0x600, RAM + 0x780, RAM + 0x900]
BLOCKS, OUT = RAM + 0xa80, RAM + 0xf80
DT = [RAM + 0x1200 + 0x500 * i for i in range(4)]
CNT = [RAM + 0x2600 + 0x500 * i for i in range(4)]
K_ERROR, K_EMIT, K_EMPTY = 0, 1, 2
OUTLEN = 0x240
STD_BITS = [bytes([0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0]), bytes([0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0]),
            bytes([0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d]),
            bytes([0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77])]


def rb(r, n):
    return bytearray(r.getrandbits(8 * n).to_bytes(n, 'little'))


def htbl(r):
    """A JHUFF_TBL (0x118 bytes): bits[0..16] with at most 256 symbols, huffval, sent_table."""
    t = rb(r, 0x118)
    if r.random() < 0.3:
        bits = bytearray(r.choice(STD_BITS))
    else:
        bits = bytearray(17)
        for _ in range(r.choice([1, 2, 12, r.randrange(1, 257)])):
            bits[r.randrange(1, 17)] += 1
    t[0:17] = bits
    return t


def dtbl(r):
    """A c_derived_tbl (0x500 bytes): random codes, lengths 1..16 (now and then 0, a missing code)."""
    t = rb(r, 0x500)
    zero = r.choice([0, 0, 0.03, 0.3])
    t[0x400:0x500] = bytes(0 if r.random() < zero else r.randrange(1, 17) for _ in range(256))
    return t


def block(r):
    """A JBLOCK (64 shorts) as forward_DCT leaves them: mostly zeros."""
    zero = r.choice([0.3, 0.7, 0.9, 0.97, 1.0])
    big = r.choice([0, 0.05, 0.3])
    vals = []
    for k in range(64):
        if k and r.random() < zero:
            vals.append(0)
        elif r.random() < big:
            vals.append(r.randrange(-32767, 32768))
        else:
            vals.append(r.choice([-1, 1]) * r.randrange(1, r.choice([4, 16, 1024])))
    return struct.pack('<64h', *vals)


def counts(r, fib=None):
    """257 longs of symbol counts: at least one symbol of 0..255 counted. Few symbols, as a merge searches all 257 counts twice
    (about 0.1 s of emulation each); now and then dozens, or a Fibonacci run of them for code lengths past 16."""
    c = [0] * 257
    if fib is None:
        fib = r.random() < 0.15
    syms = r.sample(range(256), r.choice([1, 2, 3, r.randrange(1, 8), r.randrange(1, 8)] + [r.randrange(8, 40)] * (r.random() < 0.05)))
    if fib:  # (lengths past 16, at most about 26)
        a, b = 1, 1
        for s in r.sample(range(256), r.randrange(18, 24)):
            c[s] = a
            a, b = b, a + b
    for s in syms:
        c[s] = r.choice([1, 2, r.randrange(1, 100), r.randrange(1, 100000), 1000000000, 1000000001])
    return struct.pack('<257I', *c)


def common(r, edges=False):
    cinfo = rb(r, 0x168)
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<I', cinfo, 0x14, DEST)
    for off in [0x4c, 0x70] + [0x50 + 4 * i for i in range(8)]:
        struct.pack_into('<I', cinfo, off, 0 if r.random() < 0.03 else r.choice(HT))
    struct.pack_into('<I', cinfo, 0xc0, r.choice([0, 0, 1, 2, r.randrange(1, 10)]))
    nscan = r.choice([1, 3, r.randrange(1, 5)])
    struct.pack_into('<i', cinfo, 0xec, nscan)
    for i in range(4):
        struct.pack_into('<I', cinfo, 0xf0 + 4 * i, COMPS + 0x54 * i)
    nblocks = r.choice([1, 6, r.randrange(1, 11)])
    struct.pack_into('<i', cinfo, 0x108, nblocks)
    struct.pack_into('<10i', cinfo, 0x10c, *[r.randrange(nscan) for _ in range(10)])
    struct.pack_into('<I', cinfo, 0x164, ENT)
    comps = rb(r, 0x54 * 4)
    for c in range(4):
        struct.pack_into('<ii', comps, 0x54 * c + 0x14,
                         *[r.choice([-1, 4]) if edges and r.random() < 0.08 else r.randrange(4) for _ in range(2)])
    err = rb(r, 0x80)
    struct.pack_into('<II', err, 0, recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True))
    log = log_region()
    if r.random() < 0.3:
        fail_after(log, K_EMPTY, r.randrange(4), 0)
    dest = rb(r, 0x14)
    off = r.randrange(0x20)
    struct.pack_into('<II', dest, 0, OUT + off, r.choice([1, 2, 3, r.randrange(1, 17), 0x200 - off]))
    struct.pack_into('<I', dest, 0xc, recorder(K_EMPTY, 1, cdecl=True))
    ent = rb(r, 0x70)
    bits = r.randrange(8)
    struct.pack_into('<Ii', ent, 0xc, r.getrandbits(32) if r.random() < 0.2 else
                     r.getrandbits(bits) << (24 - bits) if bits else 0, bits)
    struct.pack_into('<4i', ent, 0x14, *[r.randrange(-2048, 2048) for _ in range(4)])
    struct.pack_into('<Ii', ent, 0x24, r.choice([0, 0, 1, r.randrange(10)]), r.randrange(8))
    for i in range(4):
        struct.pack_into('<II', ent, 0x2c + 4 * i, r.choice(DT), r.choice(DT))
        struct.pack_into('<I', ent, 0x3c + 4 * i, r.choice(DT))
        struct.pack_into('<I', ent, 0x4c + 4 * i, r.choice(CNT))
        struct.pack_into('<I', ent, 0x5c + 4 * i, r.choice(CNT))
    struct.pack_into('<I', ent, 0x6c, r.choice(CNT))
    return {CINFO: cinfo, ERR: err, MEM: struct.pack('<I', ZALLOC) + rb(r, 0x2c), DEST: dest, ENT: ent, COMPS: comps, LOG: log,
            OUT: rb(r, OUTLEN)}


def state(r, regs):
    st = rb(r, 0x24)
    off = r.randrange(0x20)
    bits = r.randrange(8)
    struct.pack_into('<IIIi', st, 0, OUT + off, r.choice([1, 2, 3, r.randrange(1, 17), 0x200 - off]),
                     r.getrandbits(32) if r.random() < 0.2 else r.getrandbits(bits) << (24 - bits) if bits else 0, bits)
    struct.pack_into('<4iI', st, 0x10, *[r.randrange(-2048, 2048) for _ in range(4)], CINFO)
    regs[STATE] = st


def tables(r, regs):
    for t in DT:
        regs[t] = dtbl(r)


def mcu_blocks(r, regs):
    regs[MCUP] = struct.pack('<10I', *[BLOCKS + 0x80 * i for i in range(10)])
    regs[BLOCKS] = b''.join(block(r) for _ in range(10))


def jinit(r):
    regs = common(r)
    regs[HEAP] = heap(0x100)
    return Call(0x651772, [CINFO], regs)


def start(r):
    regs = common(r, edges=True)
    ent = regs[ENT]
    # (the pointers an index of -1 or 4 reaches: next_restart_num +0x28 for dc_derived_tbls[-1], the dword after for
    # ac_count_ptrs[4]; null or a table, as the others)
    for off in range(0x2c, 0x70, 4):
        if r.random() < 0.3:
            struct.pack_into('<I', ent, off, 0)
    struct.pack_into('<I', ent, 0x28, r.choice([0, DT[0]]))
    for i, t in enumerate(HT):
        regs[t] = htbl(r)
    regs[HEAP] = heap(0x5000)
    return Call(0x6517f9, [CINFO, r.choice([0, 1])], regs)


def derived(r):
    regs = common(r)
    regs[HT[0]] = htbl(r)
    regs[PD] = struct.pack('<I', r.choice([0, DT[0]]))
    regs[DT[0]] = rb(r, 0x500)
    regs[HEAP] = heap(0x600)
    return Call(0x6510e0, [CINFO, HT[0], PD], regs)


def optimal(r):
    regs = common(r)
    regs[HT[0]] = rb(r, 0x118)
    regs[CNT[0]] = counts(r)
    return Call(0x6512ad, [CINFO, HT[0], CNT[0]], regs)


def dump(r):
    regs = common(r)
    state(r, regs)
    return Call(0x651f05, [STATE], regs)


def emit(r):
    regs = common(r)
    state(r, regs)
    return Call(0x651db5, [STATE, r.getrandbits(32), r.choice([0, 1, 7, 8, 16, r.randrange(17)])], regs)


def flush(r):
    regs = common(r)
    state(r, regs)
    return Call(0x652030, [STATE], regs)


def one(r):
    regs = common(r)
    state(r, regs)
    regs[BLOCKS] = block(r)
    regs[DT[0]], regs[DT[1]] = dtbl(r), dtbl(r)
    return Call(0x651bb5, [STATE, BLOCKS, r.randrange(-2048, 2048), DT[0], DT[1]], regs)


def restart(r):
    regs = common(r)
    state(r, regs)
    return Call(0x651f4c, [STATE, r.choice([0, 7, r.randrange(8), r.randrange(256)])], regs)


def mcu(r):
    regs = common(r)
    tables(r, regs)
    mcu_blocks(r, regs)
    return Call(0x651a21, [CINFO, MCUP], regs)


def finish(r):
    regs = common(r)
    return Call(0x652066, [CINFO], regs)


def htest(r):
    regs = common(r)
    regs[BLOCKS] = block(r)
    regs[CNT[0]], regs[CNT[1]] = rb(r, 0x404), rb(r, 0x404)
    return Call(0x652210, [BLOCKS, r.randrange(-2048, 2048), CNT[0], CNT[1]], regs)


def gather(r):
    regs = common(r)
    mcu_blocks(r, regs)
    for t in CNT:
        regs[t] = rb(r, 0x404)
    return Call(0x6520fe, [CINFO, MCUP], regs)


def fgather(r):
    regs = common(r)
    cinfo, comps, ent = regs[CINFO], regs[COMPS], regs[ENT]
    for c in range(4):
        struct.pack_into('<ii', comps, 0x54 * c + 0x14, r.randrange(2), r.randrange(2))
    for i in range(2):
        struct.pack_into('<I', ent, 0x4c + 4 * i, CNT[i])
        struct.pack_into('<I', ent, 0x5c + 4 * i, CNT[2 + i])
        struct.pack_into('<II', cinfo, 0x50 + 0x10 * i, *[r.choice([0, HT[0], HT[1]]) for _ in range(2)])
    for t in HT[:2]:
        regs[t] = rb(r, 0x118)
    for t in CNT:
        regs[t] = counts(r, fib=r.random() < 0.1)
    regs[HEAP] = heap(0x600)
    return Call(0x652345, [CINFO], regs)


OPS = [f for f in (jinit, start, derived, optimal, dump, emit, flush, one, restart, mcu, finish, htest, gather, fgather)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


# (the two that build optimal tables dealt half as often: they take ten to thirty times as long as the rest)
DECK = OPS + [f for f in OPS if f not in (optimal, fgather)]


def case(r):
    return r.deal(DECK)(r)


if __name__ == '__main__':
    B.main(case)
