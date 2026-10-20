# libjpeg 6a's jcmaster.c, the compressor's master control (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a, the
# module through its wrapper unit third_party/wrap/jpeg-6a/jcmaster.c) against the exe's 6a copies. cinfo at CINFO (6a's
# jpeg_compress_struct: err +0 at ERR, error_exit and emit_message cdecl recorders; mem +4 at MEM, alloc_small fncmp's ZALLOC over the
# case heap; progress +8 at PROGRESS or null (completed_passes +0xc, total_passes +0x10); image_width / height +0x18 / +0x1c,
# input_components +0x20, data_precision +0x30, num_components +0x34, comp_info +0x3c at COMPS (ten components of 0x54 bytes:
# component_index +4, h / v_samp_factor +8 / +0xc, width / height_in_blocks +0x1c / +0x20, DCT_scaled_size +0x24, downsampled
# width / height +0x28 / +0x2c, component_needed +0x30, MCU_width / height / blocks / sample_width +0x34..+0x40, last_col_width
# +0x44, last_row_height +0x48), num_scans +0xa0, scan_info +0xa4 at SCANS (0x24 bytes a scan: comps_in_scan, component_index[4],
# Ss, Se, Ah, Al), raw_data_in +0xa8, arith_code +0xac, optimize_coding +0xb0, restart_interval / restart_in_rows +0xc0 / +0xc4,
# progressive_mode +0xdc, max_h / v_samp_factor +0xe0 / +0xe4, total_iMCU_rows +0xe8, comps_in_scan +0xec, cur_comp_info[4] +0xf0,
# MCUs_per_row +0x100, MCU_rows_in_scan +0x104, blocks_in_MCU +0x108, MCU_membership[10] +0x10c, Ss..Al +0x134..+0x140; master
# +0x144 at MASTER (my_comp_master, 0x24 bytes: the three methods, call_pass_startup +0xc, is_last_pass +0x10, pass_type +0x14,
# pass_number +0x18, total_passes +0x1c, scan_number +0x20), the other modules +0x148..+0x164 in MODS, their methods cdecl
# recorders). Error codes are jpeg-6a/jerror.h's.
# ERREXIT returns here (a recorder), so the cases stay where the source, going on, stays in bounds: validate_script's component
# indexes are always below num_components (its component_sent / last_bitpos are on the stack, set only that far) and Ss / Se within
# 0..63 (it indexes last_bitpos by them), num_components at most 10 (comp_info's size here), comps_in_scan 0..4 where per_scan_setup
# goes through cur_comp_info, at most 13 blocks an MCU (JERR_BAD_MCU_SIZE past 10 goes on writing MCU_membership, up to master at
# +0x144), sampling factors 1..4 where per_scan_setup divides by them.
# usage: jpeg6a_master.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=jinit,setup,validate,prepare,select,perscan,startup,finish)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); jdiv_round_up (jutils.c)
FUNCS = {'jinit_c_master_control': (0x655ea0, 0x655fc0), 'initial_setup': (0x655fc0, 0x6562fc), 'validate_script': (0x6562fc, 0x65691e),
         'prepare_for_pass': (0x65691e, 0x656bb2), 'select_scan_parameters': (0x656bb2, 0x656d38),
         'per_scan_setup': (0x656d38, 0x65701a), 'pass_startup': (0x65701a, 0x657055), 'finish_pass_master': (0x657055, 0x657102),
         'jdiv_round_up': (0x6503d0, 0x6503e3)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, PROGRESS, MASTER, MODS, COMPS, SCANS = (RAM, RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x360, RAM + 0x400,
                                                        RAM + 0x500, RAM + 0x900)
K_ERROR, K_EMIT, K_CC, K_DS, K_PREP, K_FDCT, K_ENT, K_ENT_FINISH, K_COEF, K_MAIN, K_FRAME, K_SCAN = range(12)
# the modules in MODS: cinfo offset, MODS offset, (slot, recorder, stack arguments)...
MODULES = [(0x158, 0x00, [(0, K_CC, 1)]), (0x15c, 0x10, [(0, K_DS, 1)]), (0x14c, 0x20, [(0, K_PREP, 2)]),
           (0x160, 0x30, [(0, K_FDCT, 1)]), (0x164, 0x40, [(0, K_ENT, 2), (8, K_ENT_FINISH, 1)]), (0x150, 0x50, [(0, K_COEF, 2)]),
           (0x148, 0x60, [(0, K_MAIN, 2)]), (0x154, 0x70, [(8, K_FRAME, 1), (0xc, K_SCAN, 1)])]


def sampling(r, ncomps, bad=False):
    """Sampling factors for ncomps components, at most 13 blocks an MCU (10 unless bad)."""
    while True:
        f = [(r.choice([1, 1, 2, 2, 3, 4]), r.choice([1, 1, 2, 2, 3, 4])) for _ in range(ncomps)]
        if sum(h * v for h, v in f) <= (13 if bad else 10):
            return f


def script(r, ncomps):
    """A scan script: sequential or progressive (jpeg_simple_progression's shape), sometimes mutated, component indexes always in
    range and Ss / Se within 0..63 (see the header)."""
    scans = []
    if r.random() < 0.4:  # sequential: the components in one or more scans
        order = list(range(ncomps))
        while order:
            n = min(len(order), r.choice([1, 2, 3, 4]))
            scans.append([n, order[:n], 0, 63, 0, 0])
            order = order[n:]
    else:
        scans.append([min(ncomps, 4), list(range(min(ncomps, 4))), 0, 0, 0, 1])
        for c in range(ncomps):
            scans.append([1, [c], 1, 5, 0, 2])
            scans.append([1, [c], 6, 63, 0, 2])
        scans.append([min(ncomps, 4), list(range(min(ncomps, 4))), 0, 0, 1, 0])
        for c in range(ncomps):
            scans.append([1, [c], 1, 63, 2, 1])
    for _ in range(r.choice([0, 0, 1, 1, 2])):
        s = r.choice(scans)
        what = r.randrange(7)
        if what == 0:
            s[0] = min(ncomps, r.choice([0, 1, 2, 3, 4]))
            s[1] = sorted(r.sample(range(ncomps), s[0]))
        elif what == 1 and len(s[1]) > 1:
            s[1] = list(reversed(s[1]))
        elif what == 2:
            s[2] = r.randrange(64)
        elif what == 3:
            s[3] = r.randrange(64)
        elif what == 4:
            s[4] = r.choice([0, 1, 2, 13, 14, -1])
        elif what == 5:
            s[5] = r.choice([0, 1, 2, 13, 14, -1])
        else:
            scans.remove(s) if len(scans) > 1 else None
    out = bytearray()
    for n, idx, ss, se, ah, al in scans[:24]:
        out += struct.pack('<5i4i', n, *(list(idx) + [0] * (4 - len(idx))), ss, se, ah, al)
    return out, min(len(scans), 24)


def common(r, ncomps=None, bad=False):
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<III', cinfo, 0, ERR, MEM, PROGRESS if r.random() < 0.6 else 0)
    if ncomps is None:
        ncomps = r.choice([1, 3, 3, 4, 2, 1]) if r.random() < 0.9 else r.choice([0, 5, 10, -1])
    width = r.choice([1, 8, 17, 640, r.randrange(1, 3000)]) if r.random() < 0.9 else r.choice([0, 65500, 65501, 0x7fffffff, 0x80000000])
    height = r.choice([1, 8, 16, 480, r.randrange(1, 3000)]) if r.random() < 0.9 else r.choice([0, 65500, 65501, 0x80000000])
    struct.pack_into('<IIi', cinfo, 0x18, width, height, r.choice([1, 3, 3, 4]) if r.random() < 0.9 else r.choice([0, -1, 0x7fffffff]))
    struct.pack_into('<ii', cinfo, 0x30, 8 if r.random() < 0.9 else r.choice([12, 0]), ncomps)
    struct.pack_into('<I', cinfo, 0x3c, COMPS)
    comps = bytearray(r.bytes(0x54 * 10))
    f =sampling(r, max(0, min(ncomps, 4)), bad) + [(r.choice([1, 2]), r.choice([1, 2])) for _ in range(10)]
    for c in range(10):
        h, v = f[c]
        if r.random() < 0.03:
            h, v = r.choice([(0, 1), (5, 1), (1, 0), (1, 5)])
        struct.pack_into('<ii', comps, 0x54 * c + 8, h, v)
        struct.pack_into('<II', comps, 0x54 * c + 0x1c, r.randrange(1, 400), r.randrange(1, 400))
    nscans = 0
    scans = b''
    if r.random() < 0.5 and 0 < ncomps <= 10:
        scans, nscans = script(r, ncomps)
        if r.random() < 0.05:
            nscans = r.choice([0, -1])
    struct.pack_into('<iI', cinfo, 0xa0, nscans, SCANS if scans else 0)
    struct.pack_into('<iii', cinfo, 0xa8, r.choice([0, 0, 1]), r.choice([0, 0, 0, 1]), r.choice([0, 0, 1]))
    struct.pack_into('<i', cinfo, 0xc4, r.choice([0, 0, 1, 2, 100, 0x10000]))
    struct.pack_into('<iii', cinfo, 0xe0, r.choice([1, 2, 4]), r.choice([1, 2, 4]), r.randrange(100))
    nscan = r.choice([1, 1, 2, 3, 4, 0])
    struct.pack_into('<i', cinfo, 0xec, nscan)
    picks = sorted(r.sample(range(max(1, min(ncomps, 4))), min(nscan, max(1, min(ncomps, 4))))) if nscan else []
    picks += [r.randrange(4) for _ in range(4 - len(picks))]
    for i in range(4):
        struct.pack_into('<I', cinfo, 0xf0 + 4 * i, COMPS + 0x54 * picks[i])
    struct.pack_into('<I', cinfo, 0x144, MASTER)
    mods = bytearray(r.bytes(0x80))
    for off, at, slots in MODULES:
        struct.pack_into('<I', cinfo, off, MODS + at)
        for slot, k, n in slots:
            struct.pack_into('<I', mods, at + slot, recorder(k, n, cdecl=True))
    total = r.choice([1, 2, 3, 6, 20])
    master = bytearray(r.bytes(0x24))
    struct.pack_into('<5i', master, 0x10, r.choice([0, 1]), r.choice([0, 0, 1, 1, 2, 2, 3]), r.randrange(total + 1), total,
                     r.randrange(max(1, nscans)))
    err = struct.pack('<II', recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True)) + r.bytes(0x7c)
    regs = {CINFO: cinfo, ERR: err, MEM: struct.pack('<I', ZALLOC) + r.bytes(0x2c), PROGRESS: r.bytes(0x14), MASTER: master,
            MODS: mods, COMPS: comps, LOG: log_region()}
    if scans:
        regs[SCANS] = scans
    return regs


def jinit(r):
    regs = common(r)
    regs[HEAP] = heap(0x100)
    return Call(0x655ea0, [CINFO, r.choice([0, 0, 1])], regs)


def setup(r):
    return Call(0x655fc0, [CINFO], common(r))


def validate(r):
    while True:
        regs = common(r, r.choice([1, 2, 3, 3, 4, 5, 10]))
        if SCANS in regs:
            return Call(0x6562fc, [CINFO], regs)


def prepare(r):
    return Call(0x65691e, [CINFO], common(r, r.choice([1, 2, 3, 3, 4])))


def select(r):
    return Call(0x656bb2, [CINFO], common(r, r.choice([1, 2, 3, 3, 4, 5])))


def perscan(r):
    return Call(0x656d38, [CINFO], common(r, r.choice([1, 2, 3, 3, 4]), bad=True))


def startup(r):
    return Call(0x65701a, [CINFO], common(r))


def finish(r):
    return Call(0x657055, [CINFO], common(r))


OPS = [f for f in (jinit, setup, validate, prepare, select, perscan, startup, finish)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
