# Scene objects entering (src/game/frontend/sceneenter: 0x5dcfe0, 0x5e1380, 0x5e23f0) against the originals. Recorders (driver.cpp's kSceneEnterEnv): WeatherFx_SetDropCount 16, Render_FogOff 17, Render_SetAmbient 18,
# LightSlot_Set 19 (its colour hashed), Sound_PlayAt 20, LobbyScene_MakeModel 21, RideUser_Acquire 22, Online_ResetConnections 23, ListRequest_Start30 24, Class702da0_Clear 25, Dialog7cc47c_Create 26,
# HolderUser_Acquire 27, Render_FogOn 28, Render_SetFog 29, ListRequest_Start66 30, Sound_ApplySettings 31; Online_Construct 10 (as session.py). The object's slot 9 answers 0 or not in turn, the part's or the
# session's allocation failing now and then (the session then made by the second try), the session there or not, a current item or none when it stays active.
# usage: sceneenter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=View,Class,Scene)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, blob, heap, log_region, put, set_return, strings, vtable  # noqa: E402

CALLS = {'View': 0x5dcfe0, 'Class': 0x5e1380, 'Scene': 0x5e23f0}
B = Bench([(0x5dcfe0, 0x5dd220), (0x5e1380, 0x5e1450), (0x5e23f0, 0x5e2550), (0x48b390, 0x48b410)],
          hooks={0x57d910: _recorder(16, 1), 0x5d9690: _recorder(17, 0), 0x5d9230: _recorder(18, 2), 0x5d92e0: _recorder(19, 4), 0x51bfc0: _recorder(20, 6, cdecl=True), 0x5d8870: _recorder(21, 6),
                 0x5d8130: _recorder(22, 0), 0x5b6070: _recorder(23, 0), 0x5bc7d0: _recorder(24, 2), 0x5d6060: _recorder(25, 0), 0x4b8520: _recorder(26, 0, cdecl=True), 0x5d87d0: _recorder(27, 0),
                 0x5d9680: _recorder(28, 0), 0x5d96a0: _recorder(29, 3), 0x5bcd10: _recorder(30, 3), 0x51bd70: _recorder(31, 0, cdecl=True), 0x5b50b0: _recorder(10, 0)})
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
THIS, VT, ACTIVE, VIEW, GAME, SUB, HOLDER, QUEUE, GAME2, TABLE, INST, ITEM = (RAM + 0x100 * k for k in range(12))
MODEL = RAM + 0xc00


def case(r):
    op = r.deal(OPS)
    log = log_region()
    answers(log, 9, ACTIVE)
    strings(log, 21, 3)
    blob(log, 19, 1, 1)
    for k in (20, 24, 30):
        set_return(log, k, r.getrandbits(32))
    set_return(log, 20, r.getrandbits(32))
    set_return(log, 21, MODEL)
    set_return(log, 10, INST)
    this = bytearray(r.bytes(0xf0))
    put(this, 0, 'I', VT)
    put(this, 0xc, 'I', r.choice([0, ITEM, ITEM]))
    put(this, 0x38, 'I', r.f())
    regions = {LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 1, 2])), THIS: this, VT: vtable({13: 1}), VIEW: r.bytes(0x60), GAME: put(bytearray(r.bytes(0x20)), 0x10, 'I', SUB),
               SUB: put(bytearray(r.bytes(0x20)), 4, 'I', r.f()), HOLDER: put(bytearray(r.bytes(0x20)), 0x14, 'I', QUEUE), QUEUE: r.bytes(0x20), GAME2: r.bytes(0x20), TABLE: r.bytes(0x40),
               INST: r.bytes(0xf0), ITEM: put(bytearray(r.bytes(0x30)), 0x18, 'I', r.randrange(8)), 0xf82884: struct.pack('<I', GAME), 0xf82888: struct.pack('<I', GAME2), 0xf83778: struct.pack('<I', VIEW),
               0xf83774: r.bytes(4), 0xf85614: struct.pack('<I', HOLDER), 0x8bcbac: r.bytes(4), 0x8bcbc8: r.bytes(4), 0x803a48: r.bytes(4), 0x803a4c: struct.pack('<I', TABLE),
               0xf7d88c: struct.pack('<I', r.choice([0, INST])),
               ACTIVE: b''.join(struct.pack('<I', r.choice([0, 0, 0, 1, r.getrandbits(32) | 1])) for _ in range(48))}
    return Call(CALLS[op], [THIS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
