# libjpeg 6a's jcphuff.c, the progressive Huffman encoder (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a, the
# module through its wrapper unit third_party/wrap/jpeg-6a/jcphuff.c) against the exe's 6a copies. cinfo at CINFO (6a's
# jpeg_compress_struct: err +0 at ERR, error_exit +0 and emit_message +4 cdecl recorders that return; mem +4 at MEM, alloc_small
# fncmp's ZALLOC over the case heap; dest +0x14 at DEST (next_output_byte into OUT, free_in_buffer, empty_output_buffer +0xc a cdecl
# recorder, made to fail after a few calls in some cases); dc / ac_huff_tbl_ptrs +0x50 / +0x60 (and the dwords at +0x4c and +0x70
# that table numbers -1 and 4 reach) Huffman tables HT0-HT2 or null; restart_interval +0xc0, comps_in_scan +0xec, cur_comp_info
# +0xf0 at COMPS (dc / ac_tbl_no +0x14 / +0x18), blocks_in_MCU +0x108, MCU_membership +0x10c, Ss / Se / Ah / Al +0x134..+0x140;
# entropy +0x164 at ENT (phuff_entropy_encoder, 0x6c bytes, and the dword after it: gather_statistics, the encoder's buffer, the
# bit buffer, last_dc_val[4], ac_tbl_no, EOBRUN, BE, bit_buffer at BITBUF (MAX_CORR_BITS 1000 bytes), the restart count and number,
# derived tables DT0-DT1, counts CNT0-CNT1)). The MCU methods get MCUP, ten block pointers into BLOCKS.
# The edges the source never meets are kept out (its ERREXITs return here): Ss <= Se <= 63 (jpeg_natural_order and the 64
# absvalues), Al <= 13, table numbers 0..3 but in start_pass_phuff (which meets -1 and 4: what they reach is a table or null, so
# the ERREXIT goes on safely; a missing table when encoding crashes in both, the derived table reads it), EOBRUN under 0x7fff, BE
# at most 937 before an AC refinement (its MCU appends up to 63 bits to the 1000-byte buffer), emit_bits sizes up to 16.
# finish_pass_gather_phuff's tables are numbered 0..1 (did[] is indexed by them), each with its own count table, counted as
# jpeg6a_huff.py's counts() makes them (jpeg_gen_optimal_table's limits).
# usage: jpeg6a_phuff.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=jinit,start,dump,emit,flush,symbol,buffered,eobrun,restart,dcfirst,acfirst,dcrefine,acrefine,finish,fgather)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, fail_after, heap, log_region, recorder  # noqa: E402
from jpeg6a_huff import block, counts, dtbl, htbl, rb  # noqa: E402

# the exe's jcphuff.c (0x652480..0x65360e, re/libmap.csv); jchuff.c's jpeg_make_c_derived_tbl and jpeg_gen_optimal_table, jcomapi.c's
# jpeg_alloc_huff_table
B = Bench([(0x64f46f, 0x64f4a0), (0x6510e0, 0x651772), (0x652480, 0x65360e)])

CINFO, ERR, MEM, DEST, ENT, MCUP, COMPS = RAM, RAM + 0x200, RAM + 0x280, RAM + 0x2c0, RAM + 0x300, RAM + 0x3d0, RAM + 0x400
HT = [RAM + 0x600, RAM + 0x780, RAM + 0x900]
BLOCKS, OUT, BITBUF = RAM + 0xa80, RAM + 0xf80, RAM + 0x1200
DT = [RAM + 0x1600 + 0x500 * i for i in range(2)]
CNT = [RAM + 0x2000 + 0x500 * i for i in range(2)]
K_ERROR, K_EMIT, K_EMPTY = 0, 1, 2
OUTLEN = 0x240


def common(r, edges=False, tables=4):
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
    ss = r.choice([0, 1, r.randrange(64)])
    struct.pack_into('<4i', cinfo, 0x134, ss, r.choice([ss, 63, r.randrange(ss, 64)]), r.choice([0, 0, 1, r.randrange(14)]),
                     r.choice([0, 0, 1, r.randrange(14)]))
    struct.pack_into('<I', cinfo, 0x164, ENT)
    comps = rb(r, 0x54 * 4)
    for c in range(4):
        struct.pack_into('<ii', comps, 0x54 * c + 0x14,
                         *[r.choice([-1, 4]) if edges and r.random() < 0.08 else r.randrange(tables) for _ in range(2)])
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
    off = r.randrange(0x20)
    bits = r.randrange(8)
    struct.pack_into('<IIIIiI', ent, 0xc, r.choice([0, 1]), OUT + off, r.choice([1, 2, 3, r.randrange(1, 17), 0x200 - off]),
                     r.getrandbits(32) if r.random() < 0.2 else r.getrandbits(bits) << (24 - bits) if bits else 0, bits, CINFO)
    struct.pack_into('<4i', ent, 0x24, *[r.randrange(-2048, 2048) if r.random() < 0.9 else r.randrange(-32768, 32768)
                                         for _ in range(4)])
    struct.pack_into('<iIII', ent, 0x34, r.randrange(tables), r.choice([0, 0, 1, r.randrange(1, 40), 0x7ffe, r.randrange(0x7fff)]),
                     r.choice([0, 0, r.randrange(8), r.randrange(938), 937]), BITBUF)
    struct.pack_into('<Ii', ent, 0x44, r.choice([0, 0, 1, r.randrange(10)]), r.randrange(8))
    for i in range(4):
        struct.pack_into('<I', ent, 0x4c + 4 * i, r.choice(DT))
        struct.pack_into('<I', ent, 0x5c + 4 * i, r.choice(CNT))
    struct.pack_into('<I', ent, 0x6c, r.choice(CNT))
    regs = {CINFO: cinfo, ERR: err, MEM: struct.pack('<I', ZALLOC) + rb(r, 0x2c), DEST: dest, ENT: ent, COMPS: comps, LOG: log,
            OUT: rb(r, OUTLEN), BITBUF: rb(r, 1000)}
    for t in DT:
        regs[t] = dtbl(r)
    for t in CNT:
        regs[t] = rb(r, 0x404)
    return regs


def gathering(regs, on):
    struct.pack_into('<I', regs[ENT], 0xc, on)


def mcu_blocks(r, regs):
    regs[MCUP] = struct.pack('<10I', *[BLOCKS + 0x80 * i for i in range(10)])
    regs[BLOCKS] = b''.join(block(r) for _ in range(10))


def jinit(r):
    regs = common(r)
    regs[HEAP] = heap(0x100)
    return Call(0x652480, [CINFO], regs)


def start(r):
    regs = common(r, edges=True)
    ent = regs[ENT]
    # (the pointers a table number of -1 or 4 reaches: derived_tbls[-1] is next_restart_num +0x48, count_ptrs[-1] derived_tbls[3],
    # count_ptrs[4] the dword after; null or a table, as the others; and the correction bit buffer null now and then)
    for off in range(0x4c, 0x70, 4):
        if r.random() < 0.3:
            struct.pack_into('<I', ent, off, 0)
    struct.pack_into('<I', ent, 0x48, r.choice([0, DT[0]]))
    if r.random() < 0.5:
        struct.pack_into('<I', ent, 0x40, 0)
    for t in HT:
        regs[t] = htbl(r)
    regs[HEAP] = heap(0x2000)
    return Call(0x6524f5, [CINFO, r.choice([0, 1])], regs, ret='v')


def dump(r):
    regs = common(r)
    return Call(0x652aa5, [ENT], regs)


def emit(r):
    regs = common(r)
    return Call(0x652957, [ENT, r.getrandbits(32), r.choice([0, 1, 7, 8, 16, r.randrange(17)])], regs)


def flush(r):
    regs = common(r)
    return Call(0x652c74, [ENT], regs)


def symbol(r):
    regs = common(r)
    return Call(0x652b07, [ENT, r.randrange(4), r.choice([0, 0xf0, r.randrange(256)])], regs)


def buffered(r):
    regs = common(r)
    return Call(0x652d42, [ENT, BITBUF + r.randrange(0x100), r.choice([0, 1, r.randrange(64), r.randrange(700)])], regs)


def eobrun(r):
    regs = common(r)
    return Call(0x652c9d, [ENT], regs)


def restart(r):
    regs = common(r)
    return Call(0x652b6f, [ENT, r.choice([0, 7, r.randrange(8), r.randrange(256)])], regs)


def mcu(addr):
    def op(r):
        regs = common(r)
        mcu_blocks(r, regs)
        if addr == 0x6530f4 and r.random() < 0.2:
            # (BE at its limit and blocks with nothing to code: the EOB run grows, BE stays 937, the flush test's boundary)
            struct.pack_into('<I', regs[ENT], 0x3c, 937)
            regs[BLOCKS] = bytes(0x80 * 10)
        return Call(addr, [CINFO, MCUP], regs, ret='a')
    return op


dcfirst, acfirst, dcrefine, acrefine = (mcu(a) for a in (0x65278f, 0x652d82, 0x652fcc, 0x6530f4))
dcfirst.__name__, acfirst.__name__, dcrefine.__name__, acrefine.__name__ = 'dcfirst', 'acfirst', 'dcrefine', 'acrefine'


def finish(r):
    regs = common(r)
    return Call(0x653498, [CINFO], regs)


def fgather(r):
    regs = common(r, tables=2)
    gathering(regs, 1 if r.random() < 0.9 else 0)
    cinfo, ent = regs[CINFO], regs[ENT]
    for i in range(2):
        struct.pack_into('<I', ent, 0x4c + 4 * i, DT[i])
        struct.pack_into('<I', ent, 0x5c + 4 * i, CNT[i])
        struct.pack_into('<II', cinfo, 0x50 + 0x10 * i, *[r.choice([0, HT[0], HT[1]]) for _ in range(2)])
    for t in HT[:2]:
        regs[t] = rb(r, 0x118)
    for t in CNT:
        regs[t] = counts(r, chain=r.random() < 0.1)
    regs[HEAP] = heap(0x600)
    return Call(0x6534fe, [CINFO], regs)


OPS = [f for f in (jinit, start, dump, emit, flush, symbol, buffered, eobrun, restart, dcfirst, acfirst, dcrefine, acrefine, finish,
                   fgather)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


# (finish_pass_gather_phuff dealt half as often: its optimal tables take ten to thirty times as long as the rest)
DECK = OPS + [f for f in OPS if f is not fgather]


def case(r):
    return r.deal(DECK)(r)


if __name__ == '__main__':
    B.main(case)
