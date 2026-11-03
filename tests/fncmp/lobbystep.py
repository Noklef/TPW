# The lobby's step (src/game/frontend/lobbyscene: Class702cb0_Step 0x5e0470) against the original: an object of random floats with a list of up to 4 models
# (their places about the camera, so that the drop count falls under, in and over its limits), a user slot chosen or not, the states 0 to 3 with the angles
# small and finite in the two that turn (their loops never end for a NaN, and take long for a large one), the frame's scale any elsewhere. LobbyModel_Play,
# LobbyModel_TimeLeft, Sound_PlayAt, rand and the burst 0x580320 are recorders (env in driver.cpp's kLobbySceneEnv).
# usage: lobbystep.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, bf, fb, log_region, put, set_return, vtable  # noqa: E402

PLAY, LEFT, BURST, SOUND, RAND = 3, 4, 5, 6, 7   # (driver.cpp's kLobbySceneEnv)
B = Bench([(0x5e0470, 0x5e1208), (0x5e1210, 0x5e1340), (0x48bcd0, 0x48bd3d), (0x423b50, 0x423b58), (0x57d910, 0x57d91d), (0x57dc70, 0x57dc78), (0x67a830, 0x67a860)],
          hooks={0x5d83f0: _recorder(PLAY, 2), 0x5d8440: _recorder(LEFT, 0), 0x580320: _recorder(BURST, 8), 0x51bfc0: _recorder(SOUND, 6, cdecl=True), 0x67b5c0: _recorder(RAND, 0, cdecl=True)})
OBJECT, VTABLE, LIST, NODES, MODELS, FRAME, PLACE, CAMERA, USERS, SOUNDS, ANSWERS, WEATHER = (RAM, RAM + 0x100, RAM + 0x200, RAM + 0x240, RAM + 0x300, RAM + 0x600, RAM + 0x640, RAM + 0x680,
                                                                                               RAM + 0x700, RAM + 0x780, RAM + 0x7c0, RAM + 0x1000)
MODEL_SIZE = 0x70


def near(r, scale=100.0):
    return fb(r.uniform(-scale, scale))


def model(r, at):
    m = bytearray(r.bytes(MODEL_SIZE))
    put(m, 4, '2I', at + 0x50, at + 0x54)    # (the lobby models, their pointers only logged)
    put(m, 0x14, 'I', fb(r.uniform(-20, 20)))
    put(m, 0x18, 'I', r.randrange(4))
    for off in (0x3c, 0x40, 0x44):
        put(m, off, 'I', r.choice([near(r), near(r), near(r), near(r, 3000.0), r.f()]))
    put(m, 0x5c, 'I', r.choice([0, 0xffffffff, 1, 1, 1, 3, 5, 0x10000]))
    put(m, 0x60, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    return m


def case(r):
    nodes = r.randrange(5)
    count = r.choice([nodes, nodes, nodes, nodes, 0, 0xffffffff])
    state = r.choice([0, 0, 1, 1, 1, 2, 2, 2, 3])
    follow = r.random() < 0.6
    turning = follow and state in (0, 1)

    models = b''.join(model(r, MODELS + i * MODEL_SIZE) for i in range(5))
    pick = r.choice([0, 1, 2, 3, 4])

    o = r.floats(bytearray(r.bytes(0x100)), range(0x18, 0xe4, 4), r.choice([1, 100]))
    for off in (0x78, 0x7c, 0x80, 0x84, 0x88, 0x8c, 0xb4, 0xb8, 0xbc, 0xc0, 0xc4, 0xc8):      # sometimes on their targets
        if r.random() < 0.8:
            put(o, off, 'I', near(r, 10.0))
    if r.random() < 0.15:
        o[0x84:0x90] = o[0x78:0x84]
    if r.random() < 0.15:
        o[0xc0:0xcc] = o[0xb4:0xc0]
    if r.random() < 0.3:      # (the arrivals: far enough)
        put(o, 0x74, 'I', fb(1e6))
    if r.random() < 0.3:
        put(o, 0xb0, 'I', fb(1e6))
    speed = r.choice([0, 1.0, -1.0, 5.0])
    put(o, 0xac, 'I', fb(speed))
    put(o, 0xa8, 'I', fb(r.choice([speed, 2.0, 0.5, 8.0])))
    put(o, 0, 'I', VTABLE)
    put(o, 8, 'I', LIST)
    put(o, 0xc, 'I', 0 if r.random() < (0 if follow else 0.8) else MODELS + pick * MODEL_SIZE)
    put(o, 0x14, 'I', state)
    put(o, 0x18, 'I', fb(r.uniform(-20, 20)))
    put(o, 0x20, 'I', r.choice([fb(r.uniform(0, 8)), fb(r.uniform(8, 40)), fb(8.0), r.f()]))

    list_ = bytearray(r.bytes(0x10))
    put(list_, 4, 'I', NODES if nodes else 0)
    put(list_, 0xc, 'I', count)
    links = bytearray(0x28)
    for i in range(nodes):
        put(links, i * 8, '2I', MODELS + i * MODEL_SIZE, NODES + (i + 1) * 8 if i + 1 < nodes else 0)

    frame = bytearray(r.bytes(0x40))
    put(frame, 8, 'I', fb(r.uniform(0, 2)) if turning else r.choice([fb(1.0), 0, fb(r.uniform(0, 2)), r.f()]))
    put(frame, 0x10, 'I', PLACE)
    if turning and state == 1:
        put(frame, 8, 'I', r.choice([fb(r.uniform(0, 2)), fb(1.0), fb(r.uniform(2, 100)), 0, 0x7fc00000]))
    place = r.floats(bytearray(r.bytes(0x20)), (8, 0xc, 0x10), 10)
    if turning:
        put(place, 8, 'I', fb(r.uniform(-5, 5)))
    if r.random() < 0.5:
        put(place, 0xc, 'I', fb(r.uniform(8, 40)))
    elif r.random() < 0.3:     # (no range: the division by 1, which shows only with a little way left)
        put(place, 0xc, 'I', fb(8.0))
        put(o, 0x20, 'I', fb(r.uniform(8, 9)))
        put(frame, 8, 'I', 0)

    camera = bytearray(r.bytes(0x70))
    for off in (8, 0xc, 0x10):
        put(camera, off, 'I', near(r))

    users = bytearray(r.bytes(0x70))
    put(users, 0x60, 'I', r.getrandbits(8) if follow else r.choice([0xffffffff, 0xffffffff, 0]))
    if not follow and struct.unpack_from('<I', users, 0x60)[0] != 0xffffffff:
        put(o, 0xc, 'I', 0)

    log = log_region()
    table = struct.pack('<40I', *[r.choice([0, 0, 0xffffffff, 1, 500]) for _ in range(40)])
    answers(log, LEFT, ANSWERS)
    set_return(log, RAND, r.getrandbits(32))
    regions = {OBJECT: o, VTABLE: vtable({}), LIST: list_, NODES: links, MODELS: models, FRAME: frame, PLACE: place, CAMERA: camera, USERS: users,
               SOUNDS: r.bytes(0x20), ANSWERS: table, WEATHER: bytearray(0x3320), LOG: log, 0xf82884: struct.pack('<I', FRAME), 0xf83778: struct.pack('<I', CAMERA),
               0xf81ebc: struct.pack('<I', USERS), 0x8bcbac: struct.pack('<I', WEATHER), 0x803a48: r.bytes(4), 0x803a4c: struct.pack('<I', SOUNDS)}
    return Call(0x5e0470, [OBJECT], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
