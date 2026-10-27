# The coaster editor's tools switched (src/game/coaster/edittools: CoasterEdit_SwitchToTool1 0x444770, CoasterEdit_SetTools 0x442880,
# CoasterEdit_StartTool 0x445220)
# against the originals, with all they call. The world is coaster.py's: its edit-tool world (EditBounds' link as the game, moved to a
# block of 0x150 of its own, its pieces on rides 3 / 4; the recorder, its stream moved clear of the dialog's) and its RefreshTools
# dialog (up or not), with endtool.py's cars (+0x54 / +0x64 / +0x144) and rides' holders. The tool in use none / another (0, 3, 6,
# 0x3f) or the selection (0x20, endtool.py's, flag 0x80 or not); the mode 0x79c624 bit 0 or not; the chosen tools 0x79c638 and the
# offer 0x79c6e0 any. SetTools' tools: one of bits 0-9, a few, none or any; StartTool's
# tool: 1 or 2 mostly, one of bits 2-9, 3, none or any.
# usage: settools.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, put  # noqa: E402
import coaster  # noqa: E402
import endtool  # noqa: E402
import recorded_actions  # noqa: E402

B = Bench(sorted(set(coaster.RANGES + endtool.RANGES + [(0x444770, 0x444ab2), (0x442880, 0x442b23), (0x445220, 0x4454ab)])), sines=256)
GAME, VT, BUF, SEL = RAM + 0x9000, RAM + 0x9200, RAM + 0x9240, RAM + 0xa000
PIECES = RAM + 0x6800   # (coaster.edit_bounds's: four, 0x200 apart)
OPS = {'Switch': 0x444770, 'Set': 0x442880, 'Start': 0x445220}


def case(r):
    op = r.deal(PICK)
    tool = dict(coaster.edit_tool(r, 'Tool1').regions)
    regions = dict(coaster.refresh_tools(r, 'RefreshTools').regions)
    for k in (0x790fec, 0x79c638, 0x79c6e0, RAM + 0x10000, RAM + 0x10100, RAM + 0x10200):   # (its link: the edit world's instead)
        regions.pop(k, None)
    rec = bytearray(tool.pop(recorded_actions.RECORDER))
    put(rec, 0xc, 'II', VT, BUF)
    regions.update({recorded_actions.RECORDER: rec, VT: tool.pop(recorded_actions.VT), BUF: tool.pop(recorded_actions.BUF)})
    link = struct.unpack('<I', tool.pop(0x790fec))[0]
    game = bytearray(tool.pop(link))
    game += bytearray(r.bytes(0x150 - len(game)))
    for off in (0x54, 0x64):
        put(game, off, 'I', r.choice([endtool.FLASH, endtool.FLASH, endtool.KEPT, 0]))
    put(game, 0x144, 'I', r.choice([0, endtool.KEPT, endtool.FLASH]))
    pieces = bytearray(tool[PIECES])
    for k in range(4):
        put(pieces, 0x200 * k + 0x70, 'I', r.choice([3, 4]))
    tool[PIECES] = pieces
    glob = bytearray(r.bytes(0x110))
    for a in (0x79c644, 0x79c658, 0x79c664, 0x79c6a4):
        glob[a - 0x79c600:a - 0x79c600 + 4] = tool.pop(a)
    regions.update(tool)
    kind = r.choice([0, 0, 3, 6, 0x3f, 0x20, 0x20])
    flags = struct.unpack_from('<I', glob, 0x58)[0] & ~0xbf | kind
    if kind == 0x20:
        regions.update(endtool.selection(r, glob, SEL))
        flags |= r.choice([0, 0x80, 0x80])
    put(glob, 0x58, 'I', flags)
    glob[0x24] = r.getrandbits(8) & ~1 | r.choice([0, 1])
    put(glob, 0x38, 'I', r.choice([1 << r.randrange(10), r.getrandbits(10), 0, r.getrandbits(32)]))
    regions.update({GAME: game, 0x790fec: GAME.to_bytes(4, 'little'), 0x79c600: glob,
                    0x7a1ce0: r.choice([bytes(4), r.bytes(4)]), 0x7a1c7c: r.bytes(4), 0x7925d0: r.bytes(4)})
    endtool.rides(r, regions)
    if op == 'Switch':
        return Call(0x444770, [], regions)
    if op == 'Start':
        tool = r.choice([1, 1, 2, 2, 2, 1 << r.randrange(2, 10), 1 << r.randrange(2, 10), 3, 0, r.getrandbits(32)])
        return Call(0x445220, [tool], regions)
    tools = r.choice([1, 2, 3, 4, 1 << r.randrange(10), 1 << r.randrange(10), r.getrandbits(10), 0, r.getrandbits(32)])
    return Call(0x442880, [tools], regions)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
