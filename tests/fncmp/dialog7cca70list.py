# The dialog [0x7cca70]'s list filled from a session's answer (src/game/ui/dialog7cca70list: 0x4c1e40, its callback 0x4c20c0) against the original. Recorders, as Dialog7cca70ListEnv's: Clear 12, AddItem 13 (the row's five
# dwords hashed), RefreshRows 14, the censor 15 (the string's pointer, length and room), the holder's constructor 16, Owner2c_ValueByKey 17 (answering a char text), the label 18 (the local strings logged as 0; its data
# pointer filled now and then, as dialog7cdac4.py's), Online_Instance 19, 0x5b6070 20, Class702da0_Clear 21, Window_SendMessage 22. The global save info as chatcommand.py's (made when none: the first item's text empty,
# so it is the first allocation, which that fixture's mask needs).
# usage: dialog7cca70list.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, fill, heap, log_region, put, set_return, strings  # noqa: E402
import chatcommand  # noqa: E402

B = Bench([(0x4c1e40, 0x4c20b8), (0x4c20c0, 0x4c2106), (0x405280, 0x405390), (0x409410, 0x409490), (0x45b380, 0x45b5f0), (0x45b680, 0x45b9b0), (0x479230, 0x479270), (0x47ba30, 0x47ba60), (0x47bad0, 0x47bc10), (0x4f23f0, 0x4f2450), (0x5aef50, 0x5af0f0),
           (0x5b09b0, 0x5b09d0), (0x5b1680, 0x5b16c0), (0x5b2fd0, 0x5b3000), (0x5b3110, 0x5b3140), (0x5f60b0, 0x5f6160), (0x5f61e0, 0x5f6210), (0x5f6a20, 0x5f6a90), (0x5f76c0, 0x5f7720), (0x65d7bc, 0x65d832), (0x67acd0, 0x67ad00), (0x67bb90, 0x67bd98),
           (0x67d250, 0x67d2c0), (0x67d3a0, 0x67d400), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)],
          hooks={**chatcommand.wstringmap.HOOKS, 0x6649d5: _recorder(12, 0), 0x66403b: _recorder(13, 4), 0x663324: _recorder(14, 0), 0x5d3230: _recorder(15, 2), 0x5b50b0: _recorder(16, 0), 0x5bd270: _recorder(17, 1), 0x4099b0: _recorder(18, 2),
                 0x48b390: _recorder(19, 0, cdecl=True), 0x5b6070: _recorder(20, 0), 0x5d6060: _recorder(21, 0), 0x658d9f: _recorder(22, 4, cdecl=True)})
WINDOW, LIST, ANSWER, NODES, ITEMS, TEXTS, THEME, HOLDER, SCENE = (RAM + 0x20000 + 0x400 * k for k in range(9))
HOLDER_NEW = RAM + 0x24000


def case(r):
    log = log_region()
    regions = {LOG: log}
    masks = chatcommand.save(r, regions, instance=r.deal([True, True, True, False]))
    regions[HEAP] = heap(0x4000, r.deal([0, 0, 0, 0, 2]) if not masks else 0)
    regions[0x7cca70] = struct.pack('<I', WINDOW)
    regions[WINDOW] = put(put(put(bytearray(r.bytes(0x100)), 0x54, 'I', 0), 0x5c, 'I', LIST), 0x8c, 'I', 1)
    regions[LIST] = put(put(bytearray(r.bytes(0x100)), 0x54, 'I', 0), 0x8c, 'I', 0x14beb)
    count = r.deal([0, 1, 2, 2, 3])
    nodes, items, texts = bytearray(8 * count + 8), bytearray(0xc0 * count + 8), bytearray(0x400)
    for i in range(count):
        put(nodes, 8 * i, '2I', ITEMS + 0xc0 * i, NODES + 8 * (i + 1) if i + 1 < count else 0)
        item = bytearray(r.bytes(0xc0))
        length = 0 if masks and i == 0 else r.deal([0, 3, 9, 40])
        wide = bytes(b for _ in range(length) for b in struct.pack('<H', r.randrange(1, 0xffff))) + b'\0\0'
        put(item, 0x14, 'I', TEXTS + 0x100 * i)
        texts[0x100 * i:0x100 * i + len(wide)] = wide
        items[0xc0 * i:0xc0 * (i + 1)] = item
    regions[NODES], regions[ITEMS], regions[TEXTS] = nodes, items, texts
    answer = bytearray(r.bytes(0x10))
    put(answer, 4, 'I', NODES if count else 0)
    put(answer, 0xc, 'I', r.deal([0, 1, 2, 3, r.getrandbits(32)]))
    regions[ANSWER] = answer
    regions[THEME] = bytes(r.randrange(1, 0xff) for _ in range(r.deal([0, 4, 40]))) + b'\0'
    made = r.deal([True, True, True, False])
    holder = bytearray(r.bytes(0x100))
    regions[HOLDER] = holder
    regions[0xf7d88c] = struct.pack('<I', HOLDER if made else 0)
    regions[0xf85614] = struct.pack('<I', SCENE)
    regions[SCENE] = r.bytes(0x20)
    strings(log, 15, 0, 0b01)
    blob(log, 15, 0, 0b1110)
    strings(log, 18, 0, 0b11)
    if r.deal([0, 1]):
        fill(log, 18, 1, struct.pack('<I', r.getrandbits(32)), offset=8)
    blob(log, 13, 0, 0b11111)
    set_return(log, 16, HOLDER)
    set_return(log, 17, THEME)
    set_return(log, 19, r.getrandbits(32))
    if r.deal(['Fill', 'Fill', 'Fill', 'Callback', 'Error']) == 'Fill':
        return Call(0x4c1e40, [ANSWER], regions, masks=masks)
    return Call(0x4c20c0, [r.deal([0, 0, 1, 0xffffffff]), r.getrandbits(32), ANSWER], regions, masks=masks)


if __name__ == '__main__':
    B.main(case)
