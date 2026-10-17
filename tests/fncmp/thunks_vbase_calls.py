# The adjusting thunks of src/game/core/thunks whose targets are the destructors, deleters and virtual forwarders (0x42a080, 0x564af0, 0x569c70, 0x56a650, 0x579010, 0x5790b0, 0x579140, 0x58c360,
# 0x598790, 0x5987a0, 0x5987e0) and the vertex transforms (0x579060, 0x579070, 0x579080, 0x5790a0: this the first stack argument, which the transforms leave
# unused, so it is given as a mapped address) against the originals. The cases are the targets' own (their benches' case generators, op by op): the call goes to the thunk with this raised by an offset, the
# dword before it holding that offset, so that the thunk lowers it back to what the target is given.
# usage: thunks_vbase_calls.py [CASES [SEED]] [--wild|--both] [--show N]   (env ONLY=42a080,... picks thunks)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import fncmp  # noqa: E402

_init = fncmp.Bench.__init__


def _recording_init(self, ranges, *a, **k):   # the benches below say which code they run: take their ranges and hooks
    self.recorded = (list(ranges), dict(k.get('hooks') or {}))
    _init(self, ranges, *a, **k)


fncmp.Bench.__init__ = _recording_init
import buffers_nodes  # noqa: E402
import counted_rings  # noqa: E402
import deleters_nodes  # noqa: E402
import transform  # noqa: E402
import vdtor  # noqa: E402
import vobj2  # noqa: E402
import vwrap  # noqa: E402
import wavelet_decode  # noqa: E402

fncmp.Bench.__init__ = _init
from fncmp import Bench, Call  # noqa: E402

# (thunk, bench module, its op)
SOURCES = [(0x42a080, deleters_nodes, 'Del60'), (0x56a650, deleters_nodes, 'Del8'), (0x569c70, buffers_nodes, 'Del'), (0x579010, counted_rings, 'V1'), (0x5790b0, counted_rings, 'V2'),
           (0x579140, counted_rings, 'V3'), (0x58c360, vobj2, 'Del'), (0x564af0, vdtor, 'VB'), (0x5987e0, vwrap, 'Rel'), (0x5987a0, wavelet_decode, 'Decode'), (0x598790, wavelet_decode, 'Scaled'),
           (0x579060, transform, 'Vertices'), (0x579070, transform, 'Blocked'), (0x579080, transform, 'Project'), (0x5790a0, transform, 'Concat')]
RANGES, HOOKS = [(a, a + (0x10 if m is transform else 8)) for a, m, _o in SOURCES], {}
for _m in {m for _a, m, _o in SOURCES}:
    RANGES += _m.B.recorded[0]
    HOOKS.update(_m.B.recorded[1])
B = Bench(RANGES, hooks=HOOKS)
PICK = [s for s in SOURCES if not os.environ.get('ONLY') or s[0] in [int(x, 16) for x in os.environ['ONLY'].split(',')]]


def free_slot(regions, this):
    """An offset d and the address this + d - 4 of the dword that holds it, clear of every region of the case."""
    for d in (0x8000, 0x9000, 0xa000, 0xb000, 0xc000, 0x10000, 0x20000):
        slot = this + d - 4
        if all(slot + 4 <= k or k + len(v) <= slot for k, v in regions.items()):
            return d, slot
    raise SystemExit('no room for the offset dword')


def case(r):
    thunk, module, op = r.choice(PICK)
    module.OPS = [op]
    c = module.case(r)
    if module is transform:
        c.args[0] = fncmp.RAM + 0x7000
    this = c.args[0]
    d, slot = free_slot(c.regions, this)
    regions = dict(c.regions)
    regions[slot] = struct.pack('<I', d)
    return Call(thunk, [(this + d) & 0xffffffff] + c.args[1:], regions, conv=c.conv, ret=c.ret, cw=c.cw)


if __name__ == '__main__':
    B.main(case)
