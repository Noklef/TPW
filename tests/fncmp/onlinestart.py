# The online session started on a request (src/game/online/session: 0x5b87e0) against the original: its releases and resets (Online_ReleaseNews, Online_ResetCity, Online_ResetUpload, Online_ResetDownload), the dialog's
# message (Dialog7cca94_Message4IfUp), Online_Instance, ListRequest_Start66 and Dialog7cc530_Open are recorders, each with a bench of its own; the three strings' slot 5 a recorder. Logged in (+0x60) or not.
# usage: onlinestart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

ASSIGN, RELEASE, CITY, UPLOAD, DOWNLOAD, MESSAGE, INSTANCE, START, OPEN = 5, 21, 22, 23, 24, 25, 26, 27, 28
B = Bench([(0x5b87e0, 0x5b88da)], hooks={0x5b6b20: _recorder(RELEASE, 0), 0x5b7430: _recorder(CITY, 0), 0x5b8400: _recorder(UPLOAD, 0), 0x5b9040: _recorder(DOWNLOAD, 0), 0x4c25e0: _recorder(MESSAGE, 0, cdecl=True),
                                        0x48b390: _recorder(INSTANCE, 0, cdecl=True), 0x5bcd10: _recorder(START, 3), 0x4ba980: _recorder(OPEN, 1, cdecl=True)})
SESSION, THE_INSTANCE, VT = RAM, RAM + 0x1000, RAM + 0x2000


def case(r):
    log = log_region()
    set_return(log, INSTANCE, THE_INSTANCE)
    session = bytearray(r.bytes(0x3d0))
    for slot in (0x7c, 0x98, 0xb4):
        put(session, slot, 'I', VT)
    put(session, 0x60, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions = {LOG: log, SESSION: session, THE_INSTANCE: r.bytes(0x3d0), VT: vtable({ASSIGN: 1})}
    return Call(0x5b87e0, [SESSION, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
