# OverlayLoad_Button, _Slider and _Plain (src/game/gfx/overlayload, 0x488d20, 0x488ee0, 0x489060) against the original. Holder_LoadMd2 17, TextureSet_Load 18, Overlay_CtorResized 19, Overlay_CtorResizedHeight 20,
# Overlay_Ctor 21 and GuiWindows_Register 22 recorders (their own benches have them; the C strings logged by their text): the holder or none, the overlay at a fixed place (its +0x12 compared).
# The file's entry by value (random but for +0x20, the name), the unused argument, the directory (a String's address, or by value for _Plain: its data then freed); the names: the three Strings' texts with an extension of
# [0x7b96bc]'s length mostly, "!cbut", '!' ones, "w_small_shadow.md2" and its prefixes, others; the heap failing now and then (Register with 0 for _Plain, faults in the other two).
# usage: overlayload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x488d20, 0x488ed4), (0x488ee0, 0x489058), (0x489060, 0x4891ca), (0x424cf0, 0x424dc0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)],
          hooks={0x46dcf0: _recorder(17, 5, cdecl=True), 0x46ead0: _recorder(18, 3, cdecl=True), 0x477250: _recorder(19, 1), 0x477580: _recorder(20, 1), 0x476a90: _recorder(21, 1),
                 0x659a58: _recorder(22, 2, cdecl=True)})
TEXTS, DIRECTORY, HOLDER, MODEL, SKIN, OVERLAY = RAM + 0x60000, RAM + 0x61000, RAM + 0x61100, RAM + 0x61200, RAM + 0x61300, RAM + 0x61400
STEMS = ['!button', '!arrow', '!toggle', '!grip', 'tab', 'a', '']
SHADOW = 'w_small_shadow.md2'


def text(regions, k, s):
    at = TEXTS + 0x40 * k
    regions[at] = s.encode() + b'\0'
    return at


def string(regions, at, k, s):
    regions[at] = struct.pack('<5I', 0x703090, len(s), text(regions, k, s), (len(s) & ~7) + 8, 8)


def setup(r):
    """the log, the regions, the extension, the three Strings' stems and the holder"""
    log = log_region()
    regions = {LOG: log}
    regions[HEAP] = heap(0x1000, r.choice([0] * 12 + [r.randrange(1, 12)]))
    tail = r.choice(['.md2'] * 6 + ['', '.x', '.long'])
    regions[0x7b96bc] = struct.pack('<I', len(tail))
    stems = r.sample(STEMS, 3)
    for k, at in enumerate((0x7b9ab8, 0x7b9a80, 0x7b93a8)):
        string(regions, at, k, stems[k])
    holder = r.choice([HOLDER] * 6 + [0])
    regions[HOLDER] = put(bytearray(r.bytes(0x10)), 4, 'I', MODEL)
    regions[MODEL] = put(bytearray(r.bytes(0x80)), 0x78, 'I', SKIN)
    regions[SKIN] = put(bytearray(r.bytes(0x60)), 0x54, 'I', text(regions, 3, r.choice(['frame', 'w_big', ''])))
    regions[OVERLAY] = r.bytes(0x84)
    set_return(log, 17, holder)
    for k in (19, 20, 21):
        set_return(log, k, OVERLAY)
    strings(log, 17, 1)
    strings(log, 22, 2)
    return log, regions, tail, stems


def entry(r, regions, name):
    words = list(struct.unpack('<74I', r.bytes(0x128)))
    words[8] = text(regions, 4, name)
    return words + [r.getrandbits(32)]


def directory(r):
    return r.choice(['', 'data\\', 'ui\\frames\\', 'a' * 20])


def button(r):
    log, regions, tail, stems = setup(r)
    name = r.choice(stems[:2] * 3 + ['!cbut'] * 2 + [stems[2], 'other', r.choice(STEMS)]) + tail
    if r.random() < 0.05:
        name = 'x' * r.randrange(3)
    string(regions, DIRECTORY, 5, directory(r))
    return Call(0x488d20, entry(r, regions, name) + [DIRECTORY], regions, conv='c', ret='a')


def slider(r):
    log, regions, tail, stems = setup(r)
    name = r.choice([stems[2]] * 6 + [stems[0], 'other']) + tail
    if r.random() < 0.05:
        name = 'x' * r.randrange(3)
    string(regions, DIRECTORY, 5, directory(r))
    return Call(0x488ee0, entry(r, regions, name) + [DIRECTORY], regions, conv='c', ret='a')


def plain(r):
    log, regions, tail, stems = setup(r)
    name = r.choice(['frame.md2', 'w_small.md2', 'w_small_shadow', 'w_small_shadow.md', 'w_small_shadow.md2x', 'x', '', SHADOW, SHADOW, '!frame', '!', '!x.md2'] + ['panel.md2'] * 6)
    d = directory(r)
    data = text(regions, 5, d)
    return Call(0x489060, entry(r, regions, name) + [0x703090, len(d), data, (len(d) & ~7) + 8, 8], regions, conv='c', ret='a')


def case(r):
    return r.deal([button, slider, plain])(r)


if __name__ == '__main__':
    B.main(case)
