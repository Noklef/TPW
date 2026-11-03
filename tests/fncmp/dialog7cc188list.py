# Dialog7cc188_FillList (src/game/ui/dialog7cc188, 0x4a7a80) against the original. Recorders, as Dialog7cc188ListEnv's: 0x48bcd0 16 (its answer the object), swprintf 11 (it fills the buffer), crt_atexit fncmp._cdecl_log, Window7059f0_Clear 12, AddItem 14 (the text's bytes hashed), RefreshRows 13, Language_WideText 15. The
# strings a real list<string> of 0-4 nodes (a text null or empty now and then); the window [0x7cc188] with its child 0x26 among decoys; the guard, the mode setting and the session's wide string.
# usage: dialog7cc188list.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, blob_bytes, fill, heap, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x4a7a80, 0x4a7bb1), (0x5da3c0, 0x5da3c1),(0x5cd430, 0x5cd43e), (0x5cd440, 0x5cd509), (0x45b340, 0x45b9a0), (0x67acb0, 0x67acb1), (0x65d7bc, 0x65d840), (0x48b390, 0x48b3fd), (0x5b96b0, 0x5b96c0), (0x67acd0, 0x67ad00), (0x550ca0, 0x550d80)],
          hooks={0x67a7d0: _cdecl_log, 0x48bcd0: _recorder(16, 0, cdecl=True), 0x67af00: _recorder(11, 3, cdecl=True), 0x6649d5: _recorder(12, 0), 0x66403b: _recorder(14, 4), 0x663324: _recorder(13, 0), 0x485b00: _recorder(15, 1, cdecl=True)})
OBJ, NODES, TEXT, WINDOW, KIDS, SESSION, HOLDER, WIDE, NAME = (RAM + 0x1000 * k for k in range(1, 10))


def window(r):
    ids = [0x26] + [r.getrandbits(32) | 0x1000 for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(ids)
    kids = bytearray(r.bytes(0x100 * len(ids)))
    for k, ident in enumerate(ids):
        put(kids, 0x100 * k + 0x54, 'I', KIDS + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(kids, 0x100 * k + 0x8c, 'I', ident)
    return put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS), kids


def case(r):
    log = log_region()
    strings(log, 11, 0, out=1)
    fill(log, 11, 0, r.bytes(0x40))
    blob_bytes(log, 14, 0, 0, inner=(0, 0x3f))
    regions = {LOG: log, HEAP: heap(0x800)}
    n = r.choice([0, 1, 2, 3, 4])
    at = [NODES + 0x20 * i for i in range(n + 1)]
    nodes, texts = bytearray(r.bytes(0x20 * (n + 1))), bytearray()
    for i in range(n + 1):
        put(nodes, 0x20 * i, 'II', at[(i + 1) % (n + 1)], at[i - 1])
    for i in range(1, n + 1):
        form = r.choice(['text'] * 8 + ['null', 'empty'])
        put(nodes, 0x20 * i + 0xc, 'I', 0 if form == 'null' else TEXT + len(texts))
        texts += (b'' if form != 'text' else bytes(r.randrange(1, 0x100) for _ in range(r.randrange(1, 70)))) + b'\0'
    regions[TEXT] = bytes(texts) or b'\0'
    regions[NODES] = nodes
    obj = bytearray(r.bytes(0x120))
    put(obj, 0x104, 'II', at[0], n)
    regions[OBJ] = obj
    regions[0x786de4] = b'\0'
    set_return(log, 16, OBJ)
    regions[WINDOW], regions[KIDS] = window(r)
    regions[0x7cc188] = struct.pack('<I', WINDOW)
    regions[0xfb34bf] = bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([1] * 8 + [0, 2, r.getrandbits(32)]))
    regions[0x7a1a8c] = r.bytes(4)
    regions[0xf7d88c] = struct.pack('<I', SESSION)
    regions[SESSION] = put(bytearray(r.bytes(0x400)), 0x3a4, 'I', HOLDER)
    regions[HOLDER] = put(bytearray(r.bytes(0x100)), 0x9c, 'I', r.choice([WIDE] * 4 + [0, WIDE + 0x20]))
    regions[WIDE] = struct.pack('<4H', 0x41, 0x42, 0x43, 0) + bytes(0x18) + struct.pack('<H', 0)
    regions[NAME] = r.bytes(0x40)
    set_return(log, 15, NAME)
    return Call(0x4a7a80, [r.choice([0, 0, 1, r.getrandbits(32)])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
