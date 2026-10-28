# A path cell joined to the cells about it (src/game/world/pathlink: MapCell_LinkPath 0x5348d0) against the original. cellkind.py's Clear
# scene (pathapply.py's cell and models, the 5 x 5 cells about it in [0x80239c]'s map, the entries, owners and things) with the side cells'
# directions (+0xd) now and then a side's exactly; the cell of kind 3, 1 or others for mode 1, any for modes 0 and 3 (the kinds 0 and 3
# then common about it, the side cells now and then of one neighbour or none, the cell's bits any); models' low byte set or not. The sound (Sound_PlayAt): sound started or not ([0x802bc8], [0x802bd4]), the
# manager [0x802bcc] whose vtable +8 is a recorder (2 of 2) logging the command's seven dwords; the game free (0x785a2c) or not. The drag
# (modes 0 and 3): 0x820aa4 set or not, the stored step (0x763ac0 / 0x763ac4) and dx, dy -1, 0, 1, 0x80 or any; [0x8186ac] +0x36 the id
# of a side cell or another, [0x7cf83c] now and then [0x80239c]'s world (the drag reads it for that cell only) so that it can be one;
# the owner word 0x818698 a side cell's, 0 or any. Modes 2 and any do nothing but store the step.
# Reset (0x536490 MapCell_Reset): mode 1's world, the cell's attribute byte (+0x26) any bits of 1, 2, 4, 8, 0x10, 0x80, models as Link's.
# usage: pathlink.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Link,Reset)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, blob, put, vtable  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import cellkind  # noqa: E402
import pathapply  # noqa: E402
import pathmodel  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

cellkind.OPS = ['Clear']
W2 = pathmodel.W2
MANAGER, VT, SEL = RAM + 0x58000, RAM + 0x58100, RAM + 0x58200
RANGES = pathapply.merged(cellkind.RANGES + [(0x5348d0, 0x535600), (0x51bfc0, 0x51c050), (0x67a830, 0x67a860), (0x536490, 0x5365cf)])
B = Bench(RANGES, sines=256, hooks=cellkind.HOOKS)
SIDES = {(0, -1): 0x10, (1, 0): 0x40, (0, 1): 1, (-1, 0): 4}   # (the side cells and the direction facing back)


def spot(regions, x, y, xx, yy):
    """The row region and offset of the cell at (xx, yy) of cellkind.py's 5 x 5 about (x, y); None outside."""
    if not (0 <= xx < 128 and 0 <= yy < 128):
        return None
    x0 = max(0, x - 2)
    key = W2 + 0x294 + cellkind.ident(x0, yy) * 0x44
    return (key, (xx - x0) * 0x44) if key in regions else None


def case(r):
    regions = dict(cellkind.case(r).regions)
    cell = bytearray(regions[pathmodel.CELL])
    h = struct.unpack_from('<H', cell)[0]
    x, y = (h - 1) & 0x7f, (h - 1) >> 7
    op = r.deal(OPS)
    mode = 1 if op == 'Reset' else r.choice([1] * 8 + [0, 0, 3, 3, 2, r.getrandbits(32)])
    owners = [0, r.getrandbits(16)]
    ids = [h, r.getrandbits(16)]
    for (dx, dy), back in SIDES.items():
        at = spot(regions, x, y, x + dx, y + dy)
        if at is None:
            continue
        row = bytearray(regions[at[0]])
        if r.random() < 0.5:
            row[at[1] + 0xd] = r.choice([back, back, 1, 4, 0x10, 0x40])
        if mode in (0, 3) and r.random() < 0.4:
            put(row, at[1] + 8, 'I', mode)
        if mode in (0, 3) and r.random() < 0.5:   # (under two neighbours: the drag joins it)
            row[at[1] + 0xc] = row[at[1] + 0x22] = r.choice([0, back, 1 << r.randrange(8)])
        regions[at[0]] = row
        owners.append(struct.unpack_from('<H', row, at[1] + 0x10)[0])
        ids.append(cellkind.ident(x + dx, y + dy))
    if mode == 1:
        put(cell, 8, 'I', r.choice([3, 3, 3, 1, 1, 0, 9, 0xa, r.getrandbits(32)]))
    else:
        put(cell, 8, 'I', r.choice([1, 1, 1, 3, 0, 9, r.getrandbits(32)]))
        if r.random() < 0.5:   # (the diagonals a drag drops)
            cell[0xc], cell[0x22] = r.getrandbits(8), r.getrandbits(8)
    regions[pathmodel.CELL] = cell
    if mode in (0, 3) and r.random() < 0.5:
        regions[0x7cf83c] = struct.pack('<I', W2)
    sel = bytearray(r.bytes(0x40))
    put(sel, 0x36, 'H', r.choice(ids[2:] * 3 + ids[:2]) if len(ids) > 2 else r.choice(ids))
    step = [-1, 0, 1, 1, -1, 0x80, r.getrandbits(32) - (1 << 31)]
    stored = [-1, 0, 1, 1, -1, 0x80, 0x80, r.getrandbits(32) - (1 << 31)]
    log = regions[LOG]
    blob(log, 2, 1, 0x7f)
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1]))
    regions.update({0x8186ac: struct.pack('<I', SEL), SEL: sel, 0x820aa4: struct.pack('<I', r.choice([0, 1])),
                    0x763ac0: struct.pack('<2i', *(r.choice(stored) for _ in range(2))), 0x818698: struct.pack('<H', r.choice(owners)),
                    0x803a2c: r.bytes(4), 0x802bc0: g, MANAGER: struct.pack('<I', VT) + r.bytes(12), VT: vtable({2: 2}, 4)})
    models = r.choice([0, 1, 1, 0x100, r.getrandbits(32)])
    if op == 'Reset':
        cell[0x26] = sum(b for b in (1, 2, 4, 8, 8, 0x10, 0x80) if r.random() < 0.4) & 0xff | (r.getrandbits(8) & 0x60)
        return Call(0x536490, [pathmodel.CELL, models], regions, conv='t', ret='v')
    return Call(0x5348d0, [pathmodel.CELL, mode, r.choice(step), r.choice(step), models], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Link,Reset').split(',') if k]


if __name__ == '__main__':
    B.main(case)
