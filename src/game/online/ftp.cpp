#include "ftp.h"

#include "../core/crt.h"

extern "C" {

s32 __fastcall Ftp_ListLineSize(void *, void *, const char *line) {
    const CrtEnv &crt = Crt();
    for (int field = 0; field < 4; field++) {
        while (!crt.isspace(*line))
            line++;
        while (crt.isspace(*line))
            line++;
    }
    return crt.atoi(line);
}

}  // extern "C"
