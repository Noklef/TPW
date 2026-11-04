# The front end's entry points (src/game/ui/frontendentries) against the originals: every function they reach a recorder as driver.cpp's kFrontEndEntriesEnv, each with its own return.
# usage: frontendentries.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

# (a recorder stub's slot is 0-31 for each kind and argument count, and the log tells cdecl from thiscall: 0x505490's slot and SEND's are shared with a plain one)
PLAIN = {0x48a580: 0, 0x489ca0: 1, 0x489ec0: 2, 0x489f50: 3, 0x48c980: 4, 0x48a410: 5, 0x48a520: 6, 0x48a6e0: 7, 0x662420: 8, 0x4a27c0: 9, 0x4a27f0: 10, 0x662411: 11,
         0x49ac60: 12, 0x49fb30: 13, 0x4b2750: 14, 0x498d80: 15, 0x4aa480: 16, 0x4989d0: 17, 0x497bc0: 18, 0x486510: 19, 0x486630: 20, 0x4b4280: 21, 0x4bbbd0: 22}
ONE = {0x486d90: 23, 0x48c830: 24, 0x4b3f60: 25, 0x48ac40: 26, 0x48ae70: 27, 0x4bfc70: 28}
METHOD = {0x5f0b40: 29, 0x48cf70: 30, 0x4cf3e0: 31, 0x505490: 22}
SEND = 1
hooks = {a: _recorder(k, 0, cdecl=True) for a, k in PLAIN.items()}
hooks.update({a: _recorder(k, 1, cdecl=True) for a, k in ONE.items()})
hooks.update({a: _recorder(k, 0) for a, k in METHOD.items()})
hooks[0x658d9f] = _recorder(SEND, 4, cdecl=True)
# (a jump's range: its 5 bytes)
JUMPS = [0x4813b0, 0x4813c0, 0x4813d0, 0x481420, 0x481430, 0x481440, 0x481450, 0x481460, 0x481510, 0x481750, 0x481760, 0x481770, 0x481780, 0x4817e0, 0x4819e0, 0x498780, 0x4b6490, 0x4b6b80, 0x4b7a20, 0x4cf3d0, 0x4da5a0]
BIGGER = [(0x4813e0, 0x4813eb), (0x4814f0, 0x481500), (0x4816d0, 0x4816db), (0x4817c0, 0x4817de), (0x481b50, 0x481b60), (0x481b60, 0x481b70), (0x48a700, 0x48a714), (0x48a720, 0x48a739), (0x4b8ea0, 0x4b8eba), (0x4bf9d0, 0x4bf9e6)]
B = Bench([(a, a + 5 + (5 if a == 0x481510 else 0)) for a in JUMPS] + BIGGER, hooks=hooks)
METHODS = [0x4b7a20, 0x4cf3d0, 0x4da5a0]
STDCALLS = [0x4814f0, 0x481b50, 0x481b60]
CDECLS = [a for a in JUMPS if a not in METHODS] + [0x4813e0, 0x4816d0, 0x4817c0, 0x48a700, 0x48a720, 0x4b8ea0]


def case(r):
    log = log_region()
    regions = {LOG: log, RAM: r.bytes(0x40), 0x7cc418: struct.pack('<I', r.deal([0, 1, 2])), 0x7c2658: r.bytes(4), 0xfaa5d8: struct.pack('<I', r.deal([0, 1, r.getrandbits(32)])),
               0x7cb2cc: struct.pack('<I', r.deal([0, 1])), 0x7cc4b4: r.bytes(4)}
    for k in range(32):
        set_return(log, k, r.getrandbits(32))
    make = r.deal([0, 0, 0, 0, 1, 2, 3])
    if make == 0:
        return Call(r.choice(CDECLS), [], regions, ret='a')
    if make == 1:
        return Call(r.choice(METHODS), [RAM], regions, conv='t', ret='a')
    if make == 2:
        return Call(r.choice(STDCALLS), [r.getrandbits(32)], regions, conv='s', ret='a')
    return Call(0x4bf9d0, [r.deal([0, 0, r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
