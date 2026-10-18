#include "edittools.h"

#include <cstdint>

#include "../core/actionrec.h"
#include "../math/x87.h"
#include "../model/highlight.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &g(u32 address) { return *reinterpret_cast<T *>(address); }

u32 &Flags() { return g<u32>(0x0079c658); }
u8 *Piece() { return g<u8 *>(0x0079c640); }
const u8 *Def(const u8 *piece) { return at<const u8 *>(piece, 0x100); }
const u8 *Type(const u8 *piece) { return at<const u8 *>(Def(piece), 0x18); }
ActionRec *Recorder() { return reinterpret_cast<ActionRec *>(0x00785a08); }
void ClearChanged() { at<u32>(g<u8 *>(0x00790fec), 0) &= ~0x2000u; }

// The tool off: flags 0x1e00 cleared, the highlight hidden.
u32 Off(u32 flags) {
    Flags() = flags & 0xffffe1ff;
    Highlight_SetShown(0);
    return 1;
}

// The tool (bit) taken back for want of a piece.
u32 NoPiece(u32 flags, u32 bit) {
    Flags() = flags & ~bit;
    Highlight_SetShown(0);
    return 1;
}

}  // namespace

extern "C" {

u32 __cdecl CoasterEdit_ToggleAdjustF8() {
    u32 flags = Flags();
    u8 *piece = Piece();
    if (!(flags & 4) && (!piece || !(at<u32>(Type(piece), 0) & 0x200)))
        return 1;
    if (at<u8>(Def(piece), 4) & 2)
        return 1;
    if (!(flags & 0x3f)) {
        flags |= 4;
        Flags() = flags;
    } else if (flags & 4) {
        ActionRec_WriteFFW(Recorder(), nullptr, 0x3b, at<u32>(piece, 0xf8), 0, at<u16>(piece, 0));
        ClearChanged();
        flags = Flags();
        piece = Piece();
        flags &= ~4u;
        Flags() = flags;
    }
    if (!(flags & 4))
        return Off(flags);
    g<u32>(0x0079c618) = 0;
    g<u32>(0x0079c630) = g<u32>(0x00790ac8);
    g<u32>(0x0079c634) = g<u32>(0x00790acc);
    if (!piece)
        return NoPiece(flags, 4);
    g<u32>(0x0079c6fc) = 0;
    g<u32>(0x0079c6f8) = 0;
    if ((at<u8>(piece, 8) & 0x80) && at<float>(piece, 0xf8) > 0.5f) {   // (0x6fe1bc; not taken unordered)
        at<float>(piece, 0xf8) = static_cast<float>(X87Reg(at<float>(piece, 0xf8)) - X87Reg(1.0));   // (0x6fe1a8)
        piece = Piece();
    }
    g<u32>(0x0079c610) = 0;
    g<u8 *>(0x0079c614) = piece + 0xf8;
    g<u32>(0x0079c6f4) = 0;
    g<u32>(0x0079c6f0) = at<u32>(piece, 0xf8);
    if (at<u8>(piece, 8) & 0x80) {
        g<u32>(0x0079c61c) = 0xbeb33333;   // -0.35f
        g<u32>(0x0079c65c) = 0x3eb33333;   // 0.35f
    } else {
        g<u32>(0x0079c61c) = 0x3e19999a;   // 0.15f
        g<u32>(0x0079c65c) = 0x3f59999a;   // 0.85f
    }
    flags = Flags() | 0x200;
    g<u32>(0x0079c6ec) = 0x3c23d70a;   // 0.01f
    Flags() = flags;
    Highlight_SetShown(1);
    return 1;
}

u32 __cdecl CoasterEdit_ToggleAdjustE0() {
    const u8 *const game = g<const u8 *>(0x00790fec);
    if (!game || !at<u32>(game, 0x40))
        return 1;
    u32 flags = Flags();
    u8 *piece = Piece();
    if (!(flags & 2) && (!piece || !(at<u8>(Type(piece), 0) & 0x40)))
        return 1;
    if (at<u8>(Def(piece), 4) & 2)
        return 1;
    if (!(flags & 0x3f)) {
        flags |= 2;
        Flags() = flags;
    } else if (flags & 2) {
        // (the word's high half what ECX held: the flags')
        ActionRec_WriteFW(Recorder(), nullptr, 0x3a, at<u32>(piece, 0xe0), (flags & 0xffff0000u) | at<u16>(piece, 0));
        ClearChanged();
        flags = Flags();
        piece = Piece();
        flags &= ~2u;
        Flags() = flags;
    }
    if (!(flags & 2))
        return Off(flags);
    g<u32>(0x0079c668) = 0;
    g<u32>(0x0079c630) = g<u32>(0x00790ac8);
    g<u32>(0x0079c634) = g<u32>(0x00790acc);
    if (!piece)
        return NoPiece(flags, 2);
    g<u32>(0x0079c6fc) = 0;
    g<u32>(0x0079c6f8) = 0;
    g<u8 *>(0x0079c614) = piece + 0xe0;
    g<u32>(0x0079c610) = 0;
    g<u32>(0x0079c6f0) = at<u32>(piece, 0xe0);
    flags |= 0x200;
    g<float>(0x0079c61c) = static_cast<float>(X87Reg(at<float>(piece, 0xd4)) - X87Reg(at<float>(Type(piece), 0x94)));
    const X87Reg high = X87Reg(at<float>(Type(piece), 0x94)) + X87Reg(at<float>(piece, 0xd4));
    g<u32>(0x0079c6ec) = 0x3ca3d70a;   // 0.02f
    Flags() = flags;
    g<float>(0x0079c65c) = static_cast<float>(high);
    at<u32>(piece, 8) |= 1;
    Highlight_SetShown(1);
    return 1;
}

u32 __cdecl CoasterEdit_ToggleAdjustAngles() {
    u32 flags = Flags();
    u8 *piece = Piece();
    if (!(flags & 4) && (!piece || !(at<u32>(Type(piece), 0) & 0x180)))
        return 1;
    if (at<u8>(Def(piece), 4) & 2)
        return 1;
    if (!(flags & 0x3f)) {
        flags |= 0x2004;
        Flags() = flags;
    } else if (flags & 4) {
        ActionRec_WriteFFW(Recorder(), nullptr, 0x3b, at<u32>(piece, 0xd8), at<u32>(piece, 0xdc), at<u16>(piece, 0));
        ClearChanged();
        flags = Flags();
        piece = Piece();
        flags &= 0xffffdffb;
        Flags() = flags;
    }
    if (!(flags & 4))
        return Off(flags);
    g<u32>(0x0079c668) = 0;
    g<u32>(0x0079c630) = g<u32>(0x00790ac8);
    g<u32>(0x0079c618) = 0;
    g<u32>(0x0079c634) = g<u32>(0x00790acc);
    if (!piece)
        return NoPiece(flags, 4);
    if (at<u32>(Type(piece), 0) & 0x100) {
        g<u32>(0x0079c610) = 0;
        g<u8 *>(0x0079c614) = piece + 0xd8;
        flags |= 0x200;
        g<u32>(0x0079c61c) = 0xc0490e38;   // -pi
        g<u32>(0x0079c6f0) = at<u32>(piece, 0xd8);
        g<u32>(0x0079c65c) = 0x40490e38;   // pi
        g<u32>(0x0079c6ec) = 0x3ca3d70a;   // 0.02f
        Flags() = flags;
        at<u32>(piece, 8) |= 4;
        piece = Piece();
    }
    if (at<u8>(Type(piece), 0) & 0x80) {
        g<u32>(0x0079c6f8) = 0;
        g<u8 *>(0x0079c6fc) = piece + 0xdc;
        g<u32>(0x0079c6d4) = at<u32>(piece, 0xdc);
        g<u32>(0x0079c6e4) = 0xc0490e38;
        flags = Flags() | 0x400;
        g<u32>(0x0079c66c) = 0x40490e38;
        g<u32>(0x0079c6a8) = 0x3ca3d70a;
        Flags() = flags;
        at<u32>(piece, 8) |= 2;
    }
    Highlight_SetShown(1);
    return 1;
}
}
