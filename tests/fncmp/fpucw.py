# The renderer's precision switches (src/game/core/fpu: 0x591220-0x591250) and the control word guard's merge and restore (0x62f980,
# 0x62f9a0: a guard of a saved word and changed masks, none or any) against the originals: the control word they return and the one they leave. The words loaded keep the exceptions
# masked (the game's never unmask) and the reserved bits as the hardware reads them back.
# usage: fpucw.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Single,Load,LoadFull,Merge,Restore)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x591220, 0x591261), (0x62f980, 0x62f9d5)])
CALLS = {'Single': 0x591220, 'Load': 0x591240, 'LoadFull': 0x591250, 'Merge': 0x62f980, 'Restore': 0x62f9a0}


def case(r):
    op = r.deal(OPS)
    def word():
        return (r.getrandbits(32) & 0xffff1f00) | 0x7f
    if op in ('Merge', 'Restore'):  # the guard: vtable, saved word (+4), changed masks (+6, +8)
        guard = bytearray(r.bytes(0x10))
        masks = [r.choice([0, 0, r.getrandbits(16), 0x300, 0xc00, 0x1f00]) for _ in range(2)]
        guard[4:10] = (r.getrandbits(16) & 0x1f00 | 0x7f).to_bytes(2, 'little') + masks[0].to_bytes(2, 'little') + masks[1].to_bytes(2, 'little')
        if op == 'Merge':
            return Call(CALLS[op], [RAM, word()], {RAM: guard}, conv='t', ret='w')
        return Call(CALLS[op], [RAM], {RAM: guard}, conv='t', ret='c', cw=word() & 0xffff)
    args = [] if op == 'Single' else [word()]
    return Call(CALLS[op], args, {RAM: bytes(4)}, ret='wc' if op == 'Single' else 'c', cw=word() & 0xffff)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
