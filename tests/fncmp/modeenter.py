# A mouse mode entered (src/game/world/mousemode: 0x497bc0) against the original, on focus.py's Enter6fead0 set-up (modes_case: the
# holder [0x7b05e8] set or not, the current mode [0x7b05d8] a recorder object or none, the mode setting 1, the heap failing now and
# then). Ensure6feaa0: the current mode's slot 9 answering 8 half the time (else 9, 1, 0 or any). Tile15 / Tile16 (0x497d20 / 0x497e10):
# the same, the current mode's slot 9 answering 9 mostly; the focus cell's update (focuscell.py's callees) with the game mode 0x19, 0x1a or
# other and the set cell or none (-1), no selected object (0x7c2654); the new tile mode's Assign (mousetile.py's: the tile [0x7b05cc], the
# env's 0x5234d0 / 0x52f200 cdecl recorders 30 and 24); the record 0x1e on recorded_actions.py's recorder. ChatSelect / SetChat (0x4a2ac0 /
# 0x481470): the same set-up (the windows [0x7cb2ac] / [0x7cb2b0] of its tree or none, sent 6), the game object's mode +0x1da738 4 now and
# then, the word 0x7cf4f4 set now and then, the setting's guard 0xfb34c1 set or not (then read from 0x7a1a8c's bits 23-25, the atexit
# logged), the setting 1 or not. The chat entered (leave 0 with the setting 1: Window7cc8bc_EnterChat, chatavatar.py's world) is not
# dealt; with the setting 1 the chat is left only with 0x790ab0 bit 10 set (Window7cc8bc_LeaveChat's early return).
# usage: modeenter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ensure6feaa0,Tile15,Tile16,ChatSelect,SetChat)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import struct  # noqa: E402

import dialog7cc5f4  # noqa: E402
import focus  # noqa: E402
import focuscell  # noqa: E402
import recorded_actions as ra  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _cdecl_log, _recorder, put, set_return  # noqa: E402

CALLS = {'Ensure6feaa0': 0x497bc0, 'Tile15': 0x497d20, 'Tile16': 0x497e10, 'ChatSelect': 0x4a2ac0, 'SetChat': 0x481470}
GAME = RAM + 0x20000   # (the game object [0x7cf83c]: its mode +0x1da738 alone)
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
RANGES = (dialog7cc5f4.merged(focus.RANGES + focus.FACTORY + focus.MODES + [(0x497bc0, 0x497d1c), (0x46cfc0, 0x46cfd0), (0x40f3f0, 0x40f3f6)]
                              + [(0x497d20, 0x497e08), (0x497e10, 0x497ef8), (0x46c580, 0x46c5a0), (0x6b71c0, 0x6b71c6), (0x46d5a0, 0x46d65f)]
                              + focuscell.RANGES + [(0x403be0, 0x403f00), (0x52fad0, 0x52fb00), (0x51ab20, 0x51ab70), (0x412e60, 0x412e6d),
                                                    (0x412e70, 0x412e8f)]
                              + [(0x4a2ac0, 0x4a2b73), (0x481470, 0x481487), (0x516c00, 0x516c0c), (0x4bd260, 0x4bd3f5)]))
HOOKS = {0x5234d0: _recorder(30, 3, cdecl=True), 0x52f200: _recorder(24, 2, cdecl=True), 0x67a7d0: _cdecl_log}
B = Bench(RANGES, hooks=HOOKS)


def chat_case(r, op, c):
    regions = c.regions
    regions[0x7cf83c] = struct.pack('<I', GAME)
    regions[GAME + 0x1da738] = struct.pack('<I', r.choice([4] + [0, 1, 2, 3, 5] * 2 + [r.getrandbits(32)]))
    regions[0x7cf4f4] = struct.pack('<H', r.choice([0] * 8 + [1, r.getrandbits(16)]))
    regions[0xfb34c1] = bytes([r.choice([1, 1, 3, 0, 0, 2, r.getrandbits(8)])])
    regions[0x7a1a8c] = put(bytearray(4), 0, 'I', r.getrandbits(32) & ~(7 << 23) | r.choice([0, 1, 2, 3, 4, 6, 7]) << 23)
    setting = r.choice([1, 1, 0, 2, r.getrandbits(32)])
    regions[0xfb3b7c] = struct.pack('<I', setting)
    leave = r.choice([0, 1, 1, 2, r.getrandbits(32)])
    reads = not regions[0xfb34c1][0] & 1   # (the setting then read: 1 when the highest of the bits 23-25 is 24)
    if reads:
        setting = {0: 0, 1: 0, 2: 1, 3: 1}.get(struct.unpack('<I', regions[0x7a1a8c])[0] >> 23 & 7, 2)
    if op == 'SetChat':
        arg = r.choice([0, 0, 1, 2, r.getrandbits(32)])
        if setting == 1:   # (leave = arg == 0)
            arg = 0
        regions[0x790ab0] = struct.pack('<I', r.getrandbits(32) | 0x400)
        return Call(CALLS[op], [arg], regions, conv='s', ret='v')
    if setting == 1:
        leave = r.choice([1, 2, r.getrandbits(32) | 1])
    regions[0x790ab0] = struct.pack('<I', r.getrandbits(32) | 0x400)
    return Call(CALLS[op], [leave], regions, ret='v')


def case(r):
    op = r.deal(OPS)
    c = focus.case(r, ['Enter6fead0'])
    if op in ('ChatSelect', 'SetChat'):
        return chat_case(r, op, c)
    if op != 'Ensure6feaa0':
        regions = c.regions
        regions[0x81ae2c] = struct.pack('<I', r.choice([0x19, 0x1a, 0x19, 0, 5, r.getrandbits(32)]))
        steps = bytearray(r.bytes(0x58))
        put(steps, 0x50, 'II', r.randrange(0x80), r.randrange(0x80)) if r.random() < 0.6 else put(steps, 0x50, 'I', 0xffffffff)
        regions.update({focuscell.STEPS: steps, focuscell.FOCUS: r.bytes(0x30), 0x818c50: r.bytes(4), 0x7c2654: bytes(4), 0x7cf83c: r.bytes(4),
                        0x7b05cc: struct.pack('<H', r.choice([0, 1, 0x80, 0x81, r.getrandbits(16)])), 0x81eca8: r.bytes(0xc)})
        regions.update(ra.recorder(r))
        return Call(CALLS[op], [], regions, ret='v')
    set_return(c.regions[LOG], 9, r.choice([8, 8, 8, 8, 9, 1, 0, r.getrandbits(32)]))
    return Call(CALLS[op], [], c.regions, ret='v')


if __name__ == '__main__':
    B.main(case)
