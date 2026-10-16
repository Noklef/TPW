# The online session's getters (src/game/online/session: 0x5b6450-0x5b64c0) against the
# originals: logged in or not, the user's flags with and without 0x20.
# usage: session.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=LoggedIn,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5b6450, 0x5b64c4)])
CALLS = {'LoggedIn': 0x5b6450, 'Flag20': 0x5b6460, 'String14': 0x5b6480, 'String28': 0x5b6490,
         'UserName': 0x5b64a0, 'UserFlags': 0x5b64b0, 'User5c': 0x5b64c0}


def case(r):
    s = bytearray(r.bytes(0x80))
    put(s, 0x60, 'I', r.choice([0, 1, 0x100, r.getrandbits(32)]))
    put(s, 0x58, 'I', r.choice([0, 0x20, 0x1f, 0x2000, r.getrandbits(32)]))
    return Call(CALLS[r.choice(OPS)], [RAM], {RAM: s}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
