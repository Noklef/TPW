"""tools/trail.py MODULE OP CASEINDEX SEED: the emulator's last steps of one case"""
import importlib
import os
import sys

mod, op, idx, seed = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
os.environ['OPS'] = op
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'tests', 'fncmp'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import x87emu  # noqa: E402

m = importlib.import_module(mod)
cases = m.B.cases(m.case, idx + 1, seed, False)
c = cases[idx]
B = m.B
e = B.emu
e.ranges = None
e.mem.clear()
for k, v in c.regions.items():
    e.write(k, bytes(v))
e.reset()
args = c.args
if c.conv in 'tf' and args:
    e.r['ECX'] = args[0]
    args = args[1:]
for a in reversed(args):
    e.push(a & 0xffffffff)
e.push(x87emu.SENTINEL)
pc = c.address
trail = []
try:
    for _ in range(100000):
        if pc == x87emu.SENTINEL:
            break
        trail.append('%x' % pc)
        pc = e.step(pc)
except Exception as ex:
    print('ERR', ex)
print(' '.join(trail[-30:]))
print({k: '%x' % v for k, v in e.r.items()})
