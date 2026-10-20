# libjpeg 6a's jcapimin.c / jcapistd.c, the compressor's interface (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a,
# its names jpeg6a_*) against the exe's 6a copies. cinfo at CINFO (6a's jpeg_compress_struct, 0x168 bytes: err +0 at ERR, error_exit
# and emit_message cdecl recorders; mem +4 at MEM, free_pool +0x24 and self_destruct +0x28 recorders; progress +8 at PROGRESS or null;
# global_state +0x10; dest +0x14 at DEST, term_destination +0x10 a recorder; image_height +0x1c; the table pointers +0x40 / +0x50 /
# +0x60; next_scanline +0xd8; max_v_samp_factor +0xe4; total_iMCU_rows +0xe8; master +0x144, main +0x148, coef +0x150, marker
# +0x154, each a struct of recorders). Message codes are jpeg-6a/jerror.h's (JERR_BAD_STATE 0x12, JERR_BUFFER_SIZE 0x15,
# JERR_CANT_SUSPEND 0x16, JERR_TOO_LITTLE_DATA 0x42, JWRN_TOO_MUCH_DATA 0x77).
# jpeg_CreateCompress runs the memory manager's init (the library's through its method shim, which stores the exe's addresses; no case
# runs a method it stored), as jpeg6b_api.py's create: malloc, free, getenv and sscanf are fncmp's heap and recorders on both sides.
# jpeg_finish_compress loops until the master's is_last_pass is set, which a recorder cannot do through a field of the master: the
# cases where it starts clear put the master inside cinfo (at +0x20, so is_last_pass is cinfo's +0x30) and make prepare_for_pass fill
# cinfo's first 0x34 bytes as they are with is_last_pass set, so the loop runs once; the others start with it set.
# jpeg_write_scanlines' process_data gets &row_ctr (a stack address: logged as 0) and fills it with the rows it took.
# usage: jpeg6a_api.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=create,destroy,abort,suppress,finish,marker,scanlines,raw)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _alloc, _free, _recorder, fill, heap, log_region, recorder, set_return, strings  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); then what they run: jinit_memory_mgr, jcomapi.c's jpeg_abort /
# jpeg_destroy, jmemansi.c's and jpeg_mem_init
FUNCS = {'jpeg_CreateCompress': (0x64c5f0, 0x64c740), 'jpeg_destroy_compress': (0x64c740, 0x64c751),
         'jpeg_abort_compress': (0x64c751, 0x64c762), 'jpeg_suppress_tables': (0x64c762, 0x64c7fd),
         'jpeg_finish_compress': (0x64c7fd, 0x64c982), 'jpeg_write_marker': (0x64c982, 0x64c9f4),
         'jpeg_write_scanlines': (0x64cb3c, 0x64cc5e), 'jpeg_write_raw_data': (0x64cc5e, 0x64cd8f)}
CALLED = [(0x64e020, 0x64e21d), (0x64f3c0, 0x64f43e), (0x650210, 0x650265), (0x6503c1, 0x6503d0)]
K_GETENV, K_SSCANF, K_OPEN = 24, 25, 26  # (driver.cpp's kJpeg6aCrtEnv)
B = Bench(sorted(FUNCS.values()) + CALLED,
          hooks={0x67dcd0: _alloc, 0x67ad90: _free, 0x67fbf0: _recorder(K_GETENV, 1, cdecl=True),
                 0x67f620: _recorder(K_SSCANF, 4, cdecl=True), 0x650265: _recorder(K_OPEN, 3, cdecl=True)})

CINFO, ERR, MEM, PROGRESS, DEST, MASTER, MAIN, COEF, MARKER, SCANLINES, STR = (
    RAM, RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x360, RAM + 0x380, RAM + 0x3a0, RAM + 0x3c0, RAM + 0x3e0, RAM + 0x400,
    RAM + 0x440)
TABLES = RAM + 0x1000  # (table i at TABLES + 0x200 * i; only its sent_table is a region)
K_ERROR, K_EMIT, K_PREPARE, K_STARTUP, K_FINISH_PASS, K_COMPRESS, K_PROGRESS, K_TRAILER, K_TERM, K_FREE_POOL, K_SELF_DESTRUCT, \
    K_PROCESS, K_ANY_MARKER = range(13)
IN_CINFO = 0x20  # (the master inside cinfo: see the header)


def common(r):
    log = log_region()
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<IIII', cinfo, 0, ERR, MEM, PROGRESS if r.random() < 0.6 else 0, r.choice([0, 0, 1]))
    struct.pack_into('<I', cinfo, 0x14, DEST)
    for off, at in ((0x144, MASTER), (0x148, MAIN), (0x150, COEF), (0x154, MARKER)):
        struct.pack_into('<I', cinfo, off, at)
    err = struct.pack('<II', recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True)) + r.bytes(0x7c)
    mem = bytearray(r.bytes(0x30))
    struct.pack_into('<II', mem, 0x24, recorder(K_FREE_POOL, 2, cdecl=True), recorder(K_SELF_DESTRUCT, 1, cdecl=True))
    progress = struct.pack('<I', recorder(K_PROGRESS, 1, cdecl=True)) + r.bytes(0x10)
    dest = r.bytes(0x10) + struct.pack('<I', recorder(K_TERM, 1, cdecl=True))
    master = struct.pack('<IIIII', recorder(K_PREPARE, 1, cdecl=True), recorder(K_STARTUP, 1, cdecl=True),
                         recorder(K_FINISH_PASS, 1, cdecl=True), r.choice([0, 1, 0x100]), r.choice([1, 1, 0x10000]))
    main = r.bytes(4) + struct.pack('<I', recorder(K_PROCESS, 4, cdecl=True))
    coef = r.bytes(4) + struct.pack('<I', recorder(K_COMPRESS, 2, cdecl=True))
    marker = struct.pack('<I', recorder(K_ANY_MARKER, 4, cdecl=True)) + r.bytes(0xc) + struct.pack('<I', recorder(K_TRAILER, 1, cdecl=True))
    return {CINFO: cinfo, ERR: err, MEM: mem, PROGRESS: progress, DEST: dest, MASTER: master, MAIN: main, COEF: coef, MARKER: marker,
            LOG: log}


def state(r, cinfo, likely):
    struct.pack_into('<i', cinfo, 0x10, r.choice(likely) if r.random() < 0.85 else r.choice([0, 100, 101, 102, 103, 104, 200]))


def create(r):
    log = log_region()
    strings(log, K_GETENV, 1)
    strings(log, K_SSCANF, 3, out=0xc)
    set_return(log, K_GETENV, 0 if r.random() < 0.8 else STR)
    set_return(log, K_SSCANF, r.choice([0, 1, 2]))
    fill(log, K_SSCANF, 2, struct.pack('<i', r.choice([1, 100, 4000])))
    fill(log, K_SSCANF, 3, bytes([r.choice([ord('m'), ord('x')])]), which=1)
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<I', cinfo, 0, ERR)
    err = struct.pack('<II', recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True)) + r.bytes(0x7c)
    version = 61 if r.random() < 0.9 else r.choice([62, 0, 60, r.getrandbits(32)])
    size = 0x168 if r.random() < 0.9 else r.choice([0x1b0, 0x164, 0x16c, 0])
    regs = {CINFO: cinfo, ERR: err, LOG: log, HEAP: heap(0x1000, r.choice([0] * 60 + [1])), STR: b'4\0'}
    return Call(0x64c5f0, [CINFO, version, size], regs)


def destroy(r):
    regs = common(r)
    if r.random() < 0.2:
        struct.pack_into('<I', regs[CINFO], 4, 0)
    return Call(0x64c740, [CINFO], regs)


def abort(r):
    return Call(0x64c751, [CINFO], common(r))


def suppress(r):
    regs = common(r)
    cinfo = regs[CINFO]
    for i in range(12):  # 4 quant tables (+0x40), then dc / ac huff tables in pairs (+0x50 + 4 * i, +0x60 + 4 * i)
        off = 0x40 + 4 * i if i < 4 else (0x50 if i % 2 == 0 else 0x60) + 4 * ((i - 4) // 2)
        tbl = TABLES + 0x200 * i if r.random() < 0.7 else 0
        struct.pack_into('<I', cinfo, off, tbl)
        if tbl:
            regs[tbl + (0x80 if i < 4 else 0x114)] = r.bytes(4)
    return Call(0x64c762, [CINFO, r.choice([0, 1, 1, r.getrandbits(32)])], regs)


def lines(r, cinfo):
    height = r.choice([0, 1, 16, r.randrange(100)])
    struct.pack_into('<I', cinfo, 0x1c, height)
    struct.pack_into('<I', cinfo, 0xd8, r.choice([0, max(0, height - 1), height, height + 1, r.randrange(100)]))
    return height


def finish(r):
    regs = common(r)
    cinfo = regs[CINFO]
    state(r, cinfo, [101, 102, 103])
    lines(r, cinfo)
    struct.pack_into('<I', cinfo, 0xe8, r.choice([0, 1, 2, 3, 5]))
    set_return(regs[LOG], K_COMPRESS, r.choice([0, 1, 1, 1]))
    if r.random() < 0.6:  # (is_last_pass clear: the master in cinfo, see the header)
        struct.pack_into('<I', cinfo, 0x144, CINFO + IN_CINFO)
        cinfo[IN_CINFO:IN_CINFO + 0x14] = regs.pop(MASTER)[:0x10] + struct.pack('<I', 0)
        fill(regs[LOG], K_PREPARE, 0, bytes(cinfo[:IN_CINFO + 0x10]) + struct.pack('<I', r.choice([1, 0x100])))
    return Call(0x64c7fd, [CINFO], regs)


def marker(r):
    regs = common(r)
    cinfo = regs[CINFO]
    state(r, cinfo, [101, 102, 103])
    struct.pack_into('<I', cinfo, 0xd8, r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    return Call(0x64c982, [CINFO, r.choice([0xe0, 0xfe, 0xef, r.randrange(256)]), SCANLINES, r.randrange(0x10000)], regs)


def scanlines(r):
    regs = common(r)
    cinfo = regs[CINFO]
    state(r, cinfo, [101])
    lines(r, cinfo)
    num = r.choice([0, 1, 2, 8, 16, r.randrange(100)])
    log = regs[LOG]
    strings(log, K_PROCESS, 0, out=4)  # (&row_ctr: a stack address)
    fill(log, K_PROCESS, 2, struct.pack('<I', r.choice([0, 1, num, r.randrange(num + 1)])))
    regs[SCANLINES] = r.bytes(0x20)
    return Call(0x64cb3c, [CINFO, SCANLINES, num], regs, ret='a')


def raw(r):
    regs = common(r)
    cinfo = regs[CINFO]
    state(r, cinfo, [102])
    lines(r, cinfo)
    vmax = r.choice([1, 2, 4, r.randrange(5)])
    struct.pack_into('<i', cinfo, 0xe4, vmax)
    set_return(regs[LOG], K_COMPRESS, r.choice([0, 1, 1]))
    num = r.choice([8 * vmax, 8 * vmax, 8 * vmax - 1, 8 * vmax + 1, 0, r.randrange(64)])
    regs[SCANLINES] = r.bytes(0x20)
    return Call(0x64cc5e, [CINFO, SCANLINES, num], regs, ret='a')


OPS = [f for f in (create, destroy, abort, suppress, finish, marker, scanlines, raw)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
