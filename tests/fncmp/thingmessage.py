# Thing_HandleMessage (src/game/world/things: 0x50b550, thiscall (message)) against the original: a thing whose model byte (+2) is each of
# 0 to 0x14 and 0xff in turn, a message pointer anything; each model's handler the thiscall recorder k of 1 argument, k the model - 1 (models
# 6 and 8 share 0x4d4450: k 5), as tools/fncmp/driver.cpp's kThingMessageEnv has them.
# usage: thingmessage.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

HANDLERS = [0x4fb360, None, 0x4dd320, 0x4da0c0, 0x4d6cf0, 0x4d4450, 0x4d5fb0, 0x4d4450, 0x508a30, 0x4dadc0, 0x59b060, 0x509bf0, 0x4c7370,
            0x503d40, 0x5127a0, 0x4d02d0, 0x50ca50, 0x4f7c50, 0x4d2860]
B = Bench([(0x50b550, 0x50b650)], hooks={a: _recorder(k, 1) for k, a in reversed(list(enumerate(HANDLERS))) if a})
THING = RAM
MODELS = list(range(0x15)) + [0xff]


def case(r):
    thing = bytearray(r.bytes(0x10))
    thing[2] = r.deal(MODELS)
    return Call(0x50b550, [THING, r.getrandbits(32)], {THING: thing, LOG: log_region()}, conv='t')


if __name__ == '__main__':
    B.main(case)
