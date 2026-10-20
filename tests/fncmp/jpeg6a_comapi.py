# libjpeg 6a's jcomapi.c, the API shared by compression and decompression (src/game/lib/jpeg6a: the library built from
# third_party/jpeg-6a, its names jpeg6a_*) against the exe's 6a copies. cinfo at CINFO (6a's jpeg_common_struct: no client_data, its
# boolean an int: is_decompressor +0xc, global_state +0x10), mem +4 at MEM (null sometimes): alloc_small ZALLOC over the case heap
# (pool 0, the permanent one, so its zero items allocate nothing: the table's sent_table lands at the block), free_pool +0x24 and
# self_destruct +0x28 cdecl recorders (slots 0, 1).
# usage: jpeg6a_comapi.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=abort,destroy,quant,huff)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jpeg_abort': (0x64f3c0, 0x64f40c), 'jpeg_destroy': (0x64f40c, 0x64f43e), 'jpeg_alloc_quant_table': (0x64f43e, 0x64f46f),
         'jpeg_alloc_huff_table': (0x64f46f, 0x64f4a0)}
B = Bench(sorted(FUNCS.values()))

CINFO, MEM = RAM, RAM + 0x200
K_FREE_POOL, K_SELF_DESTRUCT = 0, 1


def regions(r, mem=True):
    cinfo = bytearray(r.bytes(0x40))
    struct.pack_into('<I', cinfo, 4, MEM if mem else 0)
    struct.pack_into('<I', cinfo, 0xc, r.choice([0, 1, 1, r.getrandbits(32)]))  # is_decompressor
    m = bytearray(r.bytes(0x30))
    struct.pack_into('<I', m, 0, ZALLOC)
    struct.pack_into('<II', m, 0x24, recorder(K_FREE_POOL, 2, cdecl=True), recorder(K_SELF_DESTRUCT, 1, cdecl=True))
    return {CINFO: cinfo, MEM: m, LOG: log_region(), HEAP: heap(0x400)}


def abort(r):
    return Call(0x64f3c0, [CINFO], regions(r))


def destroy(r):
    return Call(0x64f40c, [CINFO], regions(r, r.random() < 0.8))


def quant(r):
    return Call(0x64f43e, [CINFO], regions(r), ret='a')


def huff(r):
    return Call(0x64f46f, [CINFO], regions(r), ret='a')


OPS = [f for f in (abort, destroy, quant, huff) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
