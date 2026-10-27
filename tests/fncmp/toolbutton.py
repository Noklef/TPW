# The coaster editor's tool buttons (src/game/coaster/edittools: CoasterEdit_ToolButton 0x446fa0, CoasterEdit_EndAll 0x4426a0,
# CoasterEdit_StartChosenTool 0x4455c0) against the original, with all it
# calls: settools.py's world (coaster.py's edit-tool world and RefreshTools dialog, endtool.py's cars, rides' holders and selection; the
# tool in use none, another or the selection, the mode 0x79c624 bit 0 or not, the chosen tools 0x79c638 and the offer 0x79c6e0 any).
# The tool one of bits 0-8 mostly (3, 0, 0x200 and any now and then); the action 1 (on), 2 (off), 4 (all off), none or several.
# EndAll (0x4426a0): that world. Chosen (0x4455c0): that world, the game's +0x3c bits 0 / 1 any.
# usage: toolbutton.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=On,Off,All,Other,EndAll,Chosen)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, put  # noqa: E402
import coaster  # noqa: E402
import endtool  # noqa: E402
import settools  # noqa: E402

B = Bench(sorted(set(coaster.RANGES + endtool.RANGES + [(0x444770, 0x444ab2), (0x442880, 0x442b23), (0x445220, 0x4454ab), (0x446fa0, 0x447341), (0x4426a0, 0x442838), (0x4455c0, 0x445802)])),
          sines=256)
ACTIONS = {'On': [1, 1, 1, 0x11, 0xff01 & ~6], 'Off': [2, 2, 3, 0x12], 'All': [4, 4, 5, 0x14], 'Other': [0, 8, 0x10, 0x80]}
OPS = [k for k in os.environ.get('OPS', ','.join(list(ACTIONS) + ['EndAll', 'Chosen'])).split(',') if k]
OUT = RAM + 0xb000


def case(r):
    op = r.deal(OPS)
    settools.PICK = ['Start']
    c = settools.case(r)
    if op == 'EndAll':
        return Call(0x4426a0, [], c.regions)
    if op == 'Chosen':   # the game's +0x3c bits 0 / 1 any; the out any
        regions = dict(c.regions)
        game = bytearray(regions[settools.GAME])
        put(game, 0x3c, 'I', r.getrandbits(32) & ~3 | r.getrandbits(2))
        regions[settools.GAME] = game
        regions[OUT] = r.bytes(4)
        return Call(0x4455c0, [OUT], regions)
    tool = r.choice([1, 1, 2, 2, 1 << r.randrange(2, 9), 1 << r.randrange(2, 9), 1 << r.randrange(9), 3, 0, 0x200, r.getrandbits(32)])
    action = r.choice(ACTIONS[op]) | r.getrandbits(32) & ~0xff
    return Call(0x446fa0, [tool, action], c.regions)


if __name__ == '__main__':
    B.main(case)
