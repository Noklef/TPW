# The scene objects' buttons (src/game/frontend/scenesteps) against the originals. Recorders (driver.cpp's kSceneStepsEnv): LobbyModel_Play 16, FrontEnd_Dialog7cc4b4Child 17, Dialog7cc4b4_ShowTheme 18,
# Setting_ReadMode 19, crt_atexit 20, Class702cb0_NextItem 21, _PreviousItem 22, Online_ResetConnections 23, Dialog7cc47c_Message6On 24, _Message6Off 25, Dialog7cc984_Open 26, Setting_SetMode 27, Class702cb0_Step 28, 0x4b8850 29, UserSlots_Instance 30, Class702da0_Show 31, Class702bb8_Ctor 12, Class702da0_Push 13; the object's vtable recorders;
# Online_Construct 10 (as session.py). The state 0, 2 or another, the guard's bit set or not, the mode setting 2 or another, the session's +0x384 0 or not and the session there or not, the record at [+0xc]'s +0x70 or none.
# usage: scenesteps.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=End,Next,Previous,End2,Open,Mode,Next2,Previous2,Step2,Step1,Back,Done,Take,Take2)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, RAND_SEED, Bench, Call, _recorder, answers, heap, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'End': (0x5e1890, 'a'), 'Next': (0x5e1ee0, 'v'), 'Previous': (0x5e1f40, 'v'), 'End2': (0x5e2360, 'a'), 'Open': (0x5e26a0, 'v'), 'Mode': (0x5e2700, 'v'), 'Next2': (0x5e2770, 'v'), 'Previous2': (0x5e2790, 'v'), 'Step2': (0x5e22d0, 'v'), 'Step1': (0x5e1830, 'v'),
         'Back': (0x5dd610, 'a'), 'Done': (0x5dcfc0, 'a'), 'Take': (0x5dd400, 'v'), 'Take2': (0x5dd7c0, 'v')}
B = Bench([(0x5e1890, 0x5e18d0), (0x5e1ee0, 0x5e1fa0), (0x5e2360, 0x5e23c0), (0x5e26a0, 0x5e26f0), (0x5e2700, 0x5e2750), (0x5e2770, 0x5e27b0), (0x5e22d0, 0x5e2310), (0x5e1830, 0x5e1890), (0x5dd610, 0x5dd650), (0x5dcfc0, 0x5dcfe0), (0x5dd400, 0x5dd490), (0x5dd7c0, 0x5dd7e0), (0x48b390, 0x48b410)],
          hooks={0x5d83f0: _recorder(16, 2), 0x4b8ea0: _recorder(17, 0, cdecl=True), 0x4b90f0: _recorder(18, 0, cdecl=True), 0x550ca0: _recorder(19, 0), 0x67a7d0: _recorder(20, 1, cdecl=True),
                 0x5e1640: _recorder(21, 0), 0x5e1730: _recorder(22, 0), 0x5b6070: _recorder(23, 0), 0x4b8810: _recorder(24, 0, cdecl=True), 0x4b8830: _recorder(25, 0, cdecl=True),
                 0x4c0ae0: _recorder(26, 5, cdecl=True), 0x550d80: _recorder(27, 1), 0x5e0470: _recorder(28, 0), 0x4b8850: _recorder(29, 1, cdecl=True), 0x48bcd0: _recorder(30, 0, cdecl=True), 0x5d61a0: _recorder(31, 2), 0x5b50b0: _recorder(10, 0), 0x5dc3e0: _recorder(12, 2), 0x5d6110: _recorder(13, 2)})
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
THIS, HOLDER, INST, GAME, VT, USERS, QUEUE = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xb00
LIST, ANSWERS = RAM + 0xe00, RAM + 0xf00


def case(r):
    op = r.deal(OPS)
    log = log_region()
    set_return(log, 10, INST)
    set_return(log, 30, USERS)
    set_return(log, 11, r.getrandbits(32))
    this = bytearray(r.bytes(0xf0))
    put(this, 0, 'I', VT)
    put(this, 0xc, 'I', r.choice([HOLDER, HOLDER, 0]) if op in ('Open', 'Step2') else HOLDER)
    put(this, 0x14, 'I', r.choice([0, 0, 2, 2, 1, r.getrandbits(32)]))
    put(this, 0xe4, 'I', r.choice([0, 1, 1, r.getrandbits(32)]))
    put(this, 0xe8, 'I', r.choice([0, 1, 1, r.getrandbits(32)]))
    holder = bytearray(r.bytes(0x80))
    put(holder, 0x70, 'I', r.choice([0, r.getrandbits(32), RAM + 0x400]))
    inst = bytearray(r.bytes(0x400))
    put(inst, 0x384, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions = {LOG: log, HEAP: heap(), THIS: this, HOLDER: holder, INST: inst, GAME: r.bytes(0x20), VT: vtable({10: 1, 17: 0}), USERS: put(bytearray(r.bytes(0x70)), 0x60, 'i', r.choice([-1, -1, 0, 3, 1])), QUEUE: r.bytes(0x20),
               0xf85614: struct.pack('<I', RAM + 0xc00), RAM + 0xc00: put(bytearray(r.bytes(0x20)), 0x14, 'I', QUEUE), RAND_SEED: r.bytes(4), 0xf82884: struct.pack('<I', GAME), 0xf7d88c: struct.pack('<I', r.choice([0, INST, INST])),
               0xfb2758: bytes([r.choice([0, 1, 1, r.getrandbits(8)])]), 0xfb3b7c: struct.pack('<I', r.choice([0, 1, 2, 2, r.getrandbits(32)]))}
    address, ret = CALLS[op]
    if op in ('Take', 'Take2'):
        count = r.randrange(6)
        nodes = bytearray(0x60)
        for k in range(count):
            put(nodes, 0x10 + 8 * k, 'I', r.getrandbits(32))
            put(nodes, 0x14 + 8 * k, 'I', LIST + 0x18 + 8 * k if k + 1 < count else 0)
        put(nodes, 4, 'I', LIST + 0x10 if count else 0)
        answers(log, 16, ANSWERS)
        regions.update({LIST: nodes, HEAP: heap(fail=r.choice([0, 0, 1, 2])), VT: vtable({16: 1}), ANSWERS: b''.join(struct.pack('<I', r.choice([0, 1, r.getrandbits(32)])) for _ in range(8))})
        return Call(address, [THIS, LIST], regions, conv='t', ret=ret)
    if op == 'Done':
        return Call(address, [r.choice([0, 0, 1, r.getrandbits(32)]), THIS, r.getrandbits(32)], regions, ret=ret)
    if op == 'Mode':
        return Call(address, [], regions, ret=ret)
    return Call(address, [THIS], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
