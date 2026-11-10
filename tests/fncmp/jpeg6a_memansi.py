# libjpeg 6a's jmemansi.c, the memory manager's system-dependent part (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a,
# the module through its wrapper unit third_party/wrap/jpeg-6a/jmemansi.c, whose C runtime calls are the exe's) against the exe's 6a
# copies, as jpeg6b_memansi.py. jpeg_get_small / jpeg_get_large (0x650210, 0x650232) call the exe's malloc (0x67dcd0) and
# jpeg_free_small / jpeg_free_large (0x650221, 0x650243) its free (0x67ad90): here both sides get fncmp's heap for them (the emulator's
# hooks on the two addresses, the driver's Jpeg6a_SetCrtEnv), which records the sizes asked for and the blocks freed.
# jpeg_mem_available (0x650254) reads cinfo->mem (+4, at MEM) max_memory_to_use +0x2c. The backing store: jpeg_open_backing_store
# (0x650265) with the exe's tmpfile (0x67fd60) a recorder answering a FILE, null or anything (null: ERREXITS JERR_TFILE_CREATE 0x3e with
# the exe's 6a "", a .bss byte at EMPTY varied here, through the exe's strncpy, which runs; error_exit a recorder that returns), then
# the three methods (the exe's addresses, as jpeg6a.cpp's shim stores them); read_backing_store / write_backing_store (0x6502cd,
# 0x65033d) with fseek (0x67c100) and fread / fwrite (0x67e1d0, 0x67b6d0) recorders answering 0 or not, the count or not
# (JERR_TFILE_SEEK 0x40, READ 0x3f, WRITE 0x41); close_backing_store (0x6503ad) with fclose (0x67b620) a recorder.
# jpeg_mem_init / term (hand ports: FrameConst989680_6503c1, FrameStub_6503cb) stay the exe's.
# usage: jpeg6a_memansi.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=get_small,free_small,get_large,free_large,available,open,read,write,close)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _alloc, _free, _recorder, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jpeg_get_small': (0x650210, 0x650221), 'jpeg_free_small': (0x650221, 0x650232), 'jpeg_get_large': (0x650232, 0x650243),
         'jpeg_free_large': (0x650243, 0x650254), 'jpeg_mem_available': (0x650254, 0x650265),
         'jpeg_open_backing_store': (0x650265, 0x6502cd), 'read_backing_store': (0x6502cd, 0x65033d),
         'write_backing_store': (0x65033d, 0x6503ad), 'close_backing_store': (0x6503ad, 0x6503c1)}
K_ERROR = 1
K_TMPFILE, K_FSEEK, K_FREAD, K_FWRITE, K_FCLOSE = 19, 20, 21, 22, 23  # (driver.cpp's kJpeg6aCrtEnv)
B = Bench(sorted(list(FUNCS.values()) + [(0x67ae00, 0x67aefe)]),  # (and the exe's strncpy)
          hooks={0x67dcd0: _alloc, 0x67ad90: _free,  # (the exe's malloc and free: fncmp's heap, as Mem_Alloc's)
                 0x67fd60: _recorder(K_TMPFILE, 0, cdecl=True), 0x67c100: _recorder(K_FSEEK, 3, cdecl=True),
                 0x67e1d0: _recorder(K_FREAD, 4, cdecl=True), 0x67b6d0: _recorder(K_FWRITE, 4, cdecl=True),
                 0x67b620: _recorder(K_FCLOSE, 1, cdecl=True)})

CINFO, MEM, ERR, INFO, BUF = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500
EMPTY = 0xfaa538  # (the exe's 6a "", .bss)


def common(r):
    cinfo = bytearray(r.bytes(0x20))
    struct.pack_into('<I', cinfo, 4, MEM)
    return {CINFO: cinfo, MEM: r.bytes(0x2c) + struct.pack('<i', r.choice([0, 10000000, r.getrandbits(32) - (1 << 31)])) + r.bytes(0x20),
            HEAP: heap(0x2000, r.choice([0] * 9 + [1]))}


def get(r, addr):
    return Call(addr, [CINFO, r.choice([0, 1, 4, 0x2c, 0x7fc, r.randrange(0x1000)])], common(r), ret='a')


def free(r, addr):
    return Call(addr, [CINFO, r.choice([0, HEAP + 0x10, HEAP + 0x100, r.getrandbits(32)]), r.getrandbits(32)], common(r))


def get_small(r):
    return get(r, 0x650210)


def free_small(r):
    return free(r, 0x650221)


def get_large(r):
    return get(r, 0x650232)


def free_large(r):
    return free(r, 0x650243)


def available(r):
    return Call(0x650254, [CINFO] + [r.getrandbits(32) for _ in range(3)], common(r), ret='a')


def store(r):
    """cinfo (err at ERR: error_exit a recorder that returns) and a backing_store_info (its FILE at +0xc)."""
    log = log_region()
    cinfo = bytearray(r.bytes(0x20))
    struct.pack_into('<I', cinfo, 0, ERR)
    err = bytearray(r.bytes(0x84))
    struct.pack_into('<I', err, 0, recorder(K_ERROR, 1, cdecl=True))
    return {CINFO: cinfo, ERR: err, INFO: r.bytes(0x90), LOG: log}


def open_(r):
    regs = store(r)
    set_return(regs[LOG], K_TMPFILE, r.choice([0, 0, BUF, r.getrandbits(32)]))
    text = bytes(r.randrange(1, 256) for _ in range(r.choice([0, 0, 1, 0x4f, 0x50, 0x60])))
    regs[EMPTY] = text + b'\0' + r.bytes(r.choice([0, 3]))
    return Call(0x650265, [CINFO, INFO, r.getrandbits(32)], regs, ret='v')


def transfer(r, address, k):
    regs = store(r)
    count = r.choice([0, 1, 0x80, r.getrandbits(32)])
    set_return(regs[LOG], K_FSEEK, r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    set_return(regs[LOG], k, r.choice([count, count, count - 1 & 0xffffffff, 0, r.getrandbits(32)]))
    return Call(address, [CINFO, INFO, BUF, r.getrandbits(32), count], regs, ret='v')


def read(r):
    return transfer(r, 0x6502cd, K_FREAD)


def write(r):
    return transfer(r, 0x65033d, K_FWRITE)


def close(r):
    regs = store(r)
    set_return(regs[LOG], K_FCLOSE, r.choice([0, r.getrandbits(32)]))
    return Call(0x6503ad, [CINFO, INFO], regs, ret='v')


OPS = [f for f in (get_small, free_small, get_large, free_large, available, open_, read, write, close)
       if not os.environ.get('OPS') or f.__name__.rstrip('_') in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
