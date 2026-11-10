#include "edittools.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/actionrec.h"
#include "../core/addrcalls.h"
#include "../files/wrapsources.h"
#include "../math/x87.h"
#include "../ui/dialog7ca1c8.h"
#include "../model/highlight.h"
#include "../world/cellmessage.h"
#include "../world/money.h"
#include "../world/mousemode.h"
#include "../world/object.h"
#include "../world/pathdrag.h"
#include "../world/world.h"
#include "cars.h"
#include "cursor.h"
#include "helpers.h"
#include "registry.h"
#include "train.h"
#include "placecar.h"
#include "coastersupdate.h"
#include "coastersload.h"
#include "../world/rides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &g(u32 address) { return *reinterpret_cast<T *>(address); }

u32 &Flags() { return gPlacecar_79c658; }
u8 *Piece() { return gEdittools_79c640; }
const u8 *Def(const u8 *piece) { return at<const u8 *>(piece, 0x100); }
const u8 *Type(const u8 *piece) { return at<const u8 *>(Def(piece), 0x18); }
ActionRec *Recorder() { return reinterpret_cast<ActionRec *>(0x00785a08); }
void ClearChanged() { at<u32>(gCoastersload_790fec, 0) &= ~0x2000u; }

const EditToolsEnv kPortEnv = {&Coaster_DeleteCar,           &Coaster_DeleteFlaggedEnd,       &Coaster_SetState,
                               &MouseMode_EnterTile15,       &CoasterEdit_ToggleAdjustF8,     &CoasterEdit_ToggleAdjustAngles,
                               &CoasterEdit_ToggleDrag,      &CoasterEdit_ToggleAdjustE0,     &CoasterEdit_MoveCarBack,
                               &CoasterEdit_MoveCarForward,  &CoasterEdit_DeleteSpan,         &CoasterEdit_SelectPiece,
                               &TrackSelection_ClearFlags,   &CoasterLinks_FindTile,          &Coaster_DropCar,
                               &PathDrag_Finish,             &Map_SetRideCell,                &Car_TravelCost,
                               &Coaster_FlashCar,            &CoasterEdit_ToggleAdjust,       &TrackSelection_End,
                               &TrackSelection_Clear};
const EditToolsEnv *g_env = &kPortEnv;

// The current link's end car (+0x58 without +0x3c flag 1, +0x60 with flags 1 and 2, else none) when its +4 byte has flag 2: whether that
// car's +8 has all of on and none of off.
u32 EndCarHas(const u8 *game, u32 on, u32 off) {
    if (!game || !(*at<const u8 *>(game, 4) & 2)) return 0;
    const u32 state = at<u32>(game, 0x3c);
    const u8 *car;
    if (!(state & 1)) car = at<const u8 *>(game, 0x58);
    else if (state & 2) car = at<const u8 *>(game, 0x60);
    else return 0;
    const u32 flags = at<u32>(car, 8);
    return (flags & on) == on && !(flags & off) ? 1 : 0;
}

// A car's ride part entry's +0x34 (the train type's +0x2c a 0x14-byte index into its ride's model (+8) +0x28 -> +4, its +4), or
// [0x6fe1b4] for none: fld / fstp, as bits.
u32 PartEntryValue(const u8 *car) {
    const u8 *const model = at<const u8 *>(gRides_7a4610[at<u32>(car, 0x70)], 8);
    const u32 part = at<u32>(at<const u8 *>(at<const u8 *>(car, 0x100), 0x18), 0x2c);
    const u8 *const entry = at<const u8 *>(at<const u8 *>(at<const u8 *>(model, 0x28), 4), part * 0x14 + 4);
    return X87_FloatThrough(entry ? at<u32>(entry, 0x34) : gEdittools_6fe1b4);
}

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
    gCoasterEdit_79c618 = 0;
    gCoasterEdit_79c630 = g<u32>(0x00790ac8);
    gCoasterEdit_79c634 = g<u32>(0x00790acc);
    if (!piece)
        return NoPiece(flags, 4);
    gCoasterEdit_79c6fc = 0;
    gCoasterEdit_79c6f8 = 0;
    if ((at<u8>(piece, 8) & 0x80) && at<float>(piece, 0xf8) > 0.5f) {   // (0x6fe1bc; not taken unordered)
        at<float>(piece, 0xf8) = static_cast<float>(X87Reg(at<float>(piece, 0xf8)) - X87Reg(1.0));   // (0x6fe1a8)
        piece = Piece();
    }
    gCoasterEdit_79c610 = 0;
    gCoasterEdit_79c614 = piece + 0xf8;
    gCoasterEdit_79c6f4 = 0;
    gCoasterEdit_79c6f0 = at<u32>(piece, 0xf8);
    if (at<u8>(piece, 8) & 0x80) {
        gCoasterEdit_79c61c = 0xbeb33333;   // -0.35f
        gCoasterEdit_79c65c = 0x3eb33333;   // 0.35f
    } else {
        gCoasterEdit_79c61c = 0x3e19999a;   // 0.15f
        gCoasterEdit_79c65c = 0x3f59999a;   // 0.85f
    }
    flags = Flags() | 0x200;
    gCoasterEdit_79c6ec = 0x3c23d70a;   // 0.01f
    Flags() = flags;
    Highlight_SetShown(1);
    return 1;
}

u32 __cdecl CoasterEdit_ToggleAdjustE0() {
    const u8 *const game = gCoastersload_790fec;
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
    gCoasterEdit_79c668 = 0;
    gCoasterEdit_79c630 = g<u32>(0x00790ac8);
    gCoasterEdit_79c634 = g<u32>(0x00790acc);
    if (!piece)
        return NoPiece(flags, 2);
    gCoasterEdit_79c6fc = 0;
    gCoasterEdit_79c6f8 = 0;
    gCoasterEdit_79c614 = piece + 0xe0;
    gCoasterEdit_79c610 = 0;
    gCoasterEdit_79c6f0 = at<u32>(piece, 0xe0);
    flags |= 0x200;
    reinterpret_cast<float &>(gCoasterEdit_79c61c) = static_cast<float>(X87Reg(at<float>(piece, 0xd4)) - X87Reg(at<float>(Type(piece), 0x94)));
    const X87Reg high = X87Reg(at<float>(Type(piece), 0x94)) + X87Reg(at<float>(piece, 0xd4));
    gCoasterEdit_79c6ec = 0x3ca3d70a;   // 0.02f
    Flags() = flags;
    reinterpret_cast<float &>(gCoasterEdit_79c65c) = static_cast<float>(high);
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
    gCoasterEdit_79c668 = 0;
    gCoasterEdit_79c630 = g<u32>(0x00790ac8);
    gCoasterEdit_79c618 = 0;
    gCoasterEdit_79c634 = g<u32>(0x00790acc);
    if (!piece)
        return NoPiece(flags, 4);
    if (at<u32>(Type(piece), 0) & 0x100) {
        gCoasterEdit_79c610 = 0;
        gCoasterEdit_79c614 = piece + 0xd8;
        flags |= 0x200;
        gCoasterEdit_79c61c = 0xc0490e38;   // -pi
        gCoasterEdit_79c6f0 = at<u32>(piece, 0xd8);
        gCoasterEdit_79c65c = 0x40490e38;   // pi
        gCoasterEdit_79c6ec = 0x3ca3d70a;   // 0.02f
        Flags() = flags;
        at<u32>(piece, 8) |= 4;
        piece = Piece();
    }
    if (at<u8>(Type(piece), 0) & 0x80) {
        gCoasterEdit_79c6f8 = 0;
        reinterpret_cast<u8 * &>(gCoasterEdit_79c6fc) = piece + 0xdc;
        gCoasterEdit_79c6d4 = at<u32>(piece, 0xdc);
        gCoasterEdit_79c6e4 = 0xc0490e38;
        flags = Flags() | 0x400;
        gCoasterEdit_79c66c = 0x40490e38;
        gCoasterEdit_79c6a8 = 0x3ca3d70a;
        Flags() = flags;
        at<u32>(piece, 8) |= 2;
    }
    Highlight_SetShown(1);
    return 1;
}

u32 __cdecl CoasterEdit_ToggleAdjust() {
    const u8 *const game = gCoastersload_790fec;
    if (!game || !at<u32>(game, 0x40))
        return 1;
    const u8 *const piece = Piece();
    if (!piece || (at<u8>(Def(piece), 4) & 2))
        return 1;
    return (at<u8>(piece, 8) & 0x40) ? CoasterEdit_ToggleAdjustF8() : CoasterEdit_ToggleAdjustAngles();
}

u32 __cdecl CoasterEdit_ToggleDrag() {
    const u8 *game = gCoastersload_790fec;
    if (!game || !at<u32>(game, 0x40))
        return 1;
    u32 flags = Flags();
    u8 *piece = Piece();
    if (!(flags & 1) && (!piece || !(at<u8>(Type(piece), 0) & 0x20)))
        return 1;
    if (at<u8>(Def(piece), 4) & 2)   // (sic: with the tool on and no piece this faults)
        return 1;
    if (!(flags & 0x3f)) {
        flags |= 1;
        Flags() = flags;
    } else if (flags & 1) {
        ActionRec_WriteFW(Recorder(), nullptr, 0x39, at<u32>(piece, 0xe8), at<u16>(piece, 0));
        const u8 *const selected = Piece();
        const u32 pieceFlags = at<u32>(selected, 8);
        if (pieceFlags & 0x40) {
            const u8 *const other = at<const u8 *>(selected, (pieceFlags & 0x80) ? 0x64 : 0x68);
            if (other && at<u32>(other, 0x100))   // (the word's high half what ECX held: the other's +0x100)
                ActionRec_WriteFW(Recorder(), nullptr, 0x39, at<u32>(other, 0xe8), (at<u32>(other, 0x100) & 0xffff0000u) | at<u16>(other, 0));
        }
        ClearChanged();
        flags = Flags() & ~1u;
        game = gCoastersload_790fec;
        piece = Piece();
        Flags() = flags;
    }
    if (flags & 1) {
        gCoasterEdit_79c618 = 0;
        gCoasterEdit_79c630 = g<u32>(0x00790ac8);
        gCoasterEdit_79c634 = g<u32>(0x00790acc);
        if (!piece)
            return NoPiece(flags, 1);
        const double cost = Car_TravelCost(game, 0, piece, nullptr, nullptr, nullptr, nullptr);
        piece = Piece();
        gCoasterEdit_79c6f8 = 0;
        gEdittools_79c698 = static_cast<float>(cost);
        reinterpret_cast<u32 &>(gCoasterEdit_79c614) = 0;
        reinterpret_cast<u8 * &>(gCoasterEdit_79c6fc) = piece + 0xe8;
        gCoasterEdit_79c610 = 0;
        gCoasterEdit_79c6d4 = at<u32>(piece, 0xe8);
        const u8 *type = Type(piece);
        double low = CarType_Unlerp(at<float>(type, 0x68), type);
        if (!(low > 0.0))   // (fcom 0x6fe1b4: below, equal or unordered)
            low = 0.0;
        u8 *const selected = Piece();
        reinterpret_cast<float &>(gCoasterEdit_79c6e4) = static_cast<float>(low);
        const u8 *const def = Def(selected);
        const float length = static_cast<float>(Car_TrainLength(at<const u8 *>(def, 0x30 + 4 * at<u32>(def, 0x30))));
        type = at<const u8 *>(def, 0x18);
        const float scale = static_cast<float>(CarType_Scale(static_cast<float>(X87Reg(at<float>(type, 0x6c)) - X87Reg(length)), type));
        reinterpret_cast<float &>(gCoasterEdit_79c66c) = static_cast<float>(X87Reg(at<float>(selected, 0xe8)) + X87Reg(scale));
        gCoasterEdit_79c6d0 = 0;            // 0.0f (0x6fe1b4)
        gCoasterEdit_79c6a8 = 0x3ba3d70a;   // 0.005f (0x6fe1b8)
        Flags() = Flags() | 0x400;
        Highlight_SetShown(1);
        return 1;
    }
    if (!g<u32>(0x00785a2c)) {
        const s32 steps = Float_RoundUp(static_cast<float>(Car_TravelCost(game, 0, piece, nullptr, nullptr, nullptr, nullptr)));
        u8 *const money = reinterpret_cast<u8 *>(GameTable7cfb90_Get1da726_5195d0(g<u8 *>(0x007cf83c)));
        Money_Credit(money, nullptr, (s32)(long long)gEdittools_79c698);   // __ftol
        Source_Field0c(reinterpret_cast<const TrackedSource *>(money));   // (its result unused)
        Money_Debit(money, nullptr, steps);
        reinterpret_cast<u32 &>(gEdittools_79c698) = 0;
    }
    return Off(Flags());
}
}

namespace {

// 0x4423f0 / 0x4425c0: the move tool (bit) toggled; on, the selected car swapped with its neighbour (next: the one after, else the
// one before), recorded as code with the car's handle, and again while there is a game (the tool then going off).
void MoveCar(u32 bit, u32 code, bool next) {
    for (u8 *game = gCoastersload_790fec; game; game = gCoastersload_790fec) {
        if (!at<u32>(game, 0x40)) return;
        u32 flags = Flags();
        u8 *const car = Piece();
        const u32 on = flags & bit;
        if (!on) {
            if (!car) return;
            if (next ? at<u32>(car, 4) >= at<u32>(Def(car), 0x30) - 1 : at<u32>(car, 4) == 0) return;
        }
        if (at<u8>(Def(car), 4) & 2) return;   // (sic: with the tool on, a missing car faults)
        if (!(flags & 0x3f)) {
            Flags() = flags |= bit;
        } else if (on) {
            Flags() = flags &= ~bit;
        }
        if (!(flags & bit)) {
            Highlight_SetShown(0);
            return;
        }
        if (!car) {
            Flags() = flags & ~bit;
            Highlight_SetShown(0);
            return;
        }
        // (the word pushed with the pointer's high half, sic)
        ActionRec_WriteW(Recorder(), nullptr, code, ((u32)(uintptr_t)car & 0xffff0000u) | at<u16>(car, 0));
        ClearChanged();
        u8 *const piece = Piece();
        Train_SwapCars(piece, at<u8 *>(Def(piece), 0x34 + 4 * at<u32>(piece, 4) + (next ? 4 : -4)));
        Highlight_SetShown(1);
    }
}

}  // namespace

extern "C" {

void __cdecl CoasterEdit_MoveCarBack() { MoveCar(8, 0x3d, true); }

void __cdecl CoasterEdit_MoveCarForward() { MoveCar(0x10, 0x3e, false); }
}

namespace {

// What 0x442220 and 0x444360 both do first: with 0x79c624 bit 0 and a game, the game's flag 4 cleared, Flags_SetChanged (game, 0)
// and, unless its +0x3c has bit 0 without bit 1, Coaster_FlashCar (game, 1); then the tool in use (flags bits 0-5) toggled off.
void EndTool() {
    if ((reinterpret_cast<u8 &>(gEdittools_79c624) & 1) && gCoastersload_790fec) {
        at<u32>(gCoastersload_790fec, 0) &= ~4u;
        Flags_SetChanged(reinterpret_cast<u32 * &>(gCoastersload_790fec), 0);
        u8 *const game = gCoastersload_790fec;
        const u32 f = at<u32>(game, 0x3c);
        if (!(f & 1) || (f & 2)) Coaster_FlashCar(game, 1);
    }
    switch (Flags() & 0x3f) {
    case 1: CoasterEdit_ToggleDrag(); break;
    case 2: CoasterEdit_ToggleAdjustE0(); break;
    case 4: CoasterEdit_ToggleAdjust(); break;   // (inlined in both)
    case 8: CoasterEdit_MoveCarBack(); break;
    case 0x10: CoasterEdit_MoveCarForward(); break;
    case 0x20: TrackSelection_End(); break;   // (inlined in both)
    default: break;
    }
}

void SetModeState(u32 mode) {
    gEdittools_79c664 = 0;
    gEdittools_79c644 = 0;
    gEdittools_79c624 = mode;
    Flags() &= ~0x40u;
}

}  // namespace

extern "C" {

void __cdecl CoasterEdit_EndTool() {
    EndTool();
    SetModeState(0);
}

void __cdecl CoasterEdit_PickPiece(u32, u32, u32 number) {
    EndTool();
    SetModeState(2);
    const u32 piece = Track_PieceAt(gCoastersload_790fec + 0x3c, number & 0xffff);
    reinterpret_cast<u32 &>(gEdittools_79c640) = piece;
    g_env->selectPiece(reinterpret_cast<u8 *>((uintptr_t)piece));
}

void __cdecl CoasterEdit_DeleteSpanBetween(u32 first, u32 second) {
    reinterpret_cast<u32 &>(gEdittools_79c640) = Track_PieceAt(gCoastersload_790fec + 0x3c, first & 0xffff);
    g_env->deleteSpan();
    reinterpret_cast<u32 &>(gEdittools_79c640) = Track_PieceAt(gCoastersload_790fec + 0x3c, second & 0xffff);
    g_env->deleteSpan();
}

void __cdecl CoasterEdit_SetMode(u32 mode) {
    EndTool();
    SetModeState(mode);
    if ((u8)mode != 1) return;
    Coaster_FlashCar(gCoastersload_790fec, 2);
    at<u32>(gCoastersload_790fec, 0) |= 4;   // (sic: before the game is tested)
    const u8 *const game = gCoastersload_790fec;
    if (!game || !(at<u8>(at<const u8 *>(game, 4), 0) & 2)) return;
    const u32 f = at<u32>(game, 0x3c);
    bool flagged;
    if (!(f & 1))
        flagged = at<u8>(at<const u8 *>(game, 0x58), 8) & 0x80;
    else
        flagged = (f & 2) && (at<u8>(at<const u8 *>(game, 0x60), 8) & 0x80);
    if (flagged) {
        gEdittools_79c664 = 1;
        Flags() |= 0x40;
    }
}

// Tool 1 started (0x444770 and 0x442880 alike): CoasterEdit_EndTool, the mode 1, the car flashed, the game's flag 4, the end checked.
static void StartTool1() {
    CoasterEdit_EndTool();
    gEdittools_79c624 = 1;
    Coaster_FlashCar(gCoastersload_790fec, 2);
    at<u32>(gCoastersload_790fec, 0) |= 4;
    CoasterLink_CheckEnd();
}

// The rest of tool 1's start (0x444770 and 0x445220): recorded (0x37), the game's flag 0x2000 cleared, CoasterLink_ClearEnd when
// 0x79c644 and 0x79c664 are both 2, 0x79c664 = 0, 0x79c644 = 1, the bounds set.
static void Tool1Rest() {
    ActionRec_Write(Recorder(), nullptr, 0x37);
    at<u32>(gCoastersload_790fec, 0) &= ~0x2000u;
    if (gEdittools_79c644 == 2 && gEdittools_79c664 == 2) CoasterLink_ClearEnd();
    gEdittools_79c664 = 0;
    gEdittools_79c644 = 1;
    CoasterEdit_SetBounds();
}

void __cdecl CoasterEdit_SwitchToTool1() {
    CoasterEdit_Tools(nullptr);
    CoasterEdit_EndTool();
    gCoasterEdit_79c638 &= ~2u;
    StartTool1();
    Tool1Rest();
    gCoasterEdit_79c638 |= 1;
    Dialog7ca1c8_SetChecksIfUpThis(reinterpret_cast<void *>(0x007b51f0), nullptr, gCoasterEdit_79c6e0);
    Dialog7ca1c8_SelectButtonIfUpThis(reinterpret_cast<void *>(0x007b51f0), nullptr, gCoasterEdit_79c638);
}

void __cdecl CoasterEdit_SetTools(u32 tools) {
    EndTool();
    SetModeState(0);
    gCoasterEdit_79c6e0 = 0;
    gCoasterEdit_79c638 = 0;
    for (u32 bit = 1, left = tools; left; bit <<= 1) {
        if (!(left & bit)) continue;
        switch (bit) {
        case 1:
            StartTool1();
            CoasterEdit_SetTool(1);
            break;
        case 2:
            CoasterEdit_SetMode(1);
            CoasterEdit_SetTool(2);
            break;
        case 4: CoasterEdit_SetMode(2); break;
        case 8: CoasterEdit_SetMode(4); break;
        case 0x10: CoasterEdit_SetMode(8); break;
        case 0x20: CoasterEdit_SetMode(0x10); break;
        case 0x40: CoasterEdit_SetMode(0x80); break;
        case 0x80: CoasterEdit_SetMode(0x20); break;
        case 0x100: CoasterEdit_SetMode(0x40); break;
        default: break;
        }
        gCoasterEdit_79c638 |= bit;
        left &= ~bit;
    }
    CoasterEdit_RefreshTools();
}

void __cdecl CoasterEdit_StartTool(u32 tool) {
    switch (tool) {
    case 1:
        StartTool1();
        Tool1Rest();
        break;
    case 2:
        StartTool1();
        ActionRec_Write(Recorder(), nullptr, 0x38);
        at<u32>(gCoastersload_790fec, 0) &= ~0x2000u;
        if (CoasterLink_EndMarked()) {   // (the original tests the game and its type's flag 2 again: EndMarked's own tests)
            gEdittools_79c664 = 1;
            Flags() |= 0x40;
        } else {
            CoasterLink_BeginEnd();
        }
        gEdittools_79c644 = 2;
        CoasterEdit_SetBounds();
        break;
    case 4: CoasterEdit_SetMode(2); break;   // (4 .. 0x80 inlined: CoasterEdit_EndTool, the mode set)
    case 8: CoasterEdit_SetMode(4); break;
    case 0x10: CoasterEdit_SetMode(8); break;
    case 0x20: CoasterEdit_SetMode(0x10); break;
    case 0x40: CoasterEdit_SetMode(0x80); break;
    case 0x80: CoasterEdit_SetMode(0x20); break;
    case 0x100: CoasterEdit_SetMode(0x40); break;
    default: break;
    }
    gCoasterEdit_79c638 |= tool;
}

void __cdecl CoasterEdit_EndAll() {
    EndTool();
    SetModeState(0);
    gCoasterEdit_79c6e0 = 0;
    gCoasterEdit_79c638 = 0;
}

void __cdecl CoasterEdit_StartChosenTool(u32 *tool) {
    const u8 *const game = gCoastersload_790fec;
    const u32 f = at<u32>(game, 0x3c);   // (sic: before the game is tested)
    if ((f & 1) && !(f & 2)) {
        *tool = 0x10;
    } else {
        u32 marked;
        if (CoasterLink_EndMarked()) {   // (inlined)
            Flags() |= 0x40;
            marked = gEdittools_79c664 = 1;
        } else {
            marked = gEdittools_79c664;
        }
        *tool = marked != 1 ? 1 : 2;
    }
    CoasterEdit_StartTool(*tool);   // (inlined in full; the CoasterLink_SetEnd it calls tests the game and its type again)
}

void __cdecl CoasterEdit_ToolButton(u32 tool, u32 action) {
    if (action & 2) {
        if (tool & 0x1ff) {
            EndTool();
            SetModeState(0);
            gCoasterEdit_79c638 &= ~tool;
        }
    } else if (action & 1) {
        if (tool == 2) {
            StartTool1();
            CoasterEdit_SetTool(2);
            gCoasterEdit_79c638 |= tool;
        } else {
            CoasterEdit_StartTool(tool);   // (1 and 4 .. 0x100 as StartTool; SetMode called, not inlined)
        }
    } else if (action & 4) {
        CoasterEdit_EndAll();   // (inlined)
    }
    if (tool) {
        CoasterEdit_Tools(nullptr);
        Dialog7ca1c8_SetChecksIfUpThis(reinterpret_cast<void *>(0x007b51f0), nullptr, gCoasterEdit_79c6e0);
    }
}
void __cdecl CoasterEdit_SetEditing(u8 *coaster, u32 on) {
    ActionRec_WriteW(Recorder(), nullptr, 0x44, coaster && on ? at<u32>(coaster, 0x14) : 0);
    if (coaster && on) {
        if (!gCoastersload_790fec) {
            Object_RestartRide(at<u8 *>(coaster, 0x28));
            const u32 f = at<u32>(coaster, 0x3c);
            Coaster_SetState(coaster, (f & 1) && !(f & 2) ? 2 : 1);
        }
        CoasterLink_SetCurrent(coaster);
        return;
    }
    if (!gCoastersload_790fec) return;
    EndTool();
    gEdittools_79c664 = 0;
    gEdittools_79c644 = 0;
    Flags() &= ~0x40u;
    gEdittools_79c624 = 0;
    gCoasterEdit_79c6e0 = 0;
    gCoasterEdit_79c638 = 0;
    u8 *const game = gCoastersload_790fec;
    Flags_SetChanged(reinterpret_cast<u32 *>(game), 0);
    CoasterLink_SetCurrent(nullptr);
    if (at<u8>(game, 8) & 2) Object_Close(at<u8 *>(game, 0x28));
}

void __cdecl CoasterEdit_EditRide(const u8 *object, u32 on) {
    u8 *const link = object ? CoasterLink_ById(at<u32>(object, 0x20)) : nullptr;
    if (g<u32>(0x00785a2c)) return;
    if (on)
        CoasterEdit_SetEditing(link, 1);
    else
        CoasterEdit_SetEditing(nullptr, 0);
}

void __cdecl CoasterEdit_SetEditingById(u32 id) { CoasterEdit_SetEditing(CoasterLink_ById(id), id != 0); }

void __cdecl CoasterEdit_SetEditingAt(u32 index) {
    CoasterEdit_SetEditing(index ? gCoasters_790bd0[index] : nullptr, index != 0);
}

// The pieces from the lower handle (+0) of a and b up to the other (by +0x64, the other left out): flag 0x1000000 set, or 0x1004000 cleared.
static void mark_span(u8 *a, u8 *b, bool set) {
    u8 *from = a, *to = b;
    if (at<u32>(a, 0) > at<u32>(b, 0)) {
        from = b;
        to = a;
    }
    for (; from != to; from = at<u8 *>(from, 0x64)) {
        if (set)
            at<u32>(from, 8) |= 0x1000000u;
        else
            at<u32>(from, 8) &= 0xfeffbfffu;
    }
}

u32 __cdecl CoasterEdit_SelectPiece(u8 *piece) {
    if (piece && (at<u8>(at<u8 *>(piece, 0x100), 4) & 2)) return 0;
    gEdittools_79c69c = 0;
    if (!piece) return 0;
    const u32 many = at<u32>(at<u8 *>(piece, 0x100), 0x30) > 1 ? 1 : 0;
    gEdittools_79c6a4 = piece;
    Flags() |= 0x100;
    gEdittools_79c660 = at<u8 *>(piece, 0x100);
    gEdittools_79c694 = static_cast<float>(Car_TrainLength(piece));
    ActionRec_WriteWWW(Recorder(), nullptr, 0x3c, 0, 0, at<u16>(piece, 0));
    at<u32>(gCoastersload_790fec, 0) &= ~0x2000u;
    u8 *const first = at<u8 *>(piece, 0x68) ? at<u8 *>(piece, 0x68) : piece;
    u8 *const prev = gEdittools_79c628;
    gEdittools_79c62c = first;
    gEdittools_7925d0 = 3;
    u8 *const next = at<u8 *>(piece, 0x64);
    if (prev) mark_span(first, prev, false);
    gEdittools_79c628 = next ? next : piece;
    mark_span(first, next ? next : piece, true);
    Flags_SetChanged(reinterpret_cast<u32 * &>(gCoastersload_790fec), 0);
    const u8 *const def = at<u8 *>(piece, 0x100);
    s32 x = at<s32>(def, 0xc), y = at<s32>(def, 0x10);
    gEdittools_79c6d8 = x;
    gEdittools_79c6a0 = y;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= g<s32>(0x007a0314)) x = g<s32>(0x007a0314) - 1;
    if (y >= g<s32>(0x007a0374)) y = g<s32>(0x007a0374) - 1;
    PathDrag_Begin(x, y, at<u32>(at<u8 *>(piece, 0x100), 0x30) > 1 ? 1 : 0);
    Highlight_SetShown(1);
    CoasterEdit_SetBounds();
    gEdittools_79c698 = static_cast<float>(Car_TravelCost(gCoastersload_790fec, 0, piece, nullptr, nullptr, nullptr, nullptr));
    gEdittools_79c64c = static_cast<float>(Car_TravelCost(gCoastersload_790fec, 0, at<u8 *>(piece, 0x64), nullptr, nullptr, nullptr, nullptr));
    return many;
}

void __cdecl CoasterEdit_DeleteLastCar() {
    u8 *const state = gCoastersload_790fec + 0x3c;   // (the link's +0x3c, taken first)
    ActionRec_Write(Recorder(), nullptr, 0x41);
    ClearChanged();
    u8 *const game = gCoastersload_790fec;
    const u32 retool = EndCarHas(game, 0x80, 0), offer = EndCarHas(game, 0x40, 0x80);
    u8 *car;
    const bool reversed = (at<u32>(state, 0) & 2) != 0;
    if (reversed) {
        car = at<u8 *>(state, 0x24);   // (the link's +0x60)
        if (!car || (at<u8>(at<const u8 *>(car, 0x100), 4) & 2)) return;
        at<u8 *>(state, 0x24) = at<u8 *>(car, 0x68);
        g_env->deleteCar(gCoastersload_790fec, car);
    } else {
        if (at<u32>(state, 0) & 1) return;
        car = at<u8 *>(state, 0x1c);   // (the link's +0x58)
        if (!car || (at<u8>(at<const u8 *>(car, 0x100), 4) & 2)) return;
        g_env->deleteCar(game, car);
    }
    if (!g<u32>(0x00785a2c)) {
        if (retool) {
            CoasterEdit_Tools(nullptr);
            CoasterEdit_EndTool();
            gCoasterEdit_79c638 &= ~2u;
            CoasterEdit_StartTool(1);
        } else if (offer) {
            CoasterEdit_Tools(nullptr);
        }
        if (retool || offer) {
            Dialog7ca1c8_SetChecksIfUpThis(reinterpret_cast<void *>(0x007b51f0), nullptr, gCoasterEdit_79c6e0);
            Dialog7ca1c8_SelectButtonIfUpThis(reinterpret_cast<void *>(0x007b51f0), nullptr, gCoasterEdit_79c638);
        }
    }
    if (reversed) {   // (the link's new +0x60, read again)
        if (u8 *const end = at<u8 *>(state, 0x24)) at<u32>(end, 8) |= 0x50000;
    }
}

void __cdecl CoasterEdit_DeleteSpan() {
    u8 *const game = gCoastersload_790fec;
    if (!game || !at<u32>(game, 0x40)) return;
    u8 *const state = game + 0x3c;   // (the link's +0x3c, taken first)
    if (!(reinterpret_cast<u8 &>(gPlacecar_79c658) & 0x80)) {   // (the span's first end picked)
        u8 *const piece = Piece();
        gEdittools_79c6dc = piece;
        if (!piece) return;
        gEdittools_79c62c = piece;
        Flags() |= 0xa0;
        gEdittools_7925d0 = 1;
        return;
    }
    u8 *hi = Piece();
    if (!hi) return;
    const bool go = !(hi == gEdittools_79c6dc && (at<u8>(Def(hi), 4) & 2));
    const u32 cleared = TrackSelection_ClearFlags();
    reinterpret_cast<u32 &>(gEdittools_79c62c) = 0;
    reinterpret_cast<u32 &>(gEdittools_79c628) = 0;
    gEdittools_7925d0 = 0;
    if (cleared) Flags_SetChanged(reinterpret_cast<u32 *>(game), 0);
    if (!go) {
        reinterpret_cast<u32 &>(gEdittools_79c6dc) = 0;
        Flags() &= ~0x80u;
        return;
    }
    ActionRec_WriteWW(Recorder(), nullptr, 0x3f, at<u16>(gEdittools_79c6dc, 0), at<u16>(hi, 0));
    ClearChanged();
    u8 *lo = gEdittools_79c6dc;
    const u8 *const link = gCoastersload_790fec;
    u32 atEnd = 0;
    if (!at<u32>(lo, 0)) {
        lo = at<u8 *>(link, 0x58);
        gEdittools_79c6dc = lo;
        atEnd = 1;
    } else if (!at<u32>(hi, 0)) {
        hi = at<u8 *>(link, 0x58);
        atEnd = 1;
    }
    if (at<u32>(lo, 0) > at<u32>(hi, 0)) {
        u8 *const t = lo;
        lo = hi;
        gEdittools_79c6dc = lo;
        hi = t;
    }
    if (at<u8 *>(lo, 0x64) == hi && (at<u32>(lo, 8) & 0xc0) == 0xc0 && (at<u8>(hi, 8) & 0x40)) {
        if (at<u8 *>(lo, 0x68)) {
            lo = at<u8 *>(lo, 0x68);
            gEdittools_79c6dc = lo;
        }
        if (at<u8 *>(hi, 0x64)) hi = at<u8 *>(hi, 0x64);
    }
    if (lo == hi) {
        lo = at<u8 *>(lo, 0x68);
        gEdittools_79c6dc = lo;
        hi = at<u8 *>(hi, 0x64);
    }
    u8 *last = hi;
    if (at<u8 *>(lo, 0x64) != hi) {
        u8 *const next = at<u8 *>(lo, 0x64);
        gEdittools_79c6dc = next;
        hi = at<u8 *>(hi, 0x68);
        if ((at<u32>(next, 8) & 0xc0) == 0x40) {
            gEdittools_79c6dc = at<u8 *>(next, 0x68);
            lo = at<u8 *>(lo, 0x68);
        }
        if (at<u8>(hi, 8) & 0x80) last = at<u8 *>(last, 0x64);
    }
    if (last == at<u8 *>(link, 0x58)) atEnd = 1;
    at<u32>(state, 0) |= 2;
    if (at<u8 *>(lo, 0x64) == last) {
        at<u8 *>(state, 0x24) = lo;     // (the link's +0x60 / +0x64)
        at<u8 *>(state, 0x28) = last;
        at<u32>(lo, 8) |= 0x50000;
        at<u32>(last, 8) |= 0xa0000;
        const u8 *p = lo;
        if (at<u8>(lo, 8) & 0x40) {
            p = at<u8 *>(lo, 0x68);
            while (p && (at<u8>(p, 8) & 0x40)) p = at<u8 *>(p, 0x68);
        }
        const u32 value = PartEntryValue(p);   // (sic: none is read through null)
        at<u32>(gCoastersload_790fec, 0x118) = value;
    } else {
        u8 *const owner = gCoastersload_790fec;
        at<u32>(lo, 8) |= 0x50000;
        at<u32>(last, 8) |= 0xa0000;
        for (u8 *car = at<u8 *>(last, 0x68); car != lo;) {
            u8 *const before = at<u8 *>(car, 0x68);
            g_env->deleteCar(owner, car);
            car = before;
        }
        if (at<u8>(Def(last), 4) & 1) at<u32>(owner, 0x118) = at<u32>(at<u8 *>(owner, 4), 0xa4);
    }
    if (atEnd) {
        at<u32>(lo, 8) &= 0xfffaffff;
        at<u32>(gCoastersload_790fec, 0x3c) &= ~3u;
        g_env->deleteFlaggedEnd(gCoastersload_790fec);
    }
    reinterpret_cast<u32 &>(gEdittools_79c6dc) = 0;
    Flags_SetChanged(reinterpret_cast<u32 *>(gCoastersload_790fec), 1);
    Flags() &= ~0xa0u;
    const u32 retool = EndCarHas(gCoastersload_790fec, 0x80, 0);
    CoasterEdit_EndTool();
    gCoasterEdit_79c6e0 = 0;
    gCoasterEdit_79c638 = 0;
    if (retool) {
        CoasterEdit_StartTool(2);
    } else {
        CoasterEdit_SetMode(1);
        CoasterEdit_SetTool(1);
        gCoasterEdit_79c638 |= 1;
    }
    CoasterEdit_Tools(nullptr);
    Dialog7ca1c8_SetChecksIfUpThis(reinterpret_cast<void *>(0x007b51f0), nullptr, gCoasterEdit_79c6e0);
    Dialog7ca1c8_SelectButtonIfUpThis(reinterpret_cast<void *>(0x007b51f0), nullptr, gCoasterEdit_79c638);
    CoasterEdit_SetBounds();
    g_env->setState(gCoastersload_790fec, 1);
    g_env->enterTile15();
}

u32 __cdecl CoasterEdit_Click(u32 press) {
    if (g<u32>(0x00785a2c)) return 0;
    const u32 ride = static_cast<u32>(reinterpret_cast<uintptr_t>(Highlight_Current()));
    const u8 flags = reinterpret_cast<u8 &>(gPlacecar_79c658);
    const auto find = [ride]() {   // (the car under the cursor, to 0x79c640; sic: with no link this faults)
        u8 *const car = CoasterLink_FindCarOnRide(ride);
        gEdittools_79c640 = car;
        return car;
    };
    switch (reinterpret_cast<u8 &>(gEdittools_79c624)) {
    case 2:
        if (!press || g<u32>(0x007a1c7c)) return 0;
        return g_env->selectPiece(find());
    case 4: {
        if (press ? (flags & 4) : !(flags & 4)) return 0;
        u8 *const car = press ? find() : Piece();
        const u8 *const game = gCoastersload_790fec;
        if (!game || !at<u32>(game, 0x40) || !car || (at<u8>(Def(car), 4) & 2)) return 0;
        if (at<u8>(car, 8) & 0x40)
            g_env->toggleAdjustF8();
        else
            g_env->toggleAdjustAngles();
        return 0;
    }
    case 8:
        if (press && !(flags & 1))
            find();
        else if (press || !(flags & 1))
            return 0;
        g_env->toggleDrag();
        return 0;
    case 0x10:
        if (press ? (flags & 2) : !(flags & 2)) return 0;
        if (press) find();
        g_env->toggleAdjustE0();
        return 0;
    case 0x20:
        if (press) return 0;
        find();
        g_env->moveCarBack();
        return 0;
    case 0x40:
        if (press) return 0;
        find();
        g_env->moveCarForward();
        return 0;
    case 0x80:
        if (press) return 0;
        find();
        g_env->deleteSpan();
        return 0;
    default:
        return 0;
    }
}
}

namespace {

// The end 0x445a70 has twice and 0x446050 once: CoasterEdit_EndTool inlined, its calls through the env. How each has tools 4 and 0x20:
enum EndCopy {
    kEndCalled,    // CoasterEdit_ToggleAdjust and TrackSelection_End called (0x445a70's first)
    kEndInlined,   // CoasterEdit_ToggleAdjust called, TrackSelection_End inlined (flag 0x80 as read before the switch) calling
                   // TrackSelection_Clear (0x445a70's second)
    kAllInlined,   // both inlined, TrackSelection_Clear too (0x446050's)
};

void EndToolVia(EndCopy copy) {
    const EditToolsEnv *const env = g_env;
    if ((reinterpret_cast<u8 &>(gEdittools_79c624) & 1) && gCoastersload_790fec) {
        at<u32>(gCoastersload_790fec, 0) &= ~4u;
        Flags_SetChanged(reinterpret_cast<u32 * &>(gCoastersload_790fec), 0);
        u8 *const game = gCoastersload_790fec;
        const u32 f = at<u32>(game, 0x3c);
        if (!(f & 1) || (f & 2)) env->flashCar(game, 1);
    }
    const u32 flags = Flags();
    switch (flags & 0x3f) {
    case 1: env->toggleDrag(); break;
    case 2: env->toggleAdjustE0(); break;
    case 4: {
        if (copy != kAllInlined) {
            env->toggleAdjust();
            break;
        }
        const u8 *const game = gCoastersload_790fec;
        const u8 *const piece = Piece();
        if (!game || !at<u32>(game, 0x40) || !piece || (at<u8>(Def(piece), 4) & 2)) break;
        if (at<u8>(piece, 8) & 0x40)
            env->toggleAdjustF8();
        else
            env->toggleAdjustAngles();
        break;
    }
    case 8: env->moveCarBack(); break;
    case 0x10: env->moveCarForward(); break;
    case 0x20:
        if (copy == kEndCalled) {
            env->selectionEnd();
            break;
        }
        if (flags & 0x80) {
            Highlight_SetShown(0);
            reinterpret_cast<u32 &>(gEdittools_79c6dc) = 0;
            if (copy == kAllInlined)
                TrackSelection_Clear();
            else
                env->selectionClear();
        }
        Flags() &= ~0xa0u;
        break;
    default: break;
    }
    SetModeState(0);
}

}  // namespace

extern "C" {

u32 __cdecl CoasterEdit_PlaceSelected(s32 x, s32 y) {
    const EditToolsEnv *const env = g_env;
    gEdittools_79c69c = 0;
    if (gEdittools_79c624 != 2 || !gEdittools_79c6a4) {
        Fn_52c2a0();
        Flags() &= ~0x100u;
        gEdittools_79c6a4 = nullptr;
        if (g<u32>(0x00785a2c)) EndToolVia(kEndInlined);
        return 0;
    }
    const u8 *const placed = gEdittools_79c6a4;
    const u32 cleared = env->clearSelectionFlags();
    reinterpret_cast<u32 &>(gEdittools_79c62c) = 0;
    reinterpret_cast<u32 &>(gEdittools_79c628) = 0;
    gEdittools_7925d0 = 0;
    if (cleared) Flags_SetChanged(reinterpret_cast<u32 * &>(gCoastersload_790fec), 0);
    u8 *const tile = env->findTile((u32)x, (u32)y, nullptr);
    u8 *const piece = gEdittools_79c6a4;
    bool settled = false;
    if (tile && (at<u32>(tile, 0x18) != at<u32>(Def(piece), 0x18) || at<u32>(tile, 0x14) != at<u32>(Def(piece), 0x14))) {
        env->dropCar(gCoastersload_790fec, piece, gEdittools_79c660, gEdittools_79c6d8, gEdittools_79c6a0, nullptr, nullptr, 1, 0);
        env->pathDragFinish();
        if (!(at<u8>(tile, 4) & 2))
            env->setRideCell((u32)(uintptr_t)gCoastersload_790fec, at<s32>(tile, 0xc), at<s32>(tile, 0x10),
                             at<u32>(at<const u8 *>(tile, 0x34), 0x70), 0);
    } else {
        float outX, outY;
        env->dropCar(gCoastersload_790fec, piece, tile, x, y, &outX, &outY, 1, 0);
        if (!g<u32>(0x00785a2c) && (gEdittools_79c6d8 != x || gEdittools_79c6a0 != y)) {
            u8 *const game = gCoastersload_790fec;
            s32 cost;
            u8 *money;
            if (at<u8>(game, 0x3c) & 1) {
                const u8 *const sel = gEdittools_79c6a4;
                cost = Float_RoundUp(static_cast<float>(env->travelCost(game, 0, sel, &outX, &outY, nullptr, nullptr)));
                cost += Float_RoundUp(static_cast<float>(env->travelCost(game, 0, at<const u8 *>(sel, 0x64), nullptr, nullptr, &outX, &outY)));
                money = reinterpret_cast<u8 *>(GameTable7cfb90_Get1da726_5195d0(g<u8 *>(0x007cf83c)));
                Money_Credit(money, nullptr, (s32)(long long)gEdittools_79c698);   // __ftol
                Money_Credit(money, nullptr, (s32)(long long)gEdittools_79c64c);   // (behind a test of its address: always)
            } else {
                cost = Float_RoundUp(static_cast<float>(env->travelCost(game, 0, gEdittools_79c6a4, &outX, &outY, nullptr, nullptr)));
                money = reinterpret_cast<u8 *>(GameTable7cfb90_Get1da726_5195d0(g<u8 *>(0x007cf83c)));
                Money_Credit(money, nullptr, (s32)(long long)gEdittools_79c698);
            }
            Source_Field0c(reinterpret_cast<const TrackedSource *>(money));   // (its result unused)
            Money_Debit(money, nullptr, cost);
            reinterpret_cast<u32 &>(gEdittools_79c698) = 0;
        }
        settled = true;
    }
    Flags_SetChanged(reinterpret_cast<u32 * &>(gCoastersload_790fec), 1);
    Highlight_SetShown(0);
    Flags() &= ~0x100u;
    gEdittools_79c6a4 = nullptr;
    gEdittools_79c660 = nullptr;
    if (g<u32>(0x00785a2c)) EndToolVia(kEndCalled);
    return settled ? at<u32>(placed, 0x70) : 0;
}

void __cdecl CoasterEdit_CancelSelected() {
    const EditToolsEnv *const env = g_env;
    gEdittools_79c69c = 0;
    if (!(Flags() & 0x100)) return;
    Flags() &= ~0x100u;
    TrackSelection_Clear();   // (inlined)
    u8 *const piece = gEdittools_79c6a4;
    if (at<u8 *>(piece, 0x100) == gEdittools_79c660) gEdittools_79c660 = nullptr;
    env->dropCar(gCoastersload_790fec, piece, gEdittools_79c660, gEdittools_79c6d8, gEdittools_79c6a0, nullptr, nullptr, 1, 0);
    env->pathDragFinish();
    Highlight_SetShown(0);
    gEdittools_79c6a4 = nullptr;
    gEdittools_79c660 = nullptr;
    if (g<u32>(0x00785a2c)) EndToolVia(kAllInlined);
}
}

void EditTools_SetEnv(const EditToolsEnv *env) { g_env = env ? env : &kPortEnv; }

namespace {
AddrCalls g_rebuildCalls;

void RebuildOne(u8 *coaster) {
    gCoastersupdate_792218 = coaster;
    gCoastersload_790fe4 = coaster + 0x3c;
    g_rebuildCalls.Cd(0x0043eb70, (u32)(uintptr_t)coaster);   // Coaster_Rebuild
}
}  // namespace

void EditTools_SetRebuildCalls(const AddrCall *table, u32 count) { g_rebuildCalls.Set(table, count); }

extern "C" void __cdecl EditTools_RebuildCurrent() {
    u8 *const coaster = gCoastersload_790fec;
    if (coaster[0] & 1) RebuildOne(coaster);
}

extern "C" void __cdecl EditTools_RebuildIndexed(u32 index) {
    u8 *const coaster = gCoasters_790bd0[index];
    if (coaster && (coaster[0] & 1)) RebuildOne(coaster);
}
