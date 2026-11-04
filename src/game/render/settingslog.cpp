#include "settingslog.h"

#include "../core/crt.h"
#include "enginedebug.h"

namespace {

u32 flags(const u8 *settings) { return *reinterpret_cast<const u32 *>(settings); }
u32 value(const u8 *settings, u32 offset) { return *reinterpret_cast<const u32 *>(settings + offset); }
double number(const u8 *settings, u32 offset) { return *reinterpret_cast<const float *>(settings + offset); }

template <unsigned N, typename... Args> void log_line(char (&line)[N], const char *format, Args... args) {
    Crt().sprintf(line, format, args...);
    EngineDebug_Log(line);
}

}  // namespace

extern "C" {

void __fastcall Settings_LogDisplay(const u8 *settings) {
    char line[0x100];
    log_line(line, "\n\nDisplay info");
    log_line(line, "Triplebuffer: %d", flags(settings) & 1);
    log_line(line, "Enable fog flags: %d", flags(settings) & 0x2000);
    log_line(line, "Sprite shadows: %d", flags(settings) & 0x20000);
    log_line(line, "ZBuffer: (begin) %f, (ui end/linear begin) %f, (linear end/log begin) %f, (log end) %f", number(settings, 0x48), number(settings, 0x4c), number(settings, 0x50), number(settings, 0x54));
    log_line(line, "Sprite shadows: %d", flags(settings) & 0x20000);
    EngineDebug_Log(line);   // sic: the same line again
}

void __fastcall Settings_LogSystem(const u8 *settings) {
    char line[0x64];
    log_line(line, "\n\nSystem info");
    log_line(line, "Clock Frequency: %d", (s32)value(settings, 0x14) / 1000);
    log_line(line, "RAM detected: %d", value(settings, 0x18));
    log_line(line, "Renderer: %d", value(settings, 0x1c));
    log_line(line, "Math: %d", value(settings, 0x20));
    log_line(line, "\n\n***RELEASE VERSION***");
}

void __fastcall Settings_LogTexturing(const u8 *settings) {
    char line[0x64];
    log_line(line, "\n\nTexturing info");
    log_line(line, "Using procedural textures: %d", flags(settings) & 4);
    log_line(line, "Mipmapping enabled: %d", flags(settings) & 2);
    log_line(line, "Trilinear filtering enabled: %d", flags(settings) & 8);
    log_line(line, "Anisotropic filtering enabled: %d", flags(settings) & 0x10);
    log_line(line, "UV fixups: regular %f, textU %f, textV %f, delta %f:%f", number(settings, 0x40d8), number(settings, 0x40e0), number(settings, 0x40e4), number(settings, 0x40e8), number(settings, 0x40ec));
}
}
