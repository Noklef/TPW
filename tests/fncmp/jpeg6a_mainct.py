# libjpeg 6a's jcmainct.c, the compressor's main buffer controller (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a,
# the module through its wrapper unit third_party/wrap/jpeg-6a/jcmainct.c) against the exe's 6a copies. cinfo at CINFO (6a's
# jpeg_compress_struct: err +0 at ERR, error_exit a cdecl recorder; mem +4 at MEM, alloc_small fncmp's ZALLOC over the case heap and
# alloc_sarray +8 a recorder; num_components +0x34, comp_info +0x3c at COMPS (v_samp_factor +0xc, width_in_blocks +0x1c);
# raw_data_in +0xa8; total_iMCU_rows +0xe8; main +0x148 at MAIN (my_main_controller, 0x40 bytes: start_pass, process_data,
# cur_iMCU_row +8, rowgroup_ctr +0xc, suspended +0x10, pass_mode +0x14, buffer[10] +0x18); prep +0x14c with pre_process_data +4 and
# coef +0x150 with compress_data +4 in MODS, recorders). Without FULL_MAIN_BUFFER_SUPPORTED (the exe's build too) start_pass_main
# takes JBUF_PASS_THRU only (JERR_BAD_BUFFER_MODE 4 for the others) and jinit_c_main_controller's full buffer is that error.
# pre_process_data fills the row group counter it is handed (&main->rowgroup_ctr, in MAIN), so the iMCU-row loop ends: at
# total_iMCU_rows, or when the count stays short of 8, or when compress_data answers FALSE; in_row_ctr is in ROWCTR.
# usage: jpeg6a_mainct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,start,process)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, fill, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jinit_c_main_controller': (0x6504c0, 0x650589), 'start_pass_main': (0x650589, 0x650608),
         'process_data_simple_main': (0x650608, 0x6506f2)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, MAIN, MODS, COMPS, ROWCTR, INPUT = RAM, RAM + 0x200, RAM + 0x280, RAM + 0x2c0, RAM + 0x300, RAM + 0x340, RAM + 0x700, RAM + 0x720
K_ERROR, K_SARRAY, K_PRE, K_COMPRESS = 0, 1, 2, 3


def common(r):
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<i4xI', cinfo, 0x34, r.choice([1, 3, 3, 4, 10, 0]), COMPS)
    struct.pack_into('<i', cinfo, 0xa8, 0 if r.random() < 0.8 else r.choice([1, 0x100]))
    struct.pack_into('<I', cinfo, 0xe8, r.choice([0, 1, 2, 3, 5]))
    struct.pack_into('<III', cinfo, 0x148, MAIN, MODS, MODS + 0x10)
    comps = bytearray(r.bytes(0x54 * 10))
    for c in range(10):
        struct.pack_into('<i', comps, 0x54 * c + 0xc, r.choice([1, 2, 4]))
        struct.pack_into('<I', comps, 0x54 * c + 0x1c, r.randrange(1, 300))
    mods = bytearray(r.bytes(0x20))
    struct.pack_into('<I', mods, 4, recorder(K_PRE, 7, cdecl=True))
    struct.pack_into('<I', mods, 0x14, recorder(K_COMPRESS, 2, cdecl=True))
    main = bytearray(r.bytes(0x40))
    struct.pack_into('<IIi', main, 8, r.choice([0, 0, 1, 2]), r.choice([0, 0, 3, 8, 8, 9]), r.choice([0, 1]))
    log = log_region()
    fill(log, K_PRE, 5, struct.pack('<I', r.choice([8, 8, 8, 8, 3, 0, 9])))
    set_return(log, K_COMPRESS, r.choice([1, 1, 1, 0]))
    set_return(log, K_SARRAY, r.getrandbits(32))
    return {CINFO: cinfo, ERR: struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x7c),
            MEM: struct.pack('<III', ZALLOC, r.getrandbits(32), recorder(K_SARRAY, 4, cdecl=True)) + r.bytes(0x24),
            MAIN: main, MODS: mods, COMPS: comps, LOG: log}


def jinit(r):
    regs = common(r)
    regs[HEAP] = heap(0x100)
    return Call(0x6504c0, [CINFO, r.choice([0, 0, 0, 1])], regs)


def start(r):
    return Call(0x650589, [CINFO, r.choice([0, 0, 0, 1, 2, 3, 4])], common(r))


def process(r):
    regs = common(r)
    regs[ROWCTR] = r.bytes(4)
    regs[INPUT] = r.bytes(0x10)
    return Call(0x650608, [CINFO, INPUT, ROWCTR, r.choice([0, 1, 8, 16, r.randrange(100)])], regs)


OPS = [f for f in (jinit, start, process) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
