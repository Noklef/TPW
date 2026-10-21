#include "soundchannel.h"

#include "vcall.h"

extern "C" {

u32 *__fastcall SoundChannel_Apply(u8 *channel, void *, u32 *out, u8 *functor) {
    vcall<void>(functor, 0, out, channel);
    return out;
}

}  // extern "C"
