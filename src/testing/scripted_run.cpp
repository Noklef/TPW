// TEST TOOLING, NOT PART OF THE DECOMPILATION. See scripted_run.h.
#include "scripted_run.h"

#include <cstdio>
#include <cstring>
#include <initializer_list>

#include "../game/core/staticobjects.h"
#include "../game/render/d3dsetup.h"
#include "../game/world/world.h"
#include "log.h"

namespace tpw::testing {

namespace {

// tpw.ini [tpw]: script=1 turns the run on; shot_ms the time between frame dumps (0: none); out the folder for them.
bool g_on;
DWORD g_shotMs;
char g_out[MAX_PATH];

using CreateDevice_t = HRESULT(__stdcall *)(void *, const GUID *, void **, void *);
using GetState_t = HRESULT(__stdcall *)(void *, DWORD, void *);
using GetData_t = HRESULT(__stdcall *)(void *, DWORD, DWORD *, DWORD *, DWORD);
CreateDevice_t o_createDevice;
GetState_t o_getState;
GetData_t o_getData;
void *g_mouse;

const GUID kSysMouse = {0x6f1d2b60, 0xd5a0, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};

// The game's own places (the original's addresses): the world, the engine and its front surface, the screen's size, the cursor.
template <typename T> T &game(unsigned address) { return *reinterpret_cast<T *>(static_cast<UINT_PTR>(address)); }
int ViewInt(int offset) { return *reinterpret_cast<int *>(reinterpret_cast<BYTE *>(&gView_8bcbc8) + offset); }
bool InPark() { return gWorldPtr != nullptr; }
bool Cursor(int *x, int *y) {
    __try {
        const int *const mouse = reinterpret_cast<int *(__cdecl *)()>(static_cast<UINT_PTR>(0x005f5640))();
        if (reinterpret_cast<UINT_PTR>(mouse) < 0x10000) return false;
        *x = mouse[0], *y = mouse[1];
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void Patch(void **slot, void *to, void **old) {
    DWORD before;
    if (*slot == to || !VirtualProtect(slot, sizeof *slot, PAGE_READWRITE, &before)) return;
    *old = *slot, *slot = to;
    VirtualProtect(slot, sizeof *slot, before, &before);
}

// The front surface (the frame on the screen, with the game's cursor) as a 24-bit .bmp.
void Shot(DWORD now) {
    static unsigned index;
    BYTE *const engine = *reinterpret_cast<BYTE **>(reinterpret_cast<BYTE *>(&gD3dsetup_879a20) + 4);
    if (!engine) return;
    for (const int offset : {0x10, 0x14}) {   // the front; the back when the front cannot be locked
        void **const surface = *reinterpret_cast<void ***>(engine + offset);
        if (!surface) continue;
        void **const vtable = reinterpret_cast<void **>(*surface);
        DWORD desc[0x7c / 4] = {0x7c};
        const HRESULT locked = reinterpret_cast<HRESULT(__stdcall *)(void *, void *, void *, DWORD, HANDLE)>(vtable[0x64 / 4])(surface, nullptr, desc, 1 | 0x10, nullptr);
        if (locked != 0) {
            log_line("[%6.1fs] shot: lock of the surface +%x failed %08X", now / 1000.0, offset, (unsigned)locked);
            continue;
        }
        const int height = (int)desc[2], width = (int)desc[3], pitch = (int)desc[4], bits = (int)desc[0x54 / 4];
        const DWORD masks[3] = {desc[0x58 / 4], desc[0x5c / 4], desc[0x60 / 4]};
        const BYTE *const pixels = reinterpret_cast<const BYTE *>(static_cast<UINT_PTR>(desc[0x24 / 4]));
        char path[MAX_PATH];
        std::snprintf(path, sizeof path, "%sorig-%03u.bmp", g_out, index);
        if (FILE *const file = pixels && (bits == 16 || bits == 32) && width > 0 && height > 0 ? std::fopen(path, "wb") : nullptr) {
            const int row = (width * 3 + 3) & ~3;
            BITMAPFILEHEADER head = {0x4d42, (DWORD)(54 + row * height), 0, 0, 54};
            BITMAPINFOHEADER info = {40, width, height, 1, 24, 0, (DWORD)(row * height)};
            std::fwrite(&head, sizeof head, 1, file), std::fwrite(&info, sizeof info, 1, file);
            static BYTE line[4096 * 3 + 4];
            int shift[3], scale[3];   // each colour: its mask's place and its width
            for (int c = 0; c < 3; ++c) {
                shift[c] = 0, scale[c] = 0;
                while (masks[c] && !(masks[c] >> shift[c] & 1)) ++shift[c];
                while (masks[c] >> (shift[c] + scale[c]) & 1) ++scale[c];
            }
            for (int y = height - 1; y >= 0 && width <= 4096; --y) {
                const BYTE *const from = pixels + y * pitch;
                for (int x = 0; x < width; ++x) {
                    const DWORD value = bits == 16 ? *reinterpret_cast<const WORD *>(from + x * 2) : *reinterpret_cast<const DWORD *>(from + x * 4);
                    for (int c = 0; c < 3; ++c) line[x * 3 + 2 - c] = scale[c] ? (BYTE)(((value & masks[c]) >> shift[c]) * 255 / ((1u << scale[c]) - 1)) : 0;
                }
                std::fwrite(line, row, 1, file);
            }
            std::fclose(file);
        }
        reinterpret_cast<HRESULT(__stdcall *)(void *, void *)>(vtable[0x80 / 4])(surface, nullptr);
        int cx = -1, cy = -1;
        Cursor(&cx, &cy);
        log_line("[%6.1fs] shot %u: surface +%x %d x %d, %d bits; in the park %d; cursor %d,%d; screen %d x %d", now / 1000.0, index, offset, width, height, bits,
                 (int)InPark(), cx, cy, ViewInt(8), ViewInt(0xc));
        ++index;
        return;
    }
}

// The script. Before the park: two clicks again and again (the places of the front end's two steps to "resume the last park",
// as the recompilation's test scripts use them at 4:3; a click also ends a movie). In the park: the camera button, the
// camcorder (the walk-through view), a place on the ground. Places are for a screen of 800 x 600 and scaled to the game's.
struct Step { DWORD at; int x, y; };
const Step kFront[2] = {{0, 650, 46}, {7000, 63, 462}};   // a cycle of 14 s
const Step kPark[3] = {{15000, 100, 548}, {19000, 220, 500}, {23000, 470, 350}};

int g_dx, g_dy;
bool g_down;

// Once for each read of the mouse: what the script wants now. A step moves the cursor to its place (by the difference from
// the game's own cursor, until it is there), then holds the button from 400 ms to 550 ms after its time.
void Tick() {
    static DWORD start, last, lastShot, parkAt;
    const DWORD clock = GetTickCount();
    if (!start) start = clock;
    const DWORD now = clock - start;
    if (g_shotMs && now - lastShot >= g_shotMs) lastShot = now, Shot(now);
    g_dx = g_dy = 0;
    if (now - last < 30) return;   // (the game may read the state and the buffered data in one frame)
    last = now;
    if (InPark() && !parkAt) parkAt = now, log_line("[%6.1fs] script: the park is there", now / 1000.0);
    const Step *step = nullptr;
    DWORD since = 0;
    if (!parkAt) {
        step = &kFront[now % 14000 >= 7000], since = now % 14000 - step->at;
    } else {
        for (const Step &candidate : kPark)
            if (now - parkAt >= candidate.at && now - parkAt < candidate.at + 3000) step = &candidate, since = now - parkAt - candidate.at;
    }
    g_down = false;
    int x, y;
    if (!step || since >= 3000 || !Cursor(&x, &y)) return;
    const int wide = ViewInt(8), high = ViewInt(0xc);
    if (wide <= 0 || high <= 0) return;
    g_dx = step->x * wide / 800 - x, g_dy = step->y * high / 600 - y;
    if (g_dx > 40) g_dx = 40; else if (g_dx < -40) g_dx = -40;
    if (g_dy > 40) g_dy = 40; else if (g_dy < -40) g_dy = -40;
    g_down = since >= 1500 && since < 1650;   // (40 a read reaches any place of the screen well before 1.5 s)
    static const Step *logged;
    static bool wasDown;
    if (g_down && !wasDown) log_line("[%6.1fs] script: click at %d,%d (the step's place %d,%d of 800 x 600)", now / 1000.0, x, y, step->x, step->y);
    wasDown = g_down, logged = step;
}

HRESULT __stdcall GetState(void *device, DWORD size, void *data) {
    const HRESULT result = o_getState(device, size, data);
    if (device != g_mouse || size < 16) return result;
    Tick();
    std::memset(data, 0, size);
    LONG *const axes = static_cast<LONG *>(data);
    axes[0] = g_dx, axes[1] = g_dy;
    static_cast<BYTE *>(data)[12] = g_down ? 0x80 : 0;
    return 0;
}

// The buffered form: records of 16 bytes (the offset in the mouse's state, the value, the time, the sequence).
HRESULT __stdcall GetData(void *device, DWORD size, DWORD *records, DWORD *count, DWORD flags) {
    const HRESULT result = o_getData(device, size, records, count, flags);
    if (device != g_mouse || size != 16 || !count) return result;
    Tick();
    static bool sent;
    static DWORD sequence;
    DWORD n = 0;
    const auto add = [&](DWORD offset, DWORD value) {
        if (records) records[n * 4] = offset, records[n * 4 + 1] = value, records[n * 4 + 2] = GetTickCount(), records[n * 4 + 3] = ++sequence;
        ++n;
    };
    const DWORD room = records ? *count : 3;
    if (g_dx && n < room) add(0, (DWORD)g_dx);
    if (g_dy && n < room) add(4, (DWORD)g_dy);
    if (g_down != sent && n < room) add(12, g_down ? 0x80 : 0), sent = g_down;
    *count = n;
    return 0;
}

// The game reads its mouse when DirectInput signals the device's event, which a real movement does. The script's movements
// are not real: a thread signals the event each 15 ms.
using SetEvent_t = HRESULT(__stdcall *)(void *, HANDLE);
SetEvent_t o_setEvent;
HANDLE g_event;
DWORD WINAPI Pulse(void *) {
    for (;;) {
        Sleep(15);
        if (g_event) SetEvent(g_event);
    }
}
HRESULT __stdcall SetEventNotification(void *device, HANDLE event) {
    if (device == g_mouse) {
        static bool started;
        g_event = event;
        if (!started) started = true, CloseHandle(CreateThread(nullptr, 0, Pulse, nullptr, 0, nullptr));
        log_line("[%6.1fs] script: the mouse's event %p", log_seconds(), event);
    }
    return o_setEvent(device, event);
}

HRESULT __stdcall CreateDevice(void *input, const GUID *guid, void **device, void *outer) {
    const HRESULT result = o_createDevice(input, guid, device, outer);
    if (result == 0 && guid && !std::memcmp(guid, &kSysMouse, sizeof kSysMouse) && device && *device) {
        g_mouse = *device;
        void **const vtable = *reinterpret_cast<void ***>(*device);
        Patch(&vtable[9], reinterpret_cast<void *>(&GetState), reinterpret_cast<void **>(&o_getState));
        Patch(&vtable[10], reinterpret_cast<void *>(&GetData), reinterpret_cast<void **>(&o_getData));
        Patch(&vtable[12], reinterpret_cast<void *>(&SetEventNotification), reinterpret_cast<void **>(&o_setEvent));
        log_line("[%6.1fs] script: the mouse device %p is wrapped", log_seconds(), *device);
    }
    return result;
}

}  // namespace

void start_scripted_run(const char *ini) {
    g_on = GetPrivateProfileIntA("tpw", "script", 0, ini) != 0;
    g_shotMs = GetPrivateProfileIntA("tpw", "shot_ms", 2000, ini);
    GetPrivateProfileStringA("tpw", "out", "", g_out, MAX_PATH - 20, ini);
    if (g_on) log_line("scripted run on: a frame dump each %u ms into %s", (unsigned)g_shotMs, g_out);
}

void wrap_direct_input(void *input) {
    if (!g_on || !input) return;
    void **const vtable = *reinterpret_cast<void ***>(input);
    Patch(&vtable[3], reinterpret_cast<void *>(&CreateDevice), reinterpret_cast<void **>(&o_createDevice));
}

}  // namespace tpw::testing
