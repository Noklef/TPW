# A coaster edited or left (src/game/coaster/edittools: 0x447520 CoasterEdit_SetEditing, 0x4474d0 CoasterEdit_EditRide) against the
# originals. Leave (SetEditing (none, 0), (a coaster, 0) or (none, 1)): endtool.py's world (the tool in use dealt, the game or none), the
# game's +8 bit 1 clear. Close: the game's object (+0x28, its +8 bit 1 set) objectqueue.py's Object_Close case, no tool in use (0, 3, 6,
# 0x3f), 0x79c624 bit 0 or not (Coaster_FlashCar with no cars). Enter (SetEditing (the coaster, on)): stripanim.py's Object_RestartRide
# case as the coaster's object (+0x28), no game mostly; the coaster's state any but 4 / 8, its +0x3c bits 0 / 1 either, no cars (Coaster_
# SetState's own cases are coasterstate.py's). The action recorder (0x785a08) writing or not. Edit (EditRide (object, on)): Enter's world
# with the coaster on the link list (0x790fe0, by +0x18) under the object's ride id (+0x20) or not (no object now and then), on; or
# Leave's world, off; 0x785a2c set now and then. ById (0x447da0 CoasterEdit_SetEditingById (id)): Enter's world with the coaster on the
# link list, its id or a missing one; or Leave's world with a list and the id 0. At (0x447dd0 CoasterEdit_SetEditingAt (index)): Enter's
# world, the coaster at its index (1-3) of the table 0x790bd0; or Leave's world and the index 0.
# usage: coasteredit.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Leave,Close,Enter,Edit,ById,At)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, STUBS, Bench, Call, _recorder, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import endtool  # noqa: E402
import holder  # noqa: E402
import objectqueue  # noqa: E402
import stripanim  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

HOOKS = dict(holder.HOOKS)
HOOKS[0x473c70] = _recorder(27, 3, cdecl=True)   # (endtool.py's)
B = Bench(sorted(set(endtool.RANGES + objectqueue.RANGES + stripanim.RANGES
                     + [(0x447520, 0x447751), (0x4474d0, 0x447513), (0x435730, 0x435a40), (0x430ac0, 0x430ae0), (0x436860, 0x436890),
                        (0x4346c0, 0x434700), (0x447da0, 0x447e05)])), sines=256, hooks=HOOKS)
COAST, VT, BUF, GAME, OBJ, LINKS = RAM + 0xc0000, RAM + 0xc0200, RAM + 0xc0300, RAM + 0xc0400, RAM + 0xc0600, RAM + 0xc0800


def recorder(r, regions):
    """The action recorder (0x785a08), writing or not (as movecar.py's), unless the world has one."""
    if 0x785a08 in regions:
        return
    rec = bytearray(r.bytes(0x30))
    put(rec, 0, 'II', r.choice([0, 1, 1, 1]), r.choice([0, 0, 1, 2]))
    put(rec, 0xc, 'III', VT, BUF, r.choice([0, 1, 2, 3, 4, 0x20, 0x20]))
    put(rec, 0x24, 'I', r.choice([0, 0, 0, 1]))
    regions.update({0x785a08: bytes(rec), VT: put(bytearray(r.bytes(0x10)), 0xc, 'I', STUBS + 0x10), BUF: r.bytes(0x20)})


def leave(r):
    regions, game = endtool.world(r)
    if game:
        g = bytearray(regions[endtool.GAME])
        g[8] &= ~2
        regions[endtool.GAME] = g
    recorder(r, regions)
    return regions


def close(r):
    regions = dict(objectqueue.close(r).regions)
    game = bytearray(r.bytes(0x150))
    game[8] |= 2
    put(game, 0x28, 'I', RAM)   # (objectqueue.py's object)
    for off in (0x54, 0x58, 0x60, 0x64, 0x144):
        put(game, off, 'I', 0)
    glob = bytearray(r.bytes(0x110))
    put(glob, 0x58, 'I', int.from_bytes(glob[0x58:0x5c], 'little') & ~0x3f | r.choice([0, 3, 6, 0x3f]))
    glob[0x24] = r.getrandbits(8) & ~1 | r.choice([0, 1])
    regions.update({GAME: game, 0x790fec: GAME.to_bytes(4, 'little'), 0x79c600: glob})
    recorder(r, regions)
    return regions


def enter(r):
    regions = dict(stripanim.restart_case(r).regions)
    c = bytearray(r.bytes(0x150))
    put(c, 0x28, 'I', stripanim.ROBJ)
    put(c, 8, 'I', r.getrandbits(32) & ~0x7f | r.choice([0, 1, 1, 2, 2, 3, 5, 9, 15]))
    put(c, 0x3c, 'I', r.getrandbits(32) & ~3 | r.getrandbits(2))
    for off in (0x54, 0x58, 0x60, 0x64, 0xf4, 0x144):
        put(c, off, 'I', 0)
    regions[COAST] = c
    regions[0x790fec] = (0 if r.random() < 0.8 else GAME).to_bytes(4, 'little')
    recorder(r, regions)
    return regions


def links(r, regions, coaster):
    """The link list (0x790fe0): 0-3 links, the coaster among them when given; returns an id on the list's coaster or one missing."""
    n = r.randrange(4)
    chain = [LINKS + 0x40 * k for k in range(n)]
    if coaster:
        chain.insert(r.randrange(n + 1), coaster)
    ids = r.sample(range(1, 50), len(chain))
    for k, at in enumerate(chain):
        block = bytearray(regions[at]) if at in regions else bytearray(r.bytes(0x40))
        put(block, 0x10, 'I', ids[k])
        put(block, 0x18, 'I', chain[k + 1] if k + 1 < len(chain) else 0)
        regions[at] = block
    regions[0x790fe0] = (chain[0] if chain else 0).to_bytes(4, 'little')
    hit = ids[chain.index(coaster)] if coaster else None
    return hit if hit is not None and r.random() < 0.85 else 60


def case(r):
    op = r.deal(OPS)
    if op == 'Leave':
        args = r.choice([[0, 0], [0, 0], [COAST, 0], [0, 1]])
        return Call(0x447520, args, leave(r), ret='v')
    if op == 'Close':
        return Call(0x447520, r.choice([[0, 0], [COAST, 0]]), close(r), ret='v')
    if op == 'Enter':
        return Call(0x447520, [COAST, r.choice([1, 1, r.getrandbits(32) | 1])], enter(r), ret='v')
    on = r.deal([1, 0])
    if op == 'ById':
        regions = enter(r) if on else leave(r)
        rid = links(r, regions, COAST if on else None)
        if on and rid == 60:
            regions[0x790fec] = bytes(4)   # (no coaster: SetEditing (none, 1) leaves, with no game)
        return Call(0x447da0, [rid if on else 0], regions, ret='v')
    if op == 'At':
        regions = enter(r) if on else leave(r)
        index = r.randrange(1, 4) if on else 0
        regions[0x790bd0] = put(bytearray(r.bytes(16)), 4 * index, 'I', COAST)
        return Call(0x447dd0, [index], regions, ret='v')
    regions = enter(r) if on else leave(r)
    rid = links(r, regions, COAST if on else None)
    if on and rid == 60:
        regions[0x790fec] = bytes(4)   # (no coaster: SetEditing (none, 1) leaves, with no game)
    regions[OBJ] = put(bytearray(r.bytes(0x40)), 0x20, 'I', rid)
    regions[0x785a2c] = r.choice([0, 0, 0, 0, 1, 0x100]).to_bytes(4, 'little')
    obj = OBJ if r.random() < 0.9 else 0
    if not obj and on:
        regions[0x790fec] = bytes(4)
    return Call(0x4474d0, [obj, r.getrandbits(32) | 1 if on and r.random() < 0.3 else on], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', 'Leave,Close,Enter,Edit,ById,At').split(',') if k]

if __name__ == '__main__':
    B.main(case)
