# Bench: DrawBuffer_DrawFrame 0x582170: the model list drawn, then the buffer's batches; 0 from either stops it. The EMMS of mode 4 runs as it is.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

MODELS, BATCHES = 9, 10
B = Bench([(0x582170, 0x58219b)], hooks={0x581db0: _recorder(MODELS, 0, cdecl=True), 0x582ad0: _recorder(BATCHES, 0)})


def case(r):
    log = log_region()
    set_return(log, MODELS, r.deal([0, 1, 1, 5]))
    set_return(log, BATCHES, r.deal([0, 1, 1, 5]))
    return Call(0x582170, [], {LOG: log, 0x8bcbc4: r.bytes(4), 0x8bd524: struct.pack('<I', r.deal([4, 1, 2]))}, ret='a')


if __name__ == '__main__':
    B.main(case)
