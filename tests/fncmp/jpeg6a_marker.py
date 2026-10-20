# libjpeg 6a's jcmarker.c, the marker writer (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a, the module through its
# wrapper unit third_party/wrap/jpeg-6a/jcmarker.c) against the exe's 6a copies. cinfo at CINFO (6a's jpeg_compress_struct: err +0
# at ERR, error_exit and emit_message cdecl recorders; mem +4 at MEM, alloc_small fncmp's ZALLOC over the case heap; dest +0x14 at
# DEST: next_output_byte +0 into OUT, free_in_buffer +4, empty_output_buffer +0xc a recorder; data_precision +0x30, num_components
# +0x34, jpeg_color_space +0x38, comp_info +0x3c at COMPS (components of 0x54 bytes: component_id +0, h / v_samp_factor +8 / +0xc,
# quant_tbl_no +0x10, dc / ac_tbl_no +0x14 / +0x18); quant_tbl_ptrs +0x40 into QT (JQUANT_TBL: quantval[64] +0, sent_table +0x80),
# dc / ac_huff_tbl_ptrs +0x50 / +0x60 into HT (JHUFF_TBL: bits[17] +0, huffval[256] +0x11, sent_table +0x114); arith_code +0xac,
# restart_interval +0xc0, write_JFIF_header +0xc8, density_unit +0xcc, X / Y_density +0xce / +0xd0, write_Adobe_marker +0xd4,
# progressive_mode +0xdc, comps_in_scan +0xec, cur_comp_info[4] +0xf0, Ss / Se / Ah / Al +0x134..+0x140, marker +0x154).
# A recorder cannot refill the buffer: once free_in_buffer reaches 0 and empty_output_buffer has been called, the count wraps and the
# bytes go on into OUT, which holds the most a case writes. The source reads huffval for as many symbols as bits[] counts, so the tables
# count at most 256 (more would read past the table, into each side's neighbours); a missing table is ERREXIT and then a null read (a
# crash in both), dealt rarely. write_any_marker's largest datalen, 65533, writes 64 KB: dealt rarely, with regions to match.
# usage: jpeg6a_marker.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=jinit,anymarker,byte,marker,two,header,jfif,adobe,frame,dqt,sof,scan,dht,dri,sos,trailer,tables)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); emit_dac is the exe's empty hand port
FUNCS = {'jinit_marker_writer': (0x64f4a0, 0x64f520), 'write_any_marker': (0x64f520, 0x64f580), 'emit_byte': (0x64f580, 0x64f5eb),
         'emit_marker': (0x64f5eb, 0x64f611), 'emit_2bytes': (0x64f611, 0x64f644), 'write_file_header': (0x64f644, 0x64f68a),
         'emit_jfif_app0': (0x64f68a, 0x64f776), 'emit_adobe_app14': (0x64f776, 0x64f852), 'write_frame_header': (0x64f852, 0x64f9c5),
         'emit_dqt': (0x64f9c5, 0x64fb05), 'emit_sof': (0x64fb05, 0x64fc34), 'write_scan_header': (0x64fc34, 0x64fd34),
         'emit_dht': (0x64fd34, 0x64fe7d), 'emit_dac': (0x64fe7d, 0x64fe82), 'emit_dri': (0x64fe82, 0x64febc),
         'emit_sos': (0x64febc, 0x65000f), 'write_file_trailer': (0x65000f, 0x650025), 'write_tables_only': (0x650025, 0x6500ea)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, DEST, COMPS, QT, HT, OUT, DATA = (RAM, RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x400, RAM + 0x600,
                                                   RAM + 0x1000, RAM + 0x2000, RAM + 0x3000)
BIG_OUT, BIG_DATA = RAM + 0x10000, RAM + 0x30000
K_ERROR, K_EMIT, K_EMPTY = 0, 1, 2


def huff_table(r):
    total = r.choice([0, 12, 162, r.randrange(257), 256])
    bits = [0] * 17
    for _ in range(total):
        bits[r.randrange(1, 17)] += 1
    if any(b > 255 for b in bits):
        bits = [0] + [16] * 16  # (256)
    t = bytearray(bytes(bits) + r.bytes(256) + bytes(3) + struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)])))
    return t


def common(r):
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<I', cinfo, 0x14, DEST)
    struct.pack_into('<i', cinfo, 0x30, r.choice([8, 8, 8, 12, r.randrange(256)]))
    ncomps = r.choice([1, 3, 3, 4, 2])
    struct.pack_into('<iiI', cinfo, 0x34, ncomps, r.choice([1, 2, 3, 4, 5, 0, r.randrange(8)]), COMPS)
    for i in range(4):
        struct.pack_into('<I', cinfo, 0x40 + 4 * i, QT + 0x100 * i if r.random() < 0.97 else 0)
        for j, off in enumerate((0x50, 0x60)):
            struct.pack_into('<I', cinfo, off + 4 * i, HT + 0x120 * (2 * i + j) if r.random() < 0.97 else 0)
    struct.pack_into('<i', cinfo, 0xac, r.choice([0, 0, 0, 1]))
    struct.pack_into('<I', cinfo, 0xc0, r.choice([0, 0, 1, 0xffff, r.randrange(0x10000)]))
    struct.pack_into('<I', cinfo, 0xc8, r.choice([0, 1, 1, r.getrandbits(32)]))
    struct.pack_into('<I', cinfo, 0xd4, r.choice([0, 1, 1, r.getrandbits(32)]))
    struct.pack_into('<i', cinfo, 0xdc, r.choice([0, 0, 1]))
    nscan = r.choice([1, 1, 2, 3, 4])
    struct.pack_into('<i', cinfo, 0xec, nscan)
    for i in range(4):
        struct.pack_into('<I', cinfo, 0xf0 + 4 * i, COMPS + 0x54 * r.randrange(4))
    struct.pack_into('<4i', cinfo, 0x134, r.choice([0, 0, 1, r.randrange(64)]), r.choice([0, 63, r.randrange(64)]),
                     r.choice([0, 0, 1, r.randrange(14)]), r.choice([0, 1, r.randrange(14)]))
    height, width = r.choice([1, 16, 480, 65535, 65536, r.randrange(70000)]), r.choice([1, 640, 65535, 65536, r.randrange(70000)])
    struct.pack_into('<II', cinfo, 0x18, width, height)
    comps = bytearray(r.bytes(0x54 * 4))
    for c in range(4):
        struct.pack_into('<i', comps, 0x54 * c, r.choice([1, 2, 3, c + 1, r.randrange(256)]))
        struct.pack_into('<5i', comps, 0x54 * c + 8, r.choice([1, 2, 4]), r.choice([1, 2, 4]), r.randrange(4),
                         r.choice([0, 1, r.randrange(4)]), r.choice([0, 1, r.randrange(4)]))
    err = struct.pack('<II', recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True)) + r.bytes(0x7c)
    mem = struct.pack('<I', ZALLOC) + r.bytes(0x2c)
    dest = bytearray(struct.pack('<II', OUT, r.choice([0x10000, 0x10000, 1, 2, 3, 0, r.randrange(1, 400)])) + r.bytes(4) + \
        struct.pack('<I', recorder(K_EMPTY, 1, cdecl=True)) + r.bytes(4))
    qt = bytearray()
    for _ in range(4):
        big = r.random() < 0.3
        vals = [r.randrange(1, 65536 if big and r.random() < 0.2 else 256) for _ in range(64)]
        qt += struct.pack('<64H', *vals) + struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)])) + bytes(0x7c)
    ht = bytearray()
    for _ in range(8):
        ht += huff_table(r) + bytes(8)
    log = log_region()
    set_return(log, K_EMPTY, 0 if r.random() < 0.3 else 1)
    return {CINFO: cinfo, ERR: err, MEM: mem, DEST: dest, COMPS: comps, QT: qt, HT: ht, OUT: r.bytes(0xc00), LOG: log}


def jinit(r):
    regs = common(r)
    regs[HEAP] = heap(0x100)
    return Call(0x64f4a0, [CINFO], regs)


def anymarker(r):
    regs = common(r)
    if r.random() < 0.15:  # (65533 bytes: see the header)
        regs.pop(OUT)
        regs[BIG_OUT] = bytes(0x10010)
        struct.pack_into('<I', regs[DEST], 0, BIG_OUT)
        regs[BIG_DATA] = r.bytes(0x10000)
        return Call(0x64f520, [CINFO, r.randrange(256), BIG_DATA, 65533], regs)
    regs[DATA] = r.bytes(0x200)
    return Call(0x64f520, [CINFO, r.choice([0xe0, 0xfe, r.randrange(256), r.getrandbits(32)]), DATA,
                           r.choice([0, 1, 2, r.randrange(0x200), 65534, 0xffffffff, r.getrandbits(32)])], regs)


def byte(r):
    return Call(0x64f580, [CINFO, r.choice([0, 0xff, r.randrange(256), r.getrandbits(32)])], common(r))


def marker(r):
    return Call(0x64f5eb, [CINFO, r.choice([0xd8, 0xd9, 0xc0, r.randrange(256), r.getrandbits(32)])], common(r))


def two(r):
    return Call(0x64f611, [CINFO, r.choice([0, 0xffff, r.randrange(0x10000), r.getrandbits(32)])], common(r))


def header(r):
    return Call(0x64f644, [CINFO], common(r))


def jfif(r):
    return Call(0x64f68a, [CINFO], common(r))


def adobe(r):
    return Call(0x64f776, [CINFO], common(r))


def frame(r):
    regs = common(r)
    if r.random() < 0.5:  # (baseline's edge: every table number 0 or 1 but perhaps one 2, at 8 bits, Huffman, sequential)
        struct.pack_into('<iiiI', regs[CINFO], 0x30, 8, r.choice([1, 2, 3, 4]), r.choice([1, 2, 3, 4, 5]), COMPS)
        struct.pack_into('<i', regs[CINFO], 0xac, 0)
        struct.pack_into('<i', regs[CINFO], 0xdc, 0)
        for c in range(4):
            struct.pack_into('<ii', regs[COMPS], 0x54 * c + 0x14, r.choice([0, 1]), r.choice([0, 1]))
        if r.random() < 0.7:
            struct.pack_into('<i', regs[COMPS], 0x54 * r.randrange(4) + r.choice([0x14, 0x18]), 2)
    return Call(0x64f852, [CINFO], regs)


def dqt(r):
    return Call(0x64f9c5, [CINFO, r.randrange(4)], common(r), ret='a')


def sof(r):
    return Call(0x64fb05, [CINFO, r.choice([0xc0, 0xc1, 0xc2, 0xc9])], common(r))


def scan(r):
    return Call(0x64fc34, [CINFO], common(r))


def dht(r):
    return Call(0x64fd34, [CINFO, r.randrange(4), r.choice([0, 1, 1, 0x100])], common(r))


def dri(r):
    return Call(0x64fe82, [CINFO], common(r))


def sos(r):
    regs = common(r)
    if r.random() < 0.4:  # (a progressive DC refinement scan, Huffman or arithmetic)
        struct.pack_into('<i', regs[CINFO], 0xdc, 1)
        struct.pack_into('<i', regs[CINFO], 0xac, r.choice([0, 1]))
        struct.pack_into('<ii', regs[CINFO], 0x134, 0, 0)
        struct.pack_into('<i', regs[CINFO], 0x13c, r.choice([1, r.randrange(1, 14)]))
    return Call(0x64febc, [CINFO], regs)


def trailer(r):
    return Call(0x65000f, [CINFO], common(r))


def tables(r):
    return Call(0x650025, [CINFO], common(r))


OPS = [f for f in (jinit, anymarker, byte, marker, two, header, jfif, adobe, frame, dqt, sof, scan, dht, dri, sos, trailer, tables)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
