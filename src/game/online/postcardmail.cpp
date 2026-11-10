// The virtual postcard's mail text: see postcardmail.h.
#include "postcardmail.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/stdstring.h"
#include "../files/quickload.h"

namespace {

template <typename T>
T &at(const void *base, u32 offset) {
    return *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(base) + offset);
}

const char *Text(uintptr_t address) { return reinterpret_cast<const char *>(address); }

char *const kNullString = reinterpret_cast<char *>((uintptr_t)0x006fe5b0);
const char *const kNoName = Text(0x006fd6c4);

// The append 0x4f1340 is, inlined all through this function: _Grow to the new length, then the characters.
void Add(StdString *to, const char *chars, u32 n) {
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x006fe5ac) - to->len <= n) StdStrings_Xlen();
    if (!n) return;
    const u32 length = to->len + n;
    if (StdString_Grow(to, nullptr, length, 0)) {
        std::memcpy(to->ptr + to->len, chars, n);
        StdString_Eos(to, nullptr, length);
    }
}

void Add(StdString *to, const char *text) { Add(to, text, (u32)std::strlen(text)); }

template <typename... A>
void AddFormat(StdString *to, char *buffer, uintptr_t format, A... args) {
    Crt().sprintf(buffer, Text(format), args...);
    Add(to, buffer);
}

char *OrNull(char *text) { return text ? text : kNullString; }

// Each CR and LF blanked, to the end of the text.
void CutLines(char *text) {
    for (char c = *text; c; c = *++text)
        if (c == '\r' || c == '\n') *text = 0;
}

// A label, then the bytes of a wide text as " %02x".
void AddWideHex(StdString *to, char *buffer, uintptr_t label, const char *wide) {
    Add(to, Text(label));
    const char *from = wide ? wide : kNoName;
    char *out = buffer;
    while (from[0] && !from[1]) {
        Crt().sprintf(out, Text(0x00774028), (s32)(signed char)from[0]);
        Crt().sprintf(out + 3, Text(0x00774028), (s32)(signed char)from[1]);
        from += 2;
        out += 6;
    }
    Crt().sprintf(out, Text(0x00773cdc));
    Add(to, buffer);
}

void AddAttachment(StdString *to, char *buffer, const char *boundary, const u8 *attachment) {
    AddFormat(to, buffer, 0x00773e98, boundary);
    AddFormat(to, buffer, 0x00773e78, OrNull(at<char *>(attachment, 0x14)), OrNull(at<char *>(attachment, 4)));
    Add(to, Text(0x00773e54));
    AddFormat(to, buffer, 0x00773e3c, OrNull(at<char *>(attachment, 4)));
    AddFormat(to, buffer, 0x00773e08, OrNull(at<char *>(attachment, 4)));
    AddFormat(to, buffer, 0x00773df0, OrNull(at<char *>(attachment, 4)));
    Add(to, at<char *>(attachment, 0x20));
}

}  // namespace

extern "C" void __fastcall PostcardMail_Build(u8 *mail) {
    char frame[0xc0];
    char *const buffer = frame, *const boundary = frame + 0x80;
    Crt().sprintf(boundary, Text(0x0077409c), (*reinterpret_cast<u32(__stdcall **)()>((uintptr_t)0x006fd1d0))());
    StdString *const to = &at<StdString>(mail, 0x74);
    StdString_AssignChars(to, nullptr, Text(0x00786de4), (u32)std::strlen(Text(0x00786de4)));
    CutLines(OrNull(at<char *>(mail, 4)));
    AddFormat(to, buffer, 0x00774090, OrNull(at<char *>(mail, 4)));
    Add(to, Text(0x00774088));
    for (const u8 *node = at<const u8 *>(at<const u8 *>(mail, 0x14), 0); node != at<const u8 *>(mail, 0x14);) {
        CutLines(OrNull(at<char *>(node, 0xc)));
        AddFormat(to, buffer, (u32)&gText_Quickload_74d1f0, OrNull(at<char *>(node, 0xc)));
        node = at<const u8 *>(node, 0);
        if (node != at<const u8 *>(mail, 0x14)) Add(to, Text(0x00774084));
    }
    Add(to, Text(0x00773cdc));
    CutLines(OrNull(at<char *>(mail, 0x20)));
    AddFormat(to, buffer, 0x00774074, OrNull(at<char *>(mail, 0x20)));
    AddFormat(to, buffer, 0x0077405c, at<u32>(mail, 0x3c));
    AddFormat(to, buffer, 0x00774044, at<u32>(mail, 0x40));
    if (at<u32>(mail, 0x4c)) AddWideHex(to, buffer, 0x00774030, at<char *>(mail, 0x48));
    if (at<u32>(mail, 0x5c)) AddWideHex(to, buffer, 0x00774014, at<char *>(mail, 0x58));
    AddFormat(to, buffer, 0x00774000, at<u32>(mail, 0x64));
    Add(to, Text(0x00773fec));
    AddFormat(to, buffer, 0x00773fb8, boundary);
    AddFormat(to, buffer, 0x00773f8c);
    Add(to, Text(0x00773f00));
    AddFormat(to, buffer, 0x00773ef4, boundary);
    Add(to, Text(0x00773ec4));
    Add(to, Text(0x00773ea4));
    Add(to, Text(0x00773d00));
    const u8 *const attachment = at<const u8 *>(mail, 0x68);
    if (attachment && at<char *>(attachment, 0x20)) AddAttachment(to, buffer, boundary, attachment);
    AddFormat(to, buffer, 0x00773de0, boundary);
}
