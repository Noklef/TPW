# DialupList_Fill (src/game/ui/dialuplist, 0x4ba850) against the original. Recorders (DialupListEnv): the import slots 0x6fd5c0 10 and 0x6fd5bc 11 (it fills the properties), Window7059f0_Clear 12, Window_SetShown 13, Window7059f0_AddItem 14 (the row's second dword and its text hashed), Window7059f0_RefreshRows 15, Language_WideText 16,
# 0x67c430 17 (answers from a table: 0-3 words, then none). The list by hand among decoys; [0x7c24d4] clear now and then; 0-3 connections, each usable or not.
# usage: dialuplist.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob_bytes, fill, log_region, put, recorder, set_return, strings  # noqa: E402

B = Bench([(0x4ba850, 0x4ba97c), (0x65d7bc, 0x65d840)],
          hooks={0x6649d5: _recorder(12, 0), 0x65d9dd: _recorder(13, 1), 0x66403b: _recorder(14, 4), 0x663324: _recorder(15, 0), 0x485b00: _recorder(16, 1, cdecl=True), 0x67c430: _recorder(17, 2, cdecl=True)})
DIALOG, KIDS, TABLE, WORDS = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000


def case(r):
    log = log_region()
    blob_bytes(log, 14, 0, 0xf0, inner=(0, 0x7fff))
    strings(log, 11, 0, out=0b10)
    set_return(log, 10, r.choice([0, 0, 1, 2, 3, 0x80000000]))
    records = b''.join(struct.pack('<4I', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.choice([0, 1, r.getrandbits(32)])) + r.bytes(0x10) for _ in range(3))
    fill(log, 11, 1, records, records=3)
    answers(log, 17, TABLE)
    words = r.choice([0, 1, 2, 3])
    dialog = put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS)
    ids = [0xc, r.getrandbits(32) | 0x1000]
    r.shuffle(ids)
    kids = bytearray(r.bytes(0x200))
    for k, ident in enumerate(ids):
        put(kids, 0x100 * k + 0x54, 'I', KIDS + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(kids, 0x100 * k + 0x8c, 'I', ident)
    regions = {LOG: log, DIALOG: dialog, KIDS: kids, TABLE: struct.pack('<40I', *([WORDS + 0x20 * k for k in range(words)] + [0] * (40 - words))), WORDS: r.bytes(0x100), 0x7cc530: struct.pack('<I', DIALOG), 0x7c24d4: struct.pack('<I', r.choice([0, 1, 1, 1, r.getrandbits(32)])),
               0x6fd5c0: struct.pack('<I', recorder(10, 0)), 0x6fd5bc: struct.pack('<I', recorder(11, 2))}
    return Call(0x4ba850, [r.choice([0, 1, 1, 1, 0x100, 0x101, r.getrandbits(32)])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
