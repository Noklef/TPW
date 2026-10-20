# libjpeg 6a's jcparam.c, the compression parameters (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a, its names
# jpeg6a_*) against the exe's 6a copies. cinfo at CINFO (jpeg_compress_struct, 0x168 bytes; err +0 at ERR: error_exit a cdecl
# recorder, slot 0, msg_code +0x14, msg_parm +0x18; mem +4 at MEM: alloc_small ZALLOC over the case heap; global_state +0x10,
# CSTATE_START 100 mostly; quant_tbl_ptrs[4] +0x40, each null (a new table from jpeg_alloc_quant_table, the library's, alloc_small of
# pool 0: ZALLOC's zero items give a block of no size at the heap's next address, the same in both) or a table in QTBL).
# jpeg_set_linear_quality passes the library's own copies of the two standard tables (the exe's at 0x705108 / 0x705208): read only.
# usage: jpeg6a_param.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=add_quant,linear,scaling,quality)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); jcomapi.c's jpeg_alloc_quant_table
FUNCS = {'jpeg_add_quant_table': (0x64cd90, 0x64ce78), 'jpeg_set_linear_quality': (0x64ce78, 0x64ceb3),
         'jpeg_quality_scaling': (0x64ceb3, 0x64cef8), 'jpeg_set_quality': (0x64cef8, 0x64cf20),
         'jpeg_alloc_quant_table': (0x64f43e, 0x64f46f)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, QTBL, BASIC = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x800
K_ERROR = 0
QSIZE = 0x84


def common(r):
    cinfo = bytearray(r.bytes(0x60))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<I', cinfo, 0x10, 100 if r.random() < 0.9 else r.choice([0, 101, 102, r.getrandbits(32)]))
    tables = bytearray(r.bytes(4 * QSIZE))
    for i in range(4):
        struct.pack_into('<I', cinfo, 0x40 + 4 * i, 0 if r.random() < 0.3 else QTBL + QSIZE * i)
    err = struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x3c)
    mem = struct.pack('<I', ZALLOC) + r.bytes(0x2c)
    return {CINFO: cinfo, ERR: err, MEM: mem, QTBL: tables, LOG: log_region(), HEAP: heap(0x400)}


def scale(r):
    return r.choice([0, 1, 50, 100, 200, 5000, r.randrange(-10, 300), r.randrange(5001), r.getrandbits(32)]) & 0xffffffff


def force(r):
    return r.choice([0, 1, 1, r.getrandbits(32)])


def add_quant(r):
    regs = common(r)
    basic = [r.choice([0, 1, 16, 99, 255, 256, r.randrange(256), r.randrange(1 << 16), r.getrandbits(32)]) for _ in range(64)]
    regs[BASIC] = struct.pack('<64I', *basic)
    return Call(0x64cd90, [CINFO, r.randrange(4), BASIC, scale(r), force(r)], regs)


def linear(r):
    return Call(0x64ce78, [CINFO, scale(r), force(r)], common(r))


def quality(r):
    return r.choice([-1, 0, 1, 2, 49, 50, 51, 75, 99, 100, 101, r.randrange(-200, 200), r.getrandbits(32)]) & 0xffffffff


def scaling(r):
    return Call(0x64ceb3, [quality(r)], {}, ret='a')


def set_quality(r):
    return Call(0x64cef8, [CINFO, quality(r), force(r)], common(r))


OPS = [f for f in (add_quant, linear, scaling, set_quality) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
