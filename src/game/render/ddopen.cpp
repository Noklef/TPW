// The display's DirectDraw start-up: see ddopen.h.
#include "ddopen.h"

#include <cstdint>
#include <cstring>

#include "../core/addrcalls.h"
#include "../core/crt.h"
#include "../core/vcall.h"
#include "../core/vobj3.h"
#include "../gfx/surface.h"
#include "../math/bits.h"
#include "d3dresult.h"
#include "texformat.h"
#include "viewport.h"

using d3dresult::keep;
using d3dresult::result;

namespace {

template <typename T>
T &at(const void *base, u32 offset) {
    return *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(base) + offset);
}

template <typename T>
T &global(uintptr_t address) {
    return *reinterpret_cast<T *>(address);
}

u8 *const kView = reinterpret_cast<u8 *>((uintptr_t)0x008bcbc8);
u8 *const kErrors = reinterpret_cast<u8 *>((uintptr_t)0x00879ad0);
const u32 kTextLog = 0x1000000, kStepLog = 0x2000000;
const u32 kDescSize = 0x7c;
const uintptr_t kIidDraw2 = 0x007071f0, kIidDraw4 = 0x00707200;
const uintptr_t kEnumDriver = 0x005633e0;  // (DirectDrawEnumerateA's callback, the port Quad_Push of core/seqs: the hook sends it on)

// A COM method by its offset in the vtable.
template <typename... A>
u32 Com(void *object, u32 offset, A... args) {
    using Method = u32(__stdcall *)(void *, A...);
    return (*reinterpret_cast<Method *>(*reinterpret_cast<u8 **>(object) + offset))(object, args...);
}

// An import by its slot.
template <typename... A>
u32 Import(uintptr_t slot, A... args) {
    using Function = u32(__stdcall *)(A...);
    return (*reinterpret_cast<Function *>(slot))(args...);
}

u32 Flags() { return global<u32>(0x008bd508); }
u32 Window() { return global<u32>(0x008bd518); }

// A kept call: whether it failed.
bool Fails(u32 hr) {
    keep(hr);
    return result() != 0;
}

// sprintf into the original's stack buffer (0xc8 bytes or less there: a long driver name runs over its frame; room for the longest here).
template <typename... A>
void LogText(uintptr_t format, A... args) {
    char text[0x440];
    Crt().sprintf(text, reinterpret_cast<const char *>(format), args...);
    EngineDebug_Log(text);
}

u32 Failure(u8 *object) {
    if (Flags() & kStepLog) {
        char text[0x40];
        Crt().sprintf(text, reinterpret_cast<const char *>((uintptr_t)0x007661cc), result());
        EngineDebug_LogFallback(text);
    }
    EngineErrors_Push(kErrors, nullptr, 3, result());
    Obj_ReleaseSevenComMembers(object);
    return 0;
}

u32 NoDraw4(u8 *object) {
    EngineErrors_Push(kErrors, nullptr, 3, 0xb0754016);
    return Failure(object);
}

// The GUIDs into order, each against all that follow it.
void SortDrivers(u8 *object) {
    u8 *a = object + 0x448;
    for (s32 i = 1; i - 1 < at<s32>(object, 0x788) - 1; ++i, a += 0x10) {
        u8 *b = a + 0x10;
        for (s32 j = i; j < at<s32>(object, 0x788); ++j, b += 0x10) {
            if (Crt().strncmp(reinterpret_cast<const char *>(a), reinterpret_cast<const char *>(b), 0x10) > 0) {
                u8 held[0x10];
                std::memcpy(held, a, 0x10);
                std::memcpy(a, b, 0x10);
                std::memcpy(b, held, 0x10);
            }
        }
    }
}

// The first start: whether it went well, else *answer.
bool FirstStart(u8 *object, u32 driver, u32 *answer) {
    if (Flags() & 0x10) {
        at<u32>(object, 0x788) = 1;
        at<u32>(object, 0x78c) = driver;
        if (Fails(Import(0x006fd018, nullptr, object, nullptr))) return *answer = Failure(object), false;
    } else {
        at<u32>(object, 0x788) = 0;
        Import(0x006fd014, kEnumDriver, object);
        SortDrivers(object);
        at<u32>(object, 0x78c) = driver;
        if (Fails(Import(0x006fd018, object + 0x448 + (driver << 4), object, nullptr))) return *answer = Failure(object), false;
    }
    if (Flags() & kTextLog) LogText((u32)&gText_Ddopen_7662cc, at<u32>(object, 0x788), at<u32>(object, 0x78c));
    if (Fails(Com(at<void *>(object, 0), 0, kIidDraw2, object + 4))) return *answer = Failure(object), false;
    if (Fails(Com(at<void *>(object, 0), 0, kIidDraw4, object + 8))) {
        if (Flags() & kStepLog) EngineDebug_LogFallback(gText_Ddopen_7662a8);
        return *answer = NoDraw4(object), false;
    }
    if (Fails(Com(at<void *>(object, 8), 0x6c, object + 0x20, 1))) return *answer = Failure(object), false;
    if (Flags() & kTextLog) {
        LogText((u32)&gText_Ddopen_76628c, object + 0x20);
        LogText(0x0076626c, object + 0x220);
        LogText(0x00766248, at<u32>(object, 0x420), at<u32>(object, 0x424));
        LogText(0x0076621c, at<u32>(object, 0x428), at<u32>(object, 0x42c));
    }
    std::memset(object + 0x488, 0, 0x17c);
    std::memset(object + 0x604, 0, 0x17c);
    at<u32>(object, 0x604) = 0x17c;
    at<u32>(object, 0x488) = 0x17c;
    if (Fails(Com(at<void *>(object, 8), 0x2c, object + 0x488, object + 0x604))) return *answer = Failure(object), false;
    return true;
}

// Another driver than the one in use: whether it went well, else *answer.
bool ChangeDriver(u8 *object, u32 driver, u32 *answer) {
    Obj_ReleaseSevenComMembers(object);
    at<u32>(object, 0x78c) = driver;
    if (Fails(Import(0x006fd018, object + 0x448 + (driver << 4), object, nullptr))) return *answer = Failure(object), false;
    if (Fails(Com(at<void *>(object, 0), 0, kIidDraw4, object + 8))) return *answer = NoDraw4(object), false;
    return true;
}

bool OpenFullScreen(u8 *object, s32 width, s32 height, u32 depth, u32 videoMemory, u32 *desc) {
    if (Flags() & kTextLog) LogText(0x00766208, width, height, depth);
    if (Fails(Com(at<void *>(object, 8), 0x50, Window(), 0x851))) return false;
    if (Fails(Com(at<void *>(object, 8), 0x54, width, height, depth, 0, 0))) return false;
    Import(0x006fd368, Window(), 3);
    Import(0x006fd36c, Window());
    std::memset(desc, 0, kDescSize);
    desc[0] = kDescSize;
    desc[1] = 0x21;
    desc[5] = global<u8>(0x008bcbc8) & 1 ? 2 : 1;
    desc[0x1a] = videoMemory ? 0x6218 : 0x4218;
    if (Fails(Com(at<void *>(object, 8), 0x18, desc, object + 0x10, 0))) return false;
    u32 fill[0x19];
    std::memset(fill, 0, sizeof fill);
    fill[0] = sizeof fill;
    Com(at<void *>(object, 0x10), 0x14, 0, 0, 0, 0x1000400, fill);  // (the result not looked at)
    u32 caps = 4;
    if (Fails(Com(at<void *>(object, 0x10), 0x30, &caps, object + 0x14))) return false;
    at<u32>(object, 0x780) |= 1;
    return true;
}

bool OpenWindowed(u8 *object, s32 width, s32 height, u32 *desc) {
    if (Flags() & kTextLog) LogText(0x007661f4, width, height);
    Import(0x006fd368, Window(), 9);
    Import(0x006fd36c, Window());
    std::memset(desc, 0, kDescSize);
    desc[0] = kDescSize;
    if (Fails(Com(at<void *>(object, 8), 0x30, desc))) return false;
    global<u32>(0x008bcbe4) = desc[0x15];
    if (Fails(Com(at<void *>(object, 8), 0x50, Window(), 0x808))) return false;
    s32 client[4], frame[4];
    Import(0x006fd390, Window(), client);
    Import(0x006fd38c, Window(), frame);
    frame[2] += client[0] - client[2] + global<s32>(0x008bcbd0);
    frame[3] += client[1] - client[3] + global<s32>(0x008bcbd4);
    Import(0x006fd31c, Window(), frame[0], frame[1], frame[2] - frame[0], frame[3] - frame[1], 0);
    std::memset(desc, 0, kDescSize);
    desc[0] = kDescSize;
    desc[1] = 1;
    desc[0x1a] = 0x200;
    if (Fails(Com(at<void *>(object, 8), 0x18, desc, object + 0x10, 0))) return false;
    desc[3] = (u32)width;
    desc[1] = 7;
    desc[2] = (u32)height;
    desc[0x1a] = 0x6040;
    if (Fails(Com(at<void *>(object, 8), 0x18, desc, object + 0x14, 0))) return false;
    if (Fails(Com(at<void *>(object, 8), 0x10, 0, object + 0xc, 0))) return false;
    if (Fails(Com(at<void *>(object, 0xc), 0x20, 0, Window()))) return false;
    if (Fails(Com(at<void *>(object, 0x10), 0x70, at<void *>(object, 0xc)))) return false;
    at<u32>(object, 0x780) &= ~1u;
    Com(at<void *>(object, 0x10), 0x58, desc);  // (the result not looked at)
    gDdopen_8bd4c4 = desc[0x16];
    gDdopen_8bd4cc = desc[0x17];
    gDdopen_8bd4d4 = desc[0x18];
    gDdopen_8bd4dc = desc[0x16] | desc[0x17] | desc[0x18];
    Masks_TopBits(kView);
    Import(0x006fd380, Window(), 0, frame[0], frame[1], frame[2] - frame[0], frame[3] - frame[1], 0x40);
    Import(0x006fd368, Window(), 1);
    Import(0x006fd36c, Window());
    return true;
}

}  // namespace

namespace {
AddrCalls g_restoreCalls;

// A surface's Restore (slot 0x6c): the answer kept at [0x879cd4], a failure's text logged (DxResult_Message, EngineDebug_Log).
u32 RestoreOne(u8 *surface) {
    const u32 answer = comcall<u32>(surface, 0x6c);
    gDdopen_879cd4 = answer;
    if ((s32)answer < 0) g_restoreCalls.Cd(0x00564650, g_restoreCalls.Cd(0x0056a770, answer));
    return gDdopen_879cd4;
}
}  // namespace

void Display_SetRestoreCalls(const AddrCall *table, u32 count) { g_restoreCalls.Set(table, count); }

extern "C" u32 __fastcall Display_RestoreOrOpen(u8 *self) {
    if (global<u32>(0x008bd524) == 1)
        return g_restoreCalls.Th(0x00563460, (u32)(uintptr_t)self, global<u32>(0x008bcbd0), global<u32>(0x008bcbd4),
                                 global<u32>(0x008bcbe4), at<u32>(self, 0x78c), at<u32>(self, 0x780) & 1, 1u);
    if (RestoreOne(at<u8 *>(self, 0x10))) return 0;
    return RestoreOne(at<u8 *>(self, 0x14)) == 0;
}

extern "C" u32 __fastcall DirectDraw_Open(u8 *object, void *, s32 width, s32 height, u32 depth, u32 driver, u32 fullScreen, u32 videoMemory) {
    if (Flags() & kTextLog) LogText((u32)&gText_DirectDraw_Open_766310);
    if (Flags() & kStepLog) EngineDebug_LogFallback(gText_DirectDraw_Open_7662f8);
    if (!(global<u32>(0x008bd534) & 0x800000)) {
        EngineErrors_Push(kErrors, nullptr, 3, 0xb0754017);
        return 0;
    }
    View_SetScreen(kView, nullptr, width, height, depth);
    u32 answer = 0;
    if (!at<void *>(object, 0)) {
        if (!FirstStart(object, driver, &answer)) return answer;
    } else if (driver != at<u32>(object, 0x78c)) {
        if (!ChangeDriver(object, driver, &answer)) return answer;
    }
    u32 desc[kDescSize / 4];
    if (!(fullScreen == 1 ? OpenFullScreen(object, width, height, depth, videoMemory, desc) : OpenWindowed(object, width, height, desc))) return Failure(object);
    desc[0] = kDescSize;
    const u32 hr = Com(at<void *>(object, 8), 0x30, desc);
    if (static_cast<s32>(hr) < 0) EngineDebug_Log(DxResult_Message(hr));  // (not kept)
    if (result()) return Failure(object);
    const u32 alpha = desc[0x19], red = desc[0x16], green = desc[0x17], blue = desc[0x18];
    Surface_SetMasks(kView, nullptr, alpha, Bits_Shift(alpha), Bits_Count(alpha), red, Bits_Shift(red), Bits_Count(red), green, Bits_Shift(green), Bits_Count(green), blue, Bits_Shift(blue), Bits_Count(blue));
    if (Flags() & kStepLog) EngineDebug_LogFallback(reinterpret_cast<const char *>((uintptr_t)0x007661a8));
    return 1;
}
