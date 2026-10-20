# libjpeg 6b's jdsample.c, the separate upsampler (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b, the module through
# its wrapper unit third_party/wrap/jpeg-6b/jdsample.c) against the exe's copies. cinfo at CINFO (num_components +0x24, output_width
# +0x5c, output_height +0x60, comp_info +0xc4 at COMP, max_v_samp_factor +0x114, upsample +0x1a0 at UPS, cconvert +0x1a4 at CC);
# my_upsampler: color_buf[10] +0xc, methods[10] +0x34, next_row_out +0x5c, rows_to_go +0x60, rowgroup_height[10] +0x64, h_expand[10]
# +0x8c and v_expand[10] +0x96 (bytes). The per-component methods take (cinfo, compptr, input_data, output_data_ptr): input_data is a
# row array one row in (h2v2_fancy_upsample reads input_data[-1]), *output_data_ptr the output rows. sep_upsample calls methods[ci] and
# cconvert->color_convert, cdecl recorders here (slots 0-3 and 10). int_upsample and h2v2_upsample call jcopy_sample_rows.
# jinit_upsampler (0x6460c0) is the library's through jpeg6b.cpp's shim, which puts the exe's method addresses in place of the
# library's (start_pass, upsample, methods[ci]): the bench compares them as stored. cinfo->err at ERR (error_exit a cdecl recorder,
# slot 11), cinfo->mem at MEM (alloc_small ZALLOC, alloc_sarray +8 a cdecl recorder, slot 12, whose hash lands in color_buf[ci]); it
# reads CCIR601_sampling +0x10a, do_fancy_upsampling +0x48, max_h_samp_factor +0x110, max_v +0x114, min_DCT_scaled_size +0x118 and
# each component's h / v_samp_factor +0x8 / +0xc, DCT_scaled_size +0x24, component_needed +0x30 and downsampled_width.
# usage: jpeg6b_sample.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=int,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'start_pass_upsample': (0x6462e0, 0x646300), 'sep_upsample': (0x646300, 0x646400), 'fullsize_upsample': (0x646400, 0x646410),
    'noop_upsample': (0x646410, 0x646420), 'int_upsample': (0x646420, 0x646540), 'h2v1_upsample': (0x646540, 0x6465a0),
    'h2v2_upsample': (0x6465a0, 0x646630), 'h2v1_fancy_upsample': (0x646630, 0x646700), 'h2v2_fancy_upsample': (0x646700, 0x646860),
}
FUNCS['jinit_upsampler'] = (0x6460c0, 0x6462e0)
B = Bench(sorted(list(FUNCS.values()) + [(0x6428c0, 0x642930)]))  # (and jcopy_sample_rows, which int_ and h2v2_upsample call, and jround_up, jinit_upsampler's)

CINFO, UPS, COMP, CC, PTR, CTRS, IMAGE, ROWS, OUT_ROWS = (RAM, RAM + 0x200, RAM + 0x300, RAM + 0x480, RAM + 0x4a0, RAM + 0x4b0,
                                                          RAM + 0x4c0, RAM + 0x500, RAM + 0x800)
IN, OUT = RAM + 0x10000, RAM + 0x20000
NUM_COMPONENTS, OUTPUT_WIDTH, OUTPUT_HEIGHT, COMP_INFO, MAX_V, UPSAMPLE, CCONVERT = 0x24, 0x5c, 0x60, 0xc4, 0x114, 0x1a0, 0x1a4
METHODS, NEXT_ROW_OUT, ROWS_TO_GO, ROWGROUP_HEIGHT, H_EXPAND, V_EXPAND = 0x34, 0x5c, 0x60, 0x64, 0x8c, 0x96
COMPONENT_INDEX, DOWNSAMPLED_WIDTH, COMP_SIZE = 0x4, 0x28, 0x54
K_CONVERT = 10  # color_convert's recorder slot (methods[ci]'s: ci)
IN_ROWS, IN_LEN, OUT_ROWS_N, OUT_LEN = 10, 0x80, 10, 0xa0


def cinfo_bytes(r, max_v, width):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<I', cinfo, OUTPUT_WIDTH, width)
    struct.pack_into('<i', cinfo, MAX_V, max_v)
    struct.pack_into('<I', cinfo, UPSAMPLE, UPS)
    struct.pack_into('<I', cinfo, COMP_INFO, COMP)
    struct.pack_into('<I', cinfo, CCONVERT, CC)
    return cinfo


def method(address, name):
    def op(r):
        max_v = r.choice([1, 1, 2, 2, 3, 4])
        width = r.choice([1, 2, 3, 7, 8, r.randrange(1, 60)])
        ups = bytearray(r.bytes(0xa0))
        index = r.randrange(4)
        ups[H_EXPAND + index] = r.choice([1, 2, 2, 3, 4])
        ups[V_EXPAND + index] = r.choice([1, 2, 2, 3, 4])
        comp = bytearray(r.bytes(COMP_SIZE))
        struct.pack_into('<i', comp, COMPONENT_INDEX, index)
        # (the fancy upsamplers' colctr, a JDIMENSION, is downsampled_width - 2: jinit_upsampler picks them for a width above 2 only)
        struct.pack_into('<I', comp, DOWNSAMPLED_WIDTH, r.choice([3, 4, 5, r.randrange(3, 50)]))
        in_rows = struct.pack('<%dI' % IN_ROWS, *(IN + i * IN_LEN for i in range(IN_ROWS)))
        out_rows = struct.pack('<%dI' % OUT_ROWS_N, *(OUT + i * OUT_LEN for i in range(OUT_ROWS_N)))
        regions = {CINFO: cinfo_bytes(r, max_v, width), UPS: ups, COMP: comp, ROWS: in_rows, IN: r.bytes(IN_ROWS * IN_LEN),
                   OUT_ROWS: out_rows, OUT: r.bytes(OUT_ROWS_N * OUT_LEN), PTR: struct.pack('<I', OUT_ROWS)}
        return Call(address, [CINFO, COMP, ROWS + 4, PTR], regions)
    op.__name__ = name
    return op


def start_pass(r):
    return Call(0x6462e0, [CINFO], {CINFO: cinfo_bytes(r, r.randrange(1, 5), 0), UPS: r.bytes(0xa0)})


def sep(r):
    max_v = r.choice([1, 2, 2, 4])
    components = r.randrange(1, 5)
    cinfo = cinfo_bytes(r, max_v, r.randrange(1, 60))
    struct.pack_into('<i', cinfo, NUM_COMPONENTS, components)
    ups = bytearray(r.bytes(0xa0))
    struct.pack_into('<10I', ups, METHODS, *(recorder(k, 4, cdecl=True) for k in range(10)))
    struct.pack_into('<i', ups, NEXT_ROW_OUT, r.choice([0, max_v, max_v, max_v + 1, r.randrange(max_v + 1)]))
    struct.pack_into('<I', ups, ROWS_TO_GO, r.choice([0, 1, 2, 3, 4, 8, r.randrange(100)]))
    struct.pack_into('<10i', ups, ROWGROUP_HEIGHT, *(r.randrange(1, 5) for _ in range(10)))
    out_ctr = r.randrange(4)
    cc = bytearray(r.bytes(0x18))
    struct.pack_into('<I', cc, 4, recorder(K_CONVERT, 5, cdecl=True))
    regions = {CINFO: cinfo, UPS: ups, COMP: r.bytes(4 * COMP_SIZE), CC: cc, IMAGE: struct.pack('<4I', *(IN + 0x1000 * c for c in range(4))),
               CTRS: struct.pack('<2I', r.randrange(5), out_ctr), LOG: log_region()}
    return Call(0x646300, [CINFO, IMAGE, CTRS, r.randrange(4), OUT_ROWS, CTRS + 4, out_ctr + r.choice([0, 1, 2, 4, r.randrange(8)])], regions)


ERR, MEM = RAM + 0x1000, RAM + 0x1100
K_ERROR, K_SARRAY = 11, 12


def jinit(r):
    n = r.choice([1, 3, 3, 4, r.randrange(0, 6)])
    min_dct = r.choice([1, 2, 4, 8, 8, 8])
    cinfo = cinfo_bytes(r, r.choice([1, 2, 2, r.randrange(1, 5)]), r.choice([1, 2, 7, 8, r.randrange(1, 100)]))
    struct.pack_into('<I', cinfo, 0, ERR)
    struct.pack_into('<I', cinfo, 4, MEM)
    struct.pack_into('<i', cinfo, NUM_COMPONENTS, n)
    struct.pack_into('<i', cinfo, 0x110, r.choice([1, 2, 2, r.randrange(1, 5)]))  # max_h_samp_factor
    struct.pack_into('<i', cinfo, 0x118, min_dct)
    cinfo[0x10a] = 0 if r.random() < 0.9 else r.randrange(1, 256)  # CCIR601_sampling
    cinfo[0x48] = r.choice([0, 1, 1, r.randrange(256)])            # do_fancy_upsampling
    comps = bytearray(r.bytes(5 * COMP_SIZE))
    for ci in range(5):
        o = ci * COMP_SIZE
        struct.pack_into('<ii', comps, o + 8, r.choice([1, 1, 2, r.randrange(1, 5)]), r.choice([1, 1, 2, r.randrange(1, 5)]))
        struct.pack_into('<i', comps, o + 0x24, min_dct if r.random() < 0.95 else r.choice([1, 2, 4, 8]))
        struct.pack_into('<I', comps, o + DOWNSAMPLED_WIDTH, r.choice([1, 2, 3, r.randrange(1, 50)]))
        comps[o + 0x30] = r.choice([0, 1, 1, 1, r.randrange(256)])
    return Call(0x6460c0, [CINFO], {CINFO: cinfo, COMP: comps, ERR: struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x7c),
                                    MEM: struct.pack('<3I', ZALLOC, 0, recorder(K_SARRAY, 4, cdecl=True)) + r.bytes(0x24),
                                    HEAP: heap(0x200), LOG: log_region()})


OPS = [f for f in (start_pass, jinit, sep, method(0x646400, 'fullsize'), method(0x646410, 'noop'), method(0x646420, 'int'),
                   method(0x646540, 'h2v1'), method(0x6465a0, 'h2v2'), method(0x646630, 'h2v1_fancy'), method(0x646700, 'h2v2_fancy'))
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
