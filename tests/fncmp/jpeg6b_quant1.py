# libjpeg 6b's jquant1.c, the one-pass colour quantizer (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b, the module
# through its wrapper unit third_party/wrap/jpeg-6b/jquant1.c) against the exe's copies. cinfo at CINFO (err +0 at ERR: error_exit a
# cdecl recorder (slot 0), emit_message +4 (slot 1), msg_code +0x14 / msg_parm +0x18; mem +4 at MEM: alloc_small and alloc_large
# ZALLOC over the case heap (alloc_small's pool JPOOL_IMAGE 1 is ZALLOC's items), alloc_sarray +8 a cdecl recorder (slot 2) answering
# SARRAY, eight row pointers to ROWS (0x300 bytes each: a colormap's up to 300 colours, a padded colorindex's 766 entries);
# out_color_space +0x2c, dither_mode +0x4c, desired_number_of_colors +0x54, output_width +0x5c, out_color_components +0x64,
# actual_number_of_colors +0x70, colormap +0x74, sample_range_limit +0x120 256 bytes into a random table at RANGE, cquantize +0x1a8 at
# CQ (my_cquantizer, 0x58 bytes: sv_colormap +0x10 at CMAP's four rows, sv_actual +0x14, colorindex +0x18 at CIDX's four rows (each
# 255 bytes into its 766, as create_colorindex pads them), is_padded +0x1c, Ncolors[4] +0x20 (2..16, as select_ncolors leaves them),
# row_index +0x30, odither[4] +0x34 at ODITHER's 16 x 16 ints (-255..255, which the padding takes), fserrors[4] +0x44 at FSERR's rows
# (errors within +-2000, so a pixel's sum stays inside the range-limit table), on_odd_row +0x54).
# The quantizers take an input image (IN_PTRS's rows at IN, out_color_components samples a pixel) and output rows (OUT_PTRS at OUT).
# Division by a zero maxj or ncolors - 1 is not dealt (the exe's IDIV and the library's both fault).
# What the source never meets is not dealt: out_color_space JCS_RGB (2) comes with exactly three components (select_ncolors indexes
# Ncolors by its RGB_order[3]: with more it reads past the table, whatever follows each copy's, the exe's at 0x704ec8; with fewer an
# unset Ncolors, which can make its loop run 2^32 times), five components only for jinit (which refuses them: elsewhere Ncolors[4] is
# row_index), row_index 0..15 (read before it is masked), and odither[] / fserrors[] NULL only for start_pass_1_quant (which makes them).
# usage: jpeg6b_quant1.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=jinit,select,value,largest,colormap,colorindex,odither,odither_tables,quantize,quantize3,ordered,ordered3,fs,alloc_fs,start,new_map)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); and jutils.c's jzero_far
FUNCS = {'jinit_1pass_quantizer': (0x6488f0, 0x648990), 'create_colormap': (0x648990, 0x648b00), 'select_ncolors': (0x648b00, 0x648bf0),
         'output_value': (0x648bf0, 0x648c10), 'create_colorindex': (0x648c10, 0x648d50), 'largest_input_value': (0x648d50, 0x648d70),
         'alloc_fs_workspace': (0x648d70, 0x648db0), 'start_pass_1_quant': (0x648db0, 0x648ea0),
         'create_odither_tables': (0x648ea0, 0x648f10), 'make_odither_array': (0x648f10, 0x648f80), 'color_quantize': (0x648f80, 0x649040),
         'color_quantize3': (0x649040, 0x649100), 'quantize_ord_dither': (0x649100, 0x649230),
         'quantize3_ord_dither': (0x649230, 0x649360), 'quantize_fs_dither': (0x649360, 0x649530),
         'new_color_map_1_quant': (0x649530, 0x649550), 'jzero_far': (0x642960, 0x642980)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, CQ, NCOL, SARRAY, CIDX, CMAP = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x3a0, RAM + 0x400, RAM + 0x480, RAM + 0x4a0
IN_PTRS, OUT_PTRS = RAM + 0x5a00, RAM + 0x5a40
ROWS, IDX, CMAP_ROWS, ODITHER, FSERR, RANGE, IN, OUT = (RAM + 0x1000, RAM + 0x3000, RAM + 0x3c00, RAM + 0x4000, RAM + 0x5000,
                                                       RAM + 0x5400, RAM + 0x6000, RAM + 0x7000)
K_ERROR, K_EMIT, K_SARRAY = 0, 1, 2
ROW, IDX_ROW, FS_ROW, IN_ROW, OUT_ROW = 0x300, 0x300, 0x100, 0x200, 0x80
RANGE_SIZE = 5 * 256 + 128


def common(r, nc=None, op=''):
    log = log_region()
    set_return(log, K_SARRAY, SARRAY)
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    nc = nc if nc is not None else (r.choice([1, 3, 3, 3, 4]) if r.random() < 0.95 else r.choice([0, 2, 5] if op == 'jinit' else [0, 2]))
    struct.pack_into('<i', cinfo, 0x2c, r.choice([1, 2, 2, 3, 4, 5]) if nc == 3 else r.choice([1, 3, 4, 5]))  # (see the header: JCS_RGB)
    struct.pack_into('<i', cinfo, 0x4c, r.choice([0, 1, 2]) if r.random() < 0.95 else 3)
    struct.pack_into('<i', cinfo, 0x54, r.choice([2, 8, 16, 27, 64, 216, 256, r.randrange(2, 257)]) if r.random() < 0.95 else r.choice([1, 0, 257, 300]))
    struct.pack_into('<I', cinfo, 0x5c, r.choice([0, 1, 2, 7, 16, 17, r.randrange(1, 101)]))
    struct.pack_into('<i', cinfo, 0x64, nc)
    struct.pack_into('<I', cinfo, 0x120, RANGE + 256)
    struct.pack_into('<I', cinfo, 0x1a8, CQ)
    cq = bytearray(r.bytes(0x58))
    struct.pack_into('<III', cq, 0x10, CMAP, r.randrange(1, 257), CIDX)
    cq[0x1c] = r.choice([0, 1])
    struct.pack_into('<4i', cq, 0x20, *[r.choice([2, 2, 3, 4, 6, r.randrange(2, 17)]) for _ in range(4)])
    struct.pack_into('<i', cq, 0x30, r.randrange(16))
    struct.pack_into('<4I', cq, 0x34, *[ODITHER + 0x400 * i if r.random() < 0.9 or op != 'start' else 0 for i in range(4)])
    struct.pack_into('<4I', cq, 0x44, *[FSERR + FS_ROW * i if r.random() < 0.9 or op != 'start' else 0 for i in range(4)])
    cq[0x54] = r.choice([0, 1])
    odither = b''.join(struct.pack('<i', r.randrange(-255, 256)) for _ in range(4 * 256))
    fserr = b''.join(struct.pack('<h', r.randrange(-2000, 2001)) for _ in range(4 * FS_ROW // 2))
    return nc, {CINFO: cinfo, LOG: log, HEAP: heap(0x3000),
                ERR: struct.pack('<II', recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True)) + r.bytes(0x78),
                MEM: struct.pack('<III', ZALLOC, ZALLOC, recorder(K_SARRAY, 4, cdecl=True)) + r.bytes(0x24),
                CQ: cq, NCOL: r.bytes(0x20),
                SARRAY: struct.pack('<8I', *[ROWS + ROW * i for i in range(8)]), ROWS: r.bytes(8 * ROW),
                CIDX: struct.pack('<4I', *[IDX + IDX_ROW * i + 255 for i in range(4)]), IDX: r.bytes(4 * IDX_ROW),
                CMAP: struct.pack('<4I', *[CMAP_ROWS + 256 * i for i in range(4)]), CMAP_ROWS: r.bytes(4 * 256),
                ODITHER: odither, FSERR: fserr, RANGE: r.bytes(RANGE_SIZE)}


def jinit(r):
    return Call(0x6488f0, [CINFO], common(r, op='jinit')[1])


def select(r):
    return Call(0x648b00, [CINFO, NCOL], common(r)[1], ret='a')


def value(r):
    maxj = r.choice([1, 2, 3, 5, r.randrange(1, 300)])
    return Call(0x648bf0, [CINFO, r.randrange(4), r.randrange(maxj + 1) if r.random() < 0.9 else r.randrange(1000), maxj], common(r)[1],
                ret='a')


def largest(r):
    maxj = r.choice([1, 2, 3, 5, r.randrange(1, 300)])
    return Call(0x648d50, [CINFO, r.randrange(4), r.randrange(maxj + 1) if r.random() < 0.9 else r.randrange(1000), maxj], common(r)[1],
                ret='a')


def colormap(r):
    return Call(0x648990, [CINFO], common(r)[1])


def colorindex(r):
    return Call(0x648c10, [CINFO], common(r)[1])


def odither(r):
    return Call(0x648f10, [CINFO, r.choice([2, 3, 4, 6, r.randrange(2, 257)])], common(r)[1], ret='a')


def odither_tables(r):
    nc, regions = common(r)
    cq = regions[CQ]
    for i in range(4):  # repeats, so tables are shared
        if r.random() < 0.5:
            struct.pack_into('<i', cq, 0x20 + 4 * i, struct.unpack_from('<i', cq, 0x20)[0])
    return Call(0x648ea0, [CINFO], regions)


def quantizing(r, address, nc=None):
    nc, regions = common(r, nc)
    width = struct.unpack_from('<I', regions[CINFO], 0x5c)[0]
    rows = r.choice([0, 1, 1, 2, 3, r.randrange(1, 9)])
    regions[IN_PTRS] = struct.pack('<8I', *[IN + IN_ROW * i for i in range(8)])
    regions[OUT_PTRS] = struct.pack('<8I', *[OUT + OUT_ROW * i for i in range(8)])
    regions[IN], regions[OUT] = r.bytes(8 * IN_ROW), r.bytes(8 * OUT_ROW)
    assert width * max(nc, 3) <= IN_ROW and width + 2 <= FS_ROW // 2 and width <= OUT_ROW
    return Call(address, [CINFO, IN_PTRS, OUT_PTRS, rows], regions)


def quantize(r):
    return quantizing(r, 0x648f80)


def quantize3(r):
    return quantizing(r, 0x649040, 3)


def ordered(r):
    return quantizing(r, 0x649100)


def ordered3(r):
    return quantizing(r, 0x649230, 3)


def fs(r):
    return quantizing(r, 0x649360)


def alloc_fs(r):
    return Call(0x648d70, [CINFO], common(r)[1])


def start(r):
    return Call(0x648db0, [CINFO, r.choice([0, 1])], common(r, op='start')[1])


def new_map(r):
    return Call(0x649530, [CINFO], common(r)[1])


OPS = [f for f in (jinit, select, value, largest, colormap, colorindex, odither, odither_tables, quantize, quantize3, ordered, ordered3, fs,
                   alloc_fs, start, new_map)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
