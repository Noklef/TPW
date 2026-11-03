# The dialog 0x7ca1c8's handler and Hide (src/game/ui/dialog7ca1c8: 0x4982a0, 0x4989d0) against the originals: every call a recorder numbered as Dialog7ca1c8HandlerEnv from 10. CoasterEdit_Tools' out dword filled, the two action tables' answers in turn.
# usage: dialog7ca1c8handler.py [CASES [SEED]] [--wild|--both] [--show N]
import struct
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, set_return, strings  # noqa: E402

B = Bench([(0x4982a0, 0x498755), (0x498758, 0x49877c), (0x4989d0, 0x498a2c)],
          hooks={0x497d20: _recorder(10, 0, cdecl=True), 0x497e10: _recorder(11, 0, cdecl=True), 0x497bc0: _recorder(12, 0, cdecl=True), 0x4a2aa0: _recorder(13, 1, cdecl=True), 0x446fa0: _recorder(14, 2, cdecl=True), 0x445910: _recorder(15, 1, cdecl=True), 0x486b00: _recorder(16, 1, cdecl=True),
                 0x40c900: _recorder(17, 2), 0x40c990: _recorder(18, 2), 0x497f00: _recorder(19, 0, cdecl=True), 0x4474d0: _recorder(20, 2, cdecl=True), 0x4862a0: _recorder(21, 0, cdecl=True), 0x486b40: _recorder(22, 1, cdecl=True), 0x65f66e: _recorder(23, 3), 0x4a25f0: _recorder(24, 1, cdecl=True)})
WINDOW, TABLE_A, TABLE_B, ANSWERS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300


def case(r):
    op = r.deal(['Handler', 'Handler', 'Handler', 'Handler', 'Hide'])
    log = log_region()
    regions = {LOG: log}
    regions[0x7ca1cc] = struct.pack('<I', r.choice([0, 1, r.getrandbits(32)]))
    regions[0x785a2c] = struct.pack('<I', r.choice([0, 0, 1]))
    if op == 'Hide':
        return Call(0x4989d0, [], regions, ret='v')
    regions[0x7ca1c8] = r.bytes(4)
    regions[0x7ca1d4] = r.bytes(1)
    regions[0x787c14] = struct.pack('<I', TABLE_A)
    regions[0x787294] = struct.pack('<I', TABLE_B)
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([0, 0, 1, r.getrandbits(32)]) for _ in range(40)])
    answers(log, 17, ANSWERS)
    answers(log, 18, ANSWERS)
    set_return(log, 23, r.getrandbits(32))
    fill(log, 15, 0, struct.pack('<I', r.choice([0, 0x200, 0x1fd, r.getrandbits(32)])))
    strings(log, 15, 0, 1 << 0)
    message = r.deal([0x14, 0x1d, 0x1d, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x1000a, 0x1000a, 0x1000b, 0x1000b, 0x1001d8, 0x1001d8, 0x13, 0x15, 0x100, 0x102, 0x10009, 0x1000c, 0x1001d7, 0x1001d9, r.getrandbits(32)])
    a = r.randrange(0x18, 0x23) if message == 0x101 else r.choice([0, 1, 1, r.getrandbits(32)])
    b = r.choice([1, 1, 0, 2, 0x10001, r.getrandbits(32)])
    return Call(0x4982a0, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
