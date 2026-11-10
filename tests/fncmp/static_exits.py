# The empty atexit entries of static objects whose destructors do nothing (each a bare RET, registered by its unit's static initialiser):
# SpriteFolderName_AtExit (src/game/sprite/loading, 0x540bc0), RendererFrame_MatrixAtExit (src/game/render/rendererframe, 0x5774b0),
# Transform_StaticAtExit (src/game/math/transform, 0x579b30), MouseModeText_AtExit (src/game/world/mousemode, 0x57cb00) and
# LightList_AtExit (src/game/render/lightlist, 0x584b00) against the originals, each over random bytes at its object.
# usage: static_exits.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

# name: (address, its object)
CALLS = {'SpriteFolderName': (0x540bc0, 0x875058), 'RendererFrameMatrix': (0x5774b0, 0x87a680), 'Transform': (0x579b30, 0x8bb1f8),
         'MouseModeText': (0x57cb00, 0x8bba50), 'LightList': (0x584b00, 0x8c1818)}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(a, a + 1) for a, _ in CALLS.values()])


def case(r):
    address, obj = CALLS[r.deal(OPS)]
    return Call(address, [], {obj: r.bytes(0x20)}, ret='v')


if __name__ == '__main__':
    B.main(case)
