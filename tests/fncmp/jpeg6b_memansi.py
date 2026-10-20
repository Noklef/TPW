# libjpeg 6b's jmemansi.c, the memory manager's system-dependent part (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b,
# the module through its wrapper unit third_party/wrap/jpeg-6b/jmemansi.c, whose malloc and free are the exe's) against the exe's
# copies. jpeg_get_small (0x642980, also jpeg_get_large: one copy) calls the exe's malloc (0x67dcd0) and jpeg_free_small (0x642990, also
# jpeg_free_large and zlib's zcfree) its free (0x67ad90): here both sides get fncmp's heap for them (the emulator's hooks on the two
# addresses, the driver's Jpeg6b_SetCrtEnv), which records the sizes asked for and the blocks freed. jpeg_mem_available (0x6429a0)
# reads cinfo->mem (+4, at MEM) max_memory_to_use +0x2c. jpeg_open_backing_store and its methods, and jpeg_mem_init (the hand port
# Const_1000000), stay the exe's.
# usage: jpeg6b_memansi.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=get,free,available)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, _alloc, _free, heap  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jpeg_get_small': (0x642980, 0x642990), 'jpeg_free_small': (0x642990, 0x6429a0), 'jpeg_mem_available': (0x6429a0, 0x6429c0)}
B = Bench(sorted(FUNCS.values()), hooks={0x67dcd0: _alloc, 0x67ad90: _free})  # (the exe's malloc and free: fncmp's heap, as Mem_Alloc's)

CINFO, MEM = RAM, RAM + 0x200


def common(r):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<I', cinfo, 4, MEM)
    return {CINFO: cinfo, MEM: r.bytes(0x2c) + struct.pack('<i', r.choice([0, 1000000, r.getrandbits(32) - (1 << 31)])) + r.bytes(0x20),
            HEAP: heap(0x2000, r.choice([0] * 9 + [1]))}


def get(r):
    return Call(0x642980, [CINFO, r.choice([0, 1, 4, 0x2c, 0x7fc, r.randrange(0x1000)])], common(r), ret='a')


def free(r):
    return Call(0x642990, [CINFO, r.choice([0, HEAP + 0x10, HEAP + 0x100, r.getrandbits(32)]), r.getrandbits(32)], common(r))


def available(r):
    return Call(0x6429a0, [CINFO] + [r.getrandbits(32) for _ in range(3)], common(r), ret='a')


OPS = [f for f in (get, free, available) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
