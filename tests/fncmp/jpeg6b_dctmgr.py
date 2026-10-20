# libjpeg 6b's jddctmgr.c, the IDCT manager (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b, the module through its
# wrapper unit third_party/wrap/jpeg-6b/jddctmgr.c) against the exe's copies. Both functions store method addresses, so both are
# reached through jpeg6b.cpp's shims, which put the exe's addresses in place of the library's: the bench compares them as stored.
# cinfo at CINFO (err +0 at ERR: error_exit a cdecl recorder, msg_code +0x14, msg_parm +0x18; mem +4 at MEM: alloc_small ZALLOC over
# the case heap; num_components +0x24, dct_method +0x44, comp_info +0xc4 at COMP, idct +0x19c at IDCT). my_idct_controller: start_pass,
# inverse_DCT[10] +4, cur_method[10] +0x2c. Each component: DCT_scaled_size +0x24 (1, 2, 4, 8 or another: the error path),
# component_needed +0x30, quant_table +0x4c (one of the tables at QT, or null), dct_table +0x50 (its multiplier table at TABLES).
# start_pass's float table is x87 code in the exe: FILD of the quantval, two FMULs by aanscalefactor (doubles) and an FSTP to a float.
# usage: jpeg6b_dctmgr.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,start_pass)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jinit_inverse_dct': (0x645a80, 0x645b00), 'start_pass': (0x645b00, 0x645d50)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, IDCT, COMP, QT, TABLES = RAM, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x400, RAM + 0x600, RAM + 0x1000
NUM_COMPONENTS, DCT_METHOD, COMP_INFO, IDCT_ = 0x24, 0x44, 0xc4, 0x19c
DCT_SCALED_SIZE, COMPONENT_NEEDED, QUANT_TABLE, DCT_TABLE, COMP_SIZE = 0x24, 0x30, 0x4c, 0x50, 0x54
QT_SIZE, NQ = 0x84, 3  # JQUANT_TBL: quantval[64] (UINT16), sent_table
K_ERROR = 0


def cinfo_bytes(r, n):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<i', cinfo, NUM_COMPONENTS, n)
    struct.pack_into('<I', cinfo, COMP_INFO, COMP)
    struct.pack_into('<I', cinfo, IDCT_, IDCT)
    struct.pack_into('<i', cinfo, DCT_METHOD, r.choice([0, 0, 1, 1, 2, 2, r.randrange(0, 5)]))
    return cinfo


def common(r, n):
    return {CINFO: cinfo_bytes(r, n), ERR: struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x7c),
            MEM: struct.pack('<I', ZALLOC) + r.bytes(0x2c), LOG: log_region()}


def jinit(r):
    n = r.choice([0, 1, 3, 3, 4, r.randrange(-1, 6)])
    regions = common(r, n)
    regions.update({COMP: r.bytes(5 * COMP_SIZE), HEAP: heap(0x700)})
    return Call(0x645a80, [CINFO], regions)


def quantvals(r):
    k = r.random()
    if k < 0.4:
        return [r.randrange(1, 100) for _ in range(64)]
    if k < 0.7:
        return [r.randrange(1, 256) for _ in range(64)]
    return [r.randrange(0, 0x10000) for _ in range(64)]


def start_pass(r):
    n = r.choice([1, 3, 3, 4, r.randrange(0, 6)])
    regions = common(r, n)
    comps = bytearray(r.bytes(5 * COMP_SIZE))
    idct = bytearray(r.bytes(0x54))
    for ci in range(5):
        o = ci * COMP_SIZE
        struct.pack_into('<i', comps, o + DCT_SCALED_SIZE, r.choice([1, 2, 4, 8, 8, 8, 8, r.randrange(0, 10)]))
        comps[o + COMPONENT_NEEDED] = r.choice([0, 1, 1, 1, r.randrange(256)])
        struct.pack_into('<I', comps, o + QUANT_TABLE, 0 if r.random() < 0.1 else QT + QT_SIZE * r.randrange(NQ))
        struct.pack_into('<I', comps, o + DCT_TABLE, TABLES + 0x100 * ci)
        struct.pack_into('<i', idct, 0x2c + 4 * ci, r.choice([-1, -1, 0, 1, 2, r.randrange(-2, 4)]))
    regions.update({COMP: comps, IDCT: idct, TABLES: r.bytes(0x500),
                    QT: b''.join(struct.pack('<64H', *quantvals(r)) + r.bytes(4) for _ in range(NQ))})
    return Call(0x645b00, [CINFO], regions)


OPS = [f for f in (jinit, start_pass) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
