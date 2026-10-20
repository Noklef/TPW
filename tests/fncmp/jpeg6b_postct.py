# libjpeg 6b's jdpostct.c, the decompression postprocessing controller (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b)
# against the exe's copies. cinfo at CINFO (err +0 at ERR: error_exit a cdecl recorder (slot 0), msg_code +0x14; mem +4 at MEM:
# alloc_small ZALLOC over the case heap, alloc_sarray +8, request_virt_sarray +0x10 and access_virt_sarray +0x1c cdecl recorders (slots
# 1, 2, 3, their answers hashes); quantize_colors +0x4a, output_width / height +0x5c / +0x60, out_color_components +0x64,
# max_v_samp_factor +0x114; post +0x18c at POST (my_post_controller: pub (start_pass, post_process_data), whole_image +8, buffer +0xc,
# strip_height +0x10, starting_row +0x14, next_row +0x18; 0x1c bytes); upsample +0x1a0 at UPS (upsample +4 a cdecl recorder, slot 4);
# cquantize +0x1a8 at CQ (color_quantize +4 a cdecl recorder, slot 5)).
# The upsampler's out_row_ctr is a counter on the post-processor's stack in post_process_1pass (logged as 0: the two sides' stacks
# differ) and post->next_row in the prepass; the recorder writes a row count through it (fill), as the upsampler advances it.
# usage: jpeg6b_postct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,start,pass1,prepass,pass2)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, fill, heap, log_region, recorder, strings  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jinit_d_post_controller': (0x645d50, 0x645de0), 'start_pass_dpost': (0x645de0, 0x645ee0),
         'post_process_1pass': (0x645ee0, 0x645f60), 'post_process_prepass': (0x645f60, 0x646010),
         'post_process_2pass': (0x646010, 0x6460c0), 'jround_up': (0x6428c0, 0x6428e0)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, POST, UPS, CQ, ARGS = RAM, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x340, RAM + 0x360, RAM + 0x380
K_ERROR, K_ALLOC_SARRAY, K_REQUEST, K_ACCESS, K_UPSAMPLE, K_QUANTIZE = 0, 1, 2, 3, 4, 5
QUANTIZE_COLORS, OUTPUT_WIDTH, OUTPUT_HEIGHT, OUT_COLOR_COMPONENTS, MAX_V_SAMP, POST_, UPSAMPLE_, CQUANTIZE_ = (
    0x4a, 0x5c, 0x60, 0x64, 0x114, 0x18c, 0x1a0, 0x1a8)


def common(r):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    cinfo[QUANTIZE_COLORS] = r.choice([0, 1, 1, r.randrange(256)])
    struct.pack_into('<iii', cinfo, OUTPUT_WIDTH, r.choice([1, 7, 64, r.randrange(1, 2000)]), r.choice([1, 8, 9, r.randrange(1, 2000)]),
                     r.choice([1, 3, 4]))
    struct.pack_into('<i', cinfo, MAX_V_SAMP, r.choice([1, 2, 4]))
    struct.pack_into('<I', cinfo, POST_, POST)
    struct.pack_into('<I', cinfo, UPSAMPLE_, UPS)
    struct.pack_into('<I', cinfo, CQUANTIZE_, CQ)
    mem = bytearray(r.bytes(0x34))
    struct.pack_into('<I', mem, 0, ZALLOC)
    struct.pack_into('<I', mem, 8, recorder(K_ALLOC_SARRAY, 4, cdecl=True))
    struct.pack_into('<I', mem, 0x10, recorder(K_REQUEST, 6, cdecl=True))
    struct.pack_into('<I', mem, 0x1c, recorder(K_ACCESS, 5, cdecl=True))
    return {CINFO: cinfo, ERR: struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x7c), MEM: mem, LOG: log_region(),
            UPS: r.bytes(4) + struct.pack('<I', recorder(K_UPSAMPLE, 7, cdecl=True)) + r.bytes(0x8),
            CQ: r.bytes(4) + struct.pack('<I', recorder(K_QUANTIZE, 4, cdecl=True)) + r.bytes(0x8)}


def post_bytes(r):
    p = bytearray(r.bytes(0x1c))
    strip = r.choice([1, 2, 8, 16, r.randrange(1, 40)])
    struct.pack_into('<IIIII', p, 8, 0 if r.random() < 0.2 else r.getrandbits(32), 0 if r.random() < 0.3 else r.getrandbits(32), strip,
                     r.choice([0, strip, r.randrange(0, 2000)]), r.choice([0, 0, r.randrange(0, strip + 1)]))
    return p


def jinit(r):
    regions = common(r)
    regions[HEAP] = heap(0x100)
    return Call(0x645d50, [CINFO, r.choice([0, 1]) | (r.randrange(256) << 8 if r.wild else 0)], regions)


def start(r):
    regions = common(r)
    regions[POST] = post_bytes(r)
    return Call(0x645de0, [CINFO, r.choice([0, 0, 2, 3, 3, 1, r.randrange(-2, 6)])], regions)


def process(address):
    def op(r):
        regions = common(r)
        regions[POST] = post = post_bytes(r)
        strip, next_row = struct.unpack_from('<I', post, 0x10)[0], struct.unpack_from('<I', post, 0x18)[0]
        out_avail = r.choice([0, 1, 8, r.randrange(0, 64)])
        out_ctr = r.choice([0, r.randrange(0, out_avail + 1), r.randrange(0, 70)])
        log = regions[LOG]
        if address == 0x645ee0:  # (num_rows, on the stack: logged as 0, the upsampler's answer written through it)
            strings(log, K_UPSAMPLE, 0, out=1 << 5)
            fill(log, K_UPSAMPLE, 5, struct.pack('<I', r.choice([0, 1, strip, r.randrange(0, strip + 2)])))
        elif address == 0x645f60:  # (&post->next_row: the upsampler advances it)
            fill(log, K_UPSAMPLE, 5, struct.pack('<I', r.choice([next_row, next_row + 1, strip, r.randrange(0, strip + 2)])))
        regions[ARGS] = struct.pack('<II', 0x1234 + r.randrange(16), out_ctr)
        return Call(address, [CINFO, r.getrandbits(32), ARGS, r.randrange(0, 8), r.getrandbits(32) & ~3, ARGS + 4, out_avail], regions)
    return op


pass1, prepass, pass2 = process(0x645ee0), process(0x645f60), process(0x646010)
pass1.__name__, prepass.__name__, pass2.__name__ = 'pass1', 'prepass', 'pass2'
OPS = [f for f in (jinit, start, pass1, prepass, pass2) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
