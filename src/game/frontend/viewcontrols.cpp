#include "viewcontrols.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../core/globals.h"
#include "../core/vdtor.h"
#include "../math/mat4.h"
#include "../math/x87.h"
#include "../render/randomsegments.h"
#include "../ui/dialogs.h"
#include "lobbymodel.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The view objects' shared part of their message handlers (0x5dced0, 0x5dd530): message 0x1000a's keys move the values, message 0x10006
// with 2 swaps the mode.
void view_message(u8 *self, u32 message, u32 key) {
    if (message == 0x1000a) {
        if (key == 0x2500) ViewCtl_Lower48(self);
        else if (key == 0x2600) ViewCtl_Raise50(self);
        else if (key == 0x2700) ViewCtl_Raise48(self);
        else if (key == 0x2800) ViewCtl_Lower50(self);
    } else if (message == 0x10006 && key == 2) {
        if (at<u32>(self, 0x58)) ViewCtl_UseFirstVector(self);
        else ViewCtl_UseSecondVector(self);
        Dialog7cdb0c_SetChild16006State(at<u32>(self, 0x58));
    }
}

const SceneViewsEnv kScenePorts = {&RandomSegments_Ctor, &Class702bc8_Ctor};
const SceneViewsEnv *g_sceneEnv = &kScenePorts;

// A new object of size bytes, built by ctor with vtable (0 when the memory is not there).
template <typename Ctor> u8 *new_view(u32 size, Ctor ctor, u32 vtable) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(size));
    if (!block) return nullptr;
    ctor(block);
    at<u32>(block, 0) = vtable;
    return block;
}

}  // namespace

void SceneViews_SetEnv(const SceneViewsEnv *env) { g_sceneEnv = env ? env : &kScenePorts; }

namespace {
const ViewCtlEnv kViewCtlPorts = {&Dialog7cc4c4_ShowThing};
const ViewCtlEnv *g_viewCtlEnv = &kViewCtlPorts;

// The ray through the mouse from the camera, met with a sphere of radius 116 about the origin: the nearest of the list's things within 10 of where it meets it goes to +0x70, and is shown.
void pickThing(u8 *self) {
    const u8 *const input = *reinterpret_cast<u8 **>((uintptr_t)0x00f82884);
    const float kOne = *reinterpret_cast<float *>((uintptr_t)0x00702b60);
    const float kScale = *reinterpret_cast<float *>((uintptr_t)0x00702b80);
    float in[3], out[3];
    in[0] = (float)((X87Reg(X87_MulIntDouble(*reinterpret_cast<const s32 *>(input + 0x20), *reinterpret_cast<float *>((uintptr_t)0x00702b7c))) - kOne) * kScale);
    in[2] = X87_Through(kOne);
    in[1] = (float)((X87Reg(X87_MulIntDouble(0x600 - *reinterpret_cast<const s32 *>(input + 0x24), *reinterpret_cast<float *>((uintptr_t)0x00702b84))) - kOne) *
                    *reinterpret_cast<float *>((uintptr_t)0x008bcbcc) * kScale);
    Mat4_Transform(*reinterpret_cast<const float **>((uintptr_t)0x0087afe8), nullptr, out, in, 2);
    const float *const camera = reinterpret_cast<const float *>(*reinterpret_cast<u8 **>((uintptr_t)0x00f83778) + 8);
    const float p0 = camera[0], p1 = camera[1], p2 = camera[2];
    const X87Reg a0 = X87Reg(out[0]) - p0, a1 = X87Reg(out[1]) - p1, a2 = X87Reg(out[2]) - p2;
    const float a0f = (float)a0, a1f = (float)a1;
    const float length = (float)X87_Sqrt(a0 * a0 + a1 * a1f + a2 * a2);
    const float u0 = (float)(X87Reg(a0f) / length), u1 = (float)(X87Reg(a1f) / length), u2 = (float)(a2 / length);
    const X87Reg n0(-static_cast<double>(p0)), n1(-static_cast<double>(p1)), n2(-static_cast<double>(p2));
    const X87Reg along = n0 * u0 + n1 * u1 + n2 * u2;
    const float alongf = (float)along;
    const X87Reg w0 = n0 - along * u0, w1 = n1 - X87Reg(alongf) * u1, w2 = n2 - X87Reg(alongf) * u2;
    const float missed = (float)(w0 * w0 + w1 * (float)w1 + w2 * (float)w2);
    const float kRadius = *reinterpret_cast<float *>((uintptr_t)0x00702b8c), kNear = *reinterpret_cast<float *>((uintptr_t)0x00702b90);
    bool hit = false;
    X87Reg h = 0.0;
    if (!(missed >= kRadius)) {
        const float s = (float)X87_Sqrt(X87Reg(kRadius) - missed);
        h = X87Reg(alongf) - s;
        if (!(h.v >= kNear)) h = X87Reg(s) + alongf;
        hit = h.v > kNear;
    }
    float *const point = reinterpret_cast<float *>(self + 0x5c);
    u8 *&chosen = *reinterpret_cast<u8 **>(self + 0x70);
    chosen = nullptr;
    if (hit) {
        point[0] = (float)(X87Reg(u0) * h + p0);
        point[1] = (float)(X87Reg(u1) * h + p1);
        point[2] = (float)(X87Reg(u2) * h + p2);
        float best = *reinterpret_cast<float *>((uintptr_t)0x00702b94);
        const float kWithin = *reinterpret_cast<float *>((uintptr_t)0x00702b98);
        for (const u32 *node = *reinterpret_cast<const u32 **>(*reinterpret_cast<u8 **>(self + 0x6c) + 4); node;) {
            u8 *const value = reinterpret_cast<u8 *>((uintptr_t)node[0]);
            node = reinterpret_cast<const u32 *>((uintptr_t)node[1]);
            const float *const q = reinterpret_cast<const float *>(*reinterpret_cast<u8 **>(value + 0xc) + 0x34);
            const float q0 = q[0], q1 = q[1], q2 = q[2];
            const X87Reg e0 = X87Reg(q0) - point[0], e1 = X87Reg(q1) - point[1], e2 = X87Reg(q2) - point[2];
            const float d = (float)(e0 * e0 + e1 * (float)e1 + e2 * (float)e2);
            if (!(d >= kWithin) && !(d >= best)) {
                best = d;
                chosen = value;
            }
        }
    }
    g_viewCtlEnv->showThing(chosen ? *reinterpret_cast<u8 **>(chosen + 0xc) : nullptr);
}

// The mouse at the screen's edges moves the values at +0x48 and +0x50.
void edgeScroll(u8 *self) {
    const u8 *const input = *reinterpret_cast<u8 **>((uintptr_t)0x00f82884);
    float &v48 = *reinterpret_cast<float *>(self + 0x48), &v50 = *reinterpret_cast<float *>(self + 0x50);
    const float kZero = *reinterpret_cast<float *>((uintptr_t)0x00702b88), kOne = *reinterpret_cast<float *>((uintptr_t)0x00702b60);
    const float kLow = *reinterpret_cast<float *>((uintptr_t)0x00702ba8), kHigh = *reinterpret_cast<float *>((uintptr_t)0x00702bac);
    float fx = (float)X87_MulIntDouble(*reinterpret_cast<const s32 *>(input + 0x20), *reinterpret_cast<float *>((uintptr_t)0x00702b9c));
    if (fx < kZero) fx = 0.0f;
    else if (fx > kOne) fx = 1.0f;
    X87Reg fy = X87_MulIntDouble(*reinterpret_cast<const s32 *>(input + 0x24), *reinterpret_cast<float *>((uintptr_t)0x00702ba0));
    if (fy.v < kZero) fy = X87Reg(kZero);
    else if (fy.v > kOne) fy = X87Reg(kOne);
    const X87Reg rate = X87Reg(*reinterpret_cast<float *>(self + 0x10)) * *reinterpret_cast<float *>((uintptr_t)0x00702ba4);
    if (fx < kLow) v48 = (float)X87_SubR((X87Reg(kLow) - fx) * rate, v48);
    else if (fx > kHigh) v48 = (float)((X87Reg(fx) - kHigh) * rate + v48);
    if (fy.v < kLow) v50 = (float)((X87Reg(kLow) - fy) * rate + v50);
    else if (fy.v > kHigh) v50 = (float)X87_SubR((fy - kHigh) * rate, v50);
}
}  // namespace

void ViewCtl_SetEnv(const ViewCtlEnv *env) { g_viewCtlEnv = env ? env : &kViewCtlPorts; }

extern "C" void __fastcall ViewCtl702bc8_Update(u8 *self) {
    if (*reinterpret_cast<u32 *>(self + 0x68) != 0) pickThing(self);
    const auto input = [] { return *reinterpret_cast<u8 **>((uintptr_t)0x00f82884); };
    float &v48 = *reinterpret_cast<float *>(self + 0x48), &v50 = *reinterpret_cast<float *>(self + 0x50);
    const u8 dragging = *reinterpret_cast<u8 *>((uintptr_t)0x0078d910);
    if (dragging == 0) edgeScroll(self);
    if (*reinterpret_cast<u8 *>((uintptr_t)0x0078d910) != 0 && Obj_Flags1cAnd(input(), nullptr, 2)) {
        const float step = *reinterpret_cast<float *>(self + 0x10);
        v48 = (float)(X87Reg(X87_MulIntDouble(*reinterpret_cast<s32 *>(input() + 0x20) - 0x400, *reinterpret_cast<float *>((uintptr_t)0x00702b7c))) * step + v48);
        v50 = (float)X87_SubR(X87Reg(X87_MulIntDouble(*reinterpret_cast<s32 *>(input() + 0x24) - 0x300, *reinterpret_cast<float *>((uintptr_t)0x00702b84))) * step, v50);
    }
    const float factor = *reinterpret_cast<float *>(self + 0x18);
    const X87Reg a = X87Reg(v48) * factor, b = X87Reg(v50) * factor;
    v48 = (float)a;
    v50 = (float)b;
    const float bf = (float)b;
    float &v44 = *reinterpret_cast<float *>(self + 0x44), &v4c = *reinterpret_cast<float *>(self + 0x4c);
    const float &limit = *reinterpret_cast<float *>(self + 0x40);
    v44 = (float)(X87Reg(*reinterpret_cast<float *>(input() + 8)) * a + v44);
    const X87Reg moved = X87Reg(*reinterpret_cast<float *>(input() + 8)) * bf + v4c;
    v4c = (float)moved;
    if (moved.v > limit) {
        *reinterpret_cast<u32 *>(self + 0x50) = 0;
        *reinterpret_cast<u32 *>(self + 0x4c) = *reinterpret_cast<const u32 *>(self + 0x40);
    }
    const double lowest = -static_cast<double>(limit);
    if (!(static_cast<double>(v4c) >= lowest)) {
        v4c = (float)X87Reg(lowest);
        *reinterpret_cast<u32 *>(self + 0x50) = 0;
    }
    const u8 *const targets = *reinterpret_cast<u8 **>(input() + 0x10);
    const bool second = *reinterpret_cast<u32 *>(self + 0x58) != 0;
    const float target = *reinterpret_cast<const float *>(targets + (second ? 0x18 : 0x14));
    const float kept = *reinterpret_cast<float *>(self + (second ? 0x3c : 0x38));
    float &v54 = *reinterpret_cast<float *>(self + 0x54), &v34 = *reinterpret_cast<float *>(self + 0x34);
    const float kEase = *reinterpret_cast<float *>((uintptr_t)0x00702bb0);
    const X87Reg toTarget = X87Reg(target) - v54, toKept = X87Reg(kept) - v34;
    const float dt = *reinterpret_cast<float *>(input() + 8);
    v54 = (float)X87_SubR(toTarget * dt * kEase, v54);
    const X87Reg s = X87_SubR(toKept * dt * kEase, v34);
    v34 = (float)s;
    u8 *const camera = *reinterpret_cast<u8 **>((uintptr_t)0x00f83778);
    float *const view = reinterpret_cast<float *>(camera);
    view[0x28 / 4] = (float)s;
    *reinterpret_cast<u32 *>(camera + 0x30) = 0;
    view[0x2c / 4] = (float)X87Reg(-s.v);
    const float yaw = v44, pitch = v4c;
    const double cosYaw = X87_Cos(yaw), sinYaw = X87_Sin(yaw);
    const float cosPitch = (float)X87_Cos(pitch);
    const double sinPitch = X87_Sin(pitch);
    const float out0 = (float)(X87Reg(X87_MulFloatDouble(cosPitch, cosYaw)) * v54);
    const float out1 = (float)X87_MulFloatDouble(v54, sinPitch);
    const float out2 = (float)(X87Reg(X87_MulFloatDouble(cosPitch, sinYaw)) * v54);
    view[8 / 4] = out0;
    view[0xc / 4] = out1;
    view[0x10 / 4] = out2;
    *reinterpret_cast<u32 *>(camera + 0x14) = 0;
    *reinterpret_cast<u32 *>(camera + 0x18) = 0;
    *reinterpret_cast<u32 *>(camera + 0x1c) = 0;
}

extern "C" void __fastcall Class702d78_MakeViews(u8 *self) {
    at<u8 *>(self, 4) = new_view(0xe8, g_sceneEnv->segmentsCtor, 0x00702ec0);
    at<u8 *>(self, 0xc) = new_view(0x7c, g_sceneEnv->viewCtor, 0x00702e70);
    at<u8 *>(self, 0x10) = new_view(0x7c, g_sceneEnv->viewCtor, 0x00702e20);
    at<u8 *>(self, 8) = new_view(0xec, g_sceneEnv->segmentsCtor, 0x00702dd0);
    u8 *const fifth = static_cast<u8 *>(GameHeap_Alloc(0x2c));
    if (fifth) {
        at<u32>(fifth, 0) = 0x00702d48;
        at<u32>(fifth, 4) = 0;
        vcall<void>(*reinterpret_cast<u8 **>((uintptr_t)0x00f85614), 0x18, fifth);
        at<u32>(fifth, 0xc) = 0;
        at<u32>(fifth, 0x10) = 0;
        at<u32>(fifth, 0x14) = 0;
        at<u32>(fifth, 8) = 0x00702dc8;
        at<u32>(fifth, 0) = 0x00702da0;
    }
    at<u8 *>(self, 0x14) = fifth;
    for (u8 *node = at<u8 *>(at<u8 *>(self, 0x18), 4); node;) {
        u8 *const item = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        vcall<void>(item, 4);
    }
}

extern "C" {

void __fastcall ViewCtl_Lower48(u8 *self) { at<float>(self, 0x48) = at<float>(self, 0x48) - at<float>(self, 0x14); }

void __fastcall ViewCtl_Raise48(u8 *self) { at<float>(self, 0x48) = at<float>(self, 0x14) + at<float>(self, 0x48); }

void __fastcall ViewCtl_Raise50(u8 *self) { at<float>(self, 0x50) = at<float>(self, 0x14) + at<float>(self, 0x50); }

void __fastcall ViewCtl_Lower50(u8 *self) { at<float>(self, 0x50) = at<float>(self, 0x50) - at<float>(self, 0x14); }

void __fastcall ViewCtl_UseSecondVector(u8 *self) {
    at<u32>(self, 0x58) = 1;
    Dialog7cdb0c_Child16007Message6Off();
    Obj_ChooseVector(self);
}

void __fastcall ViewCtl_UseFirstVector(u8 *self) {
    at<u32>(self, 0x58) = 0;
    Dialog7cdb0c_Child16007Message6On();
    Obj_ChooseVector(self);
}

s32 __fastcall ViewCtl702e70_ShowDialogs(u8 *) {
    Dialog7cdb0c_Message6On();
    Dialog7cc4c4_Message6On();
    return Dialog7cdb0c_Child16007Message6On();
}

s32 __fastcall ViewCtl702e20_ShowDialogs(u8 *) {
    Dialog7cdb0c_Message6On();
    return Dialog7cc4c4_Message6On();
}

s32 __fastcall Scene702dd0_ShowDialog(u8 *self) {
    const s32 result = Dialog7cc47c_Message6On();
    at<u32>(self, 0xe8) = 1;
    return result;
}

s32 __fastcall Scene702dd0_HideDialog(u8 *) { return Dialog7cc47c_Message6Off(); }

s32 __cdecl Scene702dd0_ShowDialogCallback() { return Dialog7cc47c_Message6On(); }

s32 __cdecl Scene702dd0_DialogDone(s32 accepted, u8 *self) {
    if (accepted == 0)
        return vcall<s32>(self, 0x44);
    const u32 state = at<u32>(self, 0x14);
    if (state != 0) {
        if (state == 2)
            LobbyModel_Play(at<u8 *>(at<u8 *>(self, 0xc), 8), nullptr, 1, 0);
        at<u32>(self, 0x14) = 0;
    }
    return Dialog7cc47c_Message6On();
}

void __fastcall ViewCtl702bc8_Message(u8 *self, void *, u32 message, u32 key, u32) { view_message(self, message, key); }

void __fastcall ViewCtl702e70_Message(u8 *self, void *, u32 message, u32 key, u32) {
    view_message(self, message, key);
    if (message == 0x10005 && key == 0) {
        if (const u8 *const target = at<const u8 *>(self, 0x70)) {
            *reinterpret_cast<u32 *>((uintptr_t)0x00f82888) = at<u32>(target, 0xc);
            vcall<void>(self, 0x20);
            vcall<void>(at<u8 *>(*reinterpret_cast<u8 **>((uintptr_t)0x00f85614), 8), 0x1c);
        }
    }
}

void __fastcall Obj_ChooseVector(u8 *object) {
    const u32 base = at<u32>(object, 0x58) ? 0x28 : 0x1c;
    const u32 x = at<u32>(object, base), y = at<u32>(object, base + 4), z = at<u32>(object, base + 8);
    at<u32>(object, 0x10) = x;
    at<u32>(object, 0x14) = y;
    at<u32>(object, 0x18) = z;
}

u32 __stdcall Obj_Positive40(const u8 *object) { return at<s32>(object, 0x40) > 0 ? 1 : 0; }

u32 __stdcall Obj_Pending(const u8 *object) {
    return at<u32>(object, 0x60) == 0 && at<s32>(object, 0x40) < at<s32>(object, 0x64) ? 1 : 0;
}

void __fastcall Obj_SetOrKeepC(u8 *object, void *, u32 value) {
    if (value) {
        at<u32>(object, 0xc) = value;
        at<u32>(object, 0x14) = 0;
    }
}
}  // extern "C"
