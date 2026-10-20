# libjpeg 6b's jcomapi.c, the API shared by compression and decompression (src/game/lib/jpeg6b: the library built from
# third_party/jpeg-6b) against the exe's copies. cinfo at CINFO (a jpeg_common_struct, read as a decompressor's: is_decompressor +0x10,
# global_state +0x14, marker_list +0x10c), mem +4 at MEM (null sometimes): alloc_small ZALLOC over the case heap, free_pool +0x24 and
# self_destruct +0x28 cdecl recorders (slots 0, 1).
# usage: jpeg6b_comapi.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=abort,destroy,quant,huff)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jpeg_abort': (0x63dff0, 0x63e040), 'jpeg_destroy': (0x63e040, 0x63e070), 'jpeg_alloc_quant_table': (0x63e070, 0x63e090),
         'jpeg_alloc_huff_table': (0x63e090, 0x63e0b0)}
B = Bench(sorted(FUNCS.values()))

CINFO, MEM = RAM, RAM + 0x200
K_FREE_POOL, K_SELF_DESTRUCT = 0, 1


def regions(r, mem=True):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<I', cinfo, 4, MEM if mem else 0)
    cinfo[0x10] = r.choice([0, 1, 1, r.randrange(256)])  # is_decompressor
    m = bytearray(r.bytes(0x34))
    struct.pack_into('<I', m, 0, ZALLOC)
    struct.pack_into('<II', m, 0x24, recorder(K_FREE_POOL, 2, cdecl=True), recorder(K_SELF_DESTRUCT, 1, cdecl=True))
    return {CINFO: cinfo, MEM: m, LOG: log_region(), HEAP: heap(0x400)}


def abort(r):
    return Call(0x63dff0, [CINFO], regions(r, r.random() < 0.9))


def destroy(r):
    return Call(0x63e040, [CINFO], regions(r, r.random() < 0.9))


def quant(r):
    return Call(0x63e070, [CINFO], regions(r), ret='a')


def huff(r):
    return Call(0x63e090, [CINFO], regions(r), ret='a')


OPS = [f for f in (abort, destroy, quant, huff) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
