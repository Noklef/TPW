# The message box (src/game/ui/dialogbase: 0x47f020, 0x47eed0) against the originals, all callees recorders (the slots' answers in turn, a callback of its own): the record, kinds and callbacks any, the
# memory failing now and then, no window now and then, the keys kept or not, the two globals either; the handler's messages 0x14, 0x100 and others, a button or another id.
# usage: dialogbox.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, mask_this, recorder, set_return  # noqa: E402

CD = {0x47ed80: (1, 3), 0x67acc0: (3, 1), 0x485a70: (8, 1), 0x659bc9: (13, 1), 0x486b80: (17, 0), 0x486b70: (18, 0), 0x662411: (20, 0), 0x658d9f: (23, 4), 0x486b60: (24, 0), 0x662420: (26, 0), 0x658ccc: (28, 4), 0x4862a0: (29, 0)}
TH = {0x659d00: (2, 0), 0x48f6a0: (4, 1), 0x65d1c8: (5, 1), 0x65c428: (6, 2), 0x65c4b9: (7, 2), 0x65c388: (9, 1), 0x65c5d5: (10, 4), 0x65f5b7: (11, 1), 0x65d7d0: (12, 1), 0x65d070: (14, 1), 0x65d9dd: (15, 1), 0x65e6d5: (16, 2),
      0x4092a0: (19, 2), 0x659d90: (21, 0), 0x65e705: (22, 1), 0x409300: (25, 0), 0x65d55a: (27, 0), 0x65f66e: (30, 3)}
HOOKS = {a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()}
HOOKS.update({a: _recorder(k, n) for a, (k, n) in TH.items()})
B = Bench([(0x47f020, 0x47f2b0), (0x47eed0, 0x47f017)], hooks=HOOKS)
RECORD, TABLE, WINDOW = RAM, RAM + 0x100, RAM + 0x300
SLOT, CALLBACK = 22, 31


def globals_(r, regions):
    regions[0x786ba4] = struct.pack('<I', r.choice([0, 1, 1, 2]))
    regions[0xf82884] = struct.pack('<I', r.choice([0, 0, 1]))


def case(r):
    log = log_region()
    regions = {LOG: log}
    globals_(r, regions)
    if r.randrange(2):
        set_return(log, 1, r.choice([WINDOW, WINDOW, WINDOW, WINDOW, 0]))
        set_return(log, 3, r.choice([RAM + 0x400, RAM + 0x400, RAM + 0x400, 0]))
        set_return(log, 4, r.getrandbits(32))
        for k in 8, 12, 13, 17:
            set_return(log, k, r.getrandbits(32))
        mask_this(log, 2)
        mask_this(log, 21)
        kinds = [r.choice([0, 1, 2, 3, r.getrandbits(32)]) for _ in range(3)]
        return Call(0x47f020, [RECORD, kinds[0], r.getrandbits(32), kinds[1], r.getrandbits(32), kinds[2], r.getrandbits(32), r.choice([0, 0x100, 1, r.getrandbits(32)])],
                    {**regions, RECORD: r.bytes(0x10)}, conv='c', ret='a')
    message = r.choice([0x14, 0x14, 0x100, 0x100, 0x100, 0x1000a, r.getrandbits(32)])
    a = r.choice([0x9873c7, 0x9873c8, 0x9873c9, 0x9873ca, 0x9873cb, r.getrandbits(32)]) if message == 0x100 else r.getrandbits(32)
    if message == 0x100:
        table = [r.getrandbits(32), r.choice([0, recorder(CALLBACK, 1, cdecl=True), recorder(CALLBACK, 1, cdecl=True)])]
    else:
        table = [r.choice([-1, 0, 1, r.getrandbits(32)]), r.choice([0, 1, r.getrandbits(32)]), r.choice([0, 1, r.getrandbits(32)])]
    regions[TABLE] = struct.pack('<40I', *[v & 0xffffffff for v in table + [0] * (40 - len(table))])
    answers(log, SLOT, TABLE)
    set_return(log, CALLBACK, r.choice([0, 1, r.getrandbits(32)]))
    set_return(log, 27, r.getrandbits(32))
    set_return(log, 30, r.getrandbits(32))
    return Call(0x47eed0, [WINDOW, message, a, r.getrandbits(32)], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
