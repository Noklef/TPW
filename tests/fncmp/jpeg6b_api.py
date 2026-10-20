# libjpeg 6b's jdapimin.c / jdapistd.c, the decompressor's interface (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b,
# the modules through their wrapper units third_party/wrap/jpeg-6b/jdapimin.c and jdapistd.c) against the exe's copies.
# jpeg_CreateDecompress runs the memory manager's init, then the marker reader's and the input controller's, which call the new
# manager's alloc_small; jpeg_start_decompress from DSTATE_READY runs jinit_master_decompress (jpeg6b_master.py's setup: ZALLOC and
# recorders for the memory manager, the other modules' inits in the Bench's ranges). The library's code runs those inits through the
# method shims, which store the exe's method addresses, and the driver cannot call one (its own code lies at the exe's code
# addresses): this bench sets FNCMP_JPEG6B_NATIVE, so the shims leave the library's addresses and the driver puts the exe's back in the
# heap it reports (Jpeg6b_SetNativeMethods, driver.cpp's jpeg6b_methods_back). The emulator runs the exe's memory manager; malloc,
# free, getenv and sscanf are fncmp's heap and recorders on both sides, as in jpeg6b_memmgr.py.
# Every other module's method is a recorder, and a recorder answers the same every call, so the loops the source runs until a method
# changes the state are dealt only where they end at once: jpeg_finish_decompress's consume_input answers JPEG_SUSPENDED unless
# eoi_reached is set; jpeg_start_decompress's multi-scan loop gets JPEG_SUSPENDED or JPEG_REACHED_EOI (its pass counting, on rows and
# scans, is not reached); from DSTATE_READY only buffered images (the others would run the real modules' start_pass methods through
# output_pass_setup, past what a recorder can answer); output_pass_setup's dummy pass gets a process_data that moves no row and an
# output height of at least one (the master's methods cannot clear is_dummy_pass, so a pass that ends loops forever).
# usage: jpeg6b_api.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=create,destroy,header,consume,parms,finish,start,setup,read)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _alloc, _free, _recorder, fill, heap, log_region, recorder, set_return, strings  # noqa: E402
import jpeg6b_master as master  # noqa: E402

os.environ['FNCMP_JPEG6B_NATIVE'] = '1'  # (the driver's: see the header)

# the exe's copies (re/libmap.csv): name -> (address, end); then what they run: the inits jpeg_CreateDecompress calls, the memory
# manager (its methods too), jcomapi.c's jpeg_abort / jpeg_destroy, jmemansi.c's, jpeg_mem_term (the empty Debug_Printf), and the master
FUNCS = {'jpeg_CreateDecompress': (0x630ca0, 0x630d80), 'jpeg_destroy_decompress': (0x630d80, 0x630d90),
         'jpeg_read_header': (0x630d90, 0x630e10), 'jpeg_consume_input': (0x630e10, 0x630ee0),
         'default_decompress_parms': (0x630ee0, 0x631080), 'jpeg_finish_decompress': (0x631080, 0x631140),
         'jpeg_start_decompress': (0x631460, 0x631540), 'output_pass_setup': (0x631540, 0x631610),
         'jpeg_read_scanlines': (0x631610, 0x6316b0)}
CALLED = [(0x63b2c0, 0x63b310), (0x63bb10, 0x63bb80), (0x63d3a0, 0x63e070), (0x642960, 0x6429c0), (0x642b20, 0x642b30),
          (0x5da3c0, 0x5da3d0)]
K_GETENV, K_SSCANF, K_OPEN = 24, 25, 26  # (driver.cpp's kJpeg6bCrtEnv)
B = Bench(sorted(FUNCS.values()) + CALLED + sorted(master.FUNCS.values()) + master.INITS,
          hooks={0x67dcd0: _alloc, 0x67ad90: _free, 0x67fbf0: _recorder(K_GETENV, 1, cdecl=True),
                 0x67f620: _recorder(K_SSCANF, 4, cdecl=True), 0x6429c0: _recorder(K_OPEN, 3, cdecl=True)})

CINFO, COMPS, ERR, MEM, INPUTCTL, PROGRESS, MASTER, MODS = (master.CINFO, master.COMPS, master.ERR, master.MEM, master.INPUTCTL,
                                                           master.PROGRESS, master.MASTER, master.MODS)
MAIN = master.MAIN
SRC, SCANLINES, STR = RAM + 0x2d00, RAM + 0x2e00, RAM + 0x2e80
# (the master bench's slots 0, 1 and 10..16, 21; these take the ones only its prepare / finish cases use)
K_CONSUME, K_RESET, K_INIT_SRC, K_TERM_SRC, K_PROCESS, K_PREPARE, K_FINISH_PASS, K_FREE_POOL, K_SELF_DESTRUCT = 2, 3, 4, 5, 6, 7, 8, 9, 17
K_PROGRESS = master.K_PROGRESS


def common(r):
    regs = master.common(r)
    regs[COMPS] = r.bytes(0x54 * 4)
    regs[CINFO][0x10] = 1  # (is_decompressor: jpeg_abort's DSTATE_START)
    struct.pack_into('<I', regs[CINFO], 0x18, SRC)
    inputctl = bytearray(regs[INPUTCTL])
    struct.pack_into('<II', inputctl, 0, recorder(K_CONSUME, 1, cdecl=True), recorder(K_RESET, 1, cdecl=True))
    regs[INPUTCTL] = inputctl
    mem = bytearray(regs[MEM])
    struct.pack_into('<II', mem, 0x24, recorder(K_FREE_POOL, 2, cdecl=True), recorder(K_SELF_DESTRUCT, 1, cdecl=True))
    regs[MEM] = mem
    struct.pack_into('<II', regs[MASTER], 0, recorder(K_PREPARE, 1, cdecl=True), recorder(K_FINISH_PASS, 1, cdecl=True))
    struct.pack_into('<I', regs[MODS], MAIN - MODS + 4, recorder(K_PROCESS, 4, cdecl=True))
    src = bytearray(r.bytes(0x1c))
    struct.pack_into('<I', src, 8, recorder(K_INIT_SRC, 1, cdecl=True))
    struct.pack_into('<I', src, 0x18, recorder(K_TERM_SRC, 1, cdecl=True))
    regs[SRC] = src
    return regs


def state(r, cinfo, likely, never=()):
    struct.pack_into('<i', cinfo, 0x14, r.choice(likely) if r.random() < 0.85 else
                     r.choice([s for s in [0, 0x64, 0xc7] + list(range(0xc8, 0xd4)) if s not in never]))


def create(r):
    log = log_region()
    strings(log, K_GETENV, 1)
    strings(log, K_SSCANF, 3, out=0xc)
    set_return(log, K_GETENV, 0 if r.random() < 0.8 else STR)
    set_return(log, K_SSCANF, r.choice([0, 1, 2]))
    fill(log, K_SSCANF, 2, struct.pack('<i', r.choice([1, 100, 4000])))
    fill(log, K_SSCANF, 3, bytes([r.choice([ord('m'), ord('x')])]), which=1)
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<I', cinfo, 0, ERR)
    err = struct.pack('<II', recorder(master.K_ERROR, 1, cdecl=True), recorder(master.K_EMIT, 2, cdecl=True)) + r.bytes(0x7c)
    version = 62 if r.random() < 0.9 else r.choice([61, 0, 63, r.getrandbits(32)])
    size = 0x1b0 if r.random() < 0.9 else r.choice([0x168, 0x1ac, 0x1b4, 0])
    regs = {CINFO: cinfo, ERR: err, LOG: log, HEAP: heap(0x1000, r.choice([0] * 60 + [1])), STR: b'4\0'}
    return Call(0x630ca0, [CINFO, version, size], regs)


def destroy(r):
    regs = common(r)
    if r.random() < 0.2:
        struct.pack_into('<I', regs[CINFO], 4, 0)
    return Call(0x630d80, [CINFO], regs)


def markers(r, regs):
    """The fields default_decompress_parms reads: the components (ids), the JFIF and Adobe markers."""
    cinfo = regs[CINFO]
    struct.pack_into('<i', cinfo, 0x24, r.choice([1, 3, 3, 3, 4, 4, 2, 0, 5]))
    cinfo[0x100], cinfo[0x108] = r.choice([0, 1]), r.choice([0, 1])
    cinfo[0x109] = r.choice([0, 1, 2, 3, 0xff])
    comps = bytearray(regs[COMPS])
    ids = r.choice([(1, 2, 3), (82, 71, 66), (1, 2, 4), (82, 71, 67), (0, 1, 2), tuple(r.randrange(256) for _ in range(3))])
    for ci, cid in enumerate(ids):
        struct.pack_into('<i', comps, 0x54 * ci, cid)
    regs[COMPS] = comps


def header(r):
    regs = common(r)
    state(r, regs[CINFO], [0xc8, 0xc9])
    markers(r, regs)
    set_return(regs[LOG], K_CONSUME, r.choice([0, 1, 1, 2, 2, 3]))
    return Call(0x630d90, [CINFO, r.choice([0, 1])], regs, ret='a')


def consume(r):
    regs = common(r)
    state(r, regs[CINFO], list(range(0xc8, 0xd3)))
    markers(r, regs)
    set_return(regs[LOG], K_CONSUME, r.choice([0, 1, 1, 2, 3]))
    return Call(0x630e10, [CINFO], regs, ret='a')


def parms(r):
    regs = common(r)
    markers(r, regs)
    return Call(0x630ee0, [CINFO], regs)


def finish(r):
    regs = common(r)
    cinfo = regs[CINFO]
    state(r, cinfo, [0xcd, 0xce, 0xcf, 0xd2])
    cinfo[0x40] = r.choice([0, 0, 1])
    height = r.choice([0, 1, 16, r.randrange(100)])
    struct.pack_into('<I', cinfo, 0x60, height)
    struct.pack_into('<I', cinfo, 0x78, r.choice([height, height, max(0, height - 1), height + 1, r.randrange(100)]))
    eoi = r.choice([0, 1])
    regs[INPUTCTL] = regs[INPUTCTL][:0x11] + bytes([eoi]) + regs[INPUTCTL][0x12:]
    set_return(regs[LOG], K_CONSUME, 0 if not eoi else r.choice([0, 1, 2]))  # (see the header)
    return Call(0x631080, [CINFO], regs, ret='b')


def start(r):
    regs = common(r)
    cinfo = regs[CINFO]
    ready = r.random() < 0.4
    if ready:  # (only buffered images from DSTATE_READY, see the header; jinit_master_decompress allocates its master)
        master.image(r, regs, r.random() < 0.3)
        struct.pack_into('<i', cinfo, 0x14, 0xca)
        cinfo[0x40] = 1
    else:
        state(r, cinfo, [0xcb, 0xcb, 0xcc], never=[0xca])
        cinfo[0x41] = r.choice([0, 1])
        regs[MASTER][8] = 0
    set_return(regs[LOG], K_CONSUME, r.choice([0, 2]))  # (see the header)
    struct.pack_into('<i', cinfo, 0x7c, r.randrange(10))
    return Call(0x631460, [CINFO], regs, ret='b')


def setup(r):
    regs = common(r)
    cinfo = regs[CINFO]
    state(r, cinfo, [0xcb, 0xcc, 0xcd])
    cinfo[0x41] = r.choice([0, 1])
    height = r.choice([1, 2, 16, r.randrange(1, 100)])
    struct.pack_into('<I', cinfo, 0x60, height)
    struct.pack_into('<I', cinfo, 0x78, r.choice([0, height - 1, height, height + 1]))
    regs[MASTER][8] = r.choice([0, 1])
    return Call(0x631540, [CINFO], regs, ret='b')


def read(r):
    regs = common(r)
    cinfo = regs[CINFO]
    state(r, cinfo, [0xcd])
    height = r.choice([0, 1, 16, r.randrange(100)])
    struct.pack_into('<I', cinfo, 0x60, height)
    struct.pack_into('<I', cinfo, 0x78, r.choice([0, max(0, height - 1), height, height + 1, r.randrange(100)]))
    max_lines = r.choice([0, 1, 2, 8, r.randrange(100)])
    log = regs[LOG]
    strings(log, K_PROCESS, 0, out=4)  # (&row_ctr: a stack address)
    fill(log, K_PROCESS, 2, struct.pack('<I', r.choice([0, 1, max_lines, r.randrange(max_lines + 1)])))
    regs[SCANLINES] = r.bytes(0x20)
    return Call(0x631610, [CINFO, SCANLINES, max_lines], regs, ret='a')


OPS = [f for f in (create, destroy, header, consume, parms, finish, start, setup, read)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
