# Visitor_PostUpdate (src/game/world/visitorsteps, 0x5019f0) against the original, on visitorsteps.py's world (moods(), op State) with its hooks and those of visitorqueue.py's steps:
# 0x501160, 0x4fd570, 0x4fd4e0, 0x4fde50 are recorders here (11-14), which visitorsteps.py cannot do, as its op Queue runs 0x501160. Every state 0-0x15 and past them; the guards' chain
# through the park's things; the queues empty, as a random one can loop for ever natively.
# usage: visitorpostupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, _recorder  # noqa: E402
import visitorsteps as vs  # noqa: E402

B = Bench(vs.RANGES + [(0x4ffbc0, 0x4fff13), (0x5006b0, 0x50086a), (0x4ffff0, 0x5006b0), (0x4e1670, 0x4e168b), (0x5012f0, 0x50138d), (0x4dda20, 0x4de040), (0x4e0860, 0x4e0b90),
                       (0x4e2670, 0x4e2720), (0x502430, 0x502460), (0x519610, 0x519660), (0x51bfc0, 0x51c060), (0x4dedf0, 0x4df150), (0x4e0370, 0x4e03c8), (0x4e2b50, 0x4e2b80),
                       (0x441970, 0x4419b0), (0x436860, 0x436880)],
          hooks={**{k: v for k, v in vs.HOOKS.items() if k != 0x4ddf50},   # (Object_QueuePosition: the steps of the queue call it as it is)
                 0x501160: _recorder(11, 0), 0x4fd570: _recorder(12, 0), 0x4fd4e0: _recorder(13, 1), 0x4fde50: _recorder(14, 1)})
vs.OPS[:] = ['State']


if __name__ == '__main__':
    B.main(vs.case)
